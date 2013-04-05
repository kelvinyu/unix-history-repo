/*-
 * Copyright (c) 2013 Anish Gupta (akgupt3@gmail.com)
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
 * THIS SOFTWARE IS PROVIDED BY NETAPP, INC ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL NETAPP, INC OR CONTRIBUTORS BE LIABLE
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

#ifndef _VMCB_H_
#define	_VMCB_H_

/*
 * Secure Virtual Machine: AMD64 Programmer's Manual Vol2, Chapter 15
 * Layout of VMCB: AMD64 Programmer's Manual Vol2, Appendix B
 */

/* VMCB Control offset 0xC */
#define	VMCB_INTCPT_INTR		BIT(0)
#define	VMCB_INTCPT_NMI			BIT(1)
#define	VMCB_INTCPT_SMI			BIT(2)
#define	VMCB_INTCPT_INIT		BIT(3)
#define	VMCB_INTCPT_VINTR		BIT(4)
#define	VMCB_INTCPT_CR0_WRITE		BIT(5)
#define	VMCB_INTCPT_IDTR_READ		BIT(6)
#define	VMCB_INTCPT_GDTR_READ		BIT(7)
#define	VMCB_INTCPT_LDTR_READ		BIT(8)
#define	VMCB_INTCPT_TR_READ		BIT(9)
#define	VMCB_INTCPT_IDTR_WRITE		BIT(10)
#define	VMCB_INTCPT_GDTR_WRITE		BIT(11)
#define	VMCB_INTCPT_LDTR_WRITE		BIT(12)
#define	VMCB_INTCPT_TR_WRITE		BIT(13)
#define	VMCB_INTCPT_RDTSC		BIT(14)
#define	VMCB_INTCPT_RDPMC		BIT(15)
#define	VMCB_INTCPT_PUSHF		BIT(16)
#define	VMCB_INTCPT_POPF		BIT(17)
#define	VMCB_INTCPT_CPUID		BIT(18)
#define	VMCB_INTCPT_RSM			BIT(19)
#define	VMCB_INTCPT_IRET		BIT(20)
#define	VMCB_INTCPT_INTn		BIT(21)
#define	VMCB_INTCPT_INVD		BIT(22)
#define	VMCB_INTCPT_PAUSE		BIT(23)
#define	VMCB_INTCPT_HLT			BIT(24)
#define	VMCB_INTCPT_INVPG		BIT(25)
#define	VMCB_INTCPT_INVPGA		BIT(26)
#define	VMCB_INTCPT_IO			BIT(27)
#define	VMCB_INTCPT_MSR			BIT(28)
#define	VMCB_INTCPT_TASK_SWITCH		BIT(29)
#define	VMCB_INTCPT_FERR_FREEZE		BIT(30)
#define	VMCB_INTCPT_SHUTDOWN		BIT(31)

/* VMCB Control offset 0x10 */
#define	VMCB_INTCPT_VMRUN		BIT(0)
#define	VMCB_INTCPT_VMMCALL		BIT(1)
#define	VMCB_INTCPT_VMLOAD		BIT(2)
#define	VMCB_INTCPT_VMSAVE		BIT(3)
#define	VMCB_INTCPT_STGI		BIT(4)
#define	VMCB_INTCPT_CLGI		BIT(5)
#define	VMCB_INTCPT_SKINIT		BIT(6)
#define	VMCB_INTCPT_RDTSCP		BIT(7)
#define	VMCB_INTCPT_ICEBP		BIT(8)
#define	VMCB_INTCPT_WBINVD		BIT(9)
#define	VMCB_INTCPT_MONITOR		BIT(10)
#define	VMCB_INTCPT_MWAIT		BIT(11)
#define	VMCB_INTCPT_MWAIT_ARMED		BIT(12)
#define	VMCB_INTCPT_XSETBV		BIT(13)

/* VMCB TLB control */
#define	VMCB_TLB_FLUSH_NOTHING		0	/* Flush nothing */
#define	VMCB_TLB_FLUSH_EVERYTHING	1	/* Flush entire TLB */
#define	VMCB_TLB_FLUSH_GUEST		3	/* Flush all guest entries */
#define	VMCB_TLB_FLUSH_GUEST_NONGLOBAL	7	/* Flush guest non-PG entries */

