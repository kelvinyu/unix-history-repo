/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * the University of Utah, and William Jolitz.
 *
 * %sccs.include.386.c%
 *
 *	@(#)trap.c	5.3 (Berkeley) %G%
 */

/*
 * Copyright (c) 1989, 1990 William F. Jolitz
 */

/*
 * 386 Trap and System call handleing
 */

#include "../i386/psl.h"
#include "../i386/reg.h"
#include "../i386/pte.h"
#include "../i386/segments.h"
#include "../i386/frame.h"

#include "param.h"
#include "systm.h"
#include "dir.h"
#include "user.h"
#include "proc.h"
#include "seg.h"
#include "acct.h"
#include "kernel.h"
#include "vm.h"
#include "cmap.h"

#include "../i386/trap.h"

#define	USER	0x100		/* user-mode flag added to type */

struct	sysent sysent[];
int	nsysent;
#include "dbg.h"
/*
 * Called from the trap handler when a processor trap occurs.
 */
unsigned *rcr2(), Sysbase;
extern short cpl;
/*ARGSUSED*/
trap(frame)
	struct trapframe frame;
#define type frame.tf_trapno
#define code frame.tf_err
#define pc frame.tf_eip
{
	register int *locr0 = ((int *)&frame)/*-PS*/;
	register int i;
	register struct proc *p;
	struct timeval syst;
	extern int nofault;

#ifdef DEBUG
dprintf(DALLTRAPS, "%d. trap",u.u_procp->p_pid);
dprintf(DALLTRAPS, "\npc:%x cs:%x ds:%x eflags:%x isp %x\n",
		frame.tf_eip, frame.tf_cs, frame.tf_ds, frame.tf_eflags,
		frame.tf_isp);
dprintf(DALLTRAPS, "edi %x esi %x ebp %x ebx %x esp %x\n",
		frame.tf_edi, frame.tf_esi, frame.tf_ebp,
		frame.tf_ebx, frame.tf_esp);
dprintf(DALLTRAPS, "edx %x ecx %x eax %x\n",
		frame.tf_edx, frame.tf_ecx, frame.tf_eax);
dprintf(DALLTRAPS|DPAUSE, "ec %x type %x cr0 %x cr2 %x cpl %x \n",
		frame.tf_err, frame.tf_trapno, rcr0(), rcr2(), cpl);
#endif

	locr0[tEFLAGS] &= ~PSL_NT;	/* clear nested trap XXX */
if(nofault && frame.tf_trapno != 0xc)
	{ locr0[tEIP] = nofault; return;}

	syst = u.u_ru.ru_stime;
	if (ISPL(locr0[tCS]) == SEL_UPL /*&& cpl == 0*/) {
#ifdef DEBUG
if(cpl) pg("user cpl %x trap %d", cpl, frame.tf_trapno);
if(((unsigned)frame.tf_eip) >= 0xfe000000) pg("user eip %x", frame.tf_eip);
#endif
		type |= USER;
		u.u_ar0 = locr0;
	}
	switch (type) {

	default:
bit_sucker:
#ifdef KDB
		if (kdb_trap(&psl))
			return;
#endif
splhigh();
		printf("trap type %d, code = %x, pc = %x cs = %x, eflags = %x\n", type, code, pc, frame.tf_cs, frame.tf_eflags);
		type &= ~USER;
		panic("trap");
		/*NOTREACHED*/

	case T_SEGNPFLT + USER:
	case T_PROTFLT + USER:		/* protection fault */
		u.u_code = code + BUS_SEGM_FAULT ;
		i = SIGBUS;
		break;

	case T_PRIVINFLT + USER:	/* privileged instruction fault */
	case T_RESADFLT + USER:		/* reserved addressing fault */
	case T_RESOPFLT + USER:		/* reserved operand fault */
	case T_FPOPFLT + USER:		/* coprocessor operand fault */
		u.u_code = type &~ USER;
		i = SIGILL;
		break;

	case T_ASTFLT + USER:		/* Allow process switch */
	case T_ASTFLT:
		astoff();
		if ((u.u_procp->p_flag & SOWEUPC) && u.u_prof.pr_scale) {
			addupc(pc, &u.u_prof, 1);
			u.u_procp->p_flag &= ~SOWEUPC;
		}
		goto out;

	case T_DNA + USER:
		u.u_code = FPE_FPU_NP_TRAP;
		i = SIGFPE;
		break;

	case T_BOUND + USER:
		u.u_code = FPE_SUBRNG_TRAP;
		i = SIGFPE;
		break;

	case T_OFLOW + USER:
		u.u_code = FPE_INTOVF_TRAP;
		i = SIGFPE;
		break;

	case T_DIVIDE + USER:
		u.u_code = FPE_INTDIV_TRAP;
		i = SIGFPE;
		break;

	case T_ARITHTRAP + USER:
		u.u_code = code;
		i = SIGFPE;
		break;

	/*
	 * If the user SP is above the stack segment,
	 * grow the stack automatically.
	 */
	case T_STKFLT + USER:
	case T_SEGFLT + USER:
		if (grow((unsigned)locr0[tESP]) /*|| grow(code)*/)
			goto out;
		u.u_code = code;
		i = SIGSEGV;
		break;

	case T_TABLEFLT:		/* allow page table faults in kernel */
	case T_TABLEFLT + USER:		/* page table fault */
		panic("ptable fault");

	case T_PAGEFLT:			/* allow page faults in kernel mode */
	case T_PAGEFLT + USER:		/* page fault */
			{ register u_int vp;
			struct pte *pte;

			if(u.u_procp->p_pid == 2) goto bit_sucker;
#define PGEX_P	0x01
			if (rcr2() >= &Sysbase || code & PGEX_P) {
				u.u_code = code + BUS_PAGE_FAULT;
				i = SIGBUS;
				break;
			} else {
				vp = btop((int)rcr2());
				if (vp >= dptov(u.u_procp, u.u_procp->p_dsize)
				&& vp < sptov(u.u_procp, u.u_procp->p_ssize-1)){
					if (grow((unsigned)locr0[tESP])
					|| grow(rcr2()))
						goto out;
					else	if (nofault) {
						locr0[tEIP] = nofault;
						return;
					}
#ifdef DEBUG
pg("didnt ");
printf("\npc:%x cs:%x ds:%x eflags:%x isp %x\n",
		frame.tf_eip, frame.tf_cs, frame.tf_ds, frame.tf_eflags,
		frame.tf_isp);
printf("edi %x esi %x ebp %x ebx %x esp %x\n",
		frame.tf_edi, frame.tf_esi, frame.tf_ebp,
		frame.tf_ebx, frame.tf_esp);
printf("edx %x ecx %x eax %x\n",
		frame.tf_edx, frame.tf_ecx, frame.tf_eax);
printf("ec %x type %x cr0 %x cr2 %x cpl %x \n",
		frame.tf_err, frame.tf_trapno, rcr0(), rcr2(), cpl);
#endif
					i = SIGSEGV;
					break;
				}
				i = u.u_error;
				pagein(rcr2(), 0);
				u.u_error = i;
				if (type == T_PAGEFLT) return;

				if(nofault) {
					locr0[tEIP] = nofault;
					return;
				}
				goto out;
			}
		}

	case T_TRCTRAP:	 /* trace trap -- someone single stepping lcall's */
		locr0[tEFLAGS] &= ~PSL_T;
			/* Q: how do we turn it on again? */
		return;
	
	case T_BPTFLT + USER:		/* bpt instruction fault */
	case T_TRCTRAP + USER:		/* trace trap */
		locr0[tEFLAGS] &= ~PSL_T;
		i = SIGTRAP;
		break;

	/*
	 * For T_KSPNOTVAL and T_BUSERR, can not allow spl to
	 * drop to 0 as clock could go off and we would end up
	 * doing an rei to the interrupt stack at ipl 0 (a
	 * reserved operand fault).  Instead, we allow psignal
	 * to post an ast, then return to user mode where we
	 * will reenter the kernel on the kernel's stack and
	 * can then service the signal.
	 */
	case T_KSPNOTVAL:
		if (noproc)
			panic("ksp not valid");
		/* fall thru... */
	case T_KSPNOTVAL + USER:
		printf("pid %d: ksp not valid\n", u.u_procp->p_pid);
		/* must insure valid kernel stack pointer? */
		psignal(u.u_procp, SIGKILL);
		return;

	case T_BUSERR + USER:
		u.u_code = code;
		psignal(u.u_procp, SIGBUS);
		return;
	}
	psignal(u.u_procp, i);
out:
	p = u.u_procp;

	if (p->p_cursig || ISSIG(p))
		psig(1);
	p->p_pri = p->p_usrpri;
	if (runrun) {
		/*
		 * Since we are u.u_procp, clock will normally just change
		 * our priority without moving us from one queue to another
		 * (since the running process is not on a queue.)
		 * If that happened after we setrq ourselves but before we
		 * swtch()'ed, we might not be on the queue indicated by
		 * our priority.
		 */
		(void) splclock();
		setrq(p);
		u.u_ru.ru_nivcsw++;
		swtch();
		(void)spl0();
	}
	if (u.u_prof.pr_scale) {
		int ticks;
		struct timeval *tv = &u.u_ru.ru_stime;

		ticks = ((tv->tv_sec - syst.tv_sec) * 1000 +
			(tv->tv_usec - syst.tv_usec) / 1000) / (tick / 1000);
		if (ticks)
			addupc(pc, &u.u_prof, ticks);
	}
	curpri = p->p_pri;
#undef type
#undef code
#undef pc
}

/*
 * Called from locore when a system call occurs
 */
/*ARGSUSED*/
syscall(frame)
	struct syscframe frame;
#define code frame.sf_eax	/* note: written over! */
#define pc frame.sf_eip
{
	register int *locr0 = ((int *)&frame)/*-PS*/;
	register caddr_t params;
	register int i;
	register struct sysent *callp;
	register struct proc *p;
	struct timeval syst;
	int opc;

#ifdef lint
	r0 = 0; r0 = r0; r1 = 0; r1 = r1;
#endif
	syst = u.u_ru.ru_stime;
	if (ISPL(locr0[sCS]) != SEL_UPL)
{
printf("\npc:%x cs:%x eflags:%x\n",
		frame.sf_eip, frame.sf_cs, frame.sf_eflags);
printf("edi %x esi %x ebp %x ebx %x esp %x\n",
		frame.sf_edi, frame.sf_esi, frame.sf_ebp,
		frame.sf_ebx, frame.sf_esp);
printf("edx %x ecx %x eax %x\n", frame.sf_edx, frame.sf_ecx, frame.sf_eax);
printf("cr0 %x cr2 %x cpl %x \n", rcr0(), rcr2(), cpl);
		panic("syscall");
}
#ifdef DEBUG
if(cpl) pg("User cpl %x", cpl);
#endif
	u.u_ar0 = locr0;
	params = (caddr_t)locr0[sESP] + NBPW ;
	u.u_error = 0;
	/*
	 * Reconstruct pc, assuming lcall $X,y is 7 bytes, as it is always.
	 */
	opc = pc - 7;
	callp = (code >= nsysent) ? &sysent[63] : &sysent[code];
	if (callp == sysent) {
		i = fuword(params);
		params += NBPW;
		callp = (code >= nsysent) ? &sysent[63] : &sysent[code];
	}
dprintf(DALLSYSC,"%d. call %d\n", u.u_procp->p_pid, code);
	if ((i = callp->sy_narg * sizeof (int)) &&
	    (u.u_error = copyin(params, (caddr_t)u.u_arg, (u_int)i)) != 0) {
		locr0[sEAX] = u.u_error;
		locr0[sEFLAGS] |= PSL_C;	/* carry bit */
		goto done;
	}
	u.u_r.r_val1 = 0;
	u.u_r.r_val2 = locr0[sEDX];
	if (setjmp(&u.u_qsave)) {
		if (u.u_error == 0 && u.u_eosys != RESTARTSYS)
			u.u_error = EINTR;
	} else {
		u.u_eosys = NORMALRETURN;
		(*callp->sy_call)();
	}
	if (u.u_eosys == NORMALRETURN) {
		if (u.u_error) {
/*dprintf(DSYSFAIL,"%d. fail %d %d\n",u.u_procp->p_pid,  code, u.u_error);*/
			locr0[sEAX] = u.u_error;
			locr0[sEFLAGS] |= PSL_C;	/* carry bit */
		} else {
			locr0[sEFLAGS] &= ~PSL_C;	/* clear carry bit */
			locr0[sEAX] = u.u_r.r_val1;
			locr0[sEDX] = u.u_r.r_val2;
		}
	} else if (u.u_eosys == RESTARTSYS)
		pc = opc;
	/* else if (u.u_eosys == JUSTRETURN) */
		/* nothing to do */
done:
	p = u.u_procp;
	if (p->p_cursig || ISSIG(p))
		psig(0);
	p->p_pri = p->p_usrpri;
	if (runrun) {
		/*
		 * Since we are u.u_procp, clock will normally just change
		 * our priority without moving us from one queue to another
		 * (since the running process is not on a queue.)
		 * If that happened after we setrq ourselves but before we
		 * swtch()'ed, we might not be on the queue indicated by
		 * our priority.
		 */
		(void) splclock();
		setrq(p);
		u.u_ru.ru_nivcsw++;
		swtch();
		(void)spl0();
	}
	if (u.u_prof.pr_scale) {
		int ticks;
		struct timeval *tv = &u.u_ru.ru_stime;

		ticks = ((tv->tv_sec - syst.tv_sec) * 1000 +
			(tv->tv_usec - syst.tv_usec) / 1000) / (tick / 1000);
		if (ticks)
			addupc(opc, &u.u_prof, ticks);
	}
	curpri = p->p_pri;
dprintf(DALLSYSC,"%d. rtn to %x %x\n",
	u.u_procp->p_pid, frame.sf_eip, frame.sf_cs);
#ifdef DEBUG
if(cpl) {
 printf("uSer cpl %x syscall %d\n", cpl, callp - sysent);
 spl0();
}
#endif
}

/*
 * nonexistent system call-- signal process (may want to handle it)
 * flag error if process won't see signal immediately
 * Q: should we do that all the time ??
 */
nosys()
{

	if (u.u_signal[SIGSYS] == SIG_IGN || u.u_signal[SIGSYS] == SIG_HOLD)
		u.u_error = EINVAL;
	psignal(u.u_procp, SIGSYS);
}

/*
 * Ignored system call
 */
nullsys()
{

}
