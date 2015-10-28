/*-
 * Copyright 2014 Svatopluk Kraus <onwahe@gmail.com>
 * Copyright 2014 Michal Meloun <meloun@miracle.cz>
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
 * $FreeBSD$
 */
#ifndef MACHINE_CPU_V6_H
#define MACHINE_CPU_V6_H

/* There are no user serviceable parts here, they may change without notice */
#ifndef _KERNEL
#error Only include this file in the kernel
#else

#include <machine/acle-compat.h>
#include "machine/atomic.h"
#include "machine/cpufunc.h"
#include "machine/cpuinfo.h"
#include "machine/sysreg.h"

#define CPU_ASID_KERNEL 0

vm_offset_t dcache_wb_pou_checked(vm_offset_t, vm_size_t);
vm_offset_t icache_inv_pou_checked(vm_offset_t, vm_size_t);

/*
 * Macros to generate CP15 (system control processor) read/write functions.
 */
#define _FX(s...) #s

#define _RF0(fname, aname...)						\
static __inline register_t						\
fname(void)								\
{									\
	register_t reg;							\
	__asm __volatile("mrc\t" _FX(aname): "=r" (reg));		\
	return(reg);							\
}

#define _R64F0(fname, aname)						\
static __inline uint64_t						\
fname(void)								\
{									\
	uint64_t reg;							\
	__asm __volatile("mrrc\t" _FX(aname): "=r" (reg));		\
	return(reg);							\
}

#define _WF0(fname, aname...)						\
static __inline void							\
fname(void)								\
{									\
	__asm __volatile("mcr\t" _FX(aname));				\
}

#define _WF1(fname, aname...)						\
static __inline void							\
fname(register_t reg)							\
{									\
	__asm __volatile("mcr\t" _FX(aname):: "r" (reg));		\
}

#define _W64F1(fname, aname...)						\
static __inline void							\
fname(uint64_t reg)							\
{									\
	__asm __volatile("mcrr\t" _FX(aname):: "r" (reg));		\
}

/*
 * Raw CP15  maintenance operations
 * !!! not for external use !!!
 */

/* TLB */

_WF0(_CP15_TLBIALL, CP15_TLBIALL)		/* Invalidate entire unified TLB */
#if __ARM_ARCH >= 7 && defined SMP
_WF0(_CP15_TLBIALLIS, CP15_TLBIALLIS)		/* Invalidate entire unified TLB IS */
#endif
_WF1(_CP15_TLBIASID, CP15_TLBIASID(%0))		/* Invalidate unified TLB by ASID */
#if __ARM_ARCH >= 7 && defined SMP
_WF1(_CP15_TLBIASIDIS, CP15_TLBIASIDIS(%0))	/* Invalidate unified TLB by ASID IS */
#endif
_WF1(_CP15_TLBIMVAA, CP15_TLBIMVAA(%0))		/* Invalidate unified TLB by MVA, all ASID */
#if __ARM_ARCH >= 7 && defined SMP
_WF1(_CP15_TLBIMVAAIS, CP15_TLBIMVAAIS(%0))	/* Invalidate unified TLB by MVA, all ASID IS */
#endif
_WF1(_CP15_TLBIMVA, CP15_TLBIMVA(%0))		/* Invalidate unified TLB by MVA */

_WF1(_CP15_TTB_SET, CP15_TTBR0(%0))

/* Cache and Branch predictor */

