#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>

#include <errno.h>
extern int errno;

#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <pwd.h>

#include "../general/general.h"
#include "../api/api.h"
#include "../apilib/api_exch.h"

#include "../general/globals.h"


static int shell_pid = 0;

static char *ourENVlist[200];		/* Lots of room */

static int sock = -1;

static enum { DEAD, UNCONNECTED, CONNECTED } state;

static int
    storage_location,		/* Address we have */
    storage_length = 0,		/* Length we have */
    storage_must_send = 0,	/* Storage belongs on other side of wire */
    storage_accessed = 0;	/* The storage is accessed (so leave alone)! */

static long storage[250];

static union REGS inputRegs;
static struct SREGS inputSregs;


static void
kill_connection()
{
    state = DEAD;
    (void) close(sock);
    sock = -1;
}


static int
nextstore()
{
    struct storage_descriptor sd;

    if (api_exch_incommand(EXCH_HEREIS) == -1) {
	fprintf(stderr, "Bad data from other side.\n");
	fprintf(stderr, "(Encountered at %s, %s.)\n", __FILE__, __LINE__);
	return -1;
    }
    if (api_exch_intype(EXCH_TYPE_STORE_DESC, sizeof sd, (char *)&sd) == -1) {
	storage_length = 0;
	return -1;
    }
    storage_length = ntohs(sd.length);
    storage_location = ntohl(sd.location);
    if (storage_length > sizeof storage) {
	fprintf(stderr, "API client tried to send too much storage (%d).\n",
		storage_length);
	storage_length = 0;
	return -1;
    }
    if (api_exch_intype(EXCH_TYPE_BYTES, storage_length, (char *)storage) == -1) {
	storage_length = 0;
	return -1;
    }
}


static int
doreject(message)
char	*message;
{
    struct storage_descriptor sd;
    int length = strlen(message);

    if (api_exch_outcommand(EXCH_REJECTED) == -1) {
	return -1;
    }
    sd.length = htons(length);
    if (api_exch_outtype(EXCH_TYPE_BYTES, sizeof sd, (char *)&sd) == -1) {
	return -1;
    }
    if (api_exch_outtype(EXCH_TYPE_BYTES, length, message) == -1) {
	return -1;
    }
    return 0;
}


/*
 * doconnect()
 *
 * Negotiate with the other side and try to do something.
 */

static int
doconnect()
{
    struct passwd *pwent;
    char
	promptbuf[100],
	buffer[200];
    int length;
    int was;
    struct storage_descriptor sd;

    if ((pwent = getpwuid(geteuid())) == 0) {
	return -1;
    }
    sprintf(promptbuf, "Enter password for user %s:", pwent->pw_name);
    api_exch_outcommand(EXCH_SEND_AUTH);
    api_exch_outtype(EXCH_TYPE_BYTES, strlen(promptbuf), promptbuf);
    api_exch_outtype(EXCH_TYPE_BYTES, strlen(pwent->pw_name), pwent->pw_name);
    if (api_exch_incommand(EXCH_AUTH) == -1) {
	return -1;
    }
    if (api_exch_intype(EXCH_TYPE_STORE_DESC, sizeof sd, (char *)&sd) == -1) {
	return -1;
    }
    sd.length = ntohs(sd.length);
    if (sd.length > sizeof buffer) {
	doreject("Password entered was too long");
	return 0;
    }
    if (api_exch_intype(EXCH_TYPE_BYTES, sd.length, buffer) == -1) {
	return -1;
    }
    buffer[sd.length] = 0;

    /* Is this the correct password? */
    if (strcmp(crypt(buffer, pwent->pw_passwd), pwent->pw_passwd) == 0) {
	api_exch_outcommand(EXCH_CONNECTED);
    } else {
	doreject("Invalid password");
	sleep(10);		/* Don't let us do too many of these */
    }
    return 0;
}


void
freestorage()
{
    char buffer[40];
    struct storage_descriptor sd;

    if (storage_accessed) {
	fprintf(stderr, "Internal error - attempt to free accessed storage.\n");
	fprintf(stderr, "(Enountered in file %s at line %s.)\n",
			__FILE__, __LINE__);
	quit();
    }
    if (storage_must_send == 0) {
	return;
    }
    storage_must_send = 0;
    if (api_exch_outcommand(EXCH_HEREIS) == -1) {
	kill_connection();
	return;
    }
    sd.length = htons(storage_length);
    sd.location = htonl(storage_location);
    if (api_exch_outtype(EXCH_TYPE_STORE_DESC, sizeof sd, (char *)&sd) == -1) {
	kill_connection();
	return;
    }
    if (api_exch_outtype(EXCH_TYPE_BYTES, storage_length, (char *)storage) == -1) {
	kill_connection();
	return;
    }
}


