/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)kern_prot.c	7.5 (Berkeley) %G%
 */

/*
 * System calls related to processes and protection
 */

#include "param.h"
#include "systm.h"
#include "dir.h"
#include "user.h"
#include "inode.h"
#include "proc.h"
#include "timeb.h"
#include "times.h"
#include "reboot.h"
#include "fs.h"
#include "buf.h"
#include "mount.h"
#include "quota.h"

#include "machine/reg.h"

getpid()
{

	u.u_r.r_val1 = u.u_procp->p_pid;
	u.u_r.r_val2 = u.u_procp->p_ppid;
}

getpgrp()
{
	register struct a {
		int	pid;
	} *uap = (struct a *)u.u_ap;
	register struct proc *p;

	if (uap->pid == 0)
		p = u.u_procp;
	else if ((p = pfind(uap->pid)) == 0) {
		u.u_error = ESRCH;
		return;
	}
	u.u_r.r_val1 = p->p_pgrp->pg_id;
}

getuid()
{

	u.u_r.r_val1 = u.u_ruid;
	u.u_r.r_val2 = u.u_uid;
}

getgid()
{

	u.u_r.r_val1 = u.u_rgid;
	u.u_r.r_val2 = u.u_gid;
}

getgroups()
{
	register struct	a {
		u_int	gidsetsize;
		int	*gidset;
	} *uap = (struct a *)u.u_ap;
	register gid_t *gp;
	register int *lp;
	int groups[NGROUPS];

	for (gp = &u.u_groups[NGROUPS]; gp > u.u_groups; gp--)
		if (gp[-1] != NOGROUP)
			break;
	if (uap->gidsetsize < gp - u.u_groups) {
		u.u_error = EINVAL;
		return;
	}
	uap->gidsetsize = gp - u.u_groups;
	for (lp = groups, gp = u.u_groups; lp < &groups[uap->gidsetsize]; )
		*lp++ = *gp++;
	u.u_error = copyout((caddr_t)groups, (caddr_t)uap->gidset,
	    uap->gidsetsize * sizeof (groups[0]));
	if (u.u_error)
		return;
	u.u_r.r_val1 = uap->gidsetsize;
}

setsid()
{
	register struct proc *p = u.u_procp;

	if ((p->p_pgid == p->p_pid) || pgfind(p->p_pid))
		u.u_error = EPERM;
	else {
		pgmv(p, p->p_pid, 1);
		u.u_r.r_val1 = p->p_pid;
	}
	return;
}

/*
 * set process group
 *
 * if target pid != caller's pid
 *	pid must be an inferior
 *	pid must be in same session
 *	pid can't have done an exec
 *	there must exist a pid with pgid in same session 
 * pid must not be session leader
 */
setpgrp()
{
	register struct a {
		int	pid;
		int	pgid;
	} *uap = (struct a *)u.u_ap;
	register struct proc *p;
	register struct pgrp *pgrp;

	if (uap->pid == 0)
		p = u.u_procp;
	else if ((p = pfind(uap->pid)) == 0 || !inferior(p)) {
		u.u_error = ESRCH;
		return;
	}
	else if (p != u.u_procp) { 
		if (p->p_session != u.u_procp->p_session) {
			u.u_error = EPERM;
			return;
		}
		if (p->p_flag&SEXEC) {
			u.u_error = EACCES;
			return;
		}
	}
	if (SESS_LEADER(p)) {
		u.u_error = EPERM;
		return;
	}
	if (uap->pgid == 0)
		uap->pgid = p->p_pid;
	else if ((uap->pgid != p->p_pid) &&
		(((pgrp = pgfind(uap->pgid)) == 0) || 
		   pgrp->pg_mem == NULL ||
	           pgrp->pg_session != u.u_procp->p_session)) {
		u.u_error = EPERM;
		return;
	}
	/*
	 * done checking, now doit
	 */
	pgmv(p, uap->pgid, 0);
}