_WF0(_CP15_BPIALL, CP15_BPIALL)			/* Branch predictor invalidate all */
#if __ARM_ARCH >= 7 && defined SMP
_WF0(_CP15_BPIALLIS, CP15_BPIALLIS)		/* Branch predictor invalidate all IS */
#endif
_WF1(_CP15_BPIMVA, CP15_BPIMVA(%0))		/* Branch predictor invalidate by MVA */
_WF1(_CP15_DCCIMVAC, CP15_DCCIMVAC(%0))		/* Data cache clean and invalidate by MVA PoC */
_WF1(_CP15_DCCISW, CP15_DCCISW(%0))		/* Data cache clean and invalidate by set/way */
_WF1(_CP15_DCCMVAC, CP15_DCCMVAC(%0))		/* Data cache clean by MVA PoC */
#if __ARM_ARCH >= 7
_WF1(_CP15_DCCMVAU, CP15_DCCMVAU(%0))		/* Data cache clean by MVA PoU */
#endif
_WF1(_CP15_DCCSW, CP15_DCCSW(%0))		/* Data cache clean by set/way */
_WF1(_CP15_DCIMVAC, CP15_DCIMVAC(%0))		/* Data cache invalidate by MVA PoC */
_WF1(_CP15_DCISW, CP15_DCISW(%0))		/* Data cache invalidate by set/way */
_WF0(_CP15_ICIALLU, CP15_ICIALLU)		/* Instruction cache invalidate all PoU */
#if __ARM_ARCH >= 7 && defined SMP
_WF0(_CP15_ICIALLUIS, CP15_ICIALLUIS)		/* Instruction cache invalidate all PoU IS */
#endif
_WF1(_CP15_ICIMVAU, CP15_ICIMVAU(%0))		/* Instruction cache invalidate */

/*
 * Publicly accessible functions
 */

/* Various control registers */

_RF0(cp15_cpacr_get, CP15_CPACR(%0))
_WF1(cp15_cpacr_set, CP15_CPACR(%0))
_RF0(cp15_dfsr_get, CP15_DFSR(%0))
_RF0(cp15_ifsr_get, CP15_IFSR(%0))
_WF1(cp15_prrr_set, CP15_PRRR(%0))
_WF1(cp15_nmrr_set, CP15_NMRR(%0))
_RF0(cp15_ttbr_get, CP15_TTBR0(%0))
_RF0(cp15_dfar_get, CP15_DFAR(%0))
#if __ARM_ARCH >= 7
_RF0(cp15_ifar_get, CP15_IFAR(%0))
_RF0(cp15_l2ctlr_get, CP15_L2CTLR(%0))
#endif
/* ARMv6+ and XScale */
_RF0(cp15_actlr_get, CP15_ACTLR(%0))
_WF1(cp15_actlr_set, CP15_ACTLR(%0))
#if __ARM_ARCH >= 6
_WF1(cp15_ats1cpr_set, CP15_ATS1CPR(%0));
_RF0(cp15_par_get, CP15_PAR);
_RF0(cp15_sctlr_get, CP15_SCTLR(%0))
#endif

/*CPU id registers */
_RF0(cp15_midr_get, CP15_MIDR(%0))
_RF0(cp15_ctr_get, CP15_CTR(%0))
_RF0(cp15_tcmtr_get, CP15_TCMTR(%0))
_RF0(cp15_tlbtr_get, CP15_TLBTR(%0))
_RF0(cp15_mpidr_get, CP15_MPIDR(%0))
_RF0(cp15_revidr_get, CP15_REVIDR(%0))
_RF0(cp15_ccsidr_get, CP15_CCSIDR(%0))
_RF0(cp15_clidr_get, CP15_CLIDR(%0))
_RF0(cp15_aidr_get, CP15_AIDR(%0))
_WF1(cp15_csselr_set, CP15_CSSELR(%0))
_RF0(cp15_id_pfr0_get, CP15_ID_PFR0(%0))
_RF0(cp15_id_pfr1_get, CP15_ID_PFR1(%0))
_RF0(cp15_id_dfr0_get, CP15_ID_DFR0(%0))
_RF0(cp15_id_afr0_get, CP15_ID_AFR0(%0))
_RF0(cp15_id_mmfr0_get, CP15_ID_MMFR0(%0))
_RF0(cp15_id_mmfr1_get, CP15_ID_MMFR1(%0))
_RF0(cp15_id_mmfr2_get, CP15_ID_MMFR2(%0))
_RF0(cp15_id_mmfr3_get, CP15_ID_MMFR3(%0))
_RF0(cp15_id_isar0_get, CP15_ID_ISAR0(%0))
_RF0(cp15_id_isar1_get, CP15_ID_ISAR1(%0))
_RF0(cp15_id_isar2_get, CP15_ID_ISAR2(%0))
_RF0(cp15_id_isar3_get, CP15_ID_ISAR3(%0))
_RF0(cp15_id_isar4_get, CP15_ID_ISAR4(%0))
_RF0(cp15_id_isar5_get, CP15_ID_ISAR5(%0))
_RF0(cp15_cbar_get, CP15_CBAR(%0))

