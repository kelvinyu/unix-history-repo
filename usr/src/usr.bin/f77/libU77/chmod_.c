/*
char	id_chmod[]	= "@(#)chmod_.c	1.1";
 *
 * chmod - change file mode bits
 *
 * synopsis:
 *	integer function chmod (fname, mode)
 *	character*(*) fname, mode
 */

#include "../libI77/f_errno.h"

long chmod_(name, mode, namlen, modlen)
char	*name, *mode;
long	namlen, modlen;
{
	char	nambuf[256];
	char	modbuf[32];
	int	retcode;

	if (namlen >= sizeof nambuf || modlen >= sizeof modbuf)
		return((long)(errno=F_ERARG));
	g_char(name, namlen, nambuf);
	g_char(mode, modlen, modbuf);
	if (nambuf[0] == '\0')
		return((long)(errno=ENOENT));
	if (modbuf[0] == '\0')
		return((long)(errno=F_ERARG));
	if (fork())
	{
		if (wait(&retcode) == -1)
			return((long)errno);
		return((long)retcode);
	}
	else
		execl("/bin/chmod", "chmod", modbuf, nambuf, (char *)0);
}