static int
getstorage(address, length)
{
    struct storage_descriptor sd;
    char buffer[40];

    freestorage();
    if (storage_accessed) {
	fprintf(stderr,
		"Internal error - attempt to get while storage accessed.\n");
	fprintf(stderr, "(Enountered in file %s at line %s.)\n",
			__FILE__, __LINE__);
	quit();
    }
    if (storage_must_send == 0) {
	return;
    }
    storage_must_send = 0;
    if (api_exch_outcommand(EXCH_GIMME) == -1) {
	kill_connection();
	return -1;
    }
    sd.location = htonl(storage_location);
    sd.length = htons(storage_length);
    if (api_exch_outtype(EXCH_TYPE_STORE_DESC, sizeof sd, (char *)&sd) == -1) {
	kill_connection();
	return -1;
    }
    if (nextstore() == -1) {
	kill_connection();
	return -1;
    }
    return 0;
}

void
movetous(local, es, di, length)
char
    *local;
int
    es,
    di;
int
    length;
{
    if (length > sizeof storage) {
	fprintf(stderr, "Internal API error - movetous() length too long.\n");
	fprintf(stderr, "(detected in file %s, line %d)\n", __FILE__, __LINE__);
	quit();
    } else if (length == 0) {
	return;
    }
    getstorage(di, length);
    memcpy(local, storage+(di-storage_location), length);
}

void
movetothem(es, di, local, length)
int
    es,
    di;
char
    *local;
int
    length;
{
    if (length > sizeof storage) {
	fprintf(stderr, "Internal API error - movetothem() length too long.\n");
	fprintf(stderr, "(detected in file %s, line %d)\n", __FILE__, __LINE__);
	quit();
    } else if (length == 0) {
	return;
    }
    freestorage();
    memcpy((char *)storage, local, length);
    storage_length = length;
    storage_location = di;
    storage_must_send = 1;
}


char *
access_api(location, length)
int
    location,
    length;
{
    if (storage_accessed) {
	fprintf(stderr, "Internal error - storage accessed twice\n");
	fprintf(stderr, "(Encountered in file %s, line %s.)\n",
				__FILE__, __LINE__);
	quit();
    } else if (length != 0) {
	storage_accessed = 1;
	freestorage();
	getstorage(location, length);
    }
    return (char *) storage;
}

unaccess_api(location, local, length)
int	location;
char	*local;
int	length;
{
    if (storage_accessed == 0) {
	fprintf(stderr, "Internal error - unnecessary unaccess_api call.\n");
	fprintf(stderr, "(Encountered in file %s, line %s.)\n",
			__FILE__, __LINE__);
	quit();
    }
    storage_accessed = 0;
    storage_must_send = 1;	/* Needs to go back */
}


/*
 * shell_continue() actually runs the command, and looks for API
 * requests coming back in.
 *
 * We are called from the main loop in telnet.c.
 */

int
shell_continue()
{
    switch (state) {
    case DEAD:
	pause();			/* Nothing to do */
	break;
    case UNCONNECTED:
	if (api_exch_incommand(EXCH_CONNECT) == -1) {
	    kill_connection();
	} else {
	    switch (doconnect()) {
	    case -1:
		kill_connection();
		break;
	    case 0:
		break;
	    case 1:
		state = CONNECTED;
	    }
	}
	break;
    case CONNECTED:
	if (api_exch_incommand(EXCH_REQUEST) == -1) {
	    kill_connection();
	} else if (api_exch_intype(EXCH_TYPE_BYTES, sizeof inputRegs,
				(char *)&inputRegs) == -1) {
	    kill_connection();
	} else if (api_exch_intype(EXCH_TYPE_BYTES, sizeof inputSregs,
				(char *)&inputSregs) == -1) {
	    kill_connection();
	} else if (nextstore() == -1) {
	    kill_connection();
	} else {
	    handle_api(&inputRegs, &inputSregs);
	    freestorage();			/* Send any storage back */
	    if (api_exch_outcommand(EXCH_REPLY) == -1) {
		kill_connection();
	    } else if (api_exch_outtype(EXCH_TYPE_BYTES, sizeof inputRegs,
				(char *)&inputRegs) == -1) {
		kill_connection();
	    } else if (api_exch_outtype(EXCH_TYPE_BYTES, sizeof inputSregs,
				(char *)&inputSregs) == -1) {
		kill_connection();
	    }
	    /* Done, and it all worked! */
	}
    }
    return shell_active;
}