/* Performance Monitor registers */

#if __ARM_ARCH == 6 && defined(CPU_ARM1176)
_RF0(cp15_pmuserenr_get, CP15_PMUSERENR(%0))
_WF1(cp15_pmuserenr_set, CP15_PMUSERENR(%0))
_RF0(cp15_pmcr_get, CP15_PMCR(%0))
_WF1(cp15_pmcr_set, CP15_PMCR(%0))
_RF0(cp15_pmccntr_get, CP15_PMCCNTR(%0))
_WF1(cp15_pmccntr_set, CP15_PMCCNTR(%0))
#elif __ARM_ARCH > 6
_RF0(cp15_pmcr_get, CP15_PMCR(%0))
_WF1(cp15_pmcr_set, CP15_PMCR(%0))
_RF0(cp15_pmcnten_get, CP15_PMCNTENSET(%0))
_WF1(cp15_pmcnten_set, CP15_PMCNTENSET(%0))
_WF1(cp15_pmcnten_clr, CP15_PMCNTENCLR(%0))
_RF0(cp15_pmovsr_get, CP15_PMOVSR(%0))
_WF1(cp15_pmovsr_set, CP15_PMOVSR(%0))
_WF1(cp15_pmswinc_set, CP15_PMSWINC(%0))
_RF0(cp15_pmselr_get, CP15_PMSELR(%0))
_WF1(cp15_pmselr_set, CP15_PMSELR(%0))
_RF0(cp15_pmccntr_get, CP15_PMCCNTR(%0))
_WF1(cp15_pmccntr_set, CP15_PMCCNTR(%0))
_RF0(cp15_pmxevtyper_get, CP15_PMXEVTYPER(%0))
_WF1(cp15_pmxevtyper_set, CP15_PMXEVTYPER(%0))
_RF0(cp15_pmxevcntr_get, CP15_PMXEVCNTRR(%0))
_WF1(cp15_pmxevcntr_set, CP15_PMXEVCNTRR(%0))
_RF0(cp15_pmuserenr_get, CP15_PMUSERENR(%0))
_WF1(cp15_pmuserenr_set, CP15_PMUSERENR(%0))
_RF0(cp15_pminten_get, CP15_PMINTENSET(%0))
_WF1(cp15_pminten_set, CP15_PMINTENSET(%0))
_WF1(cp15_pminten_clr, CP15_PMINTENCLR(%0))
#endif

_RF0(cp15_tpidrurw_get, CP15_TPIDRURW(%0))
_WF1(cp15_tpidrurw_set, CP15_TPIDRURW(%0))
_RF0(cp15_tpidruro_get, CP15_TPIDRURO(%0))
_WF1(cp15_tpidruro_set, CP15_TPIDRURO(%0))
_RF0(cp15_tpidrpwr_get, CP15_TPIDRPRW(%0))
_WF1(cp15_tpidrpwr_set, CP15_TPIDRPRW(%0))

