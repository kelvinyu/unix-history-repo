/*
 * Copyright (c) 1982, 1986, 1989, 1990, 1991 Regents of the University
 * of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)kern_prot.c	7.23 (Berkeley) %G%
 */

/*
 * System calls related to processes and protection
 */

#include "param.h"
#include "acct.h"
#include "systm.h"
#include "ucred.h"
#include "proc.h"
#include "timeb.h"
#include "times.h"
#include "malloc.h"

/* ARGSUSED */
getpid(p, uap, retval)
	struct proc *p;
	void *uap;
	int *retval;
{

	*retval = p->p_pid;
#if defined(COMPAT_43) || defined(COMPAT_SUNOS)
	retval[1] = p->p_pptr->p_pid;
#endif
	return (0);
}

/* ARGSUSED */
getppid(p, uap, retval)
	struct proc *p;
	void *uap;
	int *retval;
{

	*retval = p->p_pptr->p_pid;
	return (0);
}

/* Get process group ID; note that POSIX getpgrp takes no parameter */
getpgrp(p, uap, retval)
	struct proc *p;
	void *uap;
	int *retval;
{

	*retval = p->p_pgrp->pg_id;
	return (0);
}

/* ARGSUSED */
getuid(p, uap, retval)
	struct proc *p;
	void *uap;
	int *retval;
{

	*retval = p->p_cred->p_ruid;
#if defined(COMPAT_43) || defined(COMPAT_SUNOS)
	retval[1] = p->p_ucred->cr_uid;
#endif
	return (0);
}

/* ARGSUSED */
geteuid(p, uap, retval)
	struct proc *p;
	void *uap;
	int *retval;
{

	*retval = p->p_ucred->cr_uid;
	return (0);
}

/* ARGSUSED */
getgid(p, uap, retval)
	struct proc *p;
	void *uap;
	int *retval;
{

	*retval = p->p_cred->p_rgid;
#if defined(COMPAT_43) || defined(COMPAT_SUNOS)
	retval[1] = p->p_ucred->cr_groups[0];
#endif
	return (0);
}

/*
 * Get effective group ID.  The "egid" is groups[0], and could be obtained
 * via getgroups.  This syscall exists because it is somewhat painful to do
 * correctly in a library function.
 */
/* ARGSUSED */
getegid(p, uap, retval)
	struct proc *p;
	void *uap;
	int *retval;
{

	*retval = p->p_ucred->cr_groups[0];
	return (0);
}

getgroups(p, uap, retval)
	struct proc *p;
	register struct	arg {
		u_int	gidsetsize;
		int	*gidset;		/* XXX not yet POSIX */
	} *uap;
	int *retval;
{
	register struct pcred *pc = p->p_cred;
	register gid_t *gp;
	register int *lp;
	register u_int ngrp;
	int groups[NGROUPS];
	int error;

	if ((ngrp = uap->gidsetsize) == 0) {
		*retval = pc->pc_ucred->cr_ngroups;
		return (0);
	}
	if (ngrp < pc->pc_ucred->cr_ngroups)
		return (EINVAL);
	ngrp = pc->pc_ucred->cr_ngroups;
	for (gp = pc->pc_ucred->cr_groups, lp = groups; lp < &groups[ngrp]; )
		*lp++ = *gp++;
	if (error = copyout((caddr_t)groups, (caddr_t)uap->gidset,
	    ngrp * sizeof (groups[0])))
		return (error);
	*retval = ngrp;
	return (0);
}

/* ARGSUSED */
setsid(p, uap, retval)
	register struct proc *p;
	void *uap;
	int *retval;
{

	if (p->p_pgid == p->p_pid || pgfind(p->p_pid)) {
		return (EPERM);
	} else {
		enterpgrp(p, p->p_pid, 1);
		*retval = p->p_pid;
		return (0);
	}
}

/*
 * set process group (setpgid/old setpgrp)
 *
 * caller does setpgid(targpid, targpgid)
 *
 * pid must be caller or child of caller (ESRCH)
 * if a child
 *	pid must be in same session (EPERM)
 *	pid can't have done an exec (EACCES)
 * if pgid != pid
 * 	there must exist some pid in same session having pgid (EPERM)
 * pid must not be session leader (EPERM)
 */
/* ARGSUSED */
setpgid(curp, uap, retval)
	struct proc *curp;
	register struct args {
		int	pid;	/* target process id */
		int	pgid;	/* target pgrp id */
	} *uap;
	int *retval;
{
	register struct proc *targp;		/* target process */
	register struct pgrp *pgrp;		/* target pgrp */

	if (uap->pid != 0 && uap->pid != curp->p_pid) {
		if ((targp = pfind(uap->pid)) == 0 || !inferior(targp))
			return (ESRCH);
		if (targp->p_session != curp->p_session)
			return (EPERM);
		if (targp->p_flag&SEXEC)
			return (EACCES);
	} else
		targp = curp;
	if (SESS_LEADER(targp))
		return (EPERM);
	if (uap->pgid == 0)
		uap->pgid = targp->p_pid;
	else if (uap->pgid != targp->p_pid)
		if ((pgrp = pgfind(uap->pgid)) == 0 ||
	            pgrp->pg_session != curp->p_session)
			return (EPERM);
	enterpgrp(targp, uap->pgid, 0);
	return (0);
}