/* VMCB state caching */
#define	VMCB_CACHE_NONE			0	/* No caching */
#define	VMCB_CACHE_I			BIT(0)	/* Cache vectors, TSC offset */
#define	VMCB_CACHE_IOPM			BIT(1)	/* I/O and MSR permission */
#define	VMCB_CACHE_ASID			BIT(2)	/* ASID */
#define	VMCB_CACHE_TPR			BIT(3)	/* V_TPR to V_INTR_VECTOR */
#define	VMCB_CACHE_NP			BIT(4)	/* Nested Paging */
#define	VMCB_CACHE_CR			BIT(5)	/* CR0, CR3, CR4 & EFER */
#define	VMCB_CACHE_DR			BIT(6)	/* Debug registers */
#define	VMCB_CACHE_DT			BIT(7)	/* GDT/IDT */
#define	VMCB_CACHE_SEG			BIT(8)	/* User segments, CPL */
#define	VMCB_CACHE_CR2			BIT(9)	/* page fault address */
#define	VMCB_CACHE_LBR			BIT(10)	/* Last branch */


/* VMCB control event injection */
#define	VMCB_EVENTINJ_EC_VALID		BIT(11)	/* Error Code valid */
#define	VMCB_EVENTINJ_VALID		BIT(31)	/* Event valid */

#define	VMCB_EVENTINJ_VECTOR_MASK	0xFF
#define	VMCB_EVENTINJ_INTR_TYPE_SHIFT	8
#define	VMCB_EVENTINJ_ERRCODE_SHIFT	32

/* Event types that can be injected */
#define	VMCB_EVENTINJ_TYPE_INTR		0
#define	VMCB_EVENTINJ_TYPE_NMI		2
#define	VMCB_EVENTINJ_TYPE_EXCEPTION	3
#define	VMCB_EVENTINJ_TYPE_INTn		4

/* VMCB exit code, APM vol2 Appendix C */
#define	VMCB_EXIT_MC			0x52
#define	VMCB_EXIT_INTR			0x60
#define	VMCB_EXIT_PUSHF			0x70
#define	VMCB_EXIT_POPF			0x71
#define	VMCB_EXIT_CPUID			0x72
#define	VMCB_EXIT_IRET			0x74
#define	VMCB_EXIT_PAUSE			0x77
#define	VMCB_EXIT_HLT			0x78
#define	VMCB_EXIT_IO			0x7B
#define	VMCB_EXIT_MSR			0x7C
#define	VMCB_EXIT_SHUTDOWN		0x7F
#define	VMCB_EXIT_VMSAVE		0x83
#define	VMCB_EXIT_NPF			0x400
#define	VMCB_EXIT_INVALID		-1

/*
 * Nested page fault.
 * Bit definitions to decode EXITINFO1.
 */
#define	VMCB_NPF_INFO1_P		BIT(0) /* Nested page present. */
#define	VMCB_NPF_INFO1_W		BIT(1) /* Access was write. */
#define	VMCB_NPF_INFO1_U		BIT(2) /* Access was user access. */
#define	VMCB_NPF_INFO1_RSV		BIT(3) /* Reserved bits present. */
#define	VMCB_NPF_INFO1_ID		BIT(4) /* Code read. */

#define	VMCB_NPF_INFO1_GPA		BIT(32) /* Guest physical address. */
#define	VMCB_NPF_INFO1_GPT		BIT(33) /* Guest page table. */

/* VMCB save state area segment format */
struct vmcb_segment {
	uint16_t	selector;
	uint16_t	attrib;
	uint32_t	limit;
	uint64_t	base;
} __attribute__ ((__packed__));
CTASSERT(sizeof(struct vmcb_segment) == 16);

/*
 * The VMCB is divided into two areas - the first one contains various
 * control bits including the intercept vector and the second one contains
 * the guest state.
 */

