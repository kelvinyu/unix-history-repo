/*
 * Copyright (c) 1980, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)locore.s	1.2 (Berkeley) %G%
 */

#include "psl.h"
#include "pte.h"

#include "errno.h"
#include "cmap.h"

#include "../i386/trap.h"
/*#include "cpu.h"*/
/*#include "clock.h"*/


	.set	IDXSHIFT,10
	.set	SYSTEM,0xFE000000	# virtual address of system start
	/*note: gas copys sign bit (e.g. arithmetic >>), can't do SYSTEM>>22! */
	.set	SYSPDROFF,0x3F8		# Page dir

	.set	IOPHYSmem,0xa0000

/*
 * User structure is UPAGES at top of user space.
 */
	.set	_u,0xFDFFE000
	.globl	_u
	.set	UPDROFF,0x3F7
	.set	UPTEOFF,0x3FE

#define	ENTRY(name) \
	.globl _##name; _##name:
#define	ALTENTRY(name) \
	.globl _##name; _##name:
#ifdef badidea
/*
 * I/O Memory Map is 0xfffa000-0xffffffff (virtual == real)
 */
	.set	_IOmembase,0xFFFA0000
	.globl	_IOmembase
	.set	IOPDROFF,0x3FF
	.set	IOPTEOFF,0x3A0

#endif

/*
 * System page table
 * Mbmap and Usrptmap are enlarged by CLSIZE entries
 * as they are managed by resource maps starting with index 1 or CLSIZE.
 */ 
#define	SYSMAP(mname, vname, npte)		\
_##mname:	.globl	_##mname;		\
	.space	(npte)*4;			\
	.set	_##vname,ptes*4096+SYSTEM;	\
	.globl	_##vname;			\
	.set	ptes,ptes + npte
#define	ZSYSMAP(mname, vname, npte)		\
_##mname:	.globl	_##mname;		\
	.set	_##vname,ptes*4096+SYSTEM;	\
	.globl	_##vname;

	.data
	# assumed to start at data mod 4096
	.set	ptes,0
	SYSMAP(Sysmap,Sysbase,SYSPTSIZE)
	SYSMAP(Forkmap,forkutl,UPAGES)
	SYSMAP(Xswapmap,xswaputl,UPAGES)
	SYSMAP(Xswap2map,xswap2utl,UPAGES)
	SYSMAP(Swapmap,swaputl,UPAGES)
	SYSMAP(Pushmap,pushutl,UPAGES)
	SYSMAP(Vfmap,vfutl,UPAGES)
	SYSMAP(CMAP1,CADDR1,1)
	SYSMAP(CMAP2,CADDR2,1)
	SYSMAP(mmap,vmmap,1)
	SYSMAP(alignmap,alignutl,1)	/* XXX */
	SYSMAP(msgbufmap,msgbuf,MSGBUFPTECNT)
	.set mbxxx,(NMBCLUSTERS*MCLBYTES)
	.set mbyyy,(mbxxx>>PGSHIFT)
	.set mbpgs,(mbyyy+CLSIZE)
	SYSMAP(Mbmap,mbutl,mbpgs)
	/*
	 * XXX: NEED way to compute kmem size from maxusers,
	 * device complement
	 */
	SYSMAP(kmempt,kmembase,300*CLSIZE)
#ifdef	GPROF
	SYSMAP(profmap,profbase,600*CLSIZE)
#endif
	.set	atmemsz,0x100000-0xa0000
	.set	atpgs,(atmemsz>>PGSHIFT)
	SYSMAP(ATDevmem,atdevbase,atpgs)
#define USRIOSIZE 30
	SYSMAP(Usriomap,usrio,USRIOSIZE+CLSIZE) /* for PHYSIO */
	ZSYSMAP(ekmempt,kmemlimit,0)
	SYSMAP(Usrptmap,usrpt,USRPTSIZE+CLSIZE)

eSysmap:
	# .set	_Syssize,(eSysmap-_Sysmap)/4
	.set	_Syssize,ptes
	.globl	_Syssize

	/* align on next page boundary */
	# . = . + NBPG - 1 & -NBPG	/* align to page boundry-does not work*/
	# .space (PGSIZE - ((eSysmap-_Sysmap) % PGSIZE)) % PGSIZE
	.set sz,(4*ptes)%NBPG
	# .set rptes,(ptes)%1024
	# .set rptes,1024-rptes
	# .set ptes,ptes+rptes
	.set Npdes,3
	.space (NBPG - sz)

	.globl	_tMap
	# SYSMAP(Tmap,tmap,1024)