static int
child_died()
{
    union wait *status;
    register int pid;

    while ((pid = wait3(&status, WNOHANG, 0)) > 0) {
	if (pid == shell_pid) {
	    char inputbuffer[100];

	    shell_active = 0;
	    if (sock != -1) {
		(void) close(sock);
		sock = -1;
		printf("[Hit return to continue]");
		fflush(stdout);
		(void) gets(inputbuffer);
		setconnmode();
		ConnectScreen();	/* Turn screen on (if need be) */
	    }
	}
    }
    signal(SIGCHLD, child_died);
}


/*
 * Called from telnet.c to fork a lower command.com.  We
 * use the spint... routines so that we can pick up
 * interrupts generated by application programs.
 */


int
shell(argc,argv)
int	argc;
char	*argv[];
{
    int serversock, length;
    struct sockaddr_in server;
    char sockNAME[100];
    static char **whereAPI = 0;

    /* First, create the socket which will be connected to */
    serversock = socket(AF_INET, SOCK_STREAM, 0);
    if (serversock < 0) {
	perror("opening API socket");
	return 0;
    }
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = 0;
    if (bind(serversock, &server, sizeof server) < 0) {
	perror("binding API socket");
	return 0;
    }
    length = sizeof server;
    if (getsockname(serversock, &server, &length) < 0) {
	perror("getting API socket name");
	(void) close(serversock);
    }
    listen(serversock, 1);
    /* Get name to advertise in address list */
    strcpy(sockNAME, "API3270=");
    gethostname(sockNAME+strlen(sockNAME), sizeof sockNAME-strlen(sockNAME));
    if (strlen(sockNAME) > (sizeof sockNAME-10)) {
	fprintf(stderr, "Local hostname too large; using 'localhost'.\n");
	strcpy(sockNAME, "localhost");
    }
    sprintf(sockNAME+strlen(sockNAME), ":%d", ntohs(server.sin_port));

    if (whereAPI == 0) {
	char **ptr, **nextenv;
	extern char **environ;

	ptr = environ;
	nextenv = ourENVlist;
	while (*ptr) {
	    if (nextenv >= &ourENVlist[highestof(ourENVlist)-1]) {
		fprintf(stderr, "Too many environmental variables\n");
		break;
	    }
	    *nextenv++ = *ptr++;
	}
	whereAPI = nextenv++;
	*nextenv++ = 0;
	environ = ourENVlist;		/* New environment */
    }
    *whereAPI = sockNAME;

    child_died();			/* Start up signal handler */
    shell_active = 1;			/* We are running down below */
    if (shell_pid = vfork()) {
	if (shell_pid == -1) {
	    perror("vfork");
	    (void) close(serversock);
	} else {
	    fd_set fdset;
	    int i;

	    FD_ZERO(&fdset);
	    FD_SET(serversock, &fdset);
	    while (shell_active) {
		if ((i = select(serversock+1, &fdset, 0, 0, 0)) < 0) {
		    if (errno = EINTR) {
			continue;
		    } else {
			perror("in select waiting for API connection");
			break;
		    }
		} else {
		    i = accept(serversock, 0, 0);
		    if (i == -1) {
			perror("accepting API connection");
		    }
		    sock = i;
		    api_exch_init(sock);
		    state = UNCONNECTED;
		    break;
		}
	    }
	    (void) close(serversock);
	    /* If the process has already exited, we may need to close */
	    if ((shell_active == 0) && (sock != -1)) {
		(void) close(sock);
		sock = -1;
		setcommandmode();	/* In case child_died sneaked in */
	    }
	}
    } else {				/* New process */
	register int i;

	for (i = 3; i < 30; i++) {
	    (void) close(i);
	}
	if (argc == 1) {		/* Just get a shell */
	    char *cmdname;
	    extern char *getenv();

	    cmdname = getenv("SHELL");
	    execlp(cmdname, cmdname, 0);
	    perror("Exec'ing new shell...\n");
	    exit(1);
	} else {
	    execvp(argv[1], &argv[1]);
	    perror("Exec'ing command.\n");
	    exit(1);
	}
	/*NOTREACHED*/
    }
    return shell_active;		/* Go back to main loop */
}