/* VMCB control area - padded up to 1024 bytes */
struct vmcb_ctrl {
	uint16_t cr_read;	/* Offset 0, CR0-15 read/write */
	uint16_t cr_write;
	uint16_t dr_read;	/* Offset 4, DR0-DR15 */
	uint16_t dr_write;
	uint32_t exception;	/* Offset 8, bit mask for exceptions. */
	uint32_t ctrl1;		/* Offset 0xC, intercept events1 */
	uint32_t ctrl2;		/* Offset 0x10, intercept event2 */
	uint8_t	 pad1[0x28];	/* Offsets 0x14-0x3B are reserved. */
	uint16_t pause_filthresh; /* Offset 0x3C, PAUSE filter threshold */
	uint16_t pause_filcnt;  /* Offset 0x3E, PAUSE filter count */
	uint64_t iopm_base_pa;	/* 0x40: IOPM_BASE_PA */
	uint64_t msrpm_base_pa; /* 0x48: MSRPM_BASE_PA */
	uint64_t tsc_offset;	/* 0x50: TSC_OFFSET */
	uint32_t asid;		/* 0x58: Guest ASID */
	uint8_t	 tlb_ctrl;	/* 0x5C: TLB_CONTROL */
	uint8_t  pad2[3];	/* 0x5D-0x5F: Reserved. */
	uint8_t	 v_tpr;		/* 0x60: V_TPR, guest CR8 */
	uint8_t	 v_irq:1;	/* Is virtual interrupt pending? */
	uint8_t	:7; 		/* Padding */
	uint8_t v_intr_prio:4;	/* 0x62: Priority for virtual interrupt. */
	uint8_t v_ign_tpr:1;
	uint8_t :3;
	uint8_t	v_intr_masking:1; /* Guest and host sharing of RFLAGS. */
	uint8_t	:7;
	uint8_t	v_intr_vector;	/* 0x65: Vector for virtual interrupt. */
	uint8_t pad3[3];	/* Bit64-40 Reserved. */
	uint64_t intr_shadow:1; /* 0x68: Interrupt shadow, section15.2.1 APM2 */
	uint64_t :63;
	uint64_t exitcode;	/* 0x70, Exitcode */
	uint64_t exitinfo1;	/* 0x78, EXITINFO1 */
	uint64_t exitinfo2;	/* 0x80, EXITINFO2 */
	uint64_t exitintinfo;	/* 0x88, Interrupt exit value. */
	uint64_t np_enable:1;   /* 0x90, Nested paging enable. */
	uint64_t :63;
	uint8_t  pad4[0x10];	/* 0x98-0xA7 reserved. */
	uint64_t eventinj;	/* 0xA8, Event injection. */
	uint64_t n_cr3;		/* B0, Nested page table. */
	uint64_t lbr_virt_en:1;	/* Enable LBR virtualization. */
	uint64_t :63;
	uint32_t vmcb_clean;	/* 0xC0: VMCB clean bits for caching */
	uint32_t :32;		/* 0xC4: Reserved */
	uint64_t nrip;		/* 0xC8: Guest next nRIP. */
	uint8_t	inst_decode_size; /* 0xD0: Instruction decode */
	uint8_t	inst_decode_bytes[15];
	uint8_t	padd6[0x320];
} __attribute__ ((__packed__));
CTASSERT(sizeof(struct vmcb_ctrl) == 1024);

struct vmcb_state {
	struct   vmcb_segment es;
	struct   vmcb_segment cs;
	struct   vmcb_segment ss;
	struct   vmcb_segment ds;
	struct   vmcb_segment fs;
	struct   vmcb_segment gs;
	struct   vmcb_segment gdt;
	struct   vmcb_segment ldt;
	struct   vmcb_segment idt;
	struct   vmcb_segment tr;
	uint8_t	 pad1[0x2b];		/* Reserved: 0xA0-0xCA */
	uint8_t	 cpl;
	uint8_t  pad2[4];
	uint64_t efer;
	uint8_t	 pad3[0x70];		/* Reserved: 0xd8-0x147 */
	uint64_t cr4;
	uint64_t cr3;			/* Guest CR3 */
	uint64_t cr0;
	uint64_t dr7;
	uint64_t dr6;
	uint64_t rflags;
	uint64_t rip;
	uint8_t	 pad4[0x58]; 		/* Reserved: 0x180-0x1D7 */
	uint64_t rsp;
	uint8_t	 pad5[0x18]; 		/* Reserved 0x1E0-0x1F7 */
	uint64_t rax;
	uint64_t star;
	uint64_t lstar;
	uint64_t cstar;
	uint64_t sfmask;
	uint64_t kernelgsbase;
	uint64_t sysenter_cs;
	uint64_t sysenter_esp;
	uint64_t sysenter_eip;
	uint64_t cr2;
	uint8_t	 pad6[0x20];
	uint64_t g_pat;
	uint64_t dbgctl;
	uint64_t br_from;
	uint64_t br_to;
	uint64_t lastexcpfrom;
	uint64_t lastexcpto;
	uint8_t	 pad7[0x968];		/* Reserved upto end of VMCB */
} __attribute__ ((__packed__));
CTASSERT(sizeof(struct vmcb_state) == 0xC00);

struct vmcb {
	struct vmcb_ctrl ctrl;
	struct vmcb_state state;
} __attribute__ ((__packed__));
CTASSERT(sizeof(struct vmcb) == PAGE_SIZE);
CTASSERT(offsetof(struct vmcb, state) == 0x400);

#endif /* _VMCB_H_ */
