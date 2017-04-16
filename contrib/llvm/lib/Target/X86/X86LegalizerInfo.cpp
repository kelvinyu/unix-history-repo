//===- X86LegalizerInfo.cpp --------------------------------------*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
/// \file
/// This file implements the targeting of the Machinelegalizer class for X86.
/// \todo This should be generated by TableGen.
//===----------------------------------------------------------------------===//

#include "X86LegalizerInfo.h"
#include "X86Subtarget.h"
#include "X86TargetMachine.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Type.h"
#include "llvm/Target/TargetOpcodes.h"

using namespace llvm;
using namespace TargetOpcode;

#ifndef LLVM_BUILD_GLOBAL_ISEL
#error "You shouldn't build this"
#endif

X86LegalizerInfo::X86LegalizerInfo(const X86Subtarget &STI,
                                   const X86TargetMachine &TM)
    : Subtarget(STI), TM(TM) {

  setLegalizerInfo32bit();
  setLegalizerInfo64bit();
  setLegalizerInfoSSE1();
  setLegalizerInfoSSE2();

  computeTables();
}

void X86LegalizerInfo::setLegalizerInfo32bit() {

  if (Subtarget.is64Bit())
    return;

  const LLT p0 = LLT::pointer(0, 32);
  const LLT s1 = LLT::scalar(1);
  const LLT s8 = LLT::scalar(8);
  const LLT s16 = LLT::scalar(16);
  const LLT s32 = LLT::scalar(32);
  const LLT s64 = LLT::scalar(64);

  for (unsigned BinOp : {G_ADD, G_SUB})
    for (auto Ty : {s8, s16, s32})
      setAction({BinOp, Ty}, Legal);

  for (unsigned MemOp : {G_LOAD, G_STORE}) {
    for (auto Ty : {s8, s16, s32, p0})
      setAction({MemOp, Ty}, Legal);

    // And everything's fine in addrspace 0.
    setAction({MemOp, 1, p0}, Legal);
  }

  // Pointer-handling
  setAction({G_FRAME_INDEX, p0}, Legal);

  // Constants
  for (auto Ty : {s8, s16, s32, p0})
    setAction({TargetOpcode::G_CONSTANT, Ty}, Legal);

  setAction({TargetOpcode::G_CONSTANT, s1}, WidenScalar);
  setAction({TargetOpcode::G_CONSTANT, s64}, NarrowScalar);
}

void X86LegalizerInfo::setLegalizerInfo64bit() {

  if (!Subtarget.is64Bit())
    return;

  const LLT p0 = LLT::pointer(0, TM.getPointerSize() * 8);
  const LLT s1 = LLT::scalar(1);
  const LLT s8 = LLT::scalar(8);
  const LLT s16 = LLT::scalar(16);
  const LLT s32 = LLT::scalar(32);
  const LLT s64 = LLT::scalar(64);

  for (unsigned BinOp : {G_ADD, G_SUB})
    for (auto Ty : {s8, s16, s32, s64})
      setAction({BinOp, Ty}, Legal);

  for (unsigned MemOp : {G_LOAD, G_STORE}) {
    for (auto Ty : {s8, s16, s32, s64, p0})
      setAction({MemOp, Ty}, Legal);

    // And everything's fine in addrspace 0.
    setAction({MemOp, 1, p0}, Legal);
  }

  // Pointer-handling
  setAction({G_FRAME_INDEX, p0}, Legal);

  // Constants
  for (auto Ty : {s8, s16, s32, s64, p0})
    setAction({TargetOpcode::G_CONSTANT, Ty}, Legal);

  setAction({TargetOpcode::G_CONSTANT, s1}, WidenScalar);
}

void X86LegalizerInfo::setLegalizerInfoSSE1() {
  if (!Subtarget.hasSSE1())
    return;

  const LLT s32 = LLT::scalar(32);
  const LLT v4s32 = LLT::vector(4, 32);
  const LLT v2s64 = LLT::vector(2, 64);

  for (unsigned BinOp : {G_FADD, G_FSUB, G_FMUL, G_FDIV})
    for (auto Ty : {s32, v4s32})
      setAction({BinOp, Ty}, Legal);

  for (unsigned MemOp : {G_LOAD, G_STORE})
    for (auto Ty : {v4s32, v2s64})
      setAction({MemOp, Ty}, Legal);
}

void X86LegalizerInfo::setLegalizerInfoSSE2() {
  if (!Subtarget.hasSSE2())
    return;

  const LLT s64 = LLT::scalar(64);
  const LLT v4s32 = LLT::vector(4, 32);
  const LLT v2s64 = LLT::vector(2, 64);

  for (unsigned BinOp : {G_FADD, G_FSUB, G_FMUL, G_FDIV})
    for (auto Ty : {s64, v2s64})
      setAction({BinOp, Ty}, Legal);

  for (unsigned BinOp : {G_ADD, G_SUB})
    for (auto Ty : {v4s32})
      setAction({BinOp, Ty}, Legal);
}