setreuid()
{
	struct a {
		int	ruid;
		int	euid;
	} *uap;
	register int ruid, euid;

	uap = (struct a *)u.u_ap;
	ruid = uap->ruid;
	if (ruid == -1)
		ruid = u.u_ruid;
	if (u.u_ruid != ruid && u.u_uid != ruid &&
	    (u.u_error = suser(u.u_cred, &u.u_acflag)))
		return;
	euid = uap->euid;
	if (euid == -1)
		euid = u.u_uid;
	if (u.u_ruid != euid && u.u_uid != euid &&
	    (u.u_error = suser(u.u_cred, &u.u_acflag)))
		return;
	/*
	 * Everything's okay, do it.
	 */
#ifdef QUOTA
	if (u.u_quota->q_uid != ruid) {
		qclean();
		qstart(getquota((uid_t)ruid, 0, 0));
	}
#endif
	u.u_procp->p_uid = euid;
	u.u_ruid = ruid;
	u.u_uid = euid;
}

setregid()
{
	register struct a {
		int	rgid;
		int	egid;
	} *uap;
	register int rgid, egid;

	uap = (struct a *)u.u_ap;
	rgid = uap->rgid;
	if (rgid == -1)
		rgid = u.u_rgid;
	if (u.u_rgid != rgid && u.u_gid != rgid &&
	    (u.u_error = suser(u.u_cred, &u.u_acflag)))
		return;
	egid = uap->egid;
	if (egid == -1)
		egid = u.u_gid;
	if (u.u_rgid != egid && u.u_gid != egid &&
	    (u.u_error = suser(u.u_cred, &u.u_acflag)))
		return;
	if (u.u_rgid != rgid) {
		leavegroup(u.u_rgid);
		(void) entergroup((gid_t)rgid);
		u.u_rgid = rgid;
	}
	u.u_gid = egid;
}

setgroups()
{
	register struct	a {
		u_int	gidsetsize;
		int	*gidset;
	} *uap = (struct a *)u.u_ap;
	register gid_t *gp;
	register int *lp;
	int groups[NGROUPS];

	if (u.u_error = suser(u.u_cred, &u.u_acflag))
		return;
	if (uap->gidsetsize > sizeof (u.u_groups) / sizeof (u.u_groups[0])) {
		u.u_error = EINVAL;
		return;
	}
	u.u_error = copyin((caddr_t)uap->gidset, (caddr_t)groups,
	    uap->gidsetsize * sizeof (groups[0]));
	if (u.u_error)
		return;
	for (lp = groups, gp = u.u_groups; lp < &groups[uap->gidsetsize]; )
		*gp++ = *lp++;
	for ( ; gp < &u.u_groups[NGROUPS]; gp++)
		*gp = NOGROUP;
}

/*
 * Group utility functions.
 */

/*
 * Delete gid from the group set.
 */
leavegroup(gid)
	gid_t gid;
{
	register gid_t *gp;

	for (gp = u.u_groups; gp < &u.u_groups[NGROUPS]; gp++)
		if (*gp == gid)
			goto found;
	return;
found:
	for (; gp < &u.u_groups[NGROUPS-1]; gp++)
		*gp = *(gp+1);
	*gp = NOGROUP;
}

/*
 * Add gid to the group set.
 */
entergroup(gid)
	gid_t gid;
{
	register gid_t *gp;

	for (gp = u.u_groups; gp < &u.u_groups[NGROUPS]; gp++) {
		if (*gp == gid)
			return (0);
		if (*gp == NOGROUP) {
			*gp = gid;
			return (0);
		}
	}
	return (-1);
}

/*
 * Check if gid is a member of the group set.
 */
groupmember(gid)
	gid_t gid;
{
	register gid_t *gp;

	if (u.u_gid == gid)
		return (1);
	for (gp = u.u_groups; gp < &u.u_groups[NGROUPS] && *gp != NOGROUP; gp++)
		if (*gp == gid)
			return (1);
	return (0);
}

/*
 * Get login name of process owner, if available
 */

getlogname()
{
	struct a {
		char	*namebuf;
		u_int	namelen;
	} *uap = (struct a *)u.u_ap;

	if (uap->namelen > sizeof (u.u_logname))
		uap->namelen = sizeof (u.u_logname);
	u.u_error = copyout((caddr_t)u.u_logname, (caddr_t)uap->namebuf,
		uap->namelen);
}

/*
 * Set login name of process owner
 */

setlogname()
{
	struct a {
		char	*namebuf;
		u_int	namelen;
	} *uap = (struct a *)u.u_ap;

	if (u.u_error = suser(u.u_cred, &u.u_acflag))
		return;
	if (uap->namelen > sizeof (u.u_logname) - 1)
		u.u_error = EINVAL;
	else
		u.u_error = copyin((caddr_t)uap->namebuf,
			(caddr_t)u.u_logname, uap->namelen);
}
