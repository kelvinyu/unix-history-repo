/* @(#)fwrite.c	4.4 (Berkeley) %G% */
#include	<stdio.h>

fwrite(ptr, size, count, iop)
	register char *ptr;
	unsigned size, count;
	register FILE *iop;
{
	register int s;

	s = size * count;
	if (iop->_flag & _IOLBF)
		while (s > 0) {
			if (--iop->_cnt > -iop->_bufsiz && *ptr != '\n')
				*iop->_ptr++ = *ptr++;
			else if (_flsbuf(*(unsigned char *)ptr++, iop) == EOF)
				break;
			s--;
		}
	else while (s > 0) {
		if (iop->_cnt < s) {
			if (iop->_cnt > 0) {
				bcopy(ptr, iop->_ptr, iop->_cnt);
				ptr += iop->_cnt;
				iop->_ptr += iop->_cnt;
				s -= iop->_cnt;
			}
			if (_flsbuf(*(unsigned char *)ptr++, iop) == EOF)
				break;
			s--;
		}
		if (iop->_cnt >= s) {
			bcopy(ptr, iop->_ptr, s);
			iop->_ptr += s;
			iop->_cnt -= s;
			return (count);
		}
	}
	return (count - ((s + size - 1) / size));
}
