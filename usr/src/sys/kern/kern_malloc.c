/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)kern_malloc.c	7.19 (Berkeley) %G%
 */

#include "param.h"
#include "vm.h"
#include "cmap.h"
#include "time.h"
#include "proc.h"
#include "map.h"
#include "kernel.h"
#include "malloc.h"

#include "machine/pte.h"

struct kmembuckets bucket[MINBUCKET + 16];
struct kmemstats kmemstats[M_LAST];
struct kmemusage *kmemusage;
long wantkmemmap;
long malloc_reentered;
#define IN { if (malloc_reentered) panic("malloc reentered");\
			else malloc_reentered = 1;}
#define OUT (malloc_reentered = 0)

/*
 * Allocate a block of memory
 */
qaddr_t
malloc(size, type, flags)
	unsigned long size;
	int type, flags;
{
	register struct kmembuckets *kbp;
	register struct kmemusage *kup;
	long indx, npg, alloc, allocsize;
	int s;
	caddr_t va, cp;
#ifdef KMEMSTATS
	register struct kmemstats *ksp = &kmemstats[type];

	if (((unsigned long)type) > M_LAST)
		panic("malloc - bogus type");
#endif

	indx = BUCKETINDX(size);
	kbp = &bucket[indx];
	s = splimp();
	IN;
again:
#ifdef KMEMSTATS
	while (ksp->ks_memuse >= ksp->ks_limit) {
		if (flags & M_NOWAIT) {
			OUT;
			splx(s);
			return (0);
		}
		if (ksp->ks_limblocks < 65535)
			ksp->ks_limblocks++;
		OUT;
		sleep((caddr_t)ksp, PSWP+2);
		IN;
	}
#endif
	if (kbp->kb_next == NULL) {
		if (size > MAXALLOCSAVE)
			allocsize = roundup(size, CLBYTES);
		else
			allocsize = 1 << indx;
		npg = clrnd(btoc(allocsize));
		if ((flags & M_NOWAIT) && freemem < npg) {
			OUT;
			splx(s);
			return (0);
		}
		alloc = rmalloc(kmemmap, npg);
		if (alloc == 0) {
			if (flags & M_NOWAIT) {
				OUT;
				splx(s);
				return (0);
			}
#ifdef KMEMSTATS
			if (ksp->ks_mapblocks < 65535)
				ksp->ks_mapblocks++;
#endif
			wantkmemmap++;
			OUT;
			sleep((caddr_t)&wantkmemmap, PSWP+2);
			IN;
			goto again;
		}
		alloc -= CLSIZE;		/* convert to base 0 */
		(void) vmemall(&kmempt[alloc], (int)npg, &proc[0], CSYS);
		va = (caddr_t) kmemxtob(alloc);
		vmaccess(&kmempt[alloc], va, (int)npg);
#ifdef KMEMSTATS
		kbp->kb_total += kbp->kb_elmpercl;
#endif
		kup = btokup(va);
		kup->ku_indx = indx;
		if (allocsize > MAXALLOCSAVE) {
			if (npg > 65535)
				panic("malloc: allocation too large");
			kup->ku_pagecnt = npg;
#ifdef KMEMSTATS
			ksp->ks_memuse += allocsize;
#endif
			goto out;
		}
#ifdef KMEMSTATS
		kup->ku_freecnt = kbp->kb_elmpercl;
		kbp->kb_totalfree += kbp->kb_elmpercl;
#endif
		kbp->kb_next = va + (npg * NBPG) - allocsize;
		for (cp = kbp->kb_next; cp >= va; cp -= allocsize) {
			((caddr_t *)cp)[2] = (cp > va ? cp - allocsize : NULL);
			if (indx == 7) {
				long *lp = (long *)cp;
				lp[0] = lp[1] = lp[3] = lp[4] = -1;
			}
		}
	}
	va = kbp->kb_next;
	kbp->kb_next = ((caddr_t *)va)[2];
	if (indx == 7) {
		long *lp = (long *)va;
		if (lp[0] != -1 || lp[1] != -1 || lp[3] != -1 || lp[4] != -1)
			panic("malloc meddled");
	}
#ifdef KMEMSTATS
	kup = btokup(va);
	if (kup->ku_indx != indx)
		panic("malloc: wrong bucket");
	if (kup->ku_freecnt == 0)
		panic("malloc: lost data");
	kup->ku_freecnt--;
	kbp->kb_totalfree--;
	ksp->ks_memuse += 1 << indx;
out:
	kbp->kb_calls++;
	ksp->ks_inuse++;
	ksp->ks_calls++;
	if (ksp->ks_memuse > ksp->ks_maxused)
		ksp->ks_maxused = ksp->ks_memuse;
#else
out:
#endif
	OUT;
	splx(s);
	return ((qaddr_t)va);
}