/* ARGSUSED */
setuid(p, uap, retval)
	struct proc *p;
	struct args {
		int	uid;
	} *uap;
	int *retval;
{
	register struct pcred *pc = p->p_cred;
	register uid_t uid;
	int error;

	uid = uap->uid;
	if (uid != pc->p_ruid &&
	    (error = suser(pc->pc_ucred, &p->p_acflag)))
		return (error);
	/*
	 * Everything's okay, do it.  Copy credentials so other references do
	 * not see our changes.
	 */
	pc->pc_ucred = crcopy(pc->pc_ucred);
	pc->pc_ucred->cr_uid = uid;
	pc->p_ruid = uid;
	pc->p_svuid = uid;
	p->p_flag |= SUGID;
	return (0);
}

/* ARGSUSED */
seteuid(p, uap, retval)
	struct proc *p;
	struct args {
		int	euid;
	} *uap;
	int *retval;
{
	register struct pcred *pc = p->p_cred;
	register uid_t euid;
	int error;

	euid = uap->euid;
	if (euid != pc->p_ruid && euid != pc->p_svuid &&
	    (error = suser(pc->pc_ucred, &p->p_acflag)))
		return (error);
	/*
	 * Everything's okay, do it.  Copy credentials so other references do
	 * not see our changes.
	 */
	pc->pc_ucred = crcopy(pc->pc_ucred);
	pc->pc_ucred->cr_uid = euid;
	p->p_flag |= SUGID;
	return (0);
}

/* ARGSUSED */
setgid(p, uap, retval)
	struct proc *p;
	struct args {
		int	gid;
	} *uap;
	int *retval;
{
	register struct pcred *pc = p->p_cred;
	register gid_t gid;
	int error;

	gid = uap->gid;
	if (gid != pc->p_rgid && (error = suser(pc->pc_ucred, &p->p_acflag)))
		return (error);
	pc->pc_ucred = crcopy(pc->pc_ucred);
	pc->pc_ucred->cr_groups[0] = gid;
	pc->p_rgid = gid;
	pc->p_svgid = gid;		/* ??? */
	p->p_flag |= SUGID;
	return (0);
}

/* ARGSUSED */
setegid(p, uap, retval)
	struct proc *p;
	struct args {
		int	egid;
	} *uap;
	int *retval;
{
	register struct pcred *pc = p->p_cred;
	register gid_t egid;
	int error;

	egid = uap->egid;
	if (egid != pc->p_rgid && egid != pc->p_svgid &&
	    (error = suser(pc->pc_ucred, &p->p_acflag)))
		return (error);
	pc->pc_ucred = crcopy(pc->pc_ucred);
	pc->pc_ucred->cr_groups[0] = egid;
	p->p_flag |= SUGID;
	return (0);
}

#if defined(COMPAT_43) || defined(COMPAT_SUNOS)
/* ARGSUSED */
osetreuid(p, uap, retval)
	register struct proc *p;
	struct args {
		int	ruid;
		int	euid;
	} *uap;
	int *retval;
{
	register struct pcred *pc = p->p_cred;
	register uid_t ruid, euid;
	int error;

	if (uap->ruid == -1)
	if (ruid != pc->p_ruid && ruid != pc->pc_ucred->cr_uid /* XXX */ &&
	    (error = suser(pc->pc_ucred, &p->p_acflag)))
		return (error);
	if (uap->euid == -1)
	if (euid != pc->pc_ucred->cr_uid && euid != pc->p_ruid &&
	    euid != pc->p_svuid && (error = suser(pc->pc_ucred, &p->p_acflag)))
		return (error);
	/*
	 * Everything's okay, do it.  Copy credentials so other references do
	 * not see our changes.
	 */
	pc->pc_ucred = crcopy(pc->pc_ucred);
	pc->pc_ucred->cr_uid = euid;
	pc->p_ruid = ruid;
	p->p_flag |= SUGID;
	return (0);
}