/* Generic Timer registers - only use when you know the hardware is available */
_RF0(cp15_cntfrq_get, CP15_CNTFRQ(%0))
_WF1(cp15_cntfrq_set, CP15_CNTFRQ(%0))
_RF0(cp15_cntkctl_get, CP15_CNTKCTL(%0))
_WF1(cp15_cntkctl_set, CP15_CNTKCTL(%0))
_RF0(cp15_cntp_tval_get, CP15_CNTP_TVAL(%0))
_WF1(cp15_cntp_tval_set, CP15_CNTP_TVAL(%0))
_RF0(cp15_cntp_ctl_get, CP15_CNTP_CTL(%0))
_WF1(cp15_cntp_ctl_set, CP15_CNTP_CTL(%0))
_RF0(cp15_cntv_tval_get, CP15_CNTV_TVAL(%0))
_WF1(cp15_cntv_tval_set, CP15_CNTV_TVAL(%0))
_RF0(cp15_cntv_ctl_get, CP15_CNTV_CTL(%0))
_WF1(cp15_cntv_ctl_set, CP15_CNTV_CTL(%0))
_RF0(cp15_cnthctl_get, CP15_CNTHCTL(%0))
_WF1(cp15_cnthctl_set, CP15_CNTHCTL(%0))
_RF0(cp15_cnthp_tval_get, CP15_CNTHP_TVAL(%0))
_WF1(cp15_cnthp_tval_set, CP15_CNTHP_TVAL(%0))
_RF0(cp15_cnthp_ctl_get, CP15_CNTHP_CTL(%0))
_WF1(cp15_cnthp_ctl_set, CP15_CNTHP_CTL(%0))

_R64F0(cp15_cntpct_get, CP15_CNTPCT(%Q0, %R0))
_R64F0(cp15_cntvct_get, CP15_CNTVCT(%Q0, %R0))
_R64F0(cp15_cntp_cval_get, CP15_CNTP_CVAL(%Q0, %R0))
_W64F1(cp15_cntp_cval_set, CP15_CNTP_CVAL(%Q0, %R0))
_R64F0(cp15_cntv_cval_get, CP15_CNTV_CVAL(%Q0, %R0))
_W64F1(cp15_cntv_cval_set, CP15_CNTV_CVAL(%Q0, %R0))
_R64F0(cp15_cntvoff_get, CP15_CNTVOFF(%Q0, %R0))
_W64F1(cp15_cntvoff_set, CP15_CNTVOFF(%Q0, %R0))
_R64F0(cp15_cnthp_cval_get, CP15_CNTHP_CVAL(%Q0, %R0))
_W64F1(cp15_cnthp_cval_set, CP15_CNTHP_CVAL(%Q0, %R0))

#undef	_FX
#undef	_RF0
#undef	_WF0
#undef	_WF1

#if __ARM_ARCH >= 6
/*
 * Cache and TLB maintenance operations for armv6+ code.  The #else block
 * provides armv4/v5 implementations for a few of these used in common code.
 */

/*
 * TLB maintenance operations.
 */

/* Local (i.e. not broadcasting ) operations.  */

/* Flush all TLB entries (even global). */
static __inline void
tlb_flush_all_local(void)
{

	dsb();
	_CP15_TLBIALL();
	dsb();
}

/* Flush all not global TLB entries. */
static __inline void
tlb_flush_all_ng_local(void)
{

	dsb();
	_CP15_TLBIASID(CPU_ASID_KERNEL);
	dsb();
}

/* Flush single TLB entry (even global). */
static __inline void
tlb_flush_local(vm_offset_t va)
{

	KASSERT((va & PAGE_MASK) == 0, ("%s: va %#x not aligned", __func__, va));

	dsb();
	_CP15_TLBIMVA(va | CPU_ASID_KERNEL);
	dsb();
}

/* Flush range of TLB entries (even global). */
static __inline void
tlb_flush_range_local(vm_offset_t va, vm_size_t size)
{
	vm_offset_t eva = va + size;

	KASSERT((va & PAGE_MASK) == 0, ("%s: va %#x not aligned", __func__, va));
	KASSERT((size & PAGE_MASK) == 0, ("%s: size %#x not aligned", __func__,
	    size));

	dsb();
	for (; va < eva; va += PAGE_SIZE)
		_CP15_TLBIMVA(va | CPU_ASID_KERNEL);
	dsb();
}

/* Broadcasting operations. */
#if __ARM_ARCH >= 7 && defined SMP

