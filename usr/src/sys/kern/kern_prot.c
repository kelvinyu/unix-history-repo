/*	kern_prot.c	5.8	82/10/10	*/

/*
 * System calls related to processes and protection
 */

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/reg.h"
#include "../h/inode.h"
#include "../h/proc.h"
#include "../h/timeb.h"
#include "../h/times.h"
#include "../h/reboot.h"
#include "../h/fs.h"
#include "../h/conf.h"
#include "../h/buf.h"
#include "../h/mount.h"
#include "../h/quota.h"

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
		uap->pid = u.u_procp->p_pid;
	p = pfind(uap->pid);
	if (p == 0) {
		u.u_error = ESRCH;
		return;
	}
	u.u_r.r_val1 = p->p_pgrp;
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
		int	gidsetsize;
		int	*gidset;
	} *uap = (struct a *)u.u_ap;
	register int *gp;

	for (gp = &u.u_groups[NGROUPS]; gp > u.u_groups; gp--)
		if (gp[-1] >= 0)
			break;
	if (uap->gidsetsize < gp - u.u_groups) {
		u.u_error = EINVAL;
		return;
	}
	uap->gidsetsize = gp - u.u_groups;
	if (copyout((caddr_t)u.u_groups, (caddr_t)uap->gidset,
	    uap->gidsetsize * sizeof (u.u_groups[0]))) {
		u.u_error = EFAULT;
		return;
	}
	u.u_r.r_val1 = uap->gidsetsize;
}

setpgrp()
{
	register struct proc *p;
	register struct a {
		int	pid;
		int	pgrp;
	} *uap = (struct a *)u.u_ap;

	if (uap->pid == 0)
		uap->pid = u.u_procp->p_pid;
	p = pfind(uap->pid);
	if (p == 0) {
		u.u_error = ESRCH;
		return;
	}
/* need better control mechanisms for process groups */
	if (p->p_uid != u.u_uid && u.u_uid && !inferior(p)) {
		u.u_error = EPERM;
		return;
	}
	p->p_pgrp = uap->pgrp;
}

setuid()
{
	register uid;
	register struct a {
		int	uid;
	} *uap;

	uap = (struct a *)u.u_ap;
	uid = uap->uid;
	if (u.u_ruid == uid || u.u_uid == uid || suser()) {
#ifdef QUOTA
		if (u.u_quota->q_uid != uid) {
			qclean();
			qstart(getquota(uid, 0, 0));
		}
#endif
		u.u_uid = uid;
		u.u_procp->p_uid = uid;
		u.u_ruid = uid;
	}
}

setgid()
{
	register gid;
	register struct a {
		int	gid;
	} *uap;

	uap = (struct a *)u.u_ap;
	gid = uap->gid;
	if (u.u_rgid == gid || u.u_gid == gid || suser()) {
		leavegroup(u.u_gid); leavegroup(u.u_rgid);
		(void) entergroup(gid);
		u.u_gid = gid;
		u.u_rgid = gid;
	}
}

setgroups()
{
	register struct	a {
		int	gidsetsize;
		int	*gidset;
	} *uap = (struct a *)u.u_ap;
	register int *gp;

	if (!suser())
		return;
	if (uap->gidsetsize > sizeof (u.u_groups) / sizeof (u.u_groups[0])) {
		u.u_error = EINVAL;
		return;
	}
	if (copyin((caddr_t)uap->gidset, (caddr_t)u.u_groups,
	    uap->gidsetsize * sizeof (u.u_groups[0]))) {
		u.u_error = EFAULT;
		return;
	}
	for (gp = &u.u_groups[uap->gidsetsize]; gp < &u.u_groups[NGROUPS]; gp++)
		*gp = -1;
}

/*
 * Pid of zero implies current process.
 * Pgrp -1 is getpgrp system call returning
 * current process group.
 */
osetpgrp()
{
	register struct proc *p;
	register struct a {
		int	pid;
		int	pgrp;
	} *uap;

	uap = (struct a *)u.u_ap;
	if (uap->pid == 0)
		p = u.u_procp;
	else {
		p = pfind(uap->pid);
		if (p == 0) {
			u.u_error = ESRCH;
			return;
		}
	}
	if (uap->pgrp <= 0) {
		u.u_r.r_val1 = p->p_pgrp;
		return;
	}
	if (p->p_uid != u.u_uid && u.u_uid && !inferior(p)) {
		u.u_error = EPERM;
		return;
	}
	p->p_pgrp = uap->pgrp;
}
/* END DEFUNCT */

leavegroup(gid)
	int gid;
{
	register int *gp;

	for (gp = u.u_groups; gp < &u.u_groups[NGROUPS]; gp++)
		if (*gp == gid)
			goto found;
	return;
found:
	for (; gp < &u.u_groups[NGROUPS-1]; gp++)
		*gp = *(gp+1);
	*gp = -1;
}

entergroup(gid)
	int gid;
{
	register int *gp;

	for (gp = u.u_groups; gp < &u.u_groups[NGROUPS]; gp++)
		if (*gp == gid)
			return (0);
	for (gp = u.u_groups; gp < &u.u_groups[NGROUPS]; gp++)
		if (*gp < 0) {
			*gp = gid;
			return (0);
		}
	return (-1);
}