_tMap:
	.space	NBPG
	.globl	_PDR
_PDR:
	# SYSMAP(PDR,pdr,1024)
	.space	NBPG

/*
 * Initialization
 */
	.data
	.globl	_cpu
_cpu:	.long	0	# are we 386, 386sx, or 486
	.text
	.globl	start
start:
	movw	$0x1234,%ax
	movw	%ax,0x472	# warm boot
	jmp	1f
	.space	0x500	# skip over warm boot shit
1:
#ifdef notdef
	inb	$0x61,%al
	orb	$3,%al
	outb	%al,$0x61
	movl	$0xffff,%ecx
fooy:	loop	fooy
#endif
#ifdef notyet
	# XXX pass parameters on stack
/* count up memory */
	xorl	%eax,%eax		# start with base memory at 0x0
	movl	$(0xA0000/NBPG),%ecx	# look every 4K up to 640K
1:	movl	0(%eax),%ebx		# save location to check
	movl	$0xa55a5aa5,0(%eax)	# write test pattern
	cmpl	$0xa55a5aa5,0(%eax)	# does not check yet for rollover
	jne	2f
	movl	%ebx,0(%eax)		# restore memory
	addl	$NBPG,%eax
	loop	1b
2:	movl	%eax,_basemem-SYSTEM

	movl	$0x100000,%eax		# next, talley remaining memory
	movl	$((0xFA0000-0x100000)/NBPG),%ecx
1:	movl	0(%eax),%ebx		# save location to check
	movl	$0xa55a5aa5,0(%eax)	# write test pattern
	cmpl	$0xa55a5aa5,0(%eax)	# does not check yet for rollover
	jne	2f
	movl	%ebx,0(%eax)		# restore memory
	addl	$NBPG,%eax
	loop	1b
2:	movl	%eax,_abovemem-SYSTEM
#endif notyet

/* clear memory. is this redundant ? */
	movl	$_edata-SYSTEM,%edi
	movl	$_end-SYSTEM,%ecx
	addl	$NBPG-1,%ecx
	andl	$~(NBPG-1),%ecx
	movl	%ecx,%esi
	subl	%edi,%ecx
	addl	$(UPAGES*NBPG)+NBPG+NBPG+NBPG,%ecx
	#	txt+data+proc zero pt+u.
	# any other junk?
	addl	$NBPG-1,%ecx
	andl	$~(NBPG-1),%ecx
	# shrl	$2,%ecx	# convert to long word count
	xorl	%eax,%eax	# pattern
	cld
	rep
	stosb

/*
 * Map Kernel
 * N.B. don't bother with making kernel text RO, as 386
 * ignores R/W bit on kernel access!
 */
	# movl	$_Syssize,%ecx		# for this many pte s,
	movl	%esi,%ecx		# this much memory,
	shrl	$PGSHIFT,%ecx		# for this many pte s
	movl	$PG_V,%eax		#  having these bits set,
	movl	$_Sysmap-SYSTEM,%ebx	#   in the kernel page table,
					#    fill in kernel page table.
1:	movl	%eax,0(%ebx)
	addl	$NBPG,%eax			# increment physical address
	addl	$4,%ebx				# next pte
	loop	1b

/* temporary double map  virt == real */

	movl	$1024,%ecx		# for this many pte s,
	movl	$PG_V,%eax		#  having these bits set,
	movl	$_tMap-SYSTEM,%ebx	#   in the temporary page table,
					#    fill in kernel page table.
1:	movl	%eax,0(%ebx)
	addl	$NBPG,%eax			# increment physical address
	addl	$4,%ebx				# next pte
	loop	1b

#ifdef badidea
/* map I/O memory virt == real */

	movl	$(1024-IOPTEOFF),%ecx	# for this many pte s,
	movl	$(_IOmembase|PG_V),%eax	#  having these bits set, (perhaps URW?)
	movl	$_IOMap-SYSTEM,%ebx	#   in the temporary page table,
	addl	$(IOPTEOFF*4),%ebx
					#    fill in kernel page table.
1:	movl	%eax,0(%ebx)
	addl	$NBPG,%eax			# increment physical address
	addl	$4,%ebx				# next pte
	loop	1b
#endif

/* map I/O memory map */

	movl	$atpgs,%ecx		# for this many pte s,
	movl	$(IOPHYSmem|PG_V),%eax	#  having these bits set, (perhaps URW?)
	movl	$_ATDevmem-SYSTEM,%ebx	#   in the temporary page table,
					#    fill in kernel page table.