static __inline void
tlb_flush_all(void)
{

	dsb();
	_CP15_TLBIALLIS();
	dsb();
}

static __inline void
tlb_flush_all_ng(void)
{

	dsb();
	_CP15_TLBIASIDIS(CPU_ASID_KERNEL);
	dsb();
}

static __inline void
tlb_flush(vm_offset_t va)
{

	KASSERT((va & PAGE_MASK) == 0, ("%s: va %#x not aligned", __func__, va));

	dsb();
	_CP15_TLBIMVAAIS(va);
	dsb();
}

static __inline void
tlb_flush_range(vm_offset_t va,  vm_size_t size)
{
	vm_offset_t eva = va + size;

	KASSERT((va & PAGE_MASK) == 0, ("%s: va %#x not aligned", __func__, va));
	KASSERT((size & PAGE_MASK) == 0, ("%s: size %#x not aligned", __func__,
	    size));

	dsb();
	for (; va < eva; va += PAGE_SIZE)
		_CP15_TLBIMVAAIS(va);
	dsb();
}
#else /* SMP */

#define tlb_flush_all() 		tlb_flush_all_local()
#define tlb_flush_all_ng() 		tlb_flush_all_ng_local()
#define tlb_flush(va) 			tlb_flush_local(va)
#define tlb_flush_range(va, size) 	tlb_flush_range_local(va, size)

#endif /* SMP */

/*
 * Cache maintenance operations.
 */

/*  Sync I and D caches to PoU */
static __inline void
icache_sync(vm_offset_t va, vm_size_t size)
{
	vm_offset_t eva = va + size;

	dsb();
	va &= ~cpuinfo.dcache_line_mask;
	for ( ; va < eva; va += cpuinfo.dcache_line_size) {
#if __ARM_ARCH >= 7 && defined SMP
		_CP15_DCCMVAU(va);
#else
		_CP15_DCCMVAC(va);
#endif
	}
	dsb();
#if __ARM_ARCH >= 7 && defined SMP
	_CP15_ICIALLUIS();
#else
	_CP15_ICIALLU();
#endif
	dsb();
	isb();
}

/*  Invalidate I cache */
static __inline void
icache_inv_all(void)
{
#if __ARM_ARCH >= 7 && defined SMP
	_CP15_ICIALLUIS();
#else
	_CP15_ICIALLU();
#endif
	dsb();
	isb();
}

/* Invalidate branch predictor buffer */
static __inline void
bpb_inv_all(void)
{
#if __ARM_ARCH >= 7 && defined SMP
	_CP15_BPIALLIS();
#else
	_CP15_BPIALL();
#endif
	dsb();
	isb();
}

/* Write back D-cache to PoU */
static __inline void
dcache_wb_pou(vm_offset_t va, vm_size_t size)
{
	vm_offset_t eva = va + size;

	dsb();
	va &= ~cpuinfo.dcache_line_mask;
	for ( ; va < eva; va += cpuinfo.dcache_line_size) {
#if __ARM_ARCH >= 7 && defined SMP
		_CP15_DCCMVAU(va);
#else
		_CP15_DCCMVAC(va);
#endif
	}
	dsb();
}

/*
 * Invalidate D-cache to PoC
 *
 * Caches are invalidated from outermost to innermost as fresh cachelines
 * flow in this direction. In given range, if there was no dirty cacheline
 * in any cache before, no stale cacheline should remain in them after this
 * operation finishes.
 */
static __inline void
dcache_inv_poc(vm_offset_t va, vm_paddr_t pa, vm_size_t size)
{
	vm_offset_t eva = va + size;

	dsb();
	/* invalidate L2 first */
	cpu_l2cache_inv_range(pa, size);

	/* then L1 */
	va &= ~cpuinfo.dcache_line_mask;
	for ( ; va < eva; va += cpuinfo.dcache_line_size) {
		_CP15_DCIMVAC(va);
	}
	dsb();
}

