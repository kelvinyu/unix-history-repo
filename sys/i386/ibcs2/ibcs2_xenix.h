/*
 * System call prototypes.
 *
 * DO NOT EDIT-- this file is automatically generated.
 * $FreeBSD$
 * created from FreeBSD: src/sys/i386/ibcs2/syscalls.xenix,v 1.6 1999/08/28 00:44:02 peter Exp 
 */

#ifndef _IBCS2_XENIX_H_
#define	_IBCS2_XENIX_H_

#include <sys/signal.h>

#include <sys/acl.h>

struct proc;

struct thread;

#define	PAD_(t)	(sizeof(register_t) <= sizeof(t) ? \
		0 : sizeof(register_t) - sizeof(t))

struct	xenix_rdchk_args {
	int	fd;	char fd_[PAD_(int)];
};
struct	xenix_chsize_args {
	int	fd;	char fd_[PAD_(int)];
	long	size;	char size_[PAD_(long)];
};
struct	xenix_ftime_args {
	struct timeb *	tp;	char tp_[PAD_(struct timeb *)];
};
struct	xenix_nap_args {
	int	millisec;	char millisec_[PAD_(int)];
};
struct	xenix_scoinfo_args {
	register_t dummy;
};
struct	xenix_eaccess_args {
	char *	path;	char path_[PAD_(char *)];
	int	flags;	char flags_[PAD_(int)];
};
struct	ibcs2_sigaction_args {
	int	sig;	char sig_[PAD_(int)];
	struct ibcs2_sigaction *	act;	char act_[PAD_(struct ibcs2_sigaction *)];
	struct ibcs2_sigaction *	oact;	char oact_[PAD_(struct ibcs2_sigaction *)];
};
struct	ibcs2_sigprocmask_args {
	int	how;	char how_[PAD_(int)];
	ibcs2_sigset_t *	set;	char set_[PAD_(ibcs2_sigset_t *)];
	ibcs2_sigset_t *	oset;	char oset_[PAD_(ibcs2_sigset_t *)];
};
struct	ibcs2_sigpending_args {
	ibcs2_sigset_t *	mask;	char mask_[PAD_(ibcs2_sigset_t *)];
};
struct	ibcs2_sigsuspend_args {
	ibcs2_sigset_t *	mask;	char mask_[PAD_(ibcs2_sigset_t *)];
};
struct	ibcs2_getgroups_args {
	int	gidsetsize;	char gidsetsize_[PAD_(int)];
	ibcs2_gid_t *	gidset;	char gidset_[PAD_(ibcs2_gid_t *)];
};
struct	ibcs2_setgroups_args {
	int	gidsetsize;	char gidsetsize_[PAD_(int)];
	ibcs2_gid_t *	gidset;	char gidset_[PAD_(ibcs2_gid_t *)];
};
struct	ibcs2_sysconf_args {
	int	name;	char name_[PAD_(int)];
};
struct	ibcs2_pathconf_args {
	char *	path;	char path_[PAD_(char *)];
	int	name;	char name_[PAD_(int)];
};
struct	ibcs2_fpathconf_args {
	int	fd;	char fd_[PAD_(int)];
	int	name;	char name_[PAD_(int)];
};
struct	ibcs2_rename_args {
	char *	from;	char from_[PAD_(char *)];
	char *	to;	char to_[PAD_(char *)];
};
struct	xenix_utsname_args {
	long	addr;	char addr_[PAD_(long)];
};
int	xenix_rdchk(struct thread *, struct xenix_rdchk_args *);
int	xenix_chsize(struct thread *, struct xenix_chsize_args *);
int	xenix_ftime(struct thread *, struct xenix_ftime_args *);
int	xenix_nap(struct thread *, struct xenix_nap_args *);
int	xenix_scoinfo(struct thread *, struct xenix_scoinfo_args *);
int	xenix_eaccess(struct thread *, struct xenix_eaccess_args *);
int	ibcs2_sigaction(struct thread *, struct ibcs2_sigaction_args *);
int	ibcs2_sigprocmask(struct thread *, struct ibcs2_sigprocmask_args *);
int	ibcs2_sigpending(struct thread *, struct ibcs2_sigpending_args *);
int	ibcs2_sigsuspend(struct thread *, struct ibcs2_sigsuspend_args *);
int	ibcs2_getgroups(struct thread *, struct ibcs2_getgroups_args *);
int	ibcs2_setgroups(struct thread *, struct ibcs2_setgroups_args *);
int	ibcs2_sysconf(struct thread *, struct ibcs2_sysconf_args *);
int	ibcs2_pathconf(struct thread *, struct ibcs2_pathconf_args *);
int	ibcs2_fpathconf(struct thread *, struct ibcs2_fpathconf_args *);
int	ibcs2_rename(struct thread *, struct ibcs2_rename_args *);
int	xenix_utsname(struct thread *, struct xenix_utsname_args *);

#ifdef COMPAT_43


#endif /* COMPAT_43 */

#undef PAD_

#endif /* !_IBCS2_XENIX_H_ */
