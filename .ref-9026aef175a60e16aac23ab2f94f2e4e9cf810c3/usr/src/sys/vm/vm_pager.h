/*
 * Copyright (c) 1990 University of Utah.
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * the Systems Programming Group of the University of Utah Computer
 * Science Department.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)vm_pager.h	8.5 (Berkeley) %G%
 */

/*
 * Pager routine interface definition.
 * For BSD we use a cleaner version of the internal pager interface.
 */

#ifndef	_VM_PAGER_
#define	_VM_PAGER_

TAILQ_HEAD(pagerlst, pager_struct);

struct	pager_struct {
	TAILQ_ENTRY(pager_struct) pg_list;	/* links for list management */
	caddr_t			  pg_handle;	/* ext. handle (vp, dev, fp) */
	int			  pg_type;	/* type of pager */
	int			  pg_flags;	/* flags */
	struct pagerops		  *pg_ops;	/* pager operations */
	void			  *pg_data;	/* private pager data */
};

/* pager types */
#define PG_DFLT		-1
#define	PG_SWAP		0
#define	PG_VNODE	1
#define PG_DEVICE	2

/* flags */
#define PG_CLUSTERGET	1
#define PG_CLUSTERPUT	2

struct	pagerops {
	void		(*pgo_init)		/* Initialize pager. */
			    __P((void));
	vm_pager_t	(*pgo_alloc)		/* Allocate pager. */
			    __P((caddr_t, vm_size_t, vm_prot_t, vm_offset_t));
	void		(*pgo_dealloc)		/* Disassociate. */
			    __P((vm_pager_t));
	int		(*pgo_getpages)		/* Get (read) page. */
			    __P((vm_pager_t, vm_page_t *, int, boolean_t));
	int		(*pgo_putpages)		/* Put (write) page. */
			    __P((vm_pager_t, vm_page_t *, int, boolean_t));
	boolean_t  	(*pgo_haspage)		/* Does pager have page? */
			    __P((vm_pager_t, vm_offset_t));
	void		(*pgo_cluster)		/* Return range of cluster. */
			    __P((vm_pager_t, vm_offset_t,
				 vm_offset_t *, vm_offset_t *));
};

/*
 * get/put return values
 * OK	 operation was successful
 * BAD	 specified data was out of the accepted range
 * FAIL	 specified data was in range, but doesn't exist
 * PEND	 operations was initiated but not completed
 * ERROR error while accessing data that is in range and exists
 * AGAIN temporary resource shortage prevented operation from happening
 */
#define	VM_PAGER_OK	0
#define	VM_PAGER_BAD	1
#define	VM_PAGER_FAIL	2
#define	VM_PAGER_PEND	3
#define	VM_PAGER_ERROR	4
#define VM_PAGER_AGAIN	5

#ifdef KERNEL
extern struct pagerops *dfltpagerops;

vm_pager_t	 vm_pager_allocate
		    __P((int, caddr_t, vm_size_t, vm_prot_t, vm_offset_t));
vm_page_t	 vm_pager_atop __P((vm_offset_t));
void		 vm_pager_cluster
		    __P((vm_pager_t, vm_offset_t,
			 vm_offset_t *, vm_offset_t *));
void		 vm_pager_clusternull
		    __P((vm_pager_t, vm_offset_t,
			 vm_offset_t *, vm_offset_t *));
void		 vm_pager_deallocate __P((vm_pager_t));
int		 vm_pager_get_pages
		    __P((vm_pager_t, vm_page_t *, int, boolean_t));
boolean_t	 vm_pager_has_page __P((vm_pager_t, vm_offset_t));
void		 vm_pager_init __P((void));
vm_pager_t	 vm_pager_lookup __P((struct pagerlst *, caddr_t));
vm_offset_t	 vm_pager_map_pages __P((vm_page_t *, int, boolean_t));
int		 vm_pager_put_pages
		    __P((vm_pager_t, vm_page_t *, int, boolean_t));
void		 vm_pager_sync __P((void));
void		 vm_pager_unmap_pages __P((vm_offset_t, int));

#define vm_pager_cancluster(p, b)	((p)->pg_flags & (b))

/*
 * XXX compat with old interface
 */
int		 vm_pager_get __P((vm_pager_t, vm_page_t, boolean_t));
int		 vm_pager_put __P((vm_pager_t, vm_page_t, boolean_t));
#endif

#endif	/* _VM_PAGER_ */