/*
 * Discard D-cache lines to PoC, prior to overwrite by DMA engine.
 *
 * Normal invalidation does L2 then L1 to ensure that stale data from L2 doesn't
 * flow into L1 while invalidating.  This routine is intended to be used only
 * when invalidating a buffer before a DMA operation loads new data into memory.
 * The concern in this case is that dirty lines are not evicted to main memory,
 * overwriting the DMA data.  For that reason, the L1 is done first to ensure
 * that an evicted L1 line doesn't flow to L2 after the L2 has been cleaned.
 */
static __inline void
dcache_inv_poc_dma(vm_offset_t va, vm_paddr_t pa, vm_size_t size)
{
	vm_offset_t eva = va + size;

	/* invalidate L1 first */
	dsb();
	va &= ~cpuinfo.dcache_line_mask;
	for ( ; va < eva; va += cpuinfo.dcache_line_size) {
		_CP15_DCIMVAC(va);
	}
	dsb();

	/* then L2 */
	cpu_l2cache_inv_range(pa, size);
}

/*
 * Write back D-cache to PoC
 *
 * Caches are written back from innermost to outermost as dirty cachelines
 * flow in this direction. In given range, no dirty cacheline should remain
 * in any cache after this operation finishes.
 */
static __inline void
dcache_wb_poc(vm_offset_t va, vm_paddr_t pa, vm_size_t size)
{
	vm_offset_t eva = va + size;

	dsb();
	va &= ~cpuinfo.dcache_line_mask;
	for ( ; va < eva; va += cpuinfo.dcache_line_size) {
		_CP15_DCCMVAC(va);
	}
	dsb();

	cpu_l2cache_wb_range(pa, size);
}

/* Write back and invalidate D-cache to PoC */
static __inline void
dcache_wbinv_poc(vm_offset_t sva, vm_paddr_t pa, vm_size_t size)
{
	vm_offset_t va;
	vm_offset_t eva = sva + size;

	dsb();
	/* write back L1 first */
	va = sva & ~cpuinfo.dcache_line_mask;
	for ( ; va < eva; va += cpuinfo.dcache_line_size) {
		_CP15_DCCMVAC(va);
	}
	dsb();

	/* then write back and invalidate L2 */
	cpu_l2cache_wbinv_range(pa, size);

	/* then invalidate L1 */
	va = sva & ~cpuinfo.dcache_line_mask;
	for ( ; va < eva; va += cpuinfo.dcache_line_size) {
		_CP15_DCIMVAC(va);
	}
	dsb();
}

/* Set TTB0 register */
static __inline void
cp15_ttbr_set(uint32_t reg)
{
	dsb();
	_CP15_TTB_SET(reg);
	dsb();
	_CP15_BPIALL();
	dsb();
	isb();
	tlb_flush_all_ng_local();
}

#else /* ! __ARM_ARCH >= 6 */

/*
 * armv4/5 compatibility shims.
 *
 * These functions provide armv4 cache maintenance using the new armv6 names.
 * Included here are just the functions actually used now in common code; it may
 * be necessary to add things here over time.
 *
 * The callers of the dcache functions expect these routines to handle address
 * and size values which are not aligned to cacheline boundaries; the armv4 and
 * armv5 asm code handles that.
 */

static __inline void
dcache_inv_poc(vm_offset_t va, vm_paddr_t pa, vm_size_t size)
{

	cpu_dcache_inv_range(va, size);
	cpu_l2cache_inv_range(va, size);
}

static __inline void
dcache_inv_poc_dma(vm_offset_t va, vm_paddr_t pa, vm_size_t size)
{

	/* See armv6 code, above, for why we do L2 before L1 in this case. */
	cpu_l2cache_inv_range(va, size);
	cpu_dcache_inv_range(va, size);
}

static __inline void
dcache_wb_poc(vm_offset_t va, vm_paddr_t pa, vm_size_t size)
{

	cpu_dcache_wb_range(va, size);
	cpu_l2cache_wb_range(va, size);
}

#endif /* __ARM_ARCH >= 6 */

#endif /* _KERNEL */

#endif /* !MACHINE_CPU_V6_H */
