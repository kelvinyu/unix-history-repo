/*-
 * Copyright (c) 1999 John D. Polstra.
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
 *	$Id: rtld_machdep.h,v 1.1.2.2 1999/05/02 09:36:07 brian Exp $
 */

#ifndef RTLD_MACHDEP_H
#define RTLD_MACHDEP_H	1

/* Return the address of the .dynamic section in the dynamic linker. */
#define rtld_dynamic(obj) \
    ((const Elf_Dyn *)((obj)->relocbase + (Elf_Addr)&_DYNAMIC))

/* Fixup the jump slot at "where" to transfer control to "target". */
#define reloc_jmpslot(where, target)			\
    do {						\
	dbg("reloc_jmpslot: *%p = %p", (void *)(where),	\
	  (void *)(target));				\
	(*(Elf_Addr *)(where) = (Elf_Addr)(target));	\
    } while (0)

#endif