1:	movl	%eax,0(%ebx)
	addl	$NBPG,%eax			# increment physical address
	addl	$4,%ebx				# next pte
	loop	1b

/*# map proc 0's page table*/
	movl	$_Usrptmap-SYSTEM,%ebx	# get pt map address
	lea	(0*NBPG)(%esi),%eax	# physical address of pt in proc 0
	orl	$PG_V,%eax		#  having these bits set,
	movl	%eax,0(%ebx)

 /*# map proc 0's _u*/
	movl	$UPAGES,%ecx		# for this many pte s,
	lea	(2*NBPG)(%esi),%eax	# physical address of _u in proc 0
	orl	$PG_V|PG_URKW,%eax	#  having these bits set,
	lea	(0*NBPG)(%esi),%ebx	# physical address of stack pt in proc 0
	addl	$(UPTEOFF*4),%ebx
					#    fill in proc 0 stack page table.
1:	movl	%eax,0(%ebx)
	addl	$NBPG,%eax			# increment physical address
	addl	$4,%ebx				# next pte
	loop	1b

 /*# map proc 0's page directory*/
	lea	(1*NBPG)(%esi),%eax	# physical address of ptd in proc 0
	movl	%eax,%edi		# remember ptd physical address
	orl	$PG_V|PG_URKW,%eax	#  having these bits set,
	lea	(0*NBPG)(%esi),%ebx	# physical address of stack pt in proc 0
	addl	$(UPTEOFF*4),%ebx
	addl	$(UPAGES*4),%ebx
	movl	%eax,0(%ebx)

/*
 * Construct a page table directory
 * (of page directory elements - pde's)
 */
					/* kernel pde's */
	movl	$_Sysmap-SYSTEM,%eax	# physical address of kernel page table
	orl	$PG_V,%eax		# pde entry is valid
	movl	$Npdes,%ecx		# for this many pde s,
	# movl	$_PDR-SYSTEM,%ebx		# address of start of ptd
	movl	%edi,%ebx		# phys address of ptd in proc 0
	addl	$(SYSPDROFF*4), %ebx	# offset of pde for kernel
1:	movl	%eax,0(%ebx)
	addl	$NBPG,%eax			# increment physical address
	addl	$4,%ebx				# next pde
	loop	1b
					# install a pde for temporary double map
	movl	$_tMap-SYSTEM,%eax	# physical address of temp page table
	orl	$PG_V,%eax		# pde entry is valid
	# movl	$_PDR-SYSTEM,%ebx		# address of start of ptd
	movl	%edi,%ebx		# phys address of ptd in proc 0
	movl	%eax,0(%ebx)			# which is where temp maps!
#ifdef badidea
					# install a pde for IO memory
	movl	$_IOMap-SYSTEM,%eax	# physical address of temp page table
	orl	$PG_V,%eax		# pde entry is valid
	movl	$_PDR-SYSTEM,%ebx		# address of start of ptd
	# lea	(2*NBPG)(%esi),%ebx	# address of ptd in proc 0 pt
	addl	$(IOPDROFF*4), %ebx	# offset of pde for kernel
	movl	%eax,0(%ebx)			# which is where temp maps!
#endif
					# install a pde to map _u for proc 0
	lea	(0*NBPG)(%esi),%eax	# physical address of pt in proc 0
	orl	$PG_V,%eax		# pde entry is valid
	# movl	$_PDR-SYSTEM,%ebx		# address of start of ptd
	movl	%edi,%ebx		# phys address of ptd in proc 0
	addl	$(UPDROFF*4), %ebx	# offset of pde for kernel
	movl	%eax,0(%ebx)		# which is where _u maps!

	# movl	%eax,_PDR-SYSTEM+(1024-16-1)*4
	# movl	$UDOT,%eax
	# shrl	$PGSHIFT+IDXSHF,%eax
	# shll	$2,%eax
	# addl	$_PDR-SYSTEM,%eax
	# orl	$PG_V,%eax

	# movl	$_PDR-SYSTEM,%eax		# address of start of ptd
	movl	%edi,%eax		# phys address of ptd in proc 0
	movl	%eax,%cr3			# load ptd addr into mmu
	movl	$0x80000001,%eax		# and let s page!
	movl	%eax,%cr0			# NOW!

	pushl	$begin				# jump to high mem!
	ret