/* ARGSUSED */
osetregid(p, uap, retval)
	register struct proc *p;
	struct args {
		int	rgid;
		int	egid;
	} *uap;
	int *retval;
{
	register struct pcred *pc = p->p_cred;
	register gid_t rgid, egid;
	int error;

	if (uap->rgid == -1)
	if (rgid != pc->p_rgid && rgid != pc->pc_ucred->cr_groups[0] /* XXX */ &&
	    (error = suser(pc->pc_ucred, &p->p_acflag)))
		return (error);
	if (uap->egid == -1)
	if (egid != pc->pc_ucred->cr_groups[0] && egid != pc->p_rgid &&
	    egid != pc->p_svgid && (error = suser(pc->pc_ucred, &p->p_acflag)))
		return (error);
	pc->pc_ucred = crcopy(pc->pc_ucred);
	pc->pc_ucred->cr_groups[0] = egid;
	pc->p_rgid = rgid;
	p->p_flag |= SUGID;
	return (0);
}
#endif /* COMPAT_43 || COMPAT_SUNOS */

/* ARGSUSED */
setgroups(p, uap, retval)
	struct proc *p;
	struct args {
		u_int	gidsetsize;
		int	*gidset;
	} *uap;
	int *retval;
{
	register struct pcred *pc = p->p_cred;
	register gid_t *gp;
	register u_int ngrp;
	register int *lp;
	int error, groups[NGROUPS];

	if (error = suser(pc->pc_ucred, &p->p_acflag))
		return (error);
	if ((ngrp = uap->gidsetsize) > NGROUPS)
		return (EINVAL);
	if (error = copyin((caddr_t)uap->gidset, (caddr_t)groups,
	    ngrp * sizeof (groups[0])))
		return (error);
	pc->pc_ucred = crcopy(pc->pc_ucred);
	pc->pc_ucred->cr_ngroups = ngrp;
	/* convert from int's to gid_t's */
	for (gp = pc->pc_ucred->cr_groups, lp = groups; ngrp--; )
		*gp++ = *lp++;
	p->p_flag |= SUGID;
	return (0);
}

/*
 * Check if gid is a member of the group set.
 */
groupmember(gid, cred)
	gid_t gid;
	register struct ucred *cred;
{
	register gid_t *gp;
	gid_t *egp;

	egp = &(cred->cr_groups[cred->cr_ngroups]);
	for (gp = cred->cr_groups; gp < egp; gp++)
		if (*gp == gid)
			return (1);
	return (0);
}

/*
 * Test whether the specified credentials imply "super-user"
 * privilege; if so, and we have accounting info, set the flag
 * indicating use of super-powers.
 * Returns 0 or error.
 */
suser(cred, acflag)
	struct ucred *cred;
	short *acflag;
{
	if (cred->cr_uid == 0) {
		if (acflag)
			*acflag |= ASU;
		return (0);
	}
	return (EPERM);
}

/*
 * Allocate a zeroed cred structure.
 */
struct ucred *
crget()
{
	register struct ucred *cr;

	MALLOC(cr, struct ucred *, sizeof(*cr), M_CRED, M_WAITOK);
	bzero((caddr_t)cr, sizeof(*cr));
	cr->cr_ref = 1;
	return (cr);
}

/*
 * Free a cred structure.
 * Throws away space when ref count gets to 0.
 */
crfree(cr)
	struct ucred *cr;
{
	int s = splimp();			/* ??? */

	if (--cr->cr_ref != 0) {
		(void) splx(s);
		return;
	}
	FREE((caddr_t)cr, M_CRED);
	(void) splx(s);
}

/*
 * Copy cred structure to a new one and free the old one.
 */
struct ucred *
crcopy(cr)
	struct ucred *cr;
{
	struct ucred *newcr;

	if (cr->cr_ref == 1)
		return (cr);
	newcr = crget();
	*newcr = *cr;
	crfree(cr);
	newcr->cr_ref = 1;
	return (newcr);
}

/*
 * Dup cred struct to a new held one.
 */
struct ucred *
crdup(cr)
	struct ucred *cr;
{
	struct ucred *newcr;

	newcr = crget();
	*newcr = *cr;
	newcr->cr_ref = 1;
	return (newcr);
}

/*
 * Get login name, if available.
 */
/* ARGSUSED */
getlogin(p, uap, retval)
	struct proc *p;
	struct args {
		char	*namebuf;
		u_int	namelen;
	} *uap;
	int *retval;
{

	if (uap->namelen > sizeof (p->p_pgrp->pg_session->s_login))
		uap->namelen = sizeof (p->p_pgrp->pg_session->s_login);
	return (copyout((caddr_t) p->p_pgrp->pg_session->s_login,
	    (caddr_t) uap->namebuf, uap->namelen));
}

/*
 * Set login name.
 */
/* ARGSUSED */
setlogin(p, uap, retval)
	struct proc *p;
	struct args {
		char	*namebuf;
	} *uap;
	int *retval;
{
	int error;

	if (error = suser(p->p_ucred, &p->p_acflag))
		return (error);
	error = copyinstr((caddr_t) uap->namebuf,
	    (caddr_t) p->p_pgrp->pg_session->s_login,
	    sizeof (p->p_pgrp->pg_session->s_login) - 1, (u_int *)0);
	if (error == ENAMETOOLONG)
		error = EINVAL;
	return (error);
}
