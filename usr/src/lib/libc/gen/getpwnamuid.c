/*	getpwnamuid.c	4.3	84/01/31	*/

#include <stdio.h>
#include <pwd.h>
#include <ndbm.h>

#include <sys/file.h>

static char PASSWD[] = "/etc/passwd";
static char EMPTY[] = "";
static char line[BUFSIZ+1];
static struct passwd passwd;
static DBM *db = 0;

static struct passwd *
fetchpw(key)
	datum key;
{
        register char *cp, *tp;

        if (key.dptr == 0)
                return ((struct passwd *)NULL);
	key = dbmfetch(db, key);
	if (key.dptr == 0)
                return ((struct passwd *)NULL);
        cp = key.dptr;
	tp = line;

#define	EXPAND(e)	passwd.pw_/**/e = tp; while (*tp++ = *cp++);
	EXPAND(name);
	EXPAND(passwd);
	passwd.pw_uid = *(int *)cp; cp += sizeof (int);
	passwd.pw_gid = *(int *)cp; cp += sizeof (int);
	passwd.pw_quota = *(int *)cp; cp += sizeof (int);
	EXPAND(comment);
	EXPAND(gecos);
	EXPAND(dir);
	EXPAND(shell);
        return (&passwd);
}

struct passwd *
getpwnam(nam)
	char *nam;
{
        datum key;
	register struct passwd *pw;

        if ((db = ndbmopen(PASSWD, O_RDONLY)) == (DBM *)0)
                return ((struct passwd *)NULL);
	if (flock(db->db_dirf, LOCK_SH) < 0)
                return ((struct passwd *)NULL);
        key.dptr = nam;
        key.dsize = strlen(nam);
	pw = fetchpw(key);
	ndbmclose(db);
        return (pw);
}

struct passwd *
getpwuid(uid)
	int uid;
{
        datum key;
	register struct passwd *pw;

        if ((db = ndbmopen(PASSWD, O_RDONLY)) == (DBM *)0)
                return ((struct passwd *)NULL);
	if (flock(db->db_dirf, LOCK_SH) < 0)
                return ((struct passwd *)NULL);
        key.dptr = (char *) &uid;
        key.dsize = sizeof uid;
	pw = fetchpw(key);
	ndbmclose(db);
        return (pw);
}
