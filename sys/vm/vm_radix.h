/*
 * Copyright (c) 2011 Jeffrey Roberson <jeff@freebsd.org>
 * Copyright (c) 2008 Mayur Shardul <mayur.shardul@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#ifndef _VM_RADIX_H_
#define _VM_RADIX_H_

#define	VM_RADIX_BLACK	0x1	/* Black node. (leaf only) */
#define	VM_RADIX_RED	0x2	/* Red node. (leaf only) */
#define	VM_RADIX_ANY	(VM_RADIX_RED | VM_RADIX_BLACK)
#define	VM_RADIX_STACK	8	/* Nodes to store on stack. */

/*
 * Radix tree root.  The height and pointer are set together to permit
 * coherent lookups while the root is modified.
 */
struct vm_radix {
	uintptr_t	rt_root;		/* root + height */
};

#ifdef _KERNEL

/*
 * Initialize the radix tree subsystem.
 */
void	vm_radix_init(void);

/*
 * Functions which only work with black nodes. (object lock)
 */
int 	vm_radix_insert(struct vm_radix *, vm_pindex_t, void *);

/*
 * Functions which work on specified colors. (object, vm_page_queue_free locks)
 */
void	*vm_radix_color(struct vm_radix *, vm_pindex_t, int);
void	*vm_radix_lookup(struct vm_radix *, vm_pindex_t, int);
int	vm_radix_lookupn(struct vm_radix *, vm_pindex_t, vm_pindex_t, int,
	    void **, int, vm_pindex_t *, u_int *);
void	*vm_radix_lookup_le(struct vm_radix *, vm_pindex_t, int);
void	vm_radix_reclaim_allnodes(struct vm_radix *);
void	vm_radix_remove(struct vm_radix *, vm_pindex_t, int);

/*
 * Look up any entry at a position greater or equal to index.
 */
static inline void *
vm_radix_lookup_ge(struct vm_radix *rtree, vm_pindex_t index, int color)
{
        void *val;
	u_int dummy;

        if (vm_radix_lookupn(rtree, index, 0, color, &val, 1, &index, &dummy))
                return (val);
        return (NULL);
}

static inline void *
vm_radix_last(struct vm_radix *rtree, int color)
{

	return vm_radix_lookup_le(rtree, 0, color);
}

static inline void *
vm_radix_first(struct vm_radix *rtree, int color)
{

	return vm_radix_lookup_ge(rtree, 0, color);
}

static inline void *
vm_radix_next(struct vm_radix *rtree, vm_pindex_t index, int color)
{

	if (index == -1)
		return (NULL);
	return vm_radix_lookup_ge(rtree, index + 1, color);
}

static inline void *
vm_radix_prev(struct vm_radix *rtree, vm_pindex_t index, int color)
{

	if (index == 0)
		return (NULL);
	return vm_radix_lookup_le(rtree, index - 1, color);
}

#endif /* _KERNEL */
#endif /* !_VM_RADIX_H_ */