begin:
	# movl	$_u+UPAGES*NBPG-4,%eax
	# movl	$0xfdfffffc,%eax
	movl	$_Sysbase,%eax
	movl	%eax,%esp
	movl	%eax,%ebp
	movl	_Crtat,%eax
	subl	$IOPHYSmem,%eax
	addl	$_atdevbase,%eax
	movl	%eax,_Crtat
	call	_init386
/* initialize (slightly) the pcb */
	movl	$_u,%eax		# proc0 u-area
	movl	$_usrpt,%ecx
	movl	%ecx,PCB_P0BR(%eax)	# p0br: SVA of text/data user PT
	xorl	%ecx,%ecx
	movl	%ecx,PCB_P0LR(%eax)	# p0lr: 0 (doesn t really exist)
	movl	$_usrpt+NBPG,%ecx	# addr of end of PT
	subl	$P1PAGES*4,%ecx		# backwards size of P1 region
	movl	%ecx,PCB_P1BR(%eax)	# p1br: P1PAGES from end of PT
	movl	$P1PAGES-UPAGES,PCB_P1LR(%eax)	# p1lr: vax style
	movl	$CLSIZE,PCB_SZPT(%eax)	# page table size
	#clrw	a1@(PCB_FLAGS)		# clear flags
#ifdef FPUNOTYET
#endif
	pushl	%edi	# cr3
	movl	%esi,%eax
	# addl	$(UPAGES*NBPG)+NBPG+NBPG+NBPG,%esi
	addl	$(UPAGES*NBPG)+NBPG+NBPG+NBPG,%eax
	shrl	$PGSHIFT,%eax
	pushl	%eax	# firstaddr

	pushl	$20		# install sigtrampoline code
	pushl	$_u+PCB_SIGC
	pushl	$sigcode
	call	_bcopy
	addl	$12,%esp

	call 	_main

	.globl	__ucodesel,__udatasel
	movzwl	__ucodesel,%eax
	movzwl	__udatasel,%ecx
	# build outer stack frame
	pushl	%ecx	# user ss
	# pushl	$0xfdffcffc # user esp
	pushl	$0xfdffd000 # user esp
	pushl	%eax	# user cs
	pushl	$0	# user ip
	movw	%cx,%ds
	movw	%cx,%es
	lret	# goto user!
# pushal; pushl %edx; pushl $lr; call _pg; popl %eax ; popl %eax; popal ; .data ; lr: .asciz "lret %x" ; .text

	.globl	__exit
__exit:
	lidt	xaxa
	movl	$0,%esp		# segment violation
	ret
xaxa:	.long	0,0
	.set	exec,11
	.set	exit,1
	.globl	_icode
	.globl	_initflags
	.globl	_szicode
/* gas fucks up offset -- */
#define	LCALL(x,y)	.byte 0x9a ; .long y; .word x
/*
 * Icode is copied out to process 1 to exec /etc/init.
 * If the exec fails, process 1 exits.
 */
_icode:
	# pushl	$argv-_icode
	movl	$argv,%eax
	subl	$_icode,%eax
	pushl	%eax

	# pushl	$init-_icode
	movl	$init,%eax
	subl	$_icode,%eax
	pushl	%eax
	pushl	%eax	# dummy out rta

	movl	%esp,%ebp
	movl	$exec,%eax
	LCALL(0x7,0x0)
	pushl	%eax
	movl	$exit,%eax
	pushl	%eax	# dummy out rta
	LCALL(0x7,0x0)

# init:	.asciz	"/sbin/init"
init:	.asciz	"/etc/init"
	.align	2
_initflags:
	.long	0
argv:	.long	init-_icode
	.long	_initflags-_icode
	.long	0
_szicode:
	.long	_szicode-_icode
sigcode:
	movl	12(%esp),%eax	# unsure if call will dec stack 1st
	call	%eax
	xorl	%eax,%eax	# smaller movl $103,%eax
	movb	$103,%al	# sigreturn()
	LCALL(0x7,0)		# enter kernel with args on stack
	hlt			# never gets here

#ifdef	newway

#define	P(x)	(x-SYSTEM)
#define	PTE(b,o,p) \
		# build a pte pointing to physical p; leave it at loc b+o \	
		movl	p,%eax \
		andl	$0xfffff000,%eax \
		orl	$PG_V,%eax \
		movl	%eax,b(,o,4)

