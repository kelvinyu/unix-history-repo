/*
char id_dofio[] = "@(#)dofio.c	1.6";
 *
 * fortran format executer
 */

#include "fio.h"
#include "format.h"

#define DO(x)	if(n=x) err(n>0?errflag:endflag,n,dfio)
#define STKSZ 10
int cnt[STKSZ],ret[STKSZ],cp,rp;
char *dfio = "dofio";

en_fio()
{	ftnint one=1;
	return(do_fio(&one,NULL,0L));
}

/* OP_TYPE_TAB is defined in format.h,
		  it is NED for X,SLASH,APOS,H,TL,TR,T
		  ED  for I,IM,F,E,EE,D,DE,G,GE,L,A,AW
		  and returns op for other values 
 */
static int optypes[] = { OP_TYPE_TAB };
static int rep_count, in_mid;

do_fio(number,ptr,len) ftnint *number; ftnlen len; char *ptr;
{	struct syl *p;
	int n,i,more,optype;
	more = *number;
	for(;;) {
	  if( (optype = ((p= &syl_ptr[pc])->op)) > LAST_TERM )
		err(errflag,F_ERFMT,"impossible code");
#ifdef DEBUG
	  fprintf(stderr," pc=%d, cnt[%d]=%d, ret[%d]=%d, op=%d\n",
		pc,cp,cnt[cp],rp,ret[rp],optype); /*for debug*/
#endif
	  switch(optypes[optype])
	  {
	  case NED:
		DO((*doned)(p,ptr))
		pc++;
		break;
	  case ED:
		if(in_mid == NO) rep_count = p->rpcnt;
		in_mid = YES;
		while (rep_count > 0 ) {
		    if(ptr==NULL)
		    {	DO((*doend)('\n'))
			return(OK);
		    }
		    if(!more) return(OK);
		    DO((*doed)(p,ptr,len))
		    ptr += len;
		    more--;
		    rep_count--;
		}
		pc++;
		in_mid = NO;
		break;
	  case STACK:		/* repeat count */
		if(++cp==STKSZ) err(errflag,F_ERFMT,"too many nested ()")
		cnt[cp]=p->p1;
		pc++;
		break;
	  case RET:		/* open paren */
		if(++rp==STKSZ) err(errflag,F_ERFMT,"too many nested ()")
		ret[rp]=p->p1;
		pc++;
		break;
	  case GOTO:		/* close paren */
		if(--cnt[cp]<=0)
		{	cp--;
			rp--;
			pc++;
		}
		else pc = ret[rp--] + 1;
		break;
	  case REVERT:		/* end of format */
		if(ptr==NULL)
		{	DO((*doend)('\n'))
			return(OK);
		}
		if(!more) return(OK);
		rp=cp=0;
		pc = p->p1;
		DO((*dorevert)())
		break;
	  case COLON:
#ifndef KOSHER
	  case DOLAR:				/*** NOT STANDARD FORTRAN ***/
#endif
		if (ptr == NULL)
		{	DO((*doend)((char)p->p1))
			return(OK);
		}
		if (!more) return(OK);
		pc++;
		break;
#ifndef KOSHER
	  case SU:				/*** NOT STANDARD FORTRAN ***/
#endif
	  case SS:
	  case SP:
	  case S: cplus = p->p1;
		signit = p->p2;
		pc++;
		break;
	  case P:
		scale = p->p1;
		pc++;
		break;
#ifndef KOSHER
	  case R:					/*** NOT STANDARD FORTRAN ***/
		radix = p->p1;
		pc++;
		break;
	  case B:					/*** NOT STANDARD FORTRAN ***/
		if (external) cblank = curunit->ublnk;
		else cblank = 0;		/* blank = 'NULL' */
		pc++;
		break;
#endif
	  case BNZ:
		cblank = p->p1;
		pc++;
		break;
	  default:
		err(errflag,F_ERFMT,"impossible code")
	  }
	}
}

fmt_bg()
{
	in_mid = NO;
	cp=rp=pc=cursor=0;
	cnt[0]=ret[0]=0;
}
