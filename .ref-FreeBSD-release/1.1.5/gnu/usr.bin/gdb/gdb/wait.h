/* Define how to access the int that the wait system call stores.
   This has been compatible in all Unix systems since time immemorial,
   but various well-meaning people have defined various different
   words for the same old bits in the same old int (sometimes claimed
   to be a struct).  We just know it's an int and we use these macros
   to access the bits.  */

/* The following macros are defined equivalently to their definitions
   in POSIX.1.  We fail to define WNOHANG and WUNTRACED, which POSIX.1
   <sys/wait.h> defines, since our code does not use waitpid().  We
   also fail to declare wait() and waitpid().  */   

#define WIFEXITED(w)	(((w)&0377) == 0)
#define WIFSIGNALED(w)	(((w)&0377) != 0177 && ((w)&~0377) == 0)
#ifdef IBM6000

/* Unfortunately, the above comment (about being compatible in all Unix 
   systems) is not quite correct for AIX, sigh.  And AIX 3.2 can generate
   status words like 0x57c (sigtrap received after load), and gdb would
   choke on it. */

#define WIFSTOPPED(w)	((w)&0x40)

#else
#define WIFSTOPPED(w)	(((w)&0377) == 0177)
#endif

#define WEXITSTATUS(w)	(((w) >> 8) & 0377) /* same as WRETCODE */
#define WTERMSIG(w)	((w) & 0177)
#define WSTOPSIG	WEXITSTATUS

/* These are not defined in POSIX, but are used by our programs.  */

#define WAITTYPE	int

#define WCOREDUMP(w)	(((w)&0200) != 0)
#define WSETEXIT(w,status) ((w) = (0 | ((status) << 8)))
#define WSETSTOP(w,sig)	   ((w) = (0177 | ((sig) << 8)))