#define	PDE(d, v, p) \
		# build a pde at virtual addr v, pointing to physical pte p \
		movl	v,%edx			\
		andl	$0xffc00000,%edx	\
		shrl	$PGSHIFT+IDXSHFT,%edx	\
		PTE(d, %edx, p)

/* Initialize Sysmap */

	movl	$Syssize,%ecx	# this many pte s
	xorl	%ebx,%ebx	# starting at physical 0
	xorl	%edx,%edx	# starting at virtual XX000000
1:
	PTE(P(SysMap,%edx,%ebx)
	incl	%edx
	addl	$PGSIZE,%edx
	loop 1b

/* Initialize Proc 0 page table  map */
/* Initialize Udot map */

	movl	$UPAGES,%ecx	# this many pte s
	movl	$P(_end),%ebx
	addl	$PGSIZE-1,%ebx
	andl	$PGMASK,%ebx
	xorl	%ebx,%ebx	# starting at physical 0
	movl	$_u,%edx	# starting at virtual _u
1:
	PTE(P(SysMap,%edx,%ebx)
	incl	%edx
	addl	$PGSIZE,%edx
	loop 1b

/* Initialize page table directory */
	zero all entries
	PTD(P(_ptd), 0, P(SysMap))	# bottom double mapped to system
	PTD(P(_ptd), SYSTEM, P(SysMap))	# system location
	PTD(P(_ptd), _u, P(_end))	# udot&ptd

9:
/* clear memory from kernel bss and pages for proc 0 u. and page table */
#endif newway

	.globl ___udivsi3
___udivsi3:
	movl 4(%esp),%eax
	xorl %edx,%edx
	divl 8(%esp)
	ret

	.globl ___divsi3
___divsi3:
	movl 4(%esp),%eax
	xorl %edx,%edx
	cltd
	idivl 8(%esp)
	ret

	.globl	_inb
_inb:	movl	4(%esp),%edx
	subl	%eax,%eax	# clr eax
	nop
	inb	%dx,%al
	nop
	ret

	.globl	_outb
_outb:	movl	4(%esp),%edx
	movl	8(%esp),%eax
	nop
	outb	%al,%dx
	nop
	ret

	#
	# bzero (base,cnt)
	#

	.globl _bzero
	.globl _blkclr
_bzero:
_blkclr:
	pushl	%edi
	movl	8(%esp),%edi
	movl	12(%esp),%ecx
	movb	$0x00,%al
	cld
	rep
	stosb
	popl	%edi
	ret

	#
	# bcopy (src,dst,cnt)
	# NOTE: does not (yet) handle overlapped copies
	#

	.globl	_bcopy
	.globl	_copyout
	.globl	_copyin
_bcopy:
_copyout:
_copyin:
	pushl	%esi
	pushl	%edi
	movl	12(%esp),%esi
	movl	16(%esp),%edi
	movl	20(%esp),%ecx
	cld
	rep
	movsb
	popl	%edi
	popl	%esi
	movl	%ecx,%eax
	ret

	# insw(port,addr,cnt)
	.globl	_insw
_insw:
	pushl	%edi
	movw	8(%esp),%dx
	movl	12(%esp),%edi
	movl	16(%esp),%ecx
	cld
	nop
	.byte 0x66,0xf2,0x6d	# rep insw
	nop
	movl	%edi,%eax
	popl	%edi
	ret

	# outsw(port,addr,cnt)
	.globl	_outsw
_outsw:
	pushl	%esi
	movw	8(%esp),%dx
	movl	12(%esp),%esi
	movl	16(%esp),%ecx
	cld
	nop
	.byte 0x66,0xf2,0x6f	# rep outsw
	nop
	movl	%esi,%eax
	popl	%esi
	ret

	# lgdt(*gdt, ngdt)
	.globl	_lgdt
	# .globl	_gdt
xxx:	.word 31
	.long 0
_lgdt:
	movl	4(%esp),%eax
	movl	%eax,xxx+2
	movl	8(%esp),%eax
	movw	%ax,xxx
	lgdt	xxx
	jmp	1f
	nop
1:	movw	$0x10,%ax
	movw	%ax,%ds
	movw	%ax,%es
	movw	%ax,%ss
	movl	0(%esp),%eax
	pushl	%eax
	movl	$8,4(%esp)
	lret

	# lidt(*idt, nidt)
	.globl	_lidt
	# .globl	_idt
yyy:	.word	255
	.long	0 # _idt
_lidt:
	movl	4(%esp),%eax
	movl	%eax,yyy+2
	movl	8(%esp),%eax
	movw	%ax,yyy
	lidt	yyy
	ret

	# lldt(sel)
	.globl	_lldt
_lldt:
	movl	4(%esp),%eax
	lldt	%eax
	ret

	# ltr(sel)
	.globl	_ltr
_ltr:
	movl	4(%esp),%eax
	ltr	%eax
	ret

	# lcr3(cr3)
	.globl	_lcr3
	.globl	_load_cr3
_load_cr3:
_lcr3:
	movl	4(%esp),%eax
	movl	%eax,%cr3
	movl	%cr3,%eax
	ret

	# lcr0(cr0)
	.globl	_lcr0
_lcr0:
	movl	4(%esp),%eax
	movl	%eax,%cr0
	ret

	# rcr0()
	.globl	_rcr0
_rcr0:
	movl	%cr0,%eax
	ret

	# rcr2()
	.globl	_rcr2
_rcr2:
	movl	%cr2,%eax
	ret

	# rcr3()
	.globl	_rcr3
	.globl	__cr3
__cr3:
_rcr3:
	movl	%cr3,%eax
	ret

	# ssdtosd(*ssdp,*sdp)
	.globl	_ssdtosd
_ssdtosd:
	pushl	%ebx
	movl	8(%esp),%ecx
	movl	8(%ecx),%ebx
	shll	$16,%ebx
	movl	(%ecx),%edx
	roll	$16,%edx
	movb	%dh,%bl
	movb	%dl,%bh
	rorl	$8,%ebx
	movl	4(%ecx),%eax
	movw	%ax,%dx
	andl	$0xf0000,%eax
	orl	%eax,%ebx
	movl	12(%esp),%ecx
	movl	%edx,(%ecx)
	movl	%ebx,4(%ecx)
	popl	%ebx
	ret

/*
 * {fu,su},{byte,sword,word}
 */
ALTENTRY(fuiword)
ENTRY(fuword)
	movl	$fusufault,_nofault	# in case we page/protection violate
	movl	4(%esp),%edx
	movl	0(%edx),%eax
	xorl	%edx,%edx
	movl	%edx,_nofault
	ret
	
ENTRY(fusword)
	movl	$fusufault,_nofault	# in case we page/protection violate
	movl	4(%esp),%edx
	movzwl	0(%edx),%eax
	xorl	%edx,%edx
	movl	%edx,_nofault
	ret
	
ALTENTRY(fuibyte)
ENTRY(fubyte)
	movl	$fusufault,_nofault	# in case we page/protection violate
	movl	4(%esp),%edx
	movzbl	0(%edx),%eax
	xorl	%edx,%edx
	movl	%edx,_nofault
	ret
	
fusufault:
	xorl	%eax,%eax
	movl	%eax,_nofault
	decl	%eax
	ret

ALTENTRY(suiword)
ENTRY(suword)
	movl	$fusufault,_nofault	# in case we page/protection violate
	movl	4(%esp),%edx
	movl	8(%esp),%eax
	movl	%eax,0(%edx)
	xorl	%eax,%eax
	movl	%eax,_nofault
	ret
	
ENTRY(susword)
	movl	$fusufault,_nofault	# in case we page/protection violate
	movl	4(%esp),%edx
	movl	8(%esp),%eax
	movl	%eax,0(%edx)
	xorl	%eax,%eax
	movl	%eax,_nofault
	ret

ALTENTRY(suibyte)
ENTRY(subyte)
	movl	$fusufault,_nofault	# in case we page/protection violate
	movl	4(%esp),%edx
	movl	8(%esp),%eax
	movl	%eax,0(%edx)
	xorl	%eax,%eax
	movl	%eax,_nofault
	ret

	ALTENTRY(savectx)
	ENTRY(setjmp)
	movl	4(%esp),%eax
	movl	%ebx, 0(%eax)		# save ebx
	movl	%esp, 4(%eax)		# save esp
	movl	%ebp, 8(%eax)		# save ebp
	movl	%esi,12(%eax)		# save esi
	movl	%edi,16(%eax)		# save edi
	movl	(%esp),%edx		# get rta
	movl	%edx,20(%eax)		# save eip
	movl	$0,%edx			# return (0);
	movl	$0,%eax			# return (0);
	ret

	ENTRY(qsetjmp)
	movl	4(%esp),%eax
	movl	%esp, 4(%eax)		# save esp
	movl	%ebp, 8(%eax)		# save ebp
	movl	(%esp),%edx		# get rta
	movl	%edx,20(%eax)		# save eip
	xorl	%eax,%eax		# return (0);
	ret

	ENTRY(longjmp)
	movl	4(%esp),%eax
	movl	 0(%eax),%ebx		# restore ebx
	movl	 4(%eax),%esp		# restore esp
	movl	 8(%eax),%ebp		# restore ebp
	movl	12(%eax),%esi		# restore esi
	movl	16(%eax),%edi		# restore edi
	movl	20(%eax),%edx		# get rta
	movl	%edx,(%esp)		# put in return frame
	xorl	%eax,%eax		# return (1);
	incl	%eax
	ret
/*
 * The following primitives manipulate the run queues.
 * _whichqs tells which of the 32 queues _qs
 * have processes in them.  Setrq puts processes into queues, Remrq
 * removes them from queues.  The running process is on no queue,
 * other processes are on a queue related to p->p_pri, divided by 4
 * actually to shrink the 0-127 range of priorities into the 32 available
 * queues.
 */

	.globl	_whichqs,_qs,_cnt,_panic,_ctasksel
	.comm	_noproc,4
	.comm	_runrun,4
	.comm	_ctasksel,4

/*
 * Setrq(p)
 *
 * Call should be made at spl6(), and p->p_stat should be SRUN
 */
ENTRY(setrq)
	movl	4(%esp),%eax
	cmpl	$0,P_RLINK(%eax)	# should not be on q already
	je	set1
	pushl	$set2
	call	_panic
set1:
	movzbl	P_PRI(%eax),%edx
	shrl	$2,%edx
	btsl	%edx,_whichqs		# set q full bit
	shll	$3,%edx
	addl	$_qs,%edx		# locate q hdr
	movl	%edx,P_LINK(%eax)	# link process on tail of q
	movl	P_RLINK(%edx),%ecx
	movl	%ecx,P_RLINK(%eax)
	movl	%eax,P_RLINK(%edx)
	movl	%eax,P_LINK(%ecx)
	ret

set2:	.asciz	"setrq"

/*
 * Remrq(p)
 *
 * Call should be made at spl6().
 */
ENTRY(remrq)
	movl	4(%esp),%eax
	movzbl	P_PRI(%eax),%edx
	shrl	$2,%edx
	btrl	%edx,_whichqs		# clear full bit, panic if clear already
	jb	rem1
	pushl	$rem3
	call	_panic
rem1:
	pushl	%edx
	movl	P_LINK(%eax),%ecx	# unlink process
	movl	P_RLINK(%eax),%edx
	movl	%edx,P_RLINK(%ecx)
	movl	P_RLINK(%eax),%ecx
	movl	P_LINK(%eax),%edx
	movl	%edx,P_LINK(%ecx)
	popl	%edx
	movl	$_qs,%ecx
	shll	$3,%edx
	addl	%edx,%ecx
	cmpl	P_LINK(%ecx),%ecx	# q still has something?
	je	rem2
	shrl	$3,%edx			# yes, set bit as still full
	btsl	%edx,_whichqs
rem2:
	movl	$0,P_RLINK(%eax)	# zap reverse link to indicate off list
	ret

rem3:	.asciz	"remrq"
sw0:	.asciz	"swtch"
sw01:	.asciz	"swtch1"
sw02:	.asciz	"swtch2"

/*
 * When no processes are on the runq, Swtch branches to idle
 * to wait for something to come ready.
 */
	.globl	Idle
Idle:
idle:
	call	_spl0
	cmpl	$0,_whichqs
	jne	sw1
	hlt		# wait for interrupt
	jmp	idle

badsw:
	pushl	$sw0
	call	_panic
	/*NOTREACHED*/

/*
 * Swtch()
 */
ENTRY(swtch)
	movl	$1,%eax
	movl	%eax,_noproc
	incl	_cnt+V_SWTCH
sw1:
	bsfl	_whichqs,%eax	# find a full q
	jz	idle		# if none, idle
swfnd:
	cli
	btrl	%eax,_whichqs	# clear q full status
	jnb	sw1		# if it was clear, look for another
	pushl	%eax		# save which one we are using
	shll	$3,%eax
	addl	$_qs,%eax	# select q
	pushl	%eax

	cmpl	P_LINK(%eax),%eax	# linked to self? (e.g. not on list)
	je	badsw		# not possible
	movl	P_LINK(%eax),%ecx	# unlink from front of process q
	movl	P_LINK(%ecx),%edx
	movl	%edx,P_LINK(%eax)
	movl	P_RLINK(%ecx),%eax
	movl	%eax,P_RLINK(%edx)

	popl	%eax
	popl	%edx
	cmpl	P_LINK(%ecx),%eax	# q empty
	je	sw2
	btsl	%edx,_whichqs		# nope, indicate full
sw2:
	movl	$0,%eax
	movl	%eax,_noproc
	movl	%eax,_runrun
	cmpl	$0,P_WCHAN(%ecx)
	jne	badsw
	cmpb	$SRUN,P_STAT(%ecx)
	jne	badsw
	movl	%eax,P_RLINK(%ecx)
	# movl	P_ADDR(%ecx),%edx
	movl	P_CR3(%ecx),%edx
/*pushal; pushl %edx; pushl $lc; call _printf; popl %eax ; popl %eax; popal ; .data ; lc: .asciz "swtch %x" ; .text*/

/* switch to new process. first, save context as needed */
	movl	$_u,%ecx

	movl	(%esp),%eax		# Hardware registers
	movl	%eax, PCB_EIP(%ecx)
	movl	%ebx, PCB_EBX(%ecx)
	movl	%esp, PCB_ESP(%ecx)
	movl	%ebp, PCB_EBP(%ecx)
	movl	%esi, PCB_ESI(%ecx)
	movl	%edi, PCB_EDI(%ecx)

#ifdef FPUNOTYET
#endif

	movl	_CMAP2,%eax		# save temporary map PTE
	movl	%eax,PCB_CMAP2(%ecx)	# in our context


	movl	%edx,%cr3	# context switch

	movl	$_u,%edx

/* restore context */
	movl	PCB_EBX(%ecx), %ebx
	movl	PCB_ESP(%ecx), %esp
	movl	PCB_EBP(%ecx), %ebp
	movl	PCB_ESI(%ecx), %esi
	movl	PCB_EDI(%ecx), %edi
	movl	PCB_EIP(%ecx), %eax
	movl	%eax, (%esp)

#ifdef FPUNOTYET
#endif

	movl	PCB_CMAP2(%edx),%eax	# get temporary map
	movl	%eax,_CMAP2		# reload temporary map PTE
#ifdef FPUNOTYET
#endif
	cmpl	$0,PCB_SSWAP(%edx)	# do an alternate return?
	jne	res3			# yes, go reload regs
	# call	_spl0
	cli
	ret
res3:
	xorl	%eax,%eax		# inline restore context
	xchgl	PCB_SSWAP(%edx),%eax	# addr of saved context, clear it
	movl	 0(%eax),%ebx		# restore ebx
	movl	 4(%eax),%esp		# restore esp
	movl	 8(%eax),%ebp		# restore ebp
	movl	12(%eax),%esi		# restore esi
	movl	16(%eax),%edi		# restore edi
	movl	20(%eax),%edx		# get rta
	movl	%edx,(%esp)		# put in return frame
	xorl	%eax,%eax		# return (1);
	incl	%eax
	sti
	ret

/*
 * Resume(p_addr)
 * current just used to fillout u. tss so fork can fake a return to swtch
 * [ all thats really needed is esp and eip ]
 */
ENTRY(resume)
	movl	4(%esp),%ecx
	movl	(%esp),%eax	
	movl	%eax, PCB_EIP(%ecx)
	movl	%ebx, PCB_EBX(%ecx)
	movl	%esp, PCB_ESP(%ecx)
	movl	%ebp, PCB_EBP(%ecx)
	movl	%esi, PCB_ESI(%ecx)
	movl	%edi, PCB_EDI(%ecx)
#ifdef FPUNOTYET
#endif
	movl	$0,%eax
	ret

.data
	.globl	_cyloffset
_cyloffset:	.long	0
	.globl	_nofault
_nofault:	.long	0
.text
 # To be done:
	.globl _addupc
	.globl _astoff
	.globl _doadump
	.globl _inittodr
	.globl _ovbcopy
	.globl _physaddr
_addupc:
	.byte 0xcc
_astoff:
	ret
_doadump:
	.byte 0xcc
_ovbcopy:
	.byte 0xcc
_physaddr:
	.byte 0xcc
	.globl	_svfpsp,_rsfpsp
_svfpsp:
	popl %eax
	movl	%esp,svesp
	movl	%ebp,svebp
	pushl %eax
	ret

_rsfpsp:
	popl %eax
	movl	svesp,%esp
	movl	svebp,%ebp
	pushl %eax
	ret

svesp:	.long 0
svebp:	.long 0