#ifdef DIAGNOSTIC
long addrmask[] = { 0x00000000,
	0x00000001, 0x00000003, 0x00000007, 0x0000000f,
	0x0000001f, 0x0000003f, 0x0000007f, 0x000000ff,
	0x000001ff, 0x000003ff, 0x000007ff, 0x00000fff,
	0x00001fff, 0x00003fff, 0x00007fff, 0x0000ffff,
};
#endif /* DIAGNOSTIC */

/*
 * Free a block of memory allocated by malloc.
 */
void
free(addr, type)
	caddr_t addr;
	int type;
{
	register struct kmembuckets *kbp;
	register struct kmemusage *kup;
	long alloc, size;
	int s;
#ifdef KMEMSTATS
	register struct kmemstats *ksp = &kmemstats[type];
#endif

	kup = btokup(addr);
	size = 1 << kup->ku_indx;
#ifdef DIAGNOSTIC
	if (size > NBPG * CLSIZE)
		alloc = addrmask[BUCKETINDX(NBPG * CLSIZE)];
	else
		alloc = addrmask[kup->ku_indx];
	if (((u_long)addr & alloc) != 0) {
		printf("free: unaligned addr 0x%x, size %d, type %d, mask %d\n",
			addr, size, type, alloc);
		panic("free: unaligned addr");
	}
#endif /* DIAGNOSTIC */
	kbp = &bucket[kup->ku_indx];
	s = splimp();
	IN;
	if (size > MAXALLOCSAVE) {
		alloc = btokmemx(addr);
		(void) memfree(&kmempt[alloc], (int)kup->ku_pagecnt, 1);
		rmfree(kmemmap, (long)kup->ku_pagecnt, alloc + CLSIZE);
		OUT;
		if (wantkmemmap) {
			wakeup((caddr_t)&wantkmemmap);
			wantkmemmap = 0;
		}
#ifdef KMEMSTATS
		size = kup->ku_pagecnt << PGSHIFT;
		ksp->ks_memuse -= size;
		kup->ku_indx = 0;
		kup->ku_pagecnt = 0;
		if (ksp->ks_memuse + size >= ksp->ks_limit &&
		    ksp->ks_memuse < ksp->ks_limit)
			wakeup((caddr_t)ksp);
		ksp->ks_inuse--;
		kbp->kb_total -= 1;
#endif
		splx(s);
		return;
	}
	if (size == 128) {
		long *lp = (long *)addr;
		lp[0] = lp[1] = lp[3] = lp[4] = -1;
	}
#ifdef KMEMSTATS
	kup->ku_freecnt++;
	if (kup->ku_freecnt >= kbp->kb_elmpercl)
		if (kup->ku_freecnt > kbp->kb_elmpercl)
			panic("free: multiple frees");
		else if (kbp->kb_totalfree > kbp->kb_highwat)
			kbp->kb_couldfree++;
	kbp->kb_totalfree++;
	ksp->ks_memuse -= size;
	if (ksp->ks_memuse + size >= ksp->ks_limit &&
	    ksp->ks_memuse < ksp->ks_limit)
		wakeup((caddr_t)ksp);
	ksp->ks_inuse--;
#endif
	((caddr_t *)addr)[2] = kbp->kb_next;
	kbp->kb_next = addr;
	OUT;
	splx(s);
}

/*
 * Initialize the kernel memory allocator
 */
kmeminit()
{
	register long indx;
	int npg;

#if	((MAXALLOCSAVE & (MAXALLOCSAVE - 1)) != 0)
		ERROR!_kmeminit:_MAXALLOCSAVE_not_power_of_2
#endif
#if	(MAXALLOCSAVE > MINALLOCSIZE * 32768)
		ERROR!_kmeminit:_MAXALLOCSAVE_too_big
#endif
#if	(MAXALLOCSAVE < CLBYTES)
		ERROR!_kmeminit:_MAXALLOCSAVE_too_small
#endif
	npg = ekmempt - kmempt;
	rminit(kmemmap, (long)npg, (long)CLSIZE, "malloc map", npg);
#ifdef KMEMSTATS
	for (indx = 0; indx < MINBUCKET + 16; indx++) {
		if (1 << indx >= CLBYTES)
			bucket[indx].kb_elmpercl = 1;
		else
			bucket[indx].kb_elmpercl = CLBYTES / (1 << indx);
		bucket[indx].kb_highwat = 5 * bucket[indx].kb_elmpercl;
	}
	for (indx = 0; indx < M_LAST; indx++)
		kmemstats[indx].ks_limit = npg * NBPG * 6 / 10;
#endif
}
