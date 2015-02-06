//===-- RegisterContext_powerpc.h --------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef liblldb_RegisterContext_powerpc_H_
#define liblldb_RegisterContext_powerpc_H_

// GCC and DWARF Register numbers (eRegisterKindGCC & eRegisterKindDWARF)
enum
{
    gcc_dwarf_r0_powerpc = 0,
    gcc_dwarf_r1_powerpc,
    gcc_dwarf_r2_powerpc,
    gcc_dwarf_r3_powerpc,
    gcc_dwarf_r4_powerpc,
    gcc_dwarf_r5_powerpc,
    gcc_dwarf_r6_powerpc,
    gcc_dwarf_r7_powerpc,
    gcc_dwarf_r8_powerpc,
    gcc_dwarf_r9_powerpc,
    gcc_dwarf_r10_powerpc,
    gcc_dwarf_r11_powerpc,
    gcc_dwarf_r12_powerpc,
    gcc_dwarf_r13_powerpc,
    gcc_dwarf_r14_powerpc,
    gcc_dwarf_r15_powerpc,
    gcc_dwarf_r16_powerpc,
    gcc_dwarf_r17_powerpc,
    gcc_dwarf_r18_powerpc,
    gcc_dwarf_r19_powerpc,
    gcc_dwarf_r20_powerpc,
    gcc_dwarf_r21_powerpc,
    gcc_dwarf_r22_powerpc,
    gcc_dwarf_r23_powerpc,
    gcc_dwarf_r24_powerpc,
    gcc_dwarf_r25_powerpc,
    gcc_dwarf_r26_powerpc,
    gcc_dwarf_r27_powerpc,
    gcc_dwarf_r28_powerpc,
    gcc_dwarf_r29_powerpc,
    gcc_dwarf_r30_powerpc,
    gcc_dwarf_r31_powerpc,
    gcc_dwarf_f0_powerpc,
    gcc_dwarf_f1_powerpc,
    gcc_dwarf_f2_powerpc,
    gcc_dwarf_f3_powerpc,
    gcc_dwarf_f4_powerpc,
    gcc_dwarf_f5_powerpc,
    gcc_dwarf_f6_powerpc,
    gcc_dwarf_f7_powerpc,
    gcc_dwarf_f8_powerpc,
    gcc_dwarf_f9_powerpc,
    gcc_dwarf_f10_powerpc,
    gcc_dwarf_f11_powerpc,
    gcc_dwarf_f12_powerpc,
    gcc_dwarf_f13_powerpc,
    gcc_dwarf_f14_powerpc,
    gcc_dwarf_f15_powerpc,
    gcc_dwarf_f16_powerpc,
    gcc_dwarf_f17_powerpc,
    gcc_dwarf_f18_powerpc,
    gcc_dwarf_f19_powerpc,
    gcc_dwarf_f20_powerpc,
    gcc_dwarf_f21_powerpc,
    gcc_dwarf_f22_powerpc,
    gcc_dwarf_f23_powerpc,
    gcc_dwarf_f24_powerpc,
    gcc_dwarf_f25_powerpc,
    gcc_dwarf_f26_powerpc,
    gcc_dwarf_f27_powerpc,
    gcc_dwarf_f28_powerpc,
    gcc_dwarf_f29_powerpc,
    gcc_dwarf_f30_powerpc,
    gcc_dwarf_f31_powerpc,
    gcc_dwarf_cr_powerpc,
    gcc_dwarf_fpscr_powerpc,
    gcc_dwarf_xer_powerpc = 101,
    gcc_dwarf_lr_powerpc = 108,
    gcc_dwarf_ctr_powerpc,
    gcc_dwarf_pc_powerpc,
};

// GDB Register numbers (eRegisterKindGDB)
enum
{
    gdb_r0_powerpc = 0,
    gdb_r1_powerpc,
    gdb_r2_powerpc,
    gdb_r3_powerpc,
    gdb_r4_powerpc,
    gdb_r5_powerpc,
    gdb_r6_powerpc,
    gdb_r7_powerpc,
    gdb_r8_powerpc,
    gdb_r9_powerpc,
    gdb_r10_powerpc,
    gdb_r11_powerpc,
    gdb_r12_powerpc,
    gdb_r13_powerpc,
    gdb_r14_powerpc,
    gdb_r15_powerpc,
    gdb_r16_powerpc,
    gdb_r17_powerpc,
    gdb_r18_powerpc,
    gdb_r19_powerpc,
    gdb_r20_powerpc,
    gdb_r21_powerpc,
    gdb_r22_powerpc,
    gdb_r23_powerpc,
    gdb_r24_powerpc,
    gdb_r25_powerpc,
    gdb_r26_powerpc,
    gdb_r27_powerpc,
    gdb_r28_powerpc,
    gdb_r29_powerpc,
    gdb_r30_powerpc,
    gdb_r31_powerpc,
    gdb_f0_powerpc,
    gdb_f1_powerpc,
    gdb_f2_powerpc,
    gdb_f3_powerpc,
    gdb_f4_powerpc,
    gdb_f5_powerpc,
    gdb_f6_powerpc,
    gdb_f7_powerpc,
    gdb_f8_powerpc,
    gdb_f9_powerpc,
    gdb_f10_powerpc,
    gdb_f11_powerpc,
    gdb_f12_powerpc,
    gdb_f13_powerpc,
    gdb_f14_powerpc,
    gdb_f15_powerpc,
    gdb_f16_powerpc,
    gdb_f17_powerpc,
    gdb_f18_powerpc,
    gdb_f19_powerpc,
    gdb_f20_powerpc,
    gdb_f21_powerpc,
    gdb_f22_powerpc,
    gdb_f23_powerpc,
    gdb_f24_powerpc,
    gdb_f25_powerpc,
    gdb_f26_powerpc,
    gdb_f27_powerpc,
    gdb_f28_powerpc,
    gdb_f29_powerpc,
    gdb_f30_powerpc,
    gdb_f31_powerpc,
    gdb_cr_powerpc,
    gdb_fpscr_powerpc,
    gdb_xer_powerpc = 101,
    gdb_lr_powerpc = 108,
    gdb_ctr_powerpc,
    gdb_pc_powerpc,
};

#endif // liblldb_RegisterContext_powerpc_H_
