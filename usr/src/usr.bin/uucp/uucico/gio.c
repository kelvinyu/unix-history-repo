#ifndef lint
static char sccsid[] = "@(#)gio.c	5.2 (Berkeley) %G%";
#endif

#include <sys/types.h>
#include "pk.h"
#include <setjmp.h>
#include "uucp.h"
#ifdef USG
#define ftime time
#else V7
#include <sys/timeb.h>
#endif V7

extern	time_t	time();

jmp_buf Failbuf;

struct pack *Pk;

pkfail()
{
	longjmp(Failbuf, 1);
}

gturnon()
{
	struct pack *pkopen();

	if (setjmp(Failbuf))
		return FAIL;
	Pk = pkopen(Ifn, Ofn);
	if (Pk == NULL)
		return FAIL;
	return SUCCESS;
}

gturnoff()
{
	if(setjmp(Failbuf))
		return(FAIL);
	pkclose(Pk);
	return SUCCESS;
}


gwrmsg(type, str, fn)
char type;
register char *str;
{
	char bufr[BUFSIZ];
	register char *s;
	int len, i;

	if(setjmp(Failbuf))
		return(FAIL);
	bufr[0] = type;
	s = &bufr[1];
	while (*str)
		*s++ = *str++;
	*s = '\0';
	if (*(--s) == '\n')
		*s = '\0';
	len = strlen(bufr) + 1;
	if ((i = len % PACKSIZE)) {
		len = len + PACKSIZE - i;
		bufr[len - 1] = '\0';
	}
	gwrblk(bufr, len, fn);
	return SUCCESS;
}

/*ARGSUSED*/
grdmsg(str, fn)
register char *str;
{
	unsigned len;

	if(setjmp(Failbuf))
		return FAIL;
	for (;;) {
		len = pkread(Pk, str, PACKSIZE);
		if (len == 0)
			continue;
		str += len;
		if (*(str - 1) == '\0')
			break;
	}
	return SUCCESS;
}


gwrdata(fp1, fn)
FILE *fp1;
{
	char bufr[BUFSIZ];
	register int len;
	int ret, mil;
#ifdef USG
	time_t t1, t2;
#else v7
	struct timeb t1, t2;
#endif v7
	long bytes;
	char text[BUFSIZ];

	if(setjmp(Failbuf))
		return FAIL;
	bytes = 0L;
	ftime(&t1);
	while ((len = read(fileno(fp1), bufr, BUFSIZ)) > 0) {
		bytes += len;
		ret = gwrblk(bufr, len, fn);
		if (ret != len) {
			return FAIL;
		}
		if (len != BUFSIZ)
			break;
	}
	ret = gwrblk(bufr, 0, fn);
	ftime(&t2);
#ifndef USG
	t2.time -= t1.time;
	mil = t2.millitm - t1.millitm;
	if (mil < 0) {
		--t2.time;
		mil += 1000;
	}
	sprintf(text, "sent data %ld bytes %ld.%03d secs",
				bytes, (long)t2.time, mil);
	sysacct(bytes, t2.time - t1.time);
#else USG
	sprintf(text, "sent data %ld bytes %ld secs", bytes, t2 - t1);
	sysacct(bytes, t2 - t1);
#endif USG
	DEBUG(1, "%s\n", text);
	syslog(text);
	return SUCCESS;
}


grddata(fn, fp2)
FILE *fp2;
{
	register int len;
	char bufr[BUFSIZ];
#ifdef USG
	time_t t1, t2;
#else v7
	struct timeb t1, t2;
	int mil;
#endif v7
	long bytes;
	char text[BUFSIZ];

	if(setjmp(Failbuf))
		return FAIL;
	bytes = 0L;
	ftime(&t1);
	for (;;) {
		len = grdblk(bufr, BUFSIZ, fn);
		if (len < 0) {
			return FAIL;
		}
		bytes += len;
		if (write(fileno(fp2), bufr, len) != len)
			return FAIL;
		if (len < BUFSIZ)
			break;
	}
	ftime(&t2);
#ifndef USG
	t2.time -= t1.time;
	mil = t2.millitm - t1.millitm;
	if (mil < 0) {
		--t2.time;
		mil += 1000;
	}
	sprintf(text, "received data %ld bytes %ld.%03d secs",
				bytes, (long)t2.time, mil);
	sysacct(bytes, t2.time - t1.time);
#else USG
	sprintf(text, "received data %ld bytes %ld secs", bytes, t2 - t1);
	sysacct(bytes, t2 - t1);
#endif USG
	DEBUG(1, "%s\n", text);
	syslog(text);
	return SUCCESS;
}


/* call ultouch every TC calls to either grdblk or gwrblk -- rti!trt */
#define	TC	20
static	int tc = TC;

/*ARGSUSED*/
grdblk(blk, len,  fn)
register int len;
char *blk;
{
	register int i, ret;

	/* call ultouch occasionally */
	if (--tc < 0) {
		tc = TC;
		ultouch();
	}
	for (i = 0; i < len; i += ret) {
		ret = pkread(Pk, blk, len - i);
		if (ret < 0)
			return FAIL;
		blk += ret;
		if (ret == 0)
			return i;
	}
	return i;
}

/*ARGSUSED*/
gwrblk(blk, len, fn)
register char *blk;
{
	register int ret;

	/* call ultouch occasionally -- rti!trt */
	if (--tc < 0) {
		tc = TC;
		ultouch();
	}
	ret = pkwrite(Pk, blk, len);
	return ret;
}
