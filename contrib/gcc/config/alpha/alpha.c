/* Subroutines used for code generation on the DEC Alpha.
   Copyright (C) 1992, 1993, 1994, 1995, 1996, 1997, 1998, 1999,
   2000, 2001 Free Software Foundation, Inc. 
   Contributed by Richard Kenner (kenner@vlsi1.ultra.nyu.edu)

This file is part of GNU CC.

GNU CC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU CC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU CC; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */


#include "config.h"
#include "system.h"
#include "rtl.h"
#include "tree.h"
#include "regs.h"
#include "hard-reg-set.h"
#include "real.h"
#include "insn-config.h"
#include "conditions.h"
#include "output.h"
#include "insn-attr.h"
#include "flags.h"
#include "recog.h"
#include "expr.h"
#include "optabs.h"
#include "reload.h"
#include "obstack.h"
#include "except.h"
#include "function.h"
#include "toplev.h"
#include "ggc.h"
#include "integrate.h"
#include "tm_p.h"
#include "target.h"
#include "target-def.h"

/* External data.  */
extern int rtx_equal_function_value_matters;

/* Specify which cpu to schedule for.  */

enum processor_type alpha_cpu;
static const char * const alpha_cpu_name[] = 
{
  "ev4", "ev5", "ev6"
};

/* Specify how accurate floating-point traps need to be.  */

enum alpha_trap_precision alpha_tp;

/* Specify the floating-point rounding mode.  */

enum alpha_fp_rounding_mode alpha_fprm;

/* Specify which things cause traps.  */

enum alpha_fp_trap_mode alpha_fptm;

/* Strings decoded into the above options.  */

const char *alpha_cpu_string;	/* -mcpu= */
const char *alpha_tune_string;	/* -mtune= */
const char *alpha_tp_string;	/* -mtrap-precision=[p|s|i] */
const char *alpha_fprm_string;	/* -mfp-rounding-mode=[n|m|c|d] */
const char *alpha_fptm_string;	/* -mfp-trap-mode=[n|u|su|sui] */
const char *alpha_mlat_string;	/* -mmemory-latency= */

/* Save information from a "cmpxx" operation until the branch or scc is
   emitted.  */

struct alpha_compare alpha_compare;

/* Non-zero if inside of a function, because the Alpha asm can't
   handle .files inside of functions.  */

static int inside_function = FALSE;

/* The number of cycles of latency we should assume on memory reads.  */

int alpha_memory_latency = 3;

/* Whether the function needs the GP.  */

static int alpha_function_needs_gp;

/* The alias set for prologue/epilogue register save/restore.  */

static int alpha_sr_alias_set;

/* The assembler name of the current function.  */

static const char *alpha_fnname;

/* The next explicit relocation sequence number.  */
int alpha_next_sequence_number = 1;

/* The literal and gpdisp sequence numbers for this insn, as printed
   by %# and %* respectively.  */
int alpha_this_literal_sequence_number;
int alpha_this_gpdisp_sequence_number;

/* Declarations of static functions.  */
static bool decl_in_text_section
  PARAMS ((tree));
static int some_small_symbolic_mem_operand_1
  PARAMS ((rtx *, void *));
static int split_small_symbolic_mem_operand_1
  PARAMS ((rtx *, void *));
static bool local_symbol_p
  PARAMS ((rtx));
static void alpha_set_memflags_1
  PARAMS ((rtx, int, int, int));
static rtx alpha_emit_set_const_1
  PARAMS ((rtx, enum machine_mode, HOST_WIDE_INT, int));
static void alpha_expand_unaligned_load_words
  PARAMS ((rtx *out_regs, rtx smem, HOST_WIDE_INT words, HOST_WIDE_INT ofs));
static void alpha_expand_unaligned_store_words
  PARAMS ((rtx *out_regs, rtx smem, HOST_WIDE_INT words, HOST_WIDE_INT ofs));
static void alpha_sa_mask
  PARAMS ((unsigned long *imaskP, unsigned long *fmaskP));
static int find_lo_sum
  PARAMS ((rtx *, void *));
static int alpha_does_function_need_gp
  PARAMS ((void));
static int alpha_ra_ever_killed
  PARAMS ((void));
static const char *get_trap_mode_suffix
  PARAMS ((void));
static const char *get_round_mode_suffix
  PARAMS ((void));
static rtx set_frame_related_p
  PARAMS ((void));
static const char *alpha_lookup_xfloating_lib_func
  PARAMS ((enum rtx_code));
static int alpha_compute_xfloating_mode_arg
  PARAMS ((enum rtx_code, enum alpha_fp_rounding_mode));
static void alpha_emit_xfloating_libcall
  PARAMS ((const char *, rtx, rtx[], int, rtx));
static rtx alpha_emit_xfloating_compare
  PARAMS ((enum rtx_code, rtx, rtx));
static void alpha_output_function_end_prologue
  PARAMS ((FILE *));
static int alpha_adjust_cost
  PARAMS ((rtx, rtx, rtx, int));
static int alpha_issue_rate
  PARAMS ((void));
static int alpha_variable_issue
  PARAMS ((FILE *, int, rtx, int));

#if TARGET_ABI_UNICOSMK
static void alpha_init_machine_status
  PARAMS ((struct function *p));
static void alpha_mark_machine_status
  PARAMS ((struct function *p));
static void alpha_free_machine_status
  PARAMS ((struct function *p));
#endif

static void unicosmk_output_deferred_case_vectors PARAMS ((FILE *));
static void unicosmk_gen_dsib PARAMS ((unsigned long *imaskP));
static void unicosmk_output_ssib PARAMS ((FILE *, const char *));
static int unicosmk_need_dex PARAMS ((rtx));

/* Get the number of args of a function in one of two ways.  */
#if TARGET_ABI_OPEN_VMS || TARGET_ABI_UNICOSMK
#define NUM_ARGS current_function_args_info.num_args
#else
#define NUM_ARGS current_function_args_info
#endif

#define REG_PV 27
#define REG_RA 26

/* Initialize the GCC target structure.  */
#if TARGET_ABI_OPEN_VMS
const struct attribute_spec vms_attribute_table[];
static unsigned int vms_section_type_flags PARAMS ((tree, const char *, int));
static void vms_asm_named_section PARAMS ((const char *, unsigned int));
static void vms_asm_out_constructor PARAMS ((rtx, int));
static void vms_asm_out_destructor PARAMS ((rtx, int));
# undef TARGET_ATTRIBUTE_TABLE
# define TARGET_ATTRIBUTE_TABLE vms_attribute_table
# undef TARGET_SECTION_TYPE_FLAGS
# define TARGET_SECTION_TYPE_FLAGS vms_section_type_flags
#endif

#if TARGET_ABI_UNICOSMK
static void unicosmk_asm_named_section PARAMS ((const char *, unsigned int));
static void unicosmk_insert_attributes PARAMS ((tree, tree *));
static unsigned int unicosmk_section_type_flags PARAMS ((tree, const char *, 
							 int));
# undef TARGET_INSERT_ATTRIBUTES
# define TARGET_INSERT_ATTRIBUTES unicosmk_insert_attributes
# undef TARGET_SECTION_TYPE_FLAGS
# define TARGET_SECTION_TYPE_FLAGS unicosmk_section_type_flags
#endif

#undef TARGET_ASM_ALIGNED_HI_OP
#define TARGET_ASM_ALIGNED_HI_OP "\t.word\t"
#undef TARGET_ASM_ALIGNED_DI_OP
#define TARGET_ASM_ALIGNED_DI_OP "\t.quad\t"

/* Default unaligned ops are provided for ELF systems.  To get unaligned
   data for non-ELF systems, we have to turn off auto alignment.  */
#ifndef OBJECT_FORMAT_ELF
#undef TARGET_ASM_UNALIGNED_HI_OP
#define TARGET_ASM_UNALIGNED_HI_OP "\t.align 0\n\t.word\t"
#undef TARGET_ASM_UNALIGNED_SI_OP
#define TARGET_ASM_UNALIGNED_SI_OP "\t.align 0\n\t.long\t"
#undef TARGET_ASM_UNALIGNED_DI_OP
#define TARGET_ASM_UNALIGNED_DI_OP "\t.align 0\n\t.quad\t"
#endif

#undef TARGET_ASM_FUNCTION_END_PROLOGUE
#define TARGET_ASM_FUNCTION_END_PROLOGUE alpha_output_function_end_prologue

#undef TARGET_SCHED_ADJUST_COST
#define TARGET_SCHED_ADJUST_COST alpha_adjust_cost
#undef TARGET_SCHED_ISSUE_RATE
#define TARGET_SCHED_ISSUE_RATE alpha_issue_rate
#undef TARGET_SCHED_VARIABLE_ISSUE
#define TARGET_SCHED_VARIABLE_ISSUE alpha_variable_issue

struct gcc_target targetm = TARGET_INITIALIZER;

/* Parse target option strings.  */

void
override_options ()
{
  int i;
  static const struct cpu_table {
    const char *const name;
    const enum processor_type processor;
    const int flags;
  } cpu_table[] = {
#define EV5_MASK (MASK_CPU_EV5)
#define EV6_MASK (MASK_CPU_EV6|MASK_BWX|MASK_MAX|MASK_FIX)
    { "ev4",	PROCESSOR_EV4, 0 },
    { "ev45",	PROCESSOR_EV4, 0 },
    { "21064",	PROCESSOR_EV4, 0 },
    { "ev5",	PROCESSOR_EV5, EV5_MASK },
    { "21164",	PROCESSOR_EV5, EV5_MASK },
    { "ev56",	PROCESSOR_EV5, EV5_MASK|MASK_BWX },
    { "21164a",	PROCESSOR_EV5, EV5_MASK|MASK_BWX },
    { "pca56",	PROCESSOR_EV5, EV5_MASK|MASK_BWX|MASK_MAX },
    { "21164PC",PROCESSOR_EV5, EV5_MASK|MASK_BWX|MASK_MAX },
    { "21164pc",PROCESSOR_EV5, EV5_MASK|MASK_BWX|MASK_MAX },
    { "ev6",	PROCESSOR_EV6, EV6_MASK },
    { "21264",	PROCESSOR_EV6, EV6_MASK },
    { "ev67",	PROCESSOR_EV6, EV6_MASK|MASK_CIX },
    { "21264a",	PROCESSOR_EV6, EV6_MASK|MASK_CIX },
    { 0, 0, 0 }
  };
                  
  /* Unicos/Mk doesn't have shared libraries.  */
  if (TARGET_ABI_UNICOSMK && flag_pic)
    {
      warning ("-f%s ignored for Unicos/Mk (not supported)",
	       (flag_pic > 1) ? "PIC" : "pic");
      flag_pic = 0;
    }

  /* On Unicos/Mk, the native compiler consistenly generates /d suffices for 
     floating-point instructions.  Make that the default for this target.  */
  if (TARGET_ABI_UNICOSMK)
    alpha_fprm = ALPHA_FPRM_DYN;
  else
    alpha_fprm = ALPHA_FPRM_NORM;

  alpha_tp = ALPHA_TP_PROG;
  alpha_fptm = ALPHA_FPTM_N;

  /* We cannot use su and sui qualifiers for conversion instructions on 
     Unicos/Mk.  I'm not sure if this is due to assembler or hardware
     limitations.  Right now, we issue a warning if -mieee is specified
     and then ignore it; eventually, we should either get it right or
     disable the option altogether.  */

  if (TARGET_IEEE)
    {
      if (TARGET_ABI_UNICOSMK)
	warning ("-mieee not supported on Unicos/Mk");
      else
	{
	  alpha_tp = ALPHA_TP_INSN;
	  alpha_fptm = ALPHA_FPTM_SU;
	}
    }

  if (TARGET_IEEE_WITH_INEXACT)
    {
      if (TARGET_ABI_UNICOSMK)
	warning ("-mieee-with-inexact not supported on Unicos/Mk");
      else
	{
	  alpha_tp = ALPHA_TP_INSN;
	  alpha_fptm = ALPHA_FPTM_SUI;
	}
    }

  if (alpha_tp_string)
    {
      if (! strcmp (alpha_tp_string, "p"))
	alpha_tp = ALPHA_TP_PROG;
      else if (! strcmp (alpha_tp_string, "f"))
	alpha_tp = ALPHA_TP_FUNC;
      else if (! strcmp (alpha_tp_string, "i"))
	alpha_tp = ALPHA_TP_INSN;
      else
	error ("bad value `%s' for -mtrap-precision switch", alpha_tp_string);
    }

  if (alpha_fprm_string)
    {
      if (! strcmp (alpha_fprm_string, "n"))
	alpha_fprm = ALPHA_FPRM_NORM;
      else if (! strcmp (alpha_fprm_string, "m"))
	alpha_fprm = ALPHA_FPRM_MINF;
      else if (! strcmp (alpha_fprm_string, "c"))
	alpha_fprm = ALPHA_FPRM_CHOP;
      else if (! strcmp (alpha_fprm_string,"d"))
	alpha_fprm = ALPHA_FPRM_DYN;
      else
	error ("bad value `%s' for -mfp-rounding-mode switch",
	       alpha_fprm_string);
    }

  if (alpha_fptm_string)
    {
      if (strcmp (alpha_fptm_string, "n") == 0)
	alpha_fptm = ALPHA_FPTM_N;
      else if (strcmp (alpha_fptm_string, "u") == 0)
	alpha_fptm = ALPHA_FPTM_U;
      else if (strcmp (alpha_fptm_string, "su") == 0)
	alpha_fptm = ALPHA_FPTM_SU;
      else if (strcmp (alpha_fptm_string, "sui") == 0)
	alpha_fptm = ALPHA_FPTM_SUI;
      else
	error ("bad value `%s' for -mfp-trap-mode switch", alpha_fptm_string);
    }

  alpha_cpu
    = TARGET_CPU_DEFAULT & MASK_CPU_EV6 ? PROCESSOR_EV6
      : (TARGET_CPU_DEFAULT & MASK_CPU_EV5 ? PROCESSOR_EV5 : PROCESSOR_EV4);

  if (alpha_cpu_string)
    {
      for (i = 0; cpu_table [i].name; i++)
	if (! strcmp (alpha_cpu_string, cpu_table [i].name))
	  {
	    alpha_cpu = cpu_table [i].processor;
	    target_flags &= ~ (MASK_BWX | MASK_MAX | MASK_FIX | MASK_CIX
			       | MASK_CPU_EV5 | MASK_CPU_EV6);
	    target_flags |= cpu_table [i].flags;
	    break;
	  }
      if (! cpu_table [i].name)
	error ("bad value `%s' for -mcpu switch", alpha_cpu_string);
    }

  if (alpha_tune_string)
    {
      for (i = 0; cpu_table [i].name; i++)
	if (! strcmp (alpha_tune_string, cpu_table [i].name))
	  {
	    alpha_cpu = cpu_table [i].processor;
	    break;
	  }
      if (! cpu_table [i].name)
	error ("bad value `%s' for -mcpu switch", alpha_tune_string);
    }

  /* Do some sanity checks on the above options.  */

  if (TARGET_ABI_UNICOSMK && alpha_fptm != ALPHA_FPTM_N)
    {
      warning ("trap mode not supported on Unicos/Mk");
      alpha_fptm = ALPHA_FPTM_N;
    }

  if ((alpha_fptm == ALPHA_FPTM_SU || alpha_fptm == ALPHA_FPTM_SUI)
      && alpha_tp != ALPHA_TP_INSN && ! TARGET_CPU_EV6)
    {
      warning ("fp software completion requires -mtrap-precision=i");
      alpha_tp = ALPHA_TP_INSN;
    }

  if (TARGET_CPU_EV6)
    {
      /* Except for EV6 pass 1 (not released), we always have precise
	 arithmetic traps.  Which means we can do software completion
	 without minding trap shadows.  */
      alpha_tp = ALPHA_TP_PROG;
    }

  if (TARGET_FLOAT_VAX)
    {
      if (alpha_fprm == ALPHA_FPRM_MINF || alpha_fprm == ALPHA_FPRM_DYN)
	{
	  warning ("rounding mode not supported for VAX floats");
	  alpha_fprm = ALPHA_FPRM_NORM;
	}
      if (alpha_fptm == ALPHA_FPTM_SUI)
	{
	  warning ("trap mode not supported for VAX floats");
	  alpha_fptm = ALPHA_FPTM_SU;
	}
    }

  {
    char *end;
    int lat;

    if (!alpha_mlat_string)
      alpha_mlat_string = "L1";

    if (ISDIGIT ((unsigned char)alpha_mlat_string[0])
	&& (lat = strtol (alpha_mlat_string, &end, 10), *end == '\0'))
      ;
    else if ((alpha_mlat_string[0] == 'L' || alpha_mlat_string[0] == 'l')
	     && ISDIGIT ((unsigned char)alpha_mlat_string[1])
	     && alpha_mlat_string[2] == '\0')
      {
	static int const cache_latency[][4] = 
	{
	  { 3, 30, -1 },	/* ev4 -- Bcache is a guess */
	  { 2, 12, 38 },	/* ev5 -- Bcache from PC164 LMbench numbers */
	  { 3, 12, 30 },	/* ev6 -- Bcache from DS20 LMbench.  */
	};

	lat = alpha_mlat_string[1] - '0';
	if (lat <= 0 || lat > 3 || cache_latency[alpha_cpu][lat-1] == -1)
	  {
	    warning ("L%d cache latency unknown for %s",
		     lat, alpha_cpu_name[alpha_cpu]);
	    lat = 3;
	  }
	else
	  lat = cache_latency[alpha_cpu][lat-1];
      }
    else if (! strcmp (alpha_mlat_string, "main"))
      {
	/* Most current memories have about 370ns latency.  This is
	   a reasonable guess for a fast cpu.  */
	lat = 150;
      }
    else
      {
	warning ("bad value `%s' for -mmemory-latency", alpha_mlat_string);
	lat = 3;
      }

    alpha_memory_latency = lat;
  }

  /* Default the definition of "small data" to 8 bytes.  */
  if (!g_switch_set)
    g_switch_value = 8;

  /* Infer TARGET_SMALL_DATA from -fpic/-fPIC.  */
  if (flag_pic == 1)
    target_flags |= MASK_SMALL_DATA;
  else if (flag_pic == 2)
    target_flags &= ~MASK_SMALL_DATA;

  /* Align labels and loops for optimal branching.  */
  /* ??? Kludge these by not doing anything if we don't optimize and also if
     we are writing ECOFF symbols to work around a bug in DEC's assembler.  */
  if (optimize > 0 && write_symbols != SDB_DEBUG)
    {
      if (align_loops <= 0)
	align_loops = 16;
      if (align_jumps <= 0)
	align_jumps = 16;
    }
  if (align_functions <= 0)
    align_functions = 16;

  /* Acquire a unique set number for our register saves and restores.  */
  alpha_sr_alias_set = new_alias_set ();

  /* Register variables and functions with the garbage collector.  */

#if TARGET_ABI_UNICOSMK
  /* Set up function hooks.  */
  init_machine_status = alpha_init_machine_status;
  mark_machine_status = alpha_mark_machine_status;
  free_machine_status = alpha_free_machine_status;
#endif
}

/* Returns 1 if VALUE is a mask that contains full bytes of zero or ones.  */

int
zap_mask (value)
     HOST_WIDE_INT value;
{
  int i;

  for (i = 0; i < HOST_BITS_PER_WIDE_INT / HOST_BITS_PER_CHAR;
       i++, value >>= 8)
    if ((value & 0xff) != 0 && (value & 0xff) != 0xff)
      return 0;

  return 1;
}

/* Returns 1 if OP is either the constant zero or a register.  If a
   register, it must be in the proper mode unless MODE is VOIDmode.  */

int
reg_or_0_operand (op, mode)
      register rtx op;
      enum machine_mode mode;
{
  return op == const0_rtx || register_operand (op, mode);
}

/* Return 1 if OP is a constant in the range of 0-63 (for a shift) or
   any register.  */

int
reg_or_6bit_operand (op, mode)
     register rtx op;
     enum machine_mode mode;
{
  return ((GET_CODE (op) == CONST_INT
	   && (unsigned HOST_WIDE_INT) INTVAL (op) < 64)
	  || register_operand (op, mode));
}


/* Return 1 if OP is an 8-bit constant or any register.  */

int
reg_or_8bit_operand (op, mode)
     register rtx op;
     enum machine_mode mode;
{
  return ((GET_CODE (op) == CONST_INT
	   && (unsigned HOST_WIDE_INT) INTVAL (op) < 0x100)
	  || register_operand (op, mode));
}

/* Return 1 if OP is an 8-bit constant.  */

int
cint8_operand (op, mode)
     register rtx op;
     enum machine_mode mode ATTRIBUTE_UNUSED;
{
  return ((GET_CODE (op) == CONST_INT
	   && (unsigned HOST_WIDE_INT) INTVAL (op) < 0x100));
}

/* Return 1 if the operand is a valid second operand to an add insn.  */

int
add_operand (op, mode)
     register rtx op;
     enum machine_mode mode;
{
  if (GET_CODE (op) == CONST_INT)
    /* Constraints I, J, O and P are covered by K.  */
    return (CONST_OK_FOR_LETTER_P (INTVAL (op), 'K')
	    || CONST_OK_FOR_LETTER_P (INTVAL (op), 'L'));

  return register_operand (op, mode);
}

/* Return 1 if the operand is a valid second operand to a sign-extending
   add insn.  */

int
sext_add_operand (op, mode)
     register rtx op;
     enum machine_mode mode;
{
  if (GET_CODE (op) == CONST_INT)
    return (CONST_OK_FOR_LETTER_P (INTVAL (op), 'I')
	    || CONST_OK_FOR_LETTER_P (INTVAL (op), 'O'));

  return reg_not_elim_operand (op, mode);
}

/* Return 1 if OP is the constant 4 or 8.  */

int
const48_operand (op, mode)
     register rtx op;
     enum machine_mode mode ATTRIBUTE_UNUSED;
{
  return (GET_CODE (op) == CONST_INT
	  && (INTVAL (op) == 4 || INTVAL (op) == 8));
}

/* Return 1 if OP is a valid first operand to an AND insn.  */

int
and_operand (op, mode)
     register rtx op;
     enum machine_mode mode;
{
  if (GET_CODE (op) == CONST_DOUBLE && GET_MODE (op) == VOIDmode)
    return (zap_mask (CONST_DOUBLE_LOW (op))
	    && zap_mask (CONST_DOUBLE_HIGH (op)));

  if (GET_CODE (op) == CONST_INT)
    return ((unsigned HOST_WIDE_INT) INTVAL (op) < 0x100
	    || (unsigned HOST_WIDE_INT) ~ INTVAL (op) < 0x100
	    || zap_mask (INTVAL (op)));

  return register_operand (op, mode);
}

/* Return 1 if OP is a valid first operand to an IOR or XOR insn.  */

int
or_operand (op, mode)
     register rtx op;
     enum machine_mode mode;
{
  if (GET_CODE (op) == CONST_INT)
    return ((unsigned HOST_WIDE_INT) INTVAL (op) < 0x100
	    || (unsigned HOST_WIDE_INT) ~ INTVAL (op) < 0x100);

  return register_operand (op, mode);
}

/* Return 1 if OP is a constant that is the width, in bits, of an integral
   mode smaller than DImode.  */

int
mode_width_operand (op, mode)
     register rtx op;
     enum machine_mode mode ATTRIBUTE_UNUSED;
{
  return (GET_CODE (op) == CONST_INT
	  && (INTVAL (op) == 8 || INTVAL (op) == 16
	      || INTVAL (op) == 32 || INTVAL (op) == 64));
}

/* Return 1 if OP is a constant that is the width of an integral machine mode
   smaller than an integer.  */

int
mode_mask_operand (op, mode)
     register rtx op;
     enum machine_mode mode ATTRIBUTE_UNUSED;
{
#if HOST_BITS_PER_WIDE_INT == 32
  if (GET_CODE (op) == CONST_DOUBLE)
    return (CONST_DOUBLE_LOW (op) == -1
	    && (CONST_DOUBLE_HIGH (op) == -1
		|| CONST_DOUBLE_HIGH (op) == 0));
#else
  if (GET_CODE (op) == CONST_DOUBLE)
    return (CONST_DOUBLE_LOW (op) == -1 && CONST_DOUBLE_HIGH (op) == 0);
#endif

  return (GET_CODE (op) == CONST_INT
	  && (INTVAL (op) == 0xff
	      || INTVAL (op) == 0xffff
	      || INTVAL (op) == (HOST_WIDE_INT)0xffffffff
#if HOST_BITS_PER_WIDE_INT == 64
	      || INTVAL (op) == -1
#endif
	      ));
}

/* Return 1 if OP is a multiple of 8 less than 64.  */

int
mul8_operand (op, mode)
     register rtx op;
     enum machine_mode mode ATTRIBUTE_UNUSED;
{
  return (GET_CODE (op) == CONST_INT
	  && (unsigned HOST_WIDE_INT) INTVAL (op) < 64
	  && (INTVAL (op) & 7) == 0);
}

/* Return 1 if OP is the constant zero in floating-point.  */

int
fp0_operand (op, mode)
     register rtx op;
     enum machine_mode mode;
{
  return (GET_MODE (op) == mode
	  && GET_MODE_CLASS (mode) == MODE_FLOAT && op == CONST0_RTX (mode));
}

/* Return 1 if OP is the floating-point constant zero or a register.  */

int
reg_or_fp0_operand (op, mode)
     register rtx op;
     enum machine_mode mode;
{
  return fp0_operand (op, mode) || register_operand (op, mode);
}

/* Return 1 if OP is a hard floating-point register.  */

int
hard_fp_register_operand (op, mode)
     register rtx op;
     enum machine_mode mode;
{
  if (mode != VOIDmode && GET_MODE (op) != VOIDmode && mode != GET_MODE (op))
    return 0;

  if (GET_CODE (op) == SUBREG)
    op = SUBREG_REG (op);
  return GET_CODE (op) == REG && REGNO_REG_CLASS (REGNO (op)) == FLOAT_REGS;
}

/* Return 1 if OP is a hard general register.  */

int
hard_int_register_operand (op, mode)
     register rtx op;
     enum machine_mode mode;
{
  if (mode != VOIDmode && GET_MODE (op) != VOIDmode && mode != GET_MODE (op))
    return 0;

  if (GET_CODE (op) == SUBREG)
    op = SUBREG_REG (op);
  return GET_CODE (op) == REG && REGNO_REG_CLASS (REGNO (op)) == GENERAL_REGS;
}

/* Return 1 if OP is a register or a constant integer.  */


int
reg_or_cint_operand (op, mode)
    register rtx op;
    enum machine_mode mode;
{
     return (GET_CODE (op) == CONST_INT
	     || register_operand (op, mode));
}

/* Return 1 if OP is something that can be reloaded into a register;
   if it is a MEM, it need not be valid.  */

int
some_operand (op, mode)
     register rtx op;
     enum machine_mode mode;
{
  if (mode != VOIDmode && GET_MODE (op) != VOIDmode && mode != GET_MODE (op))
    return 0;

  switch (GET_CODE (op))
    {
    case REG:  case MEM:  case CONST_DOUBLE:  case CONST_INT:  case LABEL_REF:
    case SYMBOL_REF:  case CONST:  case HIGH:
      return 1;

    case SUBREG:
      return some_operand (SUBREG_REG (op), VOIDmode);

    default:
      break;
    }

  return 0;
}

/* Likewise, but don't accept constants.  */

int
some_ni_operand (op, mode)
     register rtx op;
     enum machine_mode mode;
{
  if (GET_MODE (op) != mode && mode != VOIDmode)
    return 0;

  if (GET_CODE (op) == SUBREG)
    op = SUBREG_REG (op);

  return (GET_CODE (op) == REG || GET_CODE (op) == MEM);
}

/* Return 1 if OP is a valid operand for the source of a move insn.  */

int
input_operand (op, mode)
     register rtx op;
     enum machine_mode mode;
{
  if (mode != VOIDmode && GET_MODE (op) != VOIDmode && mode != GET_MODE (op))
    return 0;

  if (GET_MODE_CLASS (mode) == MODE_FLOAT && GET_MODE (op) != mode)
    return 0;

  switch (GET_CODE (op))
    {
    case LABEL_REF:
    case SYMBOL_REF:
    case CONST:
      if (TARGET_EXPLICIT_RELOCS)
	{
	  /* We don't split symbolic operands into something unintelligable
	     until after reload, but we do not wish non-small, non-global
	     symbolic operands to be reconstructed from their high/lo_sum
	     form.  */
	  return (small_symbolic_operand (op, mode)
		  || global_symbolic_operand (op, mode));
	}

      /* This handles both the Windows/NT and OSF cases.  */
      return mode == ptr_mode || mode == DImode;

    case HIGH:
      return (TARGET_EXPLICIT_RELOCS
	      && local_symbolic_operand (XEXP (op, 0), mode));

    case REG:
    case ADDRESSOF:
      return 1;

    case SUBREG:
      if (register_operand (op, mode))
	return 1;
      /* ... fall through ...  */
    case MEM:
      return ((TARGET_BWX || (mode != HImode && mode != QImode))
	      && general_operand (op, mode));

    case CONST_DOUBLE:
      return GET_MODE_CLASS (mode) == MODE_FLOAT && op == CONST0_RTX (mode);

    case CONST_INT:
      return mode == QImode || mode == HImode || add_operand (op, mode);

    case CONSTANT_P_RTX:
      return 1;

    default:
      break;
    }

  return 0;
}

/* Return 1 if OP is a SYMBOL_REF for a function known to be in this
   file, and in the same section as the current function.  */

int
current_file_function_operand (op, mode)
     rtx op;
     enum machine_mode mode ATTRIBUTE_UNUSED;
{
  if (GET_CODE (op) != SYMBOL_REF)
    return 0;

  /* Easy test for recursion.  */
  if (op == XEXP (DECL_RTL (current_function_decl), 0))
    return 1;

  /* Otherwise, we need the DECL for the SYMBOL_REF, which we can't get.
     So SYMBOL_REF_FLAG has been declared to imply that the function is
     in the default text section.  So we must also check that the current
     function is also in the text section.  */
  if (SYMBOL_REF_FLAG (op) && decl_in_text_section (current_function_decl))
    return 1;

  return 0;
}

/* Return 1 if OP is a SYMBOL_REF for which we can make a call via bsr.  */

int
direct_call_operand (op, mode)
     rtx op;
     enum machine_mode mode;
{
  /* Must be defined in this file.  */
  if (! current_file_function_operand (op, mode))
    return 0;

  /* If profiling is implemented via linker tricks, we can't jump
     to the nogp alternate entry point.  */
  /* ??? TARGET_PROFILING_NEEDS_GP isn't really the right test,
     but is approximately correct for the OSF ABIs.  Don't know
     what to do for VMS, NT, or UMK.  */
  if (! TARGET_PROFILING_NEEDS_GP
      && ! current_function_profile)
    return 0;

  return 1;
}

/* Return true if OP is a LABEL_REF, or SYMBOL_REF or CONST referencing
   a variable known to be defined in this file.  */

static bool
local_symbol_p (op)
     rtx op;
{
  const char *str = XSTR (op, 0);

  /* ??? SYMBOL_REF_FLAG is set for local function symbols, but we
     run into problems with the rtl inliner in that the symbol was
     once external, but is local after inlining, which results in
     unrecognizable insns.  */

  return (CONSTANT_POOL_ADDRESS_P (op)
	  /* If @, then ENCODE_SECTION_INFO sez it's local.  */
	  || str[0] == '@'
	  /* If *$, then ASM_GENERATE_INTERNAL_LABEL sez it's local.  */
	  || (str[0] == '*' && str[1] == '$'));
}

int
local_symbolic_operand (op, mode)
     rtx op;
     enum machine_mode mode;
{
  if (mode != VOIDmode && GET_MODE (op) != VOIDmode && mode != GET_MODE (op))
    return 0;

  if (GET_CODE (op) == LABEL_REF)
    return 1;

  if (GET_CODE (op) == CONST
      && GET_CODE (XEXP (op, 0)) == PLUS
      && GET_CODE (XEXP (XEXP (op, 0), 1)) == CONST_INT)
    op = XEXP (XEXP (op, 0), 0);

  if (GET_CODE (op) != SYMBOL_REF)
    return 0;

  return local_symbol_p (op);
}

/* Return true if OP is a SYMBOL_REF or CONST referencing a variable
   known to be defined in this file in the small data area.  */

int
small_symbolic_operand (op, mode)
     rtx op;
     enum machine_mode mode ATTRIBUTE_UNUSED;
{
  const char *str;

  if (! TARGET_SMALL_DATA)
    return 0;

  if (mode != VOIDmode && GET_MODE (op) != VOIDmode && mode != GET_MODE (op))
    return 0;

  if (GET_CODE (op) == CONST
      && GET_CODE (XEXP (op, 0)) == PLUS
      && GET_CODE (XEXP (XEXP (op, 0), 1)) == CONST_INT)
    op = XEXP (XEXP (op, 0), 0);

  if (GET_CODE (op) != SYMBOL_REF)
    return 0;

  if (CONSTANT_POOL_ADDRESS_P (op))
    return GET_MODE_SIZE (get_pool_mode (op)) <= (unsigned) g_switch_value;
  else
    {
      str = XSTR (op, 0);
      return str[0] == '@' && str[1] == 's';
    }
}

/* Return true if OP is a SYMBOL_REF or CONST referencing a variable
   not known (or known not) to be defined in this file.  */

int
global_symbolic_operand (op, mode)
     rtx op;
     enum machine_mode mode;
{
  if (mode != VOIDmode && GET_MODE (op) != VOIDmode && mode != GET_MODE (op))
    return 0;

  if (GET_CODE (op) == CONST
      && GET_CODE (XEXP (op, 0)) == PLUS
      && GET_CODE (XEXP (XEXP (op, 0), 1)) == CONST_INT)
    op = XEXP (XEXP (op, 0), 0);

  if (GET_CODE (op) != SYMBOL_REF)
    return 0;

  return ! local_symbol_p (op);
}

/* Return 1 if OP is a valid operand for the MEM of a CALL insn.  */

int
call_operand (op, mode)
     rtx op;
     enum machine_mode mode;
{
  if (mode != Pmode)
    return 0;

  if (GET_CODE (op) == REG)
    {
      if (TARGET_ABI_OSF)
	{
	  /* Disallow virtual registers to cope with pathalogical test cases
	     such as compile/930117-1.c in which the virtual reg decomposes
	     to the frame pointer.  Which is a hard reg that is not $27.  */
	  return (REGNO (op) == 27 || REGNO (op) > LAST_VIRTUAL_REGISTER);
	}
      else
	return 1;
    }
  if (TARGET_ABI_UNICOSMK)
    return 0;
  if (GET_CODE (op) == SYMBOL_REF)
    return 1;

  return 0;
}

/* Returns 1 if OP is a symbolic operand, i.e. a symbol_ref or a label_ref,
   possibly with an offset.  */

int
symbolic_operand (op, mode)
      register rtx op;
      enum machine_mode mode;
{
  if (mode != VOIDmode && GET_MODE (op) != VOIDmode && mode != GET_MODE (op))
    return 0;
  if (GET_CODE (op) == SYMBOL_REF || GET_CODE (op) == LABEL_REF)
    return 1;
  if (GET_CODE (op) == CONST
      && GET_CODE (XEXP (op,0)) == PLUS
      && GET_CODE (XEXP (XEXP (op,0), 0)) == SYMBOL_REF
      && GET_CODE (XEXP (XEXP (op,0), 1)) == CONST_INT)
    return 1;
  return 0;
}

/* Return 1 if OP is a valid Alpha comparison operator.  Here we know which
   comparisons are valid in which insn.  */

int
alpha_comparison_operator (op, mode)
     register rtx op;
     enum machine_mode mode;
{
  enum rtx_code code = GET_CODE (op);

  if (mode != GET_MODE (op) && mode != VOIDmode)
    return 0;

  return (code == EQ || code == LE || code == LT
	  || code == LEU || code == LTU);
}

/* Return 1 if OP is a valid Alpha comparison operator against zero. 
   Here we know which comparisons are valid in which insn.  */

int
alpha_zero_comparison_operator (op, mode)
     register rtx op;
     enum machine_mode mode;
{
  enum rtx_code code = GET_CODE (op);

  if (mode != GET_MODE (op) && mode != VOIDmode)
    return 0;

  return (code == EQ || code == NE || code == LE || code == LT
	  || code == LEU || code == LTU);
}

/* Return 1 if OP is a valid Alpha swapped comparison operator.  */

int
alpha_swapped_comparison_operator (op, mode)
     register rtx op;
     enum machine_mode mode;
{
  enum rtx_code code = GET_CODE (op);

  if ((mode != GET_MODE (op) && mode != VOIDmode)
      || GET_RTX_CLASS (code) != '<')
    return 0;

  code = swap_condition (code);
  return (code == EQ || code == LE || code == LT
	  || code == LEU || code == LTU);
}

/* Return 1 if OP is a signed comparison operation.  */

int
signed_comparison_operator (op, mode)
     register rtx op;
     enum machine_mode mode ATTRIBUTE_UNUSED;
{
  enum rtx_code code = GET_CODE (op);

  if (mode != GET_MODE (op) && mode != VOIDmode)
    return 0;

  return (code == EQ || code == NE
	  || code == LE || code == LT
	  || code == GE || code == GT);
}

/* Return 1 if OP is a valid Alpha floating point comparison operator.
   Here we know which comparisons are valid in which insn.  */

int
alpha_fp_comparison_operator (op, mode)
     register rtx op;
     enum machine_mode mode;
{
  enum rtx_code code = GET_CODE (op);

  if (mode != GET_MODE (op) && mode != VOIDmode)
    return 0;

  return (code == EQ || code == LE || code == LT || code == UNORDERED);
}

/* Return 1 if this is a divide or modulus operator.  */

int
divmod_operator (op, mode)
     register rtx op;
     enum machine_mode mode ATTRIBUTE_UNUSED;
{
  switch (GET_CODE (op))
    {
    case DIV:  case MOD:  case UDIV:  case UMOD:
      return 1;

    default:
      break;
    }

  return 0;
}

/* Return 1 if this memory address is a known aligned register plus
   a constant.  It must be a valid address.  This means that we can do
   this as an aligned reference plus some offset.

   Take into account what reload will do.  */

int
aligned_memory_operand (op, mode)
     register rtx op;
     enum machine_mode mode;
{
  rtx base;

  if (reload_in_progress)
    {
      rtx tmp = op;
      if (GET_CODE (tmp) == SUBREG)
	tmp = SUBREG_REG (tmp);
      if (GET_CODE (tmp) == REG
	  && REGNO (tmp) >= FIRST_PSEUDO_REGISTER)
	{
	  op = reg_equiv_memory_loc[REGNO (tmp)];
	  if (op == 0)
	    return 0;
	}
    }

  if (GET_CODE (op) != MEM
      || GET_MODE (op) != mode)
    return 0;
  op = XEXP (op, 0);

  /* LEGITIMIZE_RELOAD_ADDRESS creates (plus (plus reg const_hi) const_lo)
     sorts of constructs.  Dig for the real base register.  */
  if (reload_in_progress
      && GET_CODE (op) == PLUS
      && GET_CODE (XEXP (op, 0)) == PLUS)
    base = XEXP (XEXP (op, 0), 0);
  else
    {
      if (! memory_address_p (mode, op))
	return 0;
      base = (GET_CODE (op) == PLUS ? XEXP (op, 0) : op);
    }

  return (GET_CODE (base) == REG && REGNO_POINTER_ALIGN (REGNO (base)) >= 32);
}

/* Similar, but return 1 if OP is a MEM which is not alignable.  */

int
unaligned_memory_operand (op, mode)
     register rtx op;
     enum machine_mode mode;
{
  rtx base;

  if (reload_in_progress)
    {
      rtx tmp = op;
      if (GET_CODE (tmp) == SUBREG)
	tmp = SUBREG_REG (tmp);
      if (GET_CODE (tmp) == REG
	  && REGNO (tmp) >= FIRST_PSEUDO_REGISTER)
	{
	  op = reg_equiv_memory_loc[REGNO (tmp)];
	  if (op == 0)
	    return 0;
	}
    }

  if (GET_CODE (op) != MEM
      || GET_MODE (op) != mode)
    return 0;
  op = XEXP (op, 0);

  /* LEGITIMIZE_RELOAD_ADDRESS creates (plus (plus reg const_hi) const_lo)
     sorts of constructs.  Dig for the real base register.  */
  if (reload_in_progress
      && GET_CODE (op) == PLUS
      && GET_CODE (XEXP (op, 0)) == PLUS)
    base = XEXP (XEXP (op, 0), 0);
  else
    {
      if (! memory_address_p (mode, op))
	return 0;
      base = (GET_CODE (op) == PLUS ? XEXP (op, 0) : op);
    }

  return (GET_CODE (base) == REG && REGNO_POINTER_ALIGN (REGNO (base)) < 32);
}

/* Return 1 if OP is either a register or an unaligned memory location.  */

int
reg_or_unaligned_mem_operand (op, mode)
     rtx op;
     enum machine_mode mode;
{
  return register_operand (op, mode) || unaligned_memory_operand (op, mode);
}

/* Return 1 if OP is any memory location.  During reload a pseudo matches.  */

int
any_memory_operand (op, mode)
     register rtx op;
     enum machine_mode mode ATTRIBUTE_UNUSED;
{
  return (GET_CODE (op) == MEM
	  || (GET_CODE (op) == SUBREG && GET_CODE (SUBREG_REG (op)) == REG)
	  || (reload_in_progress && GET_CODE (op) == REG
	      && REGNO (op) >= FIRST_PSEUDO_REGISTER)
	  || (reload_in_progress && GET_CODE (op) == SUBREG
	      && GET_CODE (SUBREG_REG (op)) == REG
	      && REGNO (SUBREG_REG (op)) >= FIRST_PSEUDO_REGISTER));
}

/* Returns 1 if OP is not an eliminable register.

   This exists to cure a pathological abort in the s8addq (et al) patterns,

	long foo () { long t; bar(); return (long) &t * 26107; }

   which run afoul of a hack in reload to cure a (presumably) similar
   problem with lea-type instructions on other targets.  But there is
   one of us and many of them, so work around the problem by selectively
   preventing combine from making the optimization.  */

int
reg_not_elim_operand (op, mode)
      register rtx op;
      enum machine_mode mode;
{
  rtx inner = op;
  if (GET_CODE (op) == SUBREG)
    inner = SUBREG_REG (op);
  if (inner == frame_pointer_rtx || inner == arg_pointer_rtx)
    return 0;

  return register_operand (op, mode);
}

/* Return 1 is OP is a memory location that is not a reference (using
   an AND) to an unaligned location.  Take into account what reload
   will do.  */

int
normal_memory_operand (op, mode)
     register rtx op;
     enum machine_mode mode ATTRIBUTE_UNUSED;
{
  if (reload_in_progress)
    {
      rtx tmp = op;
      if (GET_CODE (tmp) == SUBREG)
	tmp = SUBREG_REG (tmp);
      if (GET_CODE (tmp) == REG
	  && REGNO (tmp) >= FIRST_PSEUDO_REGISTER)
	{
	  op = reg_equiv_memory_loc[REGNO (tmp)];

	  /* This may not have been assigned an equivalent address if it will
	     be eliminated.  In that case, it doesn't matter what we do.  */
	  if (op == 0)
	    return 1;
	}
    }

  return GET_CODE (op) == MEM && GET_CODE (XEXP (op, 0)) != AND;
}

/* Accept a register, but not a subreg of any kind.  This allows us to
   avoid pathological cases in reload wrt data movement common in 
   int->fp conversion.  */

int
reg_no_subreg_operand (op, mode)
     register rtx op;
     enum machine_mode mode;
{
  if (GET_CODE (op) != REG)
    return 0;
  return register_operand (op, mode);
}

/* Recognize an addition operation that includes a constant.  Used to
   convince reload to canonize (plus (plus reg c1) c2) during register
   elimination.  */

int
addition_operation (op, mode)
     register rtx op;
     enum machine_mode mode;
{
  if (GET_MODE (op) != mode && mode != VOIDmode)
    return 0;
  if (GET_CODE (op) == PLUS
      && register_operand (XEXP (op, 0), mode)
      && GET_CODE (XEXP (op, 1)) == CONST_INT
      && CONST_OK_FOR_LETTER_P (INTVAL (XEXP (op, 1)), 'K'))
    return 1;
  return 0;
}

/* Implements CONST_OK_FOR_LETTER_P.  Return true if the value matches
   the range defined for C in [I-P].  */

bool
alpha_const_ok_for_letter_p (value, c)
     HOST_WIDE_INT value;
     int c;
{
  switch (c)
    {
    case 'I':
      /* An unsigned 8 bit constant.  */
      return (unsigned HOST_WIDE_INT) value < 0x100;
    case 'J':
      /* The constant zero.  */
      return value == 0;
    case 'K':
      /* A signed 16 bit constant.  */
      return (unsigned HOST_WIDE_INT) (value + 0x8000) < 0x10000;
    case 'L':
      /* A shifted signed 16 bit constant appropriate for LDAH.  */
      return ((value & 0xffff) == 0
              && ((value) >> 31 == -1 || value >> 31 == 0));
    case 'M':
      /* A constant that can be AND'ed with using a ZAP insn.  */
      return zap_mask (value);
    case 'N':
      /* A complemented unsigned 8 bit constant.  */
      return (unsigned HOST_WIDE_INT) (~ value) < 0x100;
    case 'O':
      /* A negated unsigned 8 bit constant.  */
      return (unsigned HOST_WIDE_INT) (- value) < 0x100;
    case 'P':
      /* The constant 1, 2 or 3.  */
      return value == 1 || value == 2 || value == 3;

    default:
      return false;
    }
}

/* Implements CONST_DOUBLE_OK_FOR_LETTER_P.  Return true if VALUE
   matches for C in [GH].  */

bool
alpha_const_double_ok_for_letter_p (value, c)
     rtx value;
     int c;
{
  switch (c)
    {
    case 'G':
      /* The floating point zero constant.  */
      return (GET_MODE_CLASS (GET_MODE (value)) == MODE_FLOAT
	      && value == CONST0_RTX (GET_MODE (value)));

    case 'H':
      /* A valid operand of a ZAP insn.  */
      return (GET_MODE (value) == VOIDmode
	      && zap_mask (CONST_DOUBLE_LOW (value))
	      && zap_mask (CONST_DOUBLE_HIGH (value)));

    default:
      return false;
    }
}

/* Implements CONST_DOUBLE_OK_FOR_LETTER_P.  Return true if VALUE
   matches for C.  */

bool
alpha_extra_constraint (value, c)
     rtx value;
     int c;
{
  switch (c)
    {
    case 'Q':
      return normal_memory_operand (value, VOIDmode);
    case 'R':
      return direct_call_operand (value, Pmode);
    case 'S':
      return (GET_CODE (value) == CONST_INT
	      && (unsigned HOST_WIDE_INT) INTVAL (value) < 64);
    case 'T':
      return GET_CODE (value) == HIGH;
    case 'U':
      return TARGET_ABI_UNICOSMK && symbolic_operand (value, VOIDmode);

    default:
      return false;
    }
}

/* Return 1 if this function can directly return via $26.  */

int
direct_return ()
{
  return (! TARGET_ABI_OPEN_VMS && ! TARGET_ABI_UNICOSMK
	  && reload_completed
	  && alpha_sa_size () == 0
	  && get_frame_size () == 0
	  && current_function_outgoing_args_size == 0
	  && current_function_pretend_args_size == 0);
}

/* Return the ADDR_VEC associated with a tablejump insn.  */

rtx
alpha_tablejump_addr_vec (insn)
     rtx insn;
{
  rtx tmp;

  tmp = JUMP_LABEL (insn);
  if (!tmp)
    return NULL_RTX;
  tmp = NEXT_INSN (tmp);
  if (!tmp)
    return NULL_RTX;
  if (GET_CODE (tmp) == JUMP_INSN
      && GET_CODE (PATTERN (tmp)) == ADDR_DIFF_VEC)
    return PATTERN (tmp);
  return NULL_RTX;
}

/* Return the label of the predicted edge, or CONST0_RTX if we don't know.  */

rtx
alpha_tablejump_best_label (insn)
     rtx insn;
{
  rtx jump_table = alpha_tablejump_addr_vec (insn);
  rtx best_label = NULL_RTX;

  /* ??? Once the CFG doesn't keep getting completely rebuilt, look
     there for edge frequency counts from profile data.  */

  if (jump_table)
    {
      int n_labels = XVECLEN (jump_table, 1);
      int best_count = -1;
      int i, j;

      for (i = 0; i < n_labels; i++)
	{
	  int count = 1;

	  for (j = i + 1; j < n_labels; j++)
	    if (XEXP (XVECEXP (jump_table, 1, i), 0)
		== XEXP (XVECEXP (jump_table, 1, j), 0))
	      count++;

	  if (count > best_count)
	    best_count = count, best_label = XVECEXP (jump_table, 1, i);
	}
    }

  return best_label ? best_label : const0_rtx;
}

/* Return true if the function DECL will be placed in the default text
   section.  */
/* ??? Ideally we'd be able to always move from a SYMBOL_REF back to the
   decl, as that would allow us to determine if two functions are in the
   same section, which is what we really want to know.  */

static bool
decl_in_text_section (decl)
     tree decl;
{
  return (DECL_SECTION_NAME (decl) == NULL_TREE
	  && ! (flag_function_sections
	        || (targetm.have_named_sections
		    && DECL_ONE_ONLY (decl))));
}

/* If we are referencing a function that is static, make the SYMBOL_REF
   special.  We use this to see indicate we can branch to this function
   without setting PV or restoring GP. 

   If this is a variable that is known to be defined locally, add "@v"
   to the name.  If in addition the variable is to go in .sdata/.sbss,
   then add "@s" instead.  */

void
alpha_encode_section_info (decl)
     tree decl;
{
  const char *symbol_str;
  bool is_local, is_small;

  if (TREE_CODE (decl) == FUNCTION_DECL)
    {
      /* We mark public functions once they are emitted; otherwise we
	 don't know that they exist in this unit of translation.  */
      if (TREE_PUBLIC (decl))
	return;
      /* Do not mark functions that are not in .text; otherwise we
	 don't know that they are near enough for a direct branch.  */
      if (! decl_in_text_section (decl))
	return;

      SYMBOL_REF_FLAG (XEXP (DECL_RTL (decl), 0)) = 1;
      return;
    }

  /* Early out if we're not going to do anything with this data.  */
  if (! TARGET_EXPLICIT_RELOCS)
    return;

  /* Careful not to prod global register variables.  */
  if (TREE_CODE (decl) != VAR_DECL
      || GET_CODE (DECL_RTL (decl)) != MEM
      || GET_CODE (XEXP (DECL_RTL (decl), 0)) != SYMBOL_REF)
    return;
    
  symbol_str = XSTR (XEXP (DECL_RTL (decl), 0), 0);

  /* A variable is considered "local" if it is defined in this module.  */

  if (DECL_EXTERNAL (decl))
    is_local = false;
  /* Linkonce and weak data is never local.  */
  else if (DECL_ONE_ONLY (decl) || DECL_WEAK (decl))
    is_local = false;
  else if (! TREE_PUBLIC (decl))
    is_local = true;
  /* If PIC, then assume that any global name can be overridden by
     symbols resolved from other modules.  */
  else if (flag_pic)
    is_local = false;
  /* Uninitialized COMMON variable may be unified with symbols
     resolved from other modules.  */
  else if (DECL_COMMON (decl)
	   && (DECL_INITIAL (decl) == NULL
	       || DECL_INITIAL (decl) == error_mark_node))
    is_local = false;
  /* Otherwise we're left with initialized (or non-common) global data
     which is of necessity defined locally.  */
  else
    is_local = true;

  /* Determine if DECL will wind up in .sdata/.sbss.  */

  is_small = false;
  if (DECL_SECTION_NAME (decl))
    {
      const char *section = TREE_STRING_POINTER (DECL_SECTION_NAME (decl));
      if (strcmp (section, ".sdata") == 0
	  || strcmp (section, ".sbss") == 0)
	is_small = true;
    }
  else
    {
      HOST_WIDE_INT size = int_size_in_bytes (TREE_TYPE (decl));

      /* If the variable has already been defined in the output file, then it
	 is too late to put it in sdata if it wasn't put there in the first
	 place.  The test is here rather than above, because if it is already
	 in sdata, then it can stay there.  */

      if (TREE_ASM_WRITTEN (decl))
	;

      /* If this is an incomplete type with size 0, then we can't put it in
	 sdata because it might be too big when completed.  */
      else if (size > 0 && size <= g_switch_value)
	is_small = true;
    }

  /* Finally, encode this into the symbol string.  */
  if (is_local)
    {
      const char *string;
      char *newstr;
      size_t len;

      if (symbol_str[0] == '@')
	{
	  if (symbol_str[1] == (is_small ? 's' : 'v'))
	    return;
	  symbol_str += 2;
	}

      len = strlen (symbol_str) + 1;
      newstr = alloca (len + 2);

      newstr[0] = '@';
      newstr[1] = (is_small ? 's' : 'v');
      memcpy (newstr + 2, symbol_str, len);
	  
      string = ggc_alloc_string (newstr, len + 2 - 1);
      XSTR (XEXP (DECL_RTL (decl), 0), 0) = string;
    }
  else if (symbol_str[0] == '@')
    abort ();
}

/* legitimate_address_p recognizes an RTL expression that is a valid
   memory address for an instruction.  The MODE argument is the
   machine mode for the MEM expression that wants to use this address.

   For Alpha, we have either a constant address or the sum of a
   register and a constant address, or just a register.  For DImode,
   any of those forms can be surrounded with an AND that clear the
   low-order three bits; this is an "unaligned" access.  */

bool
alpha_legitimate_address_p (mode, x, strict)
     enum machine_mode mode;
     rtx x;
     int strict;
{
  /* If this is an ldq_u type address, discard the outer AND.  */
  if (mode == DImode
      && GET_CODE (x) == AND
      && GET_CODE (XEXP (x, 1)) == CONST_INT
      && INTVAL (XEXP (x, 1)) == -8)
    x = XEXP (x, 0);

  /* Discard non-paradoxical subregs.  */
  if (GET_CODE (x) == SUBREG
      && (GET_MODE_SIZE (GET_MODE (x))
	  < GET_MODE_SIZE (GET_MODE (SUBREG_REG (x)))))
    x = SUBREG_REG (x);

  /* Unadorned general registers are valid.  */
  if (REG_P (x)
      && (strict
	  ? STRICT_REG_OK_FOR_BASE_P (x)
	  : NONSTRICT_REG_OK_FOR_BASE_P (x)))
    return true;

  /* Constant addresses (i.e. +/- 32k) are valid.  */
  if (CONSTANT_ADDRESS_P (x))
    return true;

  /* Register plus a small constant offset is valid.  */
  if (GET_CODE (x) == PLUS)
    {
      rtx ofs = XEXP (x, 1);
      x = XEXP (x, 0);

      /* Discard non-paradoxical subregs.  */
      if (GET_CODE (x) == SUBREG
          && (GET_MODE_SIZE (GET_MODE (x))
	      < GET_MODE_SIZE (GET_MODE (SUBREG_REG (x)))))
	x = SUBREG_REG (x);

      if (REG_P (x))
	{
	  if (! strict
	      && NONSTRICT_REG_OK_FP_BASE_P (x)
	      && GET_CODE (ofs) == CONST_INT)
	    return true;
	  if ((strict
	       ? STRICT_REG_OK_FOR_BASE_P (x)
	       : NONSTRICT_REG_OK_FOR_BASE_P (x))
	      && CONSTANT_ADDRESS_P (ofs))
	    return true;
	}
      else if (GET_CODE (x) == ADDRESSOF
	       && GET_CODE (ofs) == CONST_INT)
	return true;
    }

  /* If we're managing explicit relocations, LO_SUM is valid, as
     are small data symbols.  */
  else if (TARGET_EXPLICIT_RELOCS)
    {
      if (small_symbolic_operand (x, Pmode))
	return true;

      if (GET_CODE (x) == LO_SUM)
	{
	  rtx ofs = XEXP (x, 1);
	  x = XEXP (x, 0);

	  /* Discard non-paradoxical subregs.  */
	  if (GET_CODE (x) == SUBREG
	      && (GET_MODE_SIZE (GET_MODE (x))
		  < GET_MODE_SIZE (GET_MODE (SUBREG_REG (x)))))
	    x = SUBREG_REG (x);

	  /* Must have a valid base register.  */
	  if (! (REG_P (x)
		 && (strict
		     ? STRICT_REG_OK_FOR_BASE_P (x)
		     : NONSTRICT_REG_OK_FOR_BASE_P (x))))
	    return false;

	  /* The symbol must be local.  */
	  if (local_symbolic_operand (ofs, Pmode))
	    return true;
	}
    }

  return false;
}

/* Try machine-dependent ways of modifying an illegitimate address
   to be legitimate.  If we find one, return the new, valid address.  */

rtx
alpha_legitimize_address (x, scratch, mode)
     rtx x;
     rtx scratch;
     enum machine_mode mode ATTRIBUTE_UNUSED;
{
  HOST_WIDE_INT addend;

  /* If the address is (plus reg const_int) and the CONST_INT is not a
     valid offset, compute the high part of the constant and add it to
     the register.  Then our address is (plus temp low-part-const).  */
  if (GET_CODE (x) == PLUS
      && GET_CODE (XEXP (x, 0)) == REG
      && GET_CODE (XEXP (x, 1)) == CONST_INT
      && ! CONSTANT_ADDRESS_P (XEXP (x, 1)))
    {
      addend = INTVAL (XEXP (x, 1));
      x = XEXP (x, 0);
      goto split_addend;
    }

  /* If the address is (const (plus FOO const_int)), find the low-order
     part of the CONST_INT.  Then load FOO plus any high-order part of the
     CONST_INT into a register.  Our address is (plus reg low-part-const).
     This is done to reduce the number of GOT entries.  */
  if (!no_new_pseudos
      && GET_CODE (x) == CONST
      && GET_CODE (XEXP (x, 0)) == PLUS
      && GET_CODE (XEXP (XEXP (x, 0), 1)) == CONST_INT)
    {
      addend = INTVAL (XEXP (XEXP (x, 0), 1));
      x = force_reg (Pmode, XEXP (XEXP (x, 0), 0));
      goto split_addend;
    }

  /* If we have a (plus reg const), emit the load as in (2), then add
     the two registers, and finally generate (plus reg low-part-const) as
     our address.  */
  if (!no_new_pseudos
      && GET_CODE (x) == PLUS
      && GET_CODE (XEXP (x, 0)) == REG
      && GET_CODE (XEXP (x, 1)) == CONST
      && GET_CODE (XEXP (XEXP (x, 1), 0)) == PLUS
      && GET_CODE (XEXP (XEXP (XEXP (x, 1), 0), 1)) == CONST_INT)
    {
      addend = INTVAL (XEXP (XEXP (XEXP (x, 1), 0), 1));
      x = expand_simple_binop (Pmode, PLUS, XEXP (x, 0),
			       XEXP (XEXP (XEXP (x, 1), 0), 0),
			       NULL_RTX, 1, OPTAB_LIB_WIDEN);
      goto split_addend;
    }

  /* If this is a local symbol, split the address into HIGH/LO_SUM parts.  */
  if (TARGET_EXPLICIT_RELOCS && symbolic_operand (x, Pmode))
    {
      if (local_symbolic_operand (x, Pmode))
	{
	  if (small_symbolic_operand (x, Pmode))
	    return x;
	  else
	    {
	      if (!no_new_pseudos)
	        scratch = gen_reg_rtx (Pmode);
	      emit_insn (gen_rtx_SET (VOIDmode, scratch,
				      gen_rtx_HIGH (Pmode, x)));
	      return gen_rtx_LO_SUM (Pmode, scratch, x);
	    }
	}
    }

  return NULL;

 split_addend:
  {
    HOST_WIDE_INT low, high;

    low = ((addend & 0xffff) ^ 0x8000) - 0x8000;
    addend -= low;
    high = ((addend & 0xffffffff) ^ 0x80000000) - 0x80000000;
    addend -= high;

    if (addend)
      x = expand_simple_binop (Pmode, PLUS, x, GEN_INT (addend),
			       (no_new_pseudos ? scratch : NULL_RTX),
			       1, OPTAB_LIB_WIDEN);
    if (high)
      x = expand_simple_binop (Pmode, PLUS, x, GEN_INT (high),
			       (no_new_pseudos ? scratch : NULL_RTX),
			       1, OPTAB_LIB_WIDEN);

    return plus_constant (x, low);
  }
}

/* For TARGET_EXPLICIT_RELOCS, we don't obfuscate a SYMBOL_REF to a
   small symbolic operand until after reload.  At which point we need
   to replace (mem (symbol_ref)) with (mem (lo_sum $29 symbol_ref))
   so that sched2 has the proper dependency information.  */

int
some_small_symbolic_mem_operand (x, mode)
     rtx x;
     enum machine_mode mode ATTRIBUTE_UNUSED;
{
  return for_each_rtx (&x, some_small_symbolic_mem_operand_1, NULL);
}

static int
some_small_symbolic_mem_operand_1 (px, data)
     rtx *px;
     void *data ATTRIBUTE_UNUSED;
{
  rtx x = *px;

  if (GET_CODE (x) != MEM)
    return 0;
  x = XEXP (x, 0);

  /* If this is an ldq_u type address, discard the outer AND.  */
  if (GET_CODE (x) == AND)
    x = XEXP (x, 0);

  return small_symbolic_operand (x, Pmode) ? 1 : -1;
}

rtx
split_small_symbolic_mem_operand (x)
     rtx x;
{
  x = copy_insn (x);
  for_each_rtx (&x, split_small_symbolic_mem_operand_1, NULL);
  return x;
}

static int
split_small_symbolic_mem_operand_1 (px, data)
     rtx *px;
     void *data ATTRIBUTE_UNUSED;
{
  rtx x = *px;

  if (GET_CODE (x) != MEM)
    return 0;

  px = &XEXP (x, 0), x = *px;
  if (GET_CODE (x) == AND)
    px = &XEXP (x, 0), x = *px;

  if (small_symbolic_operand (x, Pmode))
    {
      x = gen_rtx_LO_SUM (Pmode, pic_offset_table_rtx, x);
      *px = x;
    }

  return -1;
}

/* Try a machine-dependent way of reloading an illegitimate address
   operand.  If we find one, push the reload and return the new rtx.  */
   
rtx
alpha_legitimize_reload_address (x, mode, opnum, type, ind_levels)
     rtx x;
     enum machine_mode mode ATTRIBUTE_UNUSED;
     int opnum;
     int type;
     int ind_levels ATTRIBUTE_UNUSED;
{
  /* We must recognize output that we have already generated ourselves.  */
  if (GET_CODE (x) == PLUS
      && GET_CODE (XEXP (x, 0)) == PLUS
      && GET_CODE (XEXP (XEXP (x, 0), 0)) == REG
      && GET_CODE (XEXP (XEXP (x, 0), 1)) == CONST_INT
      && GET_CODE (XEXP (x, 1)) == CONST_INT)
    {
      push_reload (XEXP (x, 0), NULL_RTX, &XEXP (x, 0), NULL,
		   BASE_REG_CLASS, GET_MODE (x), VOIDmode, 0, 0,
		   opnum, type);
      return x;
    }

  /* We wish to handle large displacements off a base register by
     splitting the addend across an ldah and the mem insn.  This
     cuts number of extra insns needed from 3 to 1.  */
  if (GET_CODE (x) == PLUS
      && GET_CODE (XEXP (x, 0)) == REG
      && REGNO (XEXP (x, 0)) < FIRST_PSEUDO_REGISTER
      && REGNO_OK_FOR_BASE_P (REGNO (XEXP (x, 0)))
      && GET_CODE (XEXP (x, 1)) == CONST_INT)
    {
      HOST_WIDE_INT val = INTVAL (XEXP (x, 1));
      HOST_WIDE_INT low = ((val & 0xffff) ^ 0x8000) - 0x8000;
      HOST_WIDE_INT high
	= (((val - low) & 0xffffffff) ^ 0x80000000) - 0x80000000;

      /* Check for 32-bit overflow.  */
      if (high + low != val)
	return NULL_RTX;

      /* Reload the high part into a base reg; leave the low part
	 in the mem directly.  */
      x = gen_rtx_PLUS (GET_MODE (x),
			gen_rtx_PLUS (GET_MODE (x), XEXP (x, 0),
				      GEN_INT (high)),
			GEN_INT (low));

      push_reload (XEXP (x, 0), NULL_RTX, &XEXP (x, 0), NULL,
		   BASE_REG_CLASS, GET_MODE (x), VOIDmode, 0, 0,
		   opnum, type);
      return x;
    }

  return NULL_RTX;
}

/* REF is an alignable memory location.  Place an aligned SImode
   reference into *PALIGNED_MEM and the number of bits to shift into
   *PBITNUM.  SCRATCH is a free register for use in reloading out
   of range stack slots.  */

void
get_aligned_mem (ref, paligned_mem, pbitnum)
     rtx ref;
     rtx *paligned_mem, *pbitnum;
{
  rtx base;
  HOST_WIDE_INT offset = 0;

  if (GET_CODE (ref) != MEM)
    abort ();

  if (reload_in_progress
      && ! memory_address_p (GET_MODE (ref), XEXP (ref, 0)))
    {
      base = find_replacement (&XEXP (ref, 0));

      if (! memory_address_p (GET_MODE (ref), base))
	abort ();
    }
  else
    {
      base = XEXP (ref, 0);
    }

  if (GET_CODE (base) == PLUS)
    offset += INTVAL (XEXP (base, 1)), base = XEXP (base, 0);

  *paligned_mem
    = widen_memory_access (ref, SImode, (offset & ~3) - offset);

  if (WORDS_BIG_ENDIAN)
    *pbitnum = GEN_INT (32 - (GET_MODE_BITSIZE (GET_MODE (ref))
			      + (offset & 3) * 8));
  else
    *pbitnum = GEN_INT ((offset & 3) * 8);
}

/* Similar, but just get the address.  Handle the two reload cases.  
   Add EXTRA_OFFSET to the address we return.  */

rtx
get_unaligned_address (ref, extra_offset)
     rtx ref;
     int extra_offset;
{
  rtx base;
  HOST_WIDE_INT offset = 0;

  if (GET_CODE (ref) != MEM)
    abort ();

  if (reload_in_progress
      && ! memory_address_p (GET_MODE (ref), XEXP (ref, 0)))
    {
      base = find_replacement (&XEXP (ref, 0));

      if (! memory_address_p (GET_MODE (ref), base))
	abort ();
    }
  else
    {
      base = XEXP (ref, 0);
    }

  if (GET_CODE (base) == PLUS)
    offset += INTVAL (XEXP (base, 1)), base = XEXP (base, 0);

  return plus_constant (base, offset + extra_offset);
}

/* On the Alpha, all (non-symbolic) constants except zero go into
   a floating-point register via memory.  Note that we cannot 
   return anything that is not a subset of CLASS, and that some
   symbolic constants cannot be dropped to memory.  */

enum reg_class
alpha_preferred_reload_class(x, class)
     rtx x;
     enum reg_class class;
{
  /* Zero is present in any register class.  */
  if (x == CONST0_RTX (GET_MODE (x)))
    return class;

  /* These sorts of constants we can easily drop to memory.  */
  if (GET_CODE (x) == CONST_INT || GET_CODE (x) == CONST_DOUBLE)
    {
      if (class == FLOAT_REGS)
	return NO_REGS;
      if (class == ALL_REGS)
	return GENERAL_REGS;
      return class;
    }

  /* All other kinds of constants should not (and in the case of HIGH
     cannot) be dropped to memory -- instead we use a GENERAL_REGS
     secondary reload.  */
  if (CONSTANT_P (x))
    return (class == ALL_REGS ? GENERAL_REGS : class);

  return class;
}

/* Loading and storing HImode or QImode values to and from memory
   usually requires a scratch register.  The exceptions are loading
   QImode and HImode from an aligned address to a general register
   unless byte instructions are permitted. 

   We also cannot load an unaligned address or a paradoxical SUBREG
   into an FP register. 

   We also cannot do integral arithmetic into FP regs, as might result
   from register elimination into a DImode fp register.  */

enum reg_class
secondary_reload_class (class, mode, x, in)
     enum reg_class class;
     enum machine_mode mode;
     rtx x;
     int in;
{
  if ((mode == QImode || mode == HImode) && ! TARGET_BWX)
    {
      if (GET_CODE (x) == MEM
	  || (GET_CODE (x) == REG && REGNO (x) >= FIRST_PSEUDO_REGISTER)
	  || (GET_CODE (x) == SUBREG
	      && (GET_CODE (SUBREG_REG (x)) == MEM
		  || (GET_CODE (SUBREG_REG (x)) == REG
		      && REGNO (SUBREG_REG (x)) >= FIRST_PSEUDO_REGISTER))))
	{
	  if (!in || !aligned_memory_operand(x, mode))
	    return GENERAL_REGS;
	}
    }

  if (class == FLOAT_REGS)
    {
      if (GET_CODE (x) == MEM && GET_CODE (XEXP (x, 0)) == AND)
	return GENERAL_REGS;

      if (GET_CODE (x) == SUBREG
	  && (GET_MODE_SIZE (GET_MODE (x))
	      > GET_MODE_SIZE (GET_MODE (SUBREG_REG (x)))))
	return GENERAL_REGS;

      if (in && INTEGRAL_MODE_P (mode)
	  && ! (memory_operand (x, mode) || x == const0_rtx))
	return GENERAL_REGS;
    }

  return NO_REGS;
}

/* Subfunction of the following function.  Update the flags of any MEM
   found in part of X.  */

static void
alpha_set_memflags_1 (x, in_struct_p, volatile_p, unchanging_p)
     rtx x;
     int in_struct_p, volatile_p, unchanging_p;
{
  int i;

  switch (GET_CODE (x))
    {
    case SEQUENCE:
    case PARALLEL:
      for (i = XVECLEN (x, 0) - 1; i >= 0; i--)
	alpha_set_memflags_1 (XVECEXP (x, 0, i), in_struct_p, volatile_p,
			      unchanging_p);
      break;

    case INSN:
      alpha_set_memflags_1 (PATTERN (x), in_struct_p, volatile_p,
			    unchanging_p);
      break;

    case SET:
      alpha_set_memflags_1 (SET_DEST (x), in_struct_p, volatile_p,
			    unchanging_p);
      alpha_set_memflags_1 (SET_SRC (x), in_struct_p, volatile_p,
			    unchanging_p);
      break;

    case MEM:
      MEM_IN_STRUCT_P (x) = in_struct_p;
      MEM_VOLATILE_P (x) = volatile_p;
      RTX_UNCHANGING_P (x) = unchanging_p;
      /* Sadly, we cannot use alias sets because the extra aliasing
	 produced by the AND interferes.  Given that two-byte quantities
	 are the only thing we would be able to differentiate anyway,
	 there does not seem to be any point in convoluting the early
	 out of the alias check.  */
      break;

    default:
      break;
    }
}

/* Given INSN, which is either an INSN or a SEQUENCE generated to
   perform a memory operation, look for any MEMs in either a SET_DEST or
   a SET_SRC and copy the in-struct, unchanging, and volatile flags from
   REF into each of the MEMs found.  If REF is not a MEM, don't do
   anything.  */

void
alpha_set_memflags (insn, ref)
     rtx insn;
     rtx ref;
{
  int in_struct_p, volatile_p, unchanging_p;

  if (GET_CODE (ref) != MEM)
    return;

  in_struct_p = MEM_IN_STRUCT_P (ref);
  volatile_p = MEM_VOLATILE_P (ref);
  unchanging_p = RTX_UNCHANGING_P (ref);

  /* This is only called from alpha.md, after having had something 
     generated from one of the insn patterns.  So if everything is
     zero, the pattern is already up-to-date.  */
  if (! in_struct_p && ! volatile_p && ! unchanging_p)
    return;

  alpha_set_memflags_1 (insn, in_struct_p, volatile_p, unchanging_p);
}

/* Try to output insns to set TARGET equal to the constant C if it can be
   done in less than N insns.  Do all computations in MODE.  Returns the place
   where the output has been placed if it can be done and the insns have been
   emitted.  If it would take more than N insns, zero is returned and no
   insns and emitted.  */

rtx
alpha_emit_set_const (target, mode, c, n)
     rtx target;
     enum machine_mode mode;
     HOST_WIDE_INT c;
     int n;
{
  rtx pat;
  int i;

  /* Try 1 insn, then 2, then up to N.  */
  for (i = 1; i <= n; i++)
    if ((pat = alpha_emit_set_const_1 (target, mode, c, i)) != 0)
      return pat;

  return 0;
}

/* Internal routine for the above to check for N or below insns.  */

static rtx
alpha_emit_set_const_1 (target, mode, c, n)
     rtx target;
     enum machine_mode mode;
     HOST_WIDE_INT c;
     int n;
{
  HOST_WIDE_INT new;
  int i, bits;
  /* Use a pseudo if highly optimizing and still generating RTL.  */
  rtx subtarget
    = (flag_expensive_optimizations && rtx_equal_function_value_matters
       ? 0 : target);
  rtx temp;

#if HOST_BITS_PER_WIDE_INT == 64
  /* We are only called for SImode and DImode.  If this is SImode, ensure that
     we are sign extended to a full word.  This does not make any sense when
     cross-compiling on a narrow machine.  */

  if (mode == SImode)
    c = ((c & 0xffffffff) ^ 0x80000000) - 0x80000000;
#endif

  /* If this is a sign-extended 32-bit constant, we can do this in at most
     three insns, so do it if we have enough insns left.  We always have
     a sign-extended 32-bit constant when compiling on a narrow machine.  */

  if (HOST_BITS_PER_WIDE_INT != 64
      || c >> 31 == -1 || c >> 31 == 0)
    {
      HOST_WIDE_INT low = ((c & 0xffff) ^ 0x8000) - 0x8000;
      HOST_WIDE_INT tmp1 = c - low;
      HOST_WIDE_INT high = (((tmp1 >> 16) & 0xffff) ^ 0x8000) - 0x8000;
      HOST_WIDE_INT extra = 0;

      /* If HIGH will be interpreted as negative but the constant is
	 positive, we must adjust it to do two ldha insns.  */

      if ((high & 0x8000) != 0 && c >= 0)
	{
	  extra = 0x4000;
	  tmp1 -= 0x40000000;
	  high = ((tmp1 >> 16) & 0xffff) - 2 * ((tmp1 >> 16) & 0x8000);
	}

      if (c == low || (low == 0 && extra == 0))
	{
	  /* We used to use copy_to_suggested_reg (GEN_INT (c), target, mode)
	     but that meant that we can't handle INT_MIN on 32-bit machines
	     (like NT/Alpha), because we recurse indefinitely through 
	     emit_move_insn to gen_movdi.  So instead, since we know exactly
	     what we want, create it explicitly.  */

	  if (target == NULL)
	    target = gen_reg_rtx (mode);
	  emit_insn (gen_rtx_SET (VOIDmode, target, GEN_INT (c)));
	  return target;
	}
      else if (n >= 2 + (extra != 0))
	{
	  temp = copy_to_suggested_reg (GEN_INT (high << 16), subtarget, mode);

	  if (extra != 0)
	    temp = expand_binop (mode, add_optab, temp, GEN_INT (extra << 16),
				 subtarget, 0, OPTAB_WIDEN);

	  return expand_binop (mode, add_optab, temp, GEN_INT (low),
			       target, 0, OPTAB_WIDEN);
	}
    }

  /* If we couldn't do it that way, try some other methods.  But if we have
     no instructions left, don't bother.  Likewise, if this is SImode and
     we can't make pseudos, we can't do anything since the expand_binop
     and expand_unop calls will widen and try to make pseudos.  */

  if (n == 1
      || (mode == SImode && ! rtx_equal_function_value_matters))
    return 0;

  /* Next, see if we can load a related constant and then shift and possibly
     negate it to get the constant we want.  Try this once each increasing
     numbers of insns.  */

  for (i = 1; i < n; i++)
    {
      /* First, see if minus some low bits, we've an easy load of
	 high bits.  */

      new = ((c & 0xffff) ^ 0x8000) - 0x8000;
      if (new != 0
          && (temp = alpha_emit_set_const (subtarget, mode, c - new, i)) != 0)
	return expand_binop (mode, add_optab, temp, GEN_INT (new),
			     target, 0, OPTAB_WIDEN);

      /* Next try complementing.  */
      if ((temp = alpha_emit_set_const (subtarget, mode, ~ c, i)) != 0)
	return expand_unop (mode, one_cmpl_optab, temp, target, 0);

      /* Next try to form a constant and do a left shift.  We can do this
	 if some low-order bits are zero; the exact_log2 call below tells
	 us that information.  The bits we are shifting out could be any
	 value, but here we'll just try the 0- and sign-extended forms of
	 the constant.  To try to increase the chance of having the same
	 constant in more than one insn, start at the highest number of
	 bits to shift, but try all possibilities in case a ZAPNOT will
	 be useful.  */

      if ((bits = exact_log2 (c & - c)) > 0)
	for (; bits > 0; bits--)
	  if ((temp = (alpha_emit_set_const
		       (subtarget, mode, c >> bits, i))) != 0
	      || ((temp = (alpha_emit_set_const
			  (subtarget, mode,
			   ((unsigned HOST_WIDE_INT) c) >> bits, i)))
		  != 0))
	    return expand_binop (mode, ashl_optab, temp, GEN_INT (bits),
				 target, 0, OPTAB_WIDEN);

      /* Now try high-order zero bits.  Here we try the shifted-in bits as
	 all zero and all ones.  Be careful to avoid shifting outside the
	 mode and to avoid shifting outside the host wide int size.  */
      /* On narrow hosts, don't shift a 1 into the high bit, since we'll
	 confuse the recursive call and set all of the high 32 bits.  */

      if ((bits = (MIN (HOST_BITS_PER_WIDE_INT, GET_MODE_SIZE (mode) * 8)
		   - floor_log2 (c) - 1 - (HOST_BITS_PER_WIDE_INT < 64))) > 0)
	for (; bits > 0; bits--)
	  if ((temp = alpha_emit_set_const (subtarget, mode,
					    c << bits, i)) != 0
	      || ((temp = (alpha_emit_set_const
			   (subtarget, mode,
			    ((c << bits) | (((HOST_WIDE_INT) 1 << bits) - 1)),
			    i)))
		  != 0))
	    return expand_binop (mode, lshr_optab, temp, GEN_INT (bits),
				 target, 1, OPTAB_WIDEN);

      /* Now try high-order 1 bits.  We get that with a sign-extension.
	 But one bit isn't enough here.  Be careful to avoid shifting outside
	 the mode and to avoid shifting outside the host wide int size.  */

      if ((bits = (MIN (HOST_BITS_PER_WIDE_INT, GET_MODE_SIZE (mode) * 8)
		   - floor_log2 (~ c) - 2)) > 0)
	for (; bits > 0; bits--)
	  if ((temp = alpha_emit_set_const (subtarget, mode,
					    c << bits, i)) != 0
	      || ((temp = (alpha_emit_set_const
			   (subtarget, mode,
			    ((c << bits) | (((HOST_WIDE_INT) 1 << bits) - 1)),
			    i)))
		  != 0))
	    return expand_binop (mode, ashr_optab, temp, GEN_INT (bits),
				 target, 0, OPTAB_WIDEN);
    }

#if HOST_BITS_PER_WIDE_INT == 64
  /* Finally, see if can load a value into the target that is the same as the
     constant except that all bytes that are 0 are changed to be 0xff.  If we
     can, then we can do a ZAPNOT to obtain the desired constant.  */

  new = c;
  for (i = 0; i < 64; i += 8)
    if ((new & ((HOST_WIDE_INT) 0xff << i)) == 0)
      new |= (HOST_WIDE_INT) 0xff << i;

  /* We are only called for SImode and DImode.  If this is SImode, ensure that
     we are sign extended to a full word.  */

  if (mode == SImode)
    new = ((new & 0xffffffff) ^ 0x80000000) - 0x80000000;

  if (new != c && new != -1
      && (temp = alpha_emit_set_const (subtarget, mode, new, n - 1)) != 0)
    return expand_binop (mode, and_optab, temp, GEN_INT (c | ~ new),
			 target, 0, OPTAB_WIDEN);
#endif

  return 0;
}

/* Having failed to find a 3 insn sequence in alpha_emit_set_const,
   fall back to a straight forward decomposition.  We do this to avoid
   exponential run times encountered when looking for longer sequences
   with alpha_emit_set_const.  */

rtx
alpha_emit_set_long_const (target, c1, c2)
     rtx target;
     HOST_WIDE_INT c1, c2;
{
  HOST_WIDE_INT d1, d2, d3, d4;

  /* Decompose the entire word */
#if HOST_BITS_PER_WIDE_INT >= 64
  if (c2 != -(c1 < 0))
    abort ();
  d1 = ((c1 & 0xffff) ^ 0x8000) - 0x8000;
  c1 -= d1;
  d2 = ((c1 & 0xffffffff) ^ 0x80000000) - 0x80000000;
  c1 = (c1 - d2) >> 32;
  d3 = ((c1 & 0xffff) ^ 0x8000) - 0x8000;
  c1 -= d3;
  d4 = ((c1 & 0xffffffff) ^ 0x80000000) - 0x80000000;
  if (c1 != d4)
    abort ();
#else
  d1 = ((c1 & 0xffff) ^ 0x8000) - 0x8000;
  c1 -= d1;
  d2 = ((c1 & 0xffffffff) ^ 0x80000000) - 0x80000000;
  if (c1 != d2)
    abort ();
  c2 += (d2 < 0);
  d3 = ((c2 & 0xffff) ^ 0x8000) - 0x8000;
  c2 -= d3;
  d4 = ((c2 & 0xffffffff) ^ 0x80000000) - 0x80000000;
  if (c2 != d4)
    abort ();
#endif

  /* Construct the high word */
  if (d4)
    {
      emit_move_insn (target, GEN_INT (d4));
      if (d3)
	emit_move_insn (target, gen_rtx_PLUS (DImode, target, GEN_INT (d3)));
    }
  else
    emit_move_insn (target, GEN_INT (d3));

  /* Shift it into place */
  emit_move_insn (target, gen_rtx_ASHIFT (DImode, target, GEN_INT (32)));

  /* Add in the low bits.  */
  if (d2)
    emit_move_insn (target, gen_rtx_PLUS (DImode, target, GEN_INT (d2)));
  if (d1)
    emit_move_insn (target, gen_rtx_PLUS (DImode, target, GEN_INT (d1)));

  return target;
}

/* Expand a move instruction; return true if all work is done.
   We don't handle non-bwx subword loads here.  */

bool
alpha_expand_mov (mode, operands)
     enum machine_mode mode;
     rtx *operands;
{
  /* If the output is not a register, the input must be.  */
  if (GET_CODE (operands[0]) == MEM
      && ! reg_or_0_operand (operands[1], mode))
    operands[1] = force_reg (mode, operands[1]);

  /* Allow legitimize_address to perform some simplifications.  */
  if (mode == Pmode && symbolic_operand (operands[1], mode))
    {
      rtx tmp = alpha_legitimize_address (operands[1], operands[0], mode);
      if (tmp)
	{
	  operands[1] = tmp;
	  return false;
	}
    }

  /* Early out for non-constants and valid constants.  */
  if (! CONSTANT_P (operands[1]) || input_operand (operands[1], mode))
    return false;

  /* Split large integers.  */
  if (GET_CODE (operands[1]) == CONST_INT
      || GET_CODE (operands[1]) == CONST_DOUBLE)
    {
      HOST_WIDE_INT i0, i1;
      rtx temp = NULL_RTX;

      if (GET_CODE (operands[1]) == CONST_INT)
	{
	  i0 = INTVAL (operands[1]);
	  i1 = -(i0 < 0);
	}
      else if (HOST_BITS_PER_WIDE_INT >= 64)
	{
	  i0 = CONST_DOUBLE_LOW (operands[1]);
	  i1 = -(i0 < 0);
	}
      else
	{
	  i0 = CONST_DOUBLE_LOW (operands[1]);
	  i1 = CONST_DOUBLE_HIGH (operands[1]);
	}

      if (HOST_BITS_PER_WIDE_INT >= 64 || i1 == -(i0 < 0))
	temp = alpha_emit_set_const (operands[0], mode, i0, 3);

      if (!temp && TARGET_BUILD_CONSTANTS)
	temp = alpha_emit_set_long_const (operands[0], i0, i1);

      if (temp)
	{
	  if (rtx_equal_p (operands[0], temp))
	    return true;
	  operands[1] = temp;
	  return false;
	}
    }

  /* Otherwise we've nothing left but to drop the thing to memory.  */
  operands[1] = force_const_mem (DImode, operands[1]);
  if (reload_in_progress)
    {
      emit_move_insn (operands[0], XEXP (operands[1], 0));
      operands[1] = copy_rtx (operands[1]);
      XEXP (operands[1], 0) = operands[0];
    }
  else
    operands[1] = validize_mem (operands[1]);
  return false;
}

/* Expand a non-bwx QImode or HImode move instruction;
   return true if all work is done.  */

bool
alpha_expand_mov_nobwx (mode, operands)
     enum machine_mode mode;
     rtx *operands;
{
  /* If the output is not a register, the input must be.  */
  if (GET_CODE (operands[0]) == MEM)
    operands[1] = force_reg (mode, operands[1]);

  /* Handle four memory cases, unaligned and aligned for either the input
     or the output.  The only case where we can be called during reload is
     for aligned loads; all other cases require temporaries.  */

  if (GET_CODE (operands[1]) == MEM
      || (GET_CODE (operands[1]) == SUBREG
	  && GET_CODE (SUBREG_REG (operands[1])) == MEM)
      || (reload_in_progress && GET_CODE (operands[1]) == REG
	  && REGNO (operands[1]) >= FIRST_PSEUDO_REGISTER)
      || (reload_in_progress && GET_CODE (operands[1]) == SUBREG
	  && GET_CODE (SUBREG_REG (operands[1])) == REG
	  && REGNO (SUBREG_REG (operands[1])) >= FIRST_PSEUDO_REGISTER))
    {
      if (aligned_memory_operand (operands[1], mode))
	{
	  if (reload_in_progress)
	    {
	      emit_insn ((mode == QImode
			  ? gen_reload_inqi_help
			  : gen_reload_inhi_help)
		         (operands[0], operands[1],
			  gen_rtx_REG (SImode, REGNO (operands[0]))));
	    }
	  else
	    {
	      rtx aligned_mem, bitnum;
	      rtx scratch = gen_reg_rtx (SImode);

	      get_aligned_mem (operands[1], &aligned_mem, &bitnum);

	      emit_insn ((mode == QImode
			  ? gen_aligned_loadqi
			  : gen_aligned_loadhi)
			 (operands[0], aligned_mem, bitnum, scratch));
	    }
	}
      else
	{
	  /* Don't pass these as parameters since that makes the generated
	     code depend on parameter evaluation order which will cause
	     bootstrap failures.  */

	  rtx temp1 = gen_reg_rtx (DImode);
	  rtx temp2 = gen_reg_rtx (DImode);
	  rtx seq = ((mode == QImode
		      ? gen_unaligned_loadqi
		      : gen_unaligned_loadhi)
		     (operands[0], get_unaligned_address (operands[1], 0),
		      temp1, temp2));

	  alpha_set_memflags (seq, operands[1]);
	  emit_insn (seq);
	}
      return true;
    }

  if (GET_CODE (operands[0]) == MEM
      || (GET_CODE (operands[0]) == SUBREG
	  && GET_CODE (SUBREG_REG (operands[0])) == MEM)
      || (reload_in_progress && GET_CODE (operands[0]) == REG
	  && REGNO (operands[0]) >= FIRST_PSEUDO_REGISTER)
      || (reload_in_progress && GET_CODE (operands[0]) == SUBREG
	  && GET_CODE (SUBREG_REG (operands[0])) == REG
	  && REGNO (operands[0]) >= FIRST_PSEUDO_REGISTER))
    {
      if (aligned_memory_operand (operands[0], mode))
	{
	  rtx aligned_mem, bitnum;
	  rtx temp1 = gen_reg_rtx (SImode);
	  rtx temp2 = gen_reg_rtx (SImode);

	  get_aligned_mem (operands[0], &aligned_mem, &bitnum);

	  emit_insn (gen_aligned_store (aligned_mem, operands[1], bitnum,
					temp1, temp2));
	}
      else
	{
	  rtx temp1 = gen_reg_rtx (DImode);
	  rtx temp2 = gen_reg_rtx (DImode);
	  rtx temp3 = gen_reg_rtx (DImode);
	  rtx seq = ((mode == QImode
		      ? gen_unaligned_storeqi
		      : gen_unaligned_storehi)
		     (get_unaligned_address (operands[0], 0),
		      operands[1], temp1, temp2, temp3));

	  alpha_set_memflags (seq, operands[0]);
	  emit_insn (seq);
	}
      return true;
    }

  return false;
}

/* Generate an unsigned DImode to FP conversion.  This is the same code
   optabs would emit if we didn't have TFmode patterns.

   For SFmode, this is the only construction I've found that can pass
   gcc.c-torture/execute/ieee/rbug.c.  No scenario that uses DFmode
   intermediates will work, because you'll get intermediate rounding
   that ruins the end result.  Some of this could be fixed by turning
   on round-to-positive-infinity, but that requires diddling the fpsr,
   which kills performance.  I tried turning this around and converting
   to a negative number, so that I could turn on /m, but either I did
   it wrong or there's something else cause I wound up with the exact
   same single-bit error.  There is a branch-less form of this same code:

	srl     $16,1,$1
	and     $16,1,$2
	cmplt   $16,0,$3
	or      $1,$2,$2
	cmovge  $16,$16,$2
	itoft	$3,$f10
	itoft	$2,$f11
	cvtqs   $f11,$f11
	adds    $f11,$f11,$f0
	fcmoveq $f10,$f11,$f0

   I'm not using it because it's the same number of instructions as
   this branch-full form, and it has more serialized long latency
   instructions on the critical path.

   For DFmode, we can avoid rounding errors by breaking up the word
   into two pieces, converting them separately, and adding them back:

   LC0: .long 0,0x5f800000

	itoft	$16,$f11
	lda	$2,LC0
	cmplt	$16,0,$1
	cpyse	$f11,$f31,$f10
	cpyse	$f31,$f11,$f11
	s4addq	$1,$2,$1
	lds	$f12,0($1)
	cvtqt	$f10,$f10
	cvtqt	$f11,$f11
	addt	$f12,$f10,$f0
	addt	$f0,$f11,$f0

   This doesn't seem to be a clear-cut win over the optabs form.
   It probably all depends on the distribution of numbers being
   converted -- in the optabs form, all but high-bit-set has a
   much lower minimum execution time.  */

void
alpha_emit_floatuns (operands)
     rtx operands[2];
{
  rtx neglab, donelab, i0, i1, f0, in, out;
  enum machine_mode mode;

  out = operands[0];
  in = force_reg (DImode, operands[1]);
  mode = GET_MODE (out);
  neglab = gen_label_rtx ();
  donelab = gen_label_rtx ();
  i0 = gen_reg_rtx (DImode);
  i1 = gen_reg_rtx (DImode);
  f0 = gen_reg_rtx (mode);

  emit_cmp_and_jump_insns (in, const0_rtx, LT, const0_rtx, DImode, 0, neglab);

  emit_insn (gen_rtx_SET (VOIDmode, out, gen_rtx_FLOAT (mode, in)));
  emit_jump_insn (gen_jump (donelab));
  emit_barrier ();

  emit_label (neglab);

  emit_insn (gen_lshrdi3 (i0, in, const1_rtx));
  emit_insn (gen_anddi3 (i1, in, const1_rtx));
  emit_insn (gen_iordi3 (i0, i0, i1));
  emit_insn (gen_rtx_SET (VOIDmode, f0, gen_rtx_FLOAT (mode, i0)));
  emit_insn (gen_rtx_SET (VOIDmode, out, gen_rtx_PLUS (mode, f0, f0)));

  emit_label (donelab);
}

/* Generate the comparison for a conditional branch.  */

rtx
alpha_emit_conditional_branch (code)
     enum rtx_code code;
{
  enum rtx_code cmp_code, branch_code;
  enum machine_mode cmp_mode, branch_mode = VOIDmode;
  rtx op0 = alpha_compare.op0, op1 = alpha_compare.op1;
  rtx tem;

  if (alpha_compare.fp_p && GET_MODE (op0) == TFmode)
    {
      if (! TARGET_HAS_XFLOATING_LIBS)
	abort ();

      /* X_floating library comparison functions return
	   -1  unordered
	    0  false
	    1  true
	 Convert the compare against the raw return value.  */

      if (code == UNORDERED || code == ORDERED)
	cmp_code = EQ;
      else
	cmp_code = code;

      op0 = alpha_emit_xfloating_compare (cmp_code, op0, op1);
      op1 = const0_rtx;
      alpha_compare.fp_p = 0;

      if (code == UNORDERED)
	code = LT;
      else if (code == ORDERED)
	code = GE;
      else
        code = GT;
    }

  /* The general case: fold the comparison code to the types of compares
     that we have, choosing the branch as necessary.  */
  switch (code)
    {
    case EQ:  case LE:  case LT:  case LEU:  case LTU:
    case UNORDERED:
      /* We have these compares: */
      cmp_code = code, branch_code = NE;
      break;

    case NE:
    case ORDERED:
      /* These must be reversed.  */
      cmp_code = reverse_condition (code), branch_code = EQ;
      break;

    case GE:  case GT: case GEU:  case GTU:
      /* For FP, we swap them, for INT, we reverse them.  */
      if (alpha_compare.fp_p)
	{
	  cmp_code = swap_condition (code);
	  branch_code = NE;
	  tem = op0, op0 = op1, op1 = tem;
	}
      else
	{
	  cmp_code = reverse_condition (code);
	  branch_code = EQ;
	}
      break;

    default:
      abort ();
    }

  if (alpha_compare.fp_p)
    {
      cmp_mode = DFmode;
      if (flag_unsafe_math_optimizations)
	{
	  /* When we are not as concerned about non-finite values, and we
	     are comparing against zero, we can branch directly.  */
	  if (op1 == CONST0_RTX (DFmode))
	    cmp_code = NIL, branch_code = code;
	  else if (op0 == CONST0_RTX (DFmode))
	    {
	      /* Undo the swap we probably did just above.  */
	      tem = op0, op0 = op1, op1 = tem;
	      branch_code = swap_condition (cmp_code);
	      cmp_code = NIL;
	    }
	}
      else
	{
	  /* ??? We mark the the branch mode to be CCmode to prevent the
	     compare and branch from being combined, since the compare 
	     insn follows IEEE rules that the branch does not.  */
	  branch_mode = CCmode;
	}
    }
  else
    {
      cmp_mode = DImode;

      /* The following optimizations are only for signed compares.  */
      if (code != LEU && code != LTU && code != GEU && code != GTU)
	{
	  /* Whee.  Compare and branch against 0 directly.  */
	  if (op1 == const0_rtx)
	    cmp_code = NIL, branch_code = code;

	  /* We want to use cmpcc/bcc when we can, since there is a zero delay
	     bypass between logicals and br/cmov on EV5.  But we don't want to
	     force valid immediate constants into registers needlessly.  */
	  else if (GET_CODE (op1) == CONST_INT)
	    {
	      HOST_WIDE_INT v = INTVAL (op1), n = -v;

	      if (! CONST_OK_FOR_LETTER_P (v, 'I')
		  && (CONST_OK_FOR_LETTER_P (n, 'K')
		      || CONST_OK_FOR_LETTER_P (n, 'L')))
		{
		  cmp_code = PLUS, branch_code = code;
		  op1 = GEN_INT (n);
		}
	    }
	}

      if (!reg_or_0_operand (op0, DImode))
	op0 = force_reg (DImode, op0);
      if (cmp_code != PLUS && !reg_or_8bit_operand (op1, DImode))
	op1 = force_reg (DImode, op1);
    }

  /* Emit an initial compare instruction, if necessary.  */
  tem = op0;
  if (cmp_code != NIL)
    {
      tem = gen_reg_rtx (cmp_mode);
      emit_move_insn (tem, gen_rtx_fmt_ee (cmp_code, cmp_mode, op0, op1));
    }

  /* Zero the operands.  */
  memset (&alpha_compare, 0, sizeof (alpha_compare));

  /* Return the branch comparison.  */
  return gen_rtx_fmt_ee (branch_code, branch_mode, tem, CONST0_RTX (cmp_mode));
}

/* Certain simplifications can be done to make invalid setcc operations
   valid.  Return the final comparison, or NULL if we can't work.  */

rtx
alpha_emit_setcc (code)
     enum rtx_code code;
{
  enum rtx_code cmp_code;
  rtx op0 = alpha_compare.op0, op1 = alpha_compare.op1;
  int fp_p = alpha_compare.fp_p;
  rtx tmp;

  /* Zero the operands.  */
  memset (&alpha_compare, 0, sizeof (alpha_compare));

  if (fp_p && GET_MODE (op0) == TFmode)
    {
      if (! TARGET_HAS_XFLOATING_LIBS)
	abort ();

      /* X_floating library comparison functions return
	   -1  unordered
	    0  false
	    1  true
	 Convert the compare against the raw return value.  */

      if (code == UNORDERED || code == ORDERED)
	cmp_code = EQ;
      else
	cmp_code = code;

      op0 = alpha_emit_xfloating_compare (cmp_code, op0, op1);
      op1 = const0_rtx;
      fp_p = 0;

      if (code == UNORDERED)
	code = LT;
      else if (code == ORDERED)
	code = GE;
      else
        code = GT;
    }

  if (fp_p && !TARGET_FIX)
    return NULL_RTX;

  /* The general case: fold the comparison code to the types of compares
     that we have, choosing the branch as necessary.  */

  cmp_code = NIL;
  switch (code)
    {
    case EQ:  case LE:  case LT:  case LEU:  case LTU:
    case UNORDERED:
      /* We have these compares.  */
      if (fp_p)
	cmp_code = code, code = NE;
      break;

    case NE:
      if (!fp_p && op1 == const0_rtx)
	break;
      /* FALLTHRU */

    case ORDERED:
      cmp_code = reverse_condition (code);
      code = EQ;
      break;

    case GE:  case GT: case GEU:  case GTU:
      /* These normally need swapping, but for integer zero we have
	 special patterns that recognize swapped operands.  */
      if (!fp_p && op1 == const0_rtx)
	break;
      code = swap_condition (code);
      if (fp_p)
	cmp_code = code, code = NE;
      tmp = op0, op0 = op1, op1 = tmp;
      break;

    default:
      abort ();
    }

  if (!fp_p)
    {
      if (!register_operand (op0, DImode))
	op0 = force_reg (DImode, op0);
      if (!reg_or_8bit_operand (op1, DImode))
	op1 = force_reg (DImode, op1);
    }

  /* Emit an initial compare instruction, if necessary.  */
  if (cmp_code != NIL)
    {
      enum machine_mode mode = fp_p ? DFmode : DImode;

      tmp = gen_reg_rtx (mode);
      emit_insn (gen_rtx_SET (VOIDmode, tmp,
			      gen_rtx_fmt_ee (cmp_code, mode, op0, op1)));

      op0 = fp_p ? gen_lowpart (DImode, tmp) : tmp;
      op1 = const0_rtx;
    }

  /* Return the setcc comparison.  */
  return gen_rtx_fmt_ee (code, DImode, op0, op1);
}


/* Rewrite a comparison against zero CMP of the form
   (CODE (cc0) (const_int 0)) so it can be written validly in
   a conditional move (if_then_else CMP ...).
   If both of the operands that set cc0 are non-zero we must emit
   an insn to perform the compare (it can't be done within
   the conditional move).  */
rtx
alpha_emit_conditional_move (cmp, mode)
     rtx cmp;
     enum machine_mode mode;
{
  enum rtx_code code = GET_CODE (cmp);
  enum rtx_code cmov_code = NE;
  rtx op0 = alpha_compare.op0;
  rtx op1 = alpha_compare.op1;
  int fp_p = alpha_compare.fp_p;
  enum machine_mode cmp_mode
    = (GET_MODE (op0) == VOIDmode ? DImode : GET_MODE (op0));
  enum machine_mode cmp_op_mode = fp_p ? DFmode : DImode;
  enum machine_mode cmov_mode = VOIDmode;
  int local_fast_math = flag_unsafe_math_optimizations;
  rtx tem;

  /* Zero the operands.  */
  memset (&alpha_compare, 0, sizeof (alpha_compare));

  if (fp_p != FLOAT_MODE_P (mode))
    {
      enum rtx_code cmp_code;

      if (! TARGET_FIX)
	return 0;

      /* If we have fp<->int register move instructions, do a cmov by
	 performing the comparison in fp registers, and move the
	 zero/non-zero value to integer registers, where we can then
	 use a normal cmov, or vice-versa.  */

      switch (code)
	{
	case EQ: case LE: case LT: case LEU: case LTU:
	  /* We have these compares.  */
	  cmp_code = code, code = NE;
	  break;

	case NE:
	  /* This must be reversed.  */
	  cmp_code = EQ, code = EQ;
	  break;

	case GE: case GT: case GEU: case GTU:
	  /* These normally need swapping, but for integer zero we have
	     special patterns that recognize swapped operands.  */
	  if (!fp_p && op1 == const0_rtx)
	    cmp_code = code, code = NE;
	  else
	    {
	      cmp_code = swap_condition (code);
	      code = NE;
	      tem = op0, op0 = op1, op1 = tem;
	    }
	  break;

	default:
	  abort ();
	}

      tem = gen_reg_rtx (cmp_op_mode);
      emit_insn (gen_rtx_SET (VOIDmode, tem,
			      gen_rtx_fmt_ee (cmp_code, cmp_op_mode,
					      op0, op1)));

      cmp_mode = cmp_op_mode = fp_p ? DImode : DFmode;
      op0 = gen_lowpart (cmp_op_mode, tem);
      op1 = CONST0_RTX (cmp_op_mode);
      fp_p = !fp_p;
      local_fast_math = 1;
    }

  /* We may be able to use a conditional move directly.
     This avoids emitting spurious compares.  */
  if (signed_comparison_operator (cmp, VOIDmode)
      && (!fp_p || local_fast_math)
      && (op0 == CONST0_RTX (cmp_mode) || op1 == CONST0_RTX (cmp_mode)))
    return gen_rtx_fmt_ee (code, VOIDmode, op0, op1);

  /* We can't put the comparison inside the conditional move;
     emit a compare instruction and put that inside the
     conditional move.  Make sure we emit only comparisons we have;
     swap or reverse as necessary.  */

  if (no_new_pseudos)
    return NULL_RTX;

  switch (code)
    {
    case EQ:  case LE:  case LT:  case LEU:  case LTU:
      /* We have these compares: */
      break;

    case NE:
      /* This must be reversed.  */
      code = reverse_condition (code);
      cmov_code = EQ;
      break;

    case GE:  case GT:  case GEU:  case GTU:
      /* These must be swapped.  */
      if (op1 != CONST0_RTX (cmp_mode))
	{
	  code = swap_condition (code);
	  tem = op0, op0 = op1, op1 = tem;
	}
      break;

    default:
      abort ();
    }

  if (!fp_p)
    {
      if (!reg_or_0_operand (op0, DImode))
	op0 = force_reg (DImode, op0);
      if (!reg_or_8bit_operand (op1, DImode))
	op1 = force_reg (DImode, op1);
    }

  /* ??? We mark the branch mode to be CCmode to prevent the compare
     and cmov from being combined, since the compare insn follows IEEE
     rules that the cmov does not.  */
  if (fp_p && !local_fast_math)
    cmov_mode = CCmode;

  tem = gen_reg_rtx (cmp_op_mode);
  emit_move_insn (tem, gen_rtx_fmt_ee (code, cmp_op_mode, op0, op1));
  return gen_rtx_fmt_ee (cmov_code, cmov_mode, tem, CONST0_RTX (cmp_op_mode));
}

/* Simplify a conditional move of two constants into a setcc with
   arithmetic.  This is done with a splitter since combine would
   just undo the work if done during code generation.  It also catches
   cases we wouldn't have before cse.  */

int
alpha_split_conditional_move (code, dest, cond, t_rtx, f_rtx)
     enum rtx_code code;
     rtx dest, cond, t_rtx, f_rtx;
{
  HOST_WIDE_INT t, f, diff;
  enum machine_mode mode;
  rtx target, subtarget, tmp;

  mode = GET_MODE (dest);
  t = INTVAL (t_rtx);
  f = INTVAL (f_rtx);
  diff = t - f;

  if (((code == NE || code == EQ) && diff < 0)
      || (code == GE || code == GT))
    {
      code = reverse_condition (code);
      diff = t, t = f, f = diff;
      diff = t - f;
    }

  subtarget = target = dest;
  if (mode != DImode)
    {
      target = gen_lowpart (DImode, dest);
      if (! no_new_pseudos)
        subtarget = gen_reg_rtx (DImode);
      else
	subtarget = target;
    }
  /* Below, we must be careful to use copy_rtx on target and subtarget
     in intermediate insns, as they may be a subreg rtx, which may not
     be shared.  */

  if (f == 0 && exact_log2 (diff) > 0
      /* On EV6, we've got enough shifters to make non-arithmatic shifts
	 viable over a longer latency cmove.  On EV5, the E0 slot is a
	 scarce resource, and on EV4 shift has the same latency as a cmove.  */
      && (diff <= 8 || alpha_cpu == PROCESSOR_EV6))
    {
      tmp = gen_rtx_fmt_ee (code, DImode, cond, const0_rtx);
      emit_insn (gen_rtx_SET (VOIDmode, copy_rtx (subtarget), tmp));

      tmp = gen_rtx_ASHIFT (DImode, copy_rtx (subtarget),
			    GEN_INT (exact_log2 (t)));
      emit_insn (gen_rtx_SET (VOIDmode, target, tmp));
    }
  else if (f == 0 && t == -1)
    {
      tmp = gen_rtx_fmt_ee (code, DImode, cond, const0_rtx);
      emit_insn (gen_rtx_SET (VOIDmode, copy_rtx (subtarget), tmp));

      emit_insn (gen_negdi2 (target, copy_rtx (subtarget)));
    }
  else if (diff == 1 || diff == 4 || diff == 8)
    {
      rtx add_op;

      tmp = gen_rtx_fmt_ee (code, DImode, cond, const0_rtx);
      emit_insn (gen_rtx_SET (VOIDmode, copy_rtx (subtarget), tmp));

      if (diff == 1)
	emit_insn (gen_adddi3 (target, copy_rtx (subtarget), GEN_INT (f)));
      else
	{
	  add_op = GEN_INT (f);
	  if (sext_add_operand (add_op, mode))
	    {
	      tmp = gen_rtx_MULT (DImode, copy_rtx (subtarget),
				  GEN_INT (diff));
	      tmp = gen_rtx_PLUS (DImode, tmp, add_op);
	      emit_insn (gen_rtx_SET (VOIDmode, target, tmp));
	    }
	  else
	    return 0;
	}
    }
  else
    return 0;

  return 1;
}

/* Look up the function X_floating library function name for the
   given operation.  */

static const char *
alpha_lookup_xfloating_lib_func (code)
     enum rtx_code code;
{
  struct xfloating_op
    {
      const enum rtx_code code;
      const char *const func;
    };

  static const struct xfloating_op vms_xfloating_ops[] = 
    {
      { PLUS,		"OTS$ADD_X" },
      { MINUS,		"OTS$SUB_X" },
      { MULT,		"OTS$MUL_X" },
      { DIV,		"OTS$DIV_X" },
      { EQ,		"OTS$EQL_X" },
      { NE,		"OTS$NEQ_X" },
      { LT,		"OTS$LSS_X" },
      { LE,		"OTS$LEQ_X" },
      { GT,		"OTS$GTR_X" },
      { GE,		"OTS$GEQ_X" },
      { FIX,		"OTS$CVTXQ" },
      { FLOAT,		"OTS$CVTQX" },
      { UNSIGNED_FLOAT,	"OTS$CVTQUX" },
      { FLOAT_EXTEND,	"OTS$CVT_FLOAT_T_X" },
      { FLOAT_TRUNCATE,	"OTS$CVT_FLOAT_X_T" },
    };

  static const struct xfloating_op osf_xfloating_ops[] = 
    {
      { PLUS,		"_OtsAddX" },
      { MINUS,		"_OtsSubX" },
      { MULT,		"_OtsMulX" },
      { DIV,		"_OtsDivX" },
      { EQ,		"_OtsEqlX" },
      { NE,		"_OtsNeqX" },
      { LT,		"_OtsLssX" },
      { LE,		"_OtsLeqX" },
      { GT,		"_OtsGtrX" },
      { GE,		"_OtsGeqX" },
      { FIX,		"_OtsCvtXQ" },
      { FLOAT,		"_OtsCvtQX" },
      { UNSIGNED_FLOAT,	"_OtsCvtQUX" },
      { FLOAT_EXTEND,	"_OtsConvertFloatTX" },
      { FLOAT_TRUNCATE,	"_OtsConvertFloatXT" },
    };

  const struct xfloating_op *ops;
  const long n = ARRAY_SIZE (osf_xfloating_ops);
  long i;

  /* How irritating.  Nothing to key off for the table.  Hardcode
     knowledge of the G_floating routines.  */
  if (TARGET_FLOAT_VAX)
    {
      if (TARGET_ABI_OPEN_VMS)
	{
	  if (code == FLOAT_EXTEND)
	    return "OTS$CVT_FLOAT_G_X";
	  if (code == FLOAT_TRUNCATE)
	    return "OTS$CVT_FLOAT_X_G";
	}
      else
	{
	  if (code == FLOAT_EXTEND)
	    return "_OtsConvertFloatGX";
	  if (code == FLOAT_TRUNCATE)
	    return "_OtsConvertFloatXG";
	}
    }

  if (TARGET_ABI_OPEN_VMS)
    ops = vms_xfloating_ops;
  else
    ops = osf_xfloating_ops;

  for (i = 0; i < n; ++i)
    if (ops[i].code == code)
      return ops[i].func;

  abort();
}

/* Most X_floating operations take the rounding mode as an argument.
   Compute that here.  */

static int
alpha_compute_xfloating_mode_arg (code, round)
     enum rtx_code code;
     enum alpha_fp_rounding_mode round;
{
  int mode;

  switch (round)
    {
    case ALPHA_FPRM_NORM:
      mode = 2;
      break;
    case ALPHA_FPRM_MINF:
      mode = 1;
      break;
    case ALPHA_FPRM_CHOP:
      mode = 0;
      break;
    case ALPHA_FPRM_DYN:
      mode = 4;
      break;
    default:
      abort ();

    /* XXX For reference, round to +inf is mode = 3.  */
    }

  if (code == FLOAT_TRUNCATE && alpha_fptm == ALPHA_FPTM_N)
    mode |= 0x10000;

  return mode;
}

/* Emit an X_floating library function call.

   Note that these functions do not follow normal calling conventions:
   TFmode arguments are passed in two integer registers (as opposed to
   indirect); TFmode return values appear in R16+R17. 

   FUNC is the function name to call.
   TARGET is where the output belongs.
   OPERANDS are the inputs.
   NOPERANDS is the count of inputs.
   EQUIV is the expression equivalent for the function.
*/

static void
alpha_emit_xfloating_libcall (func, target, operands, noperands, equiv)
     const char *func;
     rtx target;
     rtx operands[];
     int noperands;
     rtx equiv;
{
  rtx usage = NULL_RTX, tmp, reg;
  int regno = 16, i;

  start_sequence ();

  for (i = 0; i < noperands; ++i)
    {
      switch (GET_MODE (operands[i]))
	{
	case TFmode:
	  reg = gen_rtx_REG (TFmode, regno);
	  regno += 2;
	  break;

	case DFmode:
	  reg = gen_rtx_REG (DFmode, regno + 32);
	  regno += 1;
	  break;

	case VOIDmode:
	  if (GET_CODE (operands[i]) != CONST_INT)
	    abort ();
	  /* FALLTHRU */
	case DImode:
	  reg = gen_rtx_REG (DImode, regno);
	  regno += 1;
	  break;

	default:
	  abort ();
	}

      emit_move_insn (reg, operands[i]);
      usage = alloc_EXPR_LIST (0, gen_rtx_USE (VOIDmode, reg), usage);
    }

  switch (GET_MODE (target))
    {
    case TFmode:
      reg = gen_rtx_REG (TFmode, 16);
      break;
    case DFmode:
      reg = gen_rtx_REG (DFmode, 32);
      break;
    case DImode:
      reg = gen_rtx_REG (DImode, 0);
      break;
    default:
      abort ();
    }

  tmp = gen_rtx_MEM (QImode, gen_rtx_SYMBOL_REF (Pmode, (char *) func));
  tmp = emit_call_insn (GEN_CALL_VALUE (reg, tmp, const0_rtx,
					const0_rtx, const0_rtx));
  CALL_INSN_FUNCTION_USAGE (tmp) = usage;

  tmp = get_insns ();
  end_sequence ();

  emit_libcall_block (tmp, target, reg, equiv);
}

/* Emit an X_floating library function call for arithmetic (+,-,*,/).  */

void
alpha_emit_xfloating_arith (code, operands)
     enum rtx_code code;
     rtx operands[];
{
  const char *func;
  int mode;
  rtx out_operands[3];

  func = alpha_lookup_xfloating_lib_func (code);
  mode = alpha_compute_xfloating_mode_arg (code, alpha_fprm);

  out_operands[0] = operands[1];
  out_operands[1] = operands[2];
  out_operands[2] = GEN_INT (mode);
  alpha_emit_xfloating_libcall (func, operands[0], out_operands, 3,  
				gen_rtx_fmt_ee (code, TFmode, operands[1],
						operands[2]));
}

/* Emit an X_floating library function call for a comparison.  */

static rtx
alpha_emit_xfloating_compare (code, op0, op1)
     enum rtx_code code;
     rtx op0, op1;
{
  const char *func;
  rtx out, operands[2];

  func = alpha_lookup_xfloating_lib_func (code);

  operands[0] = op0;
  operands[1] = op1;
  out = gen_reg_rtx (DImode);

  /* ??? Strange mode for equiv because what's actually returned
     is -1,0,1, not a proper boolean value.  */
  alpha_emit_xfloating_libcall (func, out, operands, 2,
				gen_rtx_fmt_ee (code, CCmode, op0, op1));

  return out;
}

/* Emit an X_floating library function call for a conversion.  */

void
alpha_emit_xfloating_cvt (code, operands)
     enum rtx_code code;
     rtx operands[];
{
  int noperands = 1, mode;
  rtx out_operands[2];
  const char *func;

  func = alpha_lookup_xfloating_lib_func (code);

  out_operands[0] = operands[1];

  switch (code)
    {
    case FIX:
      mode = alpha_compute_xfloating_mode_arg (code, ALPHA_FPRM_CHOP);
      out_operands[1] = GEN_INT (mode);
      noperands = 2;
      break;
    case FLOAT_TRUNCATE:
      mode = alpha_compute_xfloating_mode_arg (code, alpha_fprm);
      out_operands[1] = GEN_INT (mode);
      noperands = 2;
      break;
    default:
      break;
    }

  alpha_emit_xfloating_libcall (func, operands[0], out_operands, noperands,
				gen_rtx_fmt_e (code, GET_MODE (operands[0]),
					       operands[1]));
}

/* Split a TFmode OP[1] into DImode OP[2,3] and likewise for
   OP[0] into OP[0,1].  Naturally, output operand ordering is
   little-endian.  */

void
alpha_split_tfmode_pair (operands)
     rtx operands[4];
{
  if (GET_CODE (operands[1]) == REG)
    {
      operands[3] = gen_rtx_REG (DImode, REGNO (operands[1]) + 1);
      operands[2] = gen_rtx_REG (DImode, REGNO (operands[1]));
    }
  else if (GET_CODE (operands[1]) == MEM)
    {
      operands[3] = adjust_address (operands[1], DImode, 8);
      operands[2] = adjust_address (operands[1], DImode, 0);
    }
  else if (operands[1] == CONST0_RTX (TFmode))
    operands[2] = operands[3] = const0_rtx;
  else
    abort ();

  if (GET_CODE (operands[0]) == REG)
    {
      operands[1] = gen_rtx_REG (DImode, REGNO (operands[0]) + 1);
      operands[0] = gen_rtx_REG (DImode, REGNO (operands[0]));
    }
  else if (GET_CODE (operands[0]) == MEM)
    {
      operands[1] = adjust_address (operands[0], DImode, 8);
      operands[0] = adjust_address (operands[0], DImode, 0);
    }
  else
    abort ();
}

/* Implement negtf2 or abstf2.  Op0 is destination, op1 is source, 
   op2 is a register containing the sign bit, operation is the 
   logical operation to be performed.  */

void
alpha_split_tfmode_frobsign (operands, operation)
     rtx operands[3];
     rtx (*operation) PARAMS ((rtx, rtx, rtx));
{
  rtx high_bit = operands[2];
  rtx scratch;
  int move;

  alpha_split_tfmode_pair (operands);

  /* Detect three flavours of operand overlap.  */
  move = 1;
  if (rtx_equal_p (operands[0], operands[2]))
    move = 0;
  else if (rtx_equal_p (operands[1], operands[2]))
    {
      if (rtx_equal_p (operands[0], high_bit))
	move = 2;
      else
	move = -1;
    }

  if (move < 0)
    emit_move_insn (operands[0], operands[2]);

  /* ??? If the destination overlaps both source tf and high_bit, then
     assume source tf is dead in its entirety and use the other half
     for a scratch register.  Otherwise "scratch" is just the proper
     destination register.  */
  scratch = operands[move < 2 ? 1 : 3];

  emit_insn ((*operation) (scratch, high_bit, operands[3]));

  if (move > 0)
    {
      emit_move_insn (operands[0], operands[2]);
      if (move > 1)
	emit_move_insn (operands[1], scratch);
    }
}

/* Use ext[wlq][lh] as the Architecture Handbook describes for extracting
   unaligned data:

           unsigned:                       signed:
   word:   ldq_u  r1,X(r11)                ldq_u  r1,X(r11)
           ldq_u  r2,X+1(r11)              ldq_u  r2,X+1(r11)
           lda    r3,X(r11)                lda    r3,X+2(r11)
           extwl  r1,r3,r1                 extql  r1,r3,r1
           extwh  r2,r3,r2                 extqh  r2,r3,r2
           or     r1.r2.r1                 or     r1,r2,r1
                                           sra    r1,48,r1

   long:   ldq_u  r1,X(r11)                ldq_u  r1,X(r11)
           ldq_u  r2,X+3(r11)              ldq_u  r2,X+3(r11)
           lda    r3,X(r11)                lda    r3,X(r11)
           extll  r1,r3,r1                 extll  r1,r3,r1
           extlh  r2,r3,r2                 extlh  r2,r3,r2
           or     r1.r2.r1                 addl   r1,r2,r1

   quad:   ldq_u  r1,X(r11)
           ldq_u  r2,X+7(r11)
           lda    r3,X(r11)
           extql  r1,r3,r1
           extqh  r2,r3,r2
           or     r1.r2.r1
*/

void
alpha_expand_unaligned_load (tgt, mem, size, ofs, sign)
     rtx tgt, mem;
     HOST_WIDE_INT size, ofs;
     int sign;
{
  rtx meml, memh, addr, extl, exth, tmp, mema;
  enum machine_mode mode;

  meml = gen_reg_rtx (DImode);
  memh = gen_reg_rtx (DImode);
  addr = gen_reg_rtx (DImode);
  extl = gen_reg_rtx (DImode);
  exth = gen_reg_rtx (DImode);

  mema = XEXP (mem, 0);
  if (GET_CODE (mema) == LO_SUM)
    mema = force_reg (Pmode, mema);

  /* AND addresses cannot be in any alias set, since they may implicitly
     alias surrounding code.  Ideally we'd have some alias set that 
     covered all types except those with alignment 8 or higher.  */

  tmp = change_address (mem, DImode,
			gen_rtx_AND (DImode, 
				     plus_constant (mema, ofs),
				     GEN_INT (-8)));
  set_mem_alias_set (tmp, 0);
  emit_move_insn (meml, tmp);

  tmp = change_address (mem, DImode,
			gen_rtx_AND (DImode, 
				     plus_constant (mema, ofs + size - 1),
				     GEN_INT (-8)));
  set_mem_alias_set (tmp, 0);
  emit_move_insn (memh, tmp);

  if (WORDS_BIG_ENDIAN && sign && (size == 2 || size == 4))
    {
      emit_move_insn (addr, plus_constant (mema, -1));

      emit_insn (gen_extqh_be (extl, meml, addr));
      emit_insn (gen_extxl_be (exth, memh, GEN_INT (64), addr));

      addr = expand_binop (DImode, ior_optab, extl, exth, tgt, 1, OPTAB_WIDEN);
      addr = expand_binop (DImode, ashr_optab, addr, GEN_INT (64 - size*8),
			   addr, 1, OPTAB_WIDEN);
    }
  else if (sign && size == 2)
    {
      emit_move_insn (addr, plus_constant (mema, ofs+2));

      emit_insn (gen_extxl_le (extl, meml, GEN_INT (64), addr));
      emit_insn (gen_extqh_le (exth, memh, addr));

      /* We must use tgt here for the target.  Alpha-vms port fails if we use
	 addr for the target, because addr is marked as a pointer and combine
	 knows that pointers are always sign-extended 32 bit values.  */
      addr = expand_binop (DImode, ior_optab, extl, exth, tgt, 1, OPTAB_WIDEN);
      addr = expand_binop (DImode, ashr_optab, addr, GEN_INT (48), 
			   addr, 1, OPTAB_WIDEN);
    }
  else
    {
      if (WORDS_BIG_ENDIAN)
	{
	  emit_move_insn (addr, plus_constant (mema, ofs+size-1));
	  switch ((int) size)
	    {
	    case 2:
	      emit_insn (gen_extwh_be (extl, meml, addr));
	      mode = HImode;
	      break;

	    case 4:
	      emit_insn (gen_extlh_be (extl, meml, addr));
	      mode = SImode;
	      break;

	    case 8:
	      emit_insn (gen_extqh_be (extl, meml, addr));
	      mode = DImode;
	      break;

	    default:
	      abort ();
	    }
	  emit_insn (gen_extxl_be (exth, memh, GEN_INT (size*8), addr));
	}
      else
	{
	  emit_move_insn (addr, plus_constant (mema, ofs));
	  emit_insn (gen_extxl_le (extl, meml, GEN_INT (size*8), addr));
	  switch ((int) size)
	    {
	    case 2:
	      emit_insn (gen_extwh_le (exth, memh, addr));
	      mode = HImode;
	      break;

	    case 4:
	      emit_insn (gen_extlh_le (exth, memh, addr));
	      mode = SImode;
	      break;

	    case 8:
	      emit_insn (gen_extqh_le (exth, memh, addr));
	      mode = DImode;
	      break;

	    default:
	      abort();
	    }
	}

      addr = expand_binop (mode, ior_optab, gen_lowpart (mode, extl),
			   gen_lowpart (mode, exth), gen_lowpart (mode, tgt),
			   sign, OPTAB_WIDEN);
    }

  if (addr != tgt)
    emit_move_insn (tgt, gen_lowpart(GET_MODE (tgt), addr));
}

/* Similarly, use ins and msk instructions to perform unaligned stores.  */

void
alpha_expand_unaligned_store (dst, src, size, ofs)
     rtx dst, src;
     HOST_WIDE_INT size, ofs;
{
  rtx dstl, dsth, addr, insl, insh, meml, memh, dsta;
  
  dstl = gen_reg_rtx (DImode);
  dsth = gen_reg_rtx (DImode);
  insl = gen_reg_rtx (DImode);
  insh = gen_reg_rtx (DImode);

  dsta = XEXP (dst, 0);
  if (GET_CODE (dsta) == LO_SUM)
    dsta = force_reg (Pmode, dsta);

  /* AND addresses cannot be in any alias set, since they may implicitly
     alias surrounding code.  Ideally we'd have some alias set that 
     covered all types except those with alignment 8 or higher.  */

  meml = change_address (dst, DImode,
			 gen_rtx_AND (DImode, 
				      plus_constant (dsta, ofs),
				      GEN_INT (-8)));
  set_mem_alias_set (meml, 0);

  memh = change_address (dst, DImode,
			 gen_rtx_AND (DImode, 
				      plus_constant (dsta, ofs + size - 1),
				      GEN_INT (-8)));
  set_mem_alias_set (memh, 0);

  emit_move_insn (dsth, memh);
  emit_move_insn (dstl, meml);
  if (WORDS_BIG_ENDIAN)
    {
      addr = copy_addr_to_reg (plus_constant (dsta, ofs+size-1));

      if (src != const0_rtx)
	{
	  switch ((int) size)
	    {
	    case 2:
	      emit_insn (gen_inswl_be (insh, gen_lowpart (HImode,src), addr));
	      break;
	    case 4:
	      emit_insn (gen_insll_be (insh, gen_lowpart (SImode,src), addr));
	      break;
	    case 8:
	      emit_insn (gen_insql_be (insh, gen_lowpart (DImode,src), addr));
	      break;
	    }
	  emit_insn (gen_insxh (insl, gen_lowpart (DImode, src),
				GEN_INT (size*8), addr));
	}

      switch ((int) size)
	{
	case 2:
	  emit_insn (gen_mskxl_be (dsth, dsth, GEN_INT (0xffff), addr));
	  break;
	case 4:
	  emit_insn (gen_mskxl_be (dsth, dsth, GEN_INT (0xffffffff), addr));
	  break;
	case 8:
	  {
#if HOST_BITS_PER_WIDE_INT == 32
	    rtx msk = immed_double_const (0xffffffff, 0xffffffff, DImode);
#else
	    rtx msk = constm1_rtx;
#endif
	    emit_insn (gen_mskxl_be (dsth, dsth, msk, addr));
	  }
	  break;
	}

      emit_insn (gen_mskxh (dstl, dstl, GEN_INT (size*8), addr));
    }
  else
    {
      addr = copy_addr_to_reg (plus_constant (dsta, ofs));

      if (src != const0_rtx)
	{
	  emit_insn (gen_insxh (insh, gen_lowpart (DImode, src),
				GEN_INT (size*8), addr));

	  switch ((int) size)
	    {
	    case 2:
	      emit_insn (gen_inswl_le (insl, gen_lowpart (HImode, src), addr));
	      break;
	    case 4:
	      emit_insn (gen_insll_le (insl, gen_lowpart (SImode, src), addr));
	      break;
	    case 8:
	      emit_insn (gen_insql_le (insl, src, addr));
	      break;
	    }
	}

      emit_insn (gen_mskxh (dsth, dsth, GEN_INT (size*8), addr));

      switch ((int) size)
	{
	case 2:
	  emit_insn (gen_mskxl_le (dstl, dstl, GEN_INT (0xffff), addr));
	  break;
	case 4:
	  emit_insn (gen_mskxl_le (dstl, dstl, GEN_INT (0xffffffff), addr));
	  break;
	case 8:
	  {
#if HOST_BITS_PER_WIDE_INT == 32
	    rtx msk = immed_double_const (0xffffffff, 0xffffffff, DImode);
#else
	    rtx msk = constm1_rtx;
#endif
	    emit_insn (gen_mskxl_le (dstl, dstl, msk, addr));
	  }
	  break;
	}
    }

  if (src != const0_rtx)
    {
      dsth = expand_binop (DImode, ior_optab, insh, dsth, dsth, 0, OPTAB_WIDEN);
      dstl = expand_binop (DImode, ior_optab, insl, dstl, dstl, 0, OPTAB_WIDEN);
    }
 
  if (WORDS_BIG_ENDIAN)
    {
      emit_move_insn (meml, dstl);
      emit_move_insn (memh, dsth);
    }
  else
    {
      /* Must store high before low for degenerate case of aligned.  */
      emit_move_insn (memh, dsth);
      emit_move_insn (meml, dstl);
    }
}

/* The block move code tries to maximize speed by separating loads and
   stores at the expense of register pressure: we load all of the data
   before we store it back out.  There are two secondary effects worth
   mentioning, that this speeds copying to/from aligned and unaligned
   buffers, and that it makes the code significantly easier to write.  */

#define MAX_MOVE_WORDS	8

/* Load an integral number of consecutive unaligned quadwords.  */

static void
alpha_expand_unaligned_load_words (out_regs, smem, words, ofs)
     rtx *out_regs;
     rtx smem;
     HOST_WIDE_INT words, ofs;
{
  rtx const im8 = GEN_INT (-8);
  rtx const i64 = GEN_INT (64);
  rtx ext_tmps[MAX_MOVE_WORDS], data_regs[MAX_MOVE_WORDS+1];
  rtx sreg, areg, tmp, smema;
  HOST_WIDE_INT i;

  smema = XEXP (smem, 0);
  if (GET_CODE (smema) == LO_SUM)
    smema = force_reg (Pmode, smema);

  /* Generate all the tmp registers we need.  */
  for (i = 0; i < words; ++i)
    {
      data_regs[i] = out_regs[i];
      ext_tmps[i] = gen_reg_rtx (DImode);
    }
  data_regs[words] = gen_reg_rtx (DImode);

  if (ofs != 0)
    smem = adjust_address (smem, GET_MODE (smem), ofs);
  
  /* Load up all of the source data.  */
  for (i = 0; i < words; ++i)
    {
      tmp = change_address (smem, DImode,
			    gen_rtx_AND (DImode,
					 plus_constant (smema, 8*i),
					 im8));
      set_mem_alias_set (tmp, 0);
      emit_move_insn (data_regs[i], tmp);
    }

  tmp = change_address (smem, DImode,
			gen_rtx_AND (DImode,
				     plus_constant (smema, 8*words - 1),
				     im8));
  set_mem_alias_set (tmp, 0);
  emit_move_insn (data_regs[words], tmp);

  /* Extract the half-word fragments.  Unfortunately DEC decided to make
     extxh with offset zero a noop instead of zeroing the register, so 
     we must take care of that edge condition ourselves with cmov.  */

  sreg = copy_addr_to_reg (smema);
  areg = expand_binop (DImode, and_optab, sreg, GEN_INT (7), NULL, 
		       1, OPTAB_WIDEN);
  if (WORDS_BIG_ENDIAN)
    emit_move_insn (sreg, plus_constant (sreg, 7));
  for (i = 0; i < words; ++i)
    {
      if (WORDS_BIG_ENDIAN)
	{
	  emit_insn (gen_extqh_be (data_regs[i], data_regs[i], sreg));
	  emit_insn (gen_extxl_be (ext_tmps[i], data_regs[i+1], i64, sreg));
	}
      else
	{
	  emit_insn (gen_extxl_le (data_regs[i], data_regs[i], i64, sreg));
	  emit_insn (gen_extqh_le (ext_tmps[i], data_regs[i+1], sreg));
	}
      emit_insn (gen_rtx_SET (VOIDmode, ext_tmps[i],
			      gen_rtx_IF_THEN_ELSE (DImode,
						    gen_rtx_EQ (DImode, areg,
								const0_rtx),
						    const0_rtx, ext_tmps[i])));
    }

  /* Merge the half-words into whole words.  */
  for (i = 0; i < words; ++i)
    {
      out_regs[i] = expand_binop (DImode, ior_optab, data_regs[i],
				  ext_tmps[i], data_regs[i], 1, OPTAB_WIDEN);
    }
}

/* Store an integral number of consecutive unaligned quadwords.  DATA_REGS
   may be NULL to store zeros.  */

static void
alpha_expand_unaligned_store_words (data_regs, dmem, words, ofs)
     rtx *data_regs;
     rtx dmem;
     HOST_WIDE_INT words, ofs;
{
  rtx const im8 = GEN_INT (-8);
  rtx const i64 = GEN_INT (64);
#if HOST_BITS_PER_WIDE_INT == 32
  rtx const im1 = immed_double_const (0xffffffff, 0xffffffff, DImode);
#else
  rtx const im1 = constm1_rtx;
#endif
  rtx ins_tmps[MAX_MOVE_WORDS];
  rtx st_tmp_1, st_tmp_2, dreg;
  rtx st_addr_1, st_addr_2, dmema;
  HOST_WIDE_INT i;

  dmema = XEXP (dmem, 0);
  if (GET_CODE (dmema) == LO_SUM)
    dmema = force_reg (Pmode, dmema);

  /* Generate all the tmp registers we need.  */
  if (data_regs != NULL)
    for (i = 0; i < words; ++i)
      ins_tmps[i] = gen_reg_rtx(DImode);
  st_tmp_1 = gen_reg_rtx(DImode);
  st_tmp_2 = gen_reg_rtx(DImode);
  
  if (ofs != 0)
    dmem = adjust_address (dmem, GET_MODE (dmem), ofs);

  st_addr_2 = change_address (dmem, DImode,
			      gen_rtx_AND (DImode,
					   plus_constant (dmema, words*8 - 1),
				       im8));
  set_mem_alias_set (st_addr_2, 0);

  st_addr_1 = change_address (dmem, DImode,
			      gen_rtx_AND (DImode, dmema, im8));
  set_mem_alias_set (st_addr_1, 0);

  /* Load up the destination end bits.  */
  emit_move_insn (st_tmp_2, st_addr_2);
  emit_move_insn (st_tmp_1, st_addr_1);

  /* Shift the input data into place.  */
  dreg = copy_addr_to_reg (dmema);
  if (WORDS_BIG_ENDIAN)
    emit_move_insn (dreg, plus_constant (dreg, 7));
  if (data_regs != NULL)
    {
      for (i = words-1; i >= 0; --i)
	{
	  if (WORDS_BIG_ENDIAN)
	    {
	      emit_insn (gen_insql_be (ins_tmps[i], data_regs[i], dreg));
	      emit_insn (gen_insxh (data_regs[i], data_regs[i], i64, dreg));
	    }
	  else
	    {
	      emit_insn (gen_insxh (ins_tmps[i], data_regs[i], i64, dreg));
	      emit_insn (gen_insql_le (data_regs[i], data_regs[i], dreg));
	    }
	}
      for (i = words-1; i > 0; --i)
	{
	  ins_tmps[i-1] = expand_binop (DImode, ior_optab, data_regs[i],
					ins_tmps[i-1], ins_tmps[i-1], 1,
					OPTAB_WIDEN);
	}
    }

  /* Split and merge the ends with the destination data.  */
  if (WORDS_BIG_ENDIAN)
    {
      emit_insn (gen_mskxl_be (st_tmp_2, st_tmp_2, im1, dreg));
      emit_insn (gen_mskxh (st_tmp_1, st_tmp_1, i64, dreg));
    }
  else
    {
      emit_insn (gen_mskxh (st_tmp_2, st_tmp_2, i64, dreg));
      emit_insn (gen_mskxl_le (st_tmp_1, st_tmp_1, im1, dreg));
    }

  if (data_regs != NULL)
    {
      st_tmp_2 = expand_binop (DImode, ior_optab, st_tmp_2, ins_tmps[words-1],
			       st_tmp_2, 1, OPTAB_WIDEN);
      st_tmp_1 = expand_binop (DImode, ior_optab, st_tmp_1, data_regs[0],
			       st_tmp_1, 1, OPTAB_WIDEN);
    }

  /* Store it all.  */
  if (WORDS_BIG_ENDIAN)
    emit_move_insn (st_addr_1, st_tmp_1);
  else
    emit_move_insn (st_addr_2, st_tmp_2);
  for (i = words-1; i > 0; --i)
    {
      rtx tmp = change_address (dmem, DImode,
				gen_rtx_AND (DImode,
					     plus_constant(dmema,
					     WORDS_BIG_ENDIAN ? i*8-1 : i*8),
					     im8));
      set_mem_alias_set (tmp, 0);
      emit_move_insn (tmp, data_regs ? ins_tmps[i-1] : const0_rtx);
    }
  if (WORDS_BIG_ENDIAN)
    emit_move_insn (st_addr_2, st_tmp_2);
  else
    emit_move_insn (st_addr_1, st_tmp_1);
}


/* Expand string/block move operations.

   operands[0] is the pointer to the destination.
   operands[1] is the pointer to the source.
   operands[2] is the number of bytes to move.
   operands[3] is the alignment.  */

int
alpha_expand_block_move (operands)
     rtx operands[];
{
  rtx bytes_rtx	= operands[2];
  rtx align_rtx = operands[3];
  HOST_WIDE_INT orig_bytes = INTVAL (bytes_rtx);
  HOST_WIDE_INT bytes = orig_bytes;
  HOST_WIDE_INT src_align = INTVAL (align_rtx) * BITS_PER_UNIT;
  HOST_WIDE_INT dst_align = src_align;
  rtx orig_src = operands[1];
  rtx orig_dst = operands[0];
  rtx data_regs[2 * MAX_MOVE_WORDS + 16];
  rtx tmp;
  unsigned int i, words, ofs, nregs = 0;
  
  if (orig_bytes <= 0)
    return 1;
  else if (orig_bytes > MAX_MOVE_WORDS * UNITS_PER_WORD)
    return 0;

  /* Look for additional alignment information from recorded register info.  */

  tmp = XEXP (orig_src, 0);
  if (GET_CODE (tmp) == REG)
    src_align = MAX (src_align, REGNO_POINTER_ALIGN (REGNO (tmp)));
  else if (GET_CODE (tmp) == PLUS
	   && GET_CODE (XEXP (tmp, 0)) == REG
	   && GET_CODE (XEXP (tmp, 1)) == CONST_INT)
    {
      unsigned HOST_WIDE_INT c = INTVAL (XEXP (tmp, 1));
      unsigned int a = REGNO_POINTER_ALIGN (REGNO (XEXP (tmp, 0)));

      if (a > src_align)
	{
          if (a >= 64 && c % 8 == 0)
	    src_align = 64;
          else if (a >= 32 && c % 4 == 0)
	    src_align = 32;
          else if (a >= 16 && c % 2 == 0)
	    src_align = 16;
	}
    }
	
  tmp = XEXP (orig_dst, 0);
  if (GET_CODE (tmp) == REG)
    dst_align = MAX (dst_align, REGNO_POINTER_ALIGN (REGNO (tmp)));
  else if (GET_CODE (tmp) == PLUS
	   && GET_CODE (XEXP (tmp, 0)) == REG
	   && GET_CODE (XEXP (tmp, 1)) == CONST_INT)
    {
      unsigned HOST_WIDE_INT c = INTVAL (XEXP (tmp, 1));
      unsigned int a = REGNO_POINTER_ALIGN (REGNO (XEXP (tmp, 0)));

      if (a > dst_align)
	{
          if (a >= 64 && c % 8 == 0)
	    dst_align = 64;
          else if (a >= 32 && c % 4 == 0)
	    dst_align = 32;
          else if (a >= 16 && c % 2 == 0)
	    dst_align = 16;
	}
    }

  /* Load the entire block into registers.  */
  if (GET_CODE (XEXP (orig_src, 0)) == ADDRESSOF)
    {
      enum machine_mode mode;

      tmp = XEXP (XEXP (orig_src, 0), 0);

      /* Don't use the existing register if we're reading more than
	 is held in the register.  Nor if there is not a mode that
	 handles the exact size.  */
      mode = mode_for_size (bytes * BITS_PER_UNIT, MODE_INT, 1);
      if (mode != BLKmode
	  && GET_MODE_SIZE (GET_MODE (tmp)) >= bytes)
	{
	  if (mode == TImode)
	    {
	      data_regs[nregs] = gen_lowpart (DImode, tmp);
	      data_regs[nregs + 1] = gen_highpart (DImode, tmp);
	      nregs += 2;
	    }
	  else
	    data_regs[nregs++] = gen_lowpart (mode, tmp);

	  goto src_done;
	}

      /* No appropriate mode; fall back on memory.  */
      orig_src = replace_equiv_address (orig_src,
					copy_addr_to_reg (XEXP (orig_src, 0)));
      src_align = GET_MODE_BITSIZE (GET_MODE (tmp));
    }

  ofs = 0;
  if (src_align >= 64 && bytes >= 8)
    {
      words = bytes / 8;

      for (i = 0; i < words; ++i)
	data_regs[nregs + i] = gen_reg_rtx (DImode);

      for (i = 0; i < words; ++i)
	emit_move_insn (data_regs[nregs + i],
			adjust_address (orig_src, DImode, ofs + i * 8));

      nregs += words;
      bytes -= words * 8;
      ofs += words * 8;
    }

  if (src_align >= 32 && bytes >= 4)
    {
      words = bytes / 4;

      for (i = 0; i < words; ++i)
	data_regs[nregs + i] = gen_reg_rtx (SImode);

      for (i = 0; i < words; ++i)
	emit_move_insn (data_regs[nregs + i],
			adjust_address (orig_src, SImode, ofs + i * 4));

      nregs += words;
      bytes -= words * 4;
      ofs += words * 4;
    }

  if (bytes >= 8)
    {
      words = bytes / 8;

      for (i = 0; i < words+1; ++i)
	data_regs[nregs + i] = gen_reg_rtx (DImode);

      alpha_expand_unaligned_load_words (data_regs + nregs, orig_src,
					 words, ofs);

      nregs += words;
      bytes -= words * 8;
      ofs += words * 8;
    }

  if (! TARGET_BWX && bytes >= 4)
    {
      data_regs[nregs++] = tmp = gen_reg_rtx (SImode);
      alpha_expand_unaligned_load (tmp, orig_src, 4, ofs, 0);
      bytes -= 4;
      ofs += 4;
    }

  if (bytes >= 2)
    {
      if (src_align >= 16)
	{
	  do {
	    data_regs[nregs++] = tmp = gen_reg_rtx (HImode);
	    emit_move_insn (tmp, adjust_address (orig_src, HImode, ofs));
	    bytes -= 2;
	    ofs += 2;
	  } while (bytes >= 2);
	}
      else if (! TARGET_BWX)
	{
	  data_regs[nregs++] = tmp = gen_reg_rtx (HImode);
	  alpha_expand_unaligned_load (tmp, orig_src, 2, ofs, 0);
	  bytes -= 2;
	  ofs += 2;
	}
    }

  while (bytes > 0)
    {
      data_regs[nregs++] = tmp = gen_reg_rtx (QImode);
      emit_move_insn (tmp, adjust_address (orig_src, QImode, ofs));
      bytes -= 1;
      ofs += 1;
    }

 src_done:

  if (nregs > ARRAY_SIZE (data_regs))
    abort ();

  /* Now save it back out again.  */

  i = 0, ofs = 0;

  if (GET_CODE (XEXP (orig_dst, 0)) == ADDRESSOF)
    {
      enum machine_mode mode;
      tmp = XEXP (XEXP (orig_dst, 0), 0);

      mode = mode_for_size (orig_bytes * BITS_PER_UNIT, MODE_INT, 1);
      if (GET_MODE (tmp) == mode)
	{
	  if (nregs == 1)
	    {
	      emit_move_insn (tmp, data_regs[0]);
	      i = 1;
	      goto dst_done;
	    }

	  else if (nregs == 2 && mode == TImode)
	    {
	      /* Undo the subregging done above when copying between
		 two TImode registers.  */
	      if (GET_CODE (data_regs[0]) == SUBREG
		  && GET_MODE (SUBREG_REG (data_regs[0])) == TImode)
		emit_move_insn (tmp, SUBREG_REG (data_regs[0]));
	      else
		{
		  rtx seq;

		  start_sequence ();
		  emit_move_insn (gen_lowpart (DImode, tmp), data_regs[0]);
		  emit_move_insn (gen_highpart (DImode, tmp), data_regs[1]);
		  seq = get_insns ();
		  end_sequence ();

		  emit_no_conflict_block (seq, tmp, data_regs[0],
					  data_regs[1], NULL_RTX);
		}

	      i = 2;
	      goto dst_done;
	    }
	}

      /* ??? If nregs > 1, consider reconstructing the word in regs.  */
      /* ??? Optimize mode < dst_mode with strict_low_part.  */

      /* No appropriate mode; fall back on memory.  We can speed things
	 up by recognizing extra alignment information.  */
      orig_dst = replace_equiv_address (orig_dst,
					copy_addr_to_reg (XEXP (orig_dst, 0)));
      dst_align = GET_MODE_BITSIZE (GET_MODE (tmp));
    }

  /* Write out the data in whatever chunks reading the source allowed.  */
  if (dst_align >= 64)
    {
      while (i < nregs && GET_MODE (data_regs[i]) == DImode)
	{
	  emit_move_insn (adjust_address (orig_dst, DImode, ofs),
			  data_regs[i]);
	  ofs += 8;
	  i++;
	}
    }

  if (dst_align >= 32)
    {
      /* If the source has remaining DImode regs, write them out in
	 two pieces.  */
      while (i < nregs && GET_MODE (data_regs[i]) == DImode)
	{
	  tmp = expand_binop (DImode, lshr_optab, data_regs[i], GEN_INT (32),
			      NULL_RTX, 1, OPTAB_WIDEN);

	  emit_move_insn (adjust_address (orig_dst, SImode, ofs),
			  gen_lowpart (SImode, data_regs[i]));
	  emit_move_insn (adjust_address (orig_dst, SImode, ofs + 4),
			  gen_lowpart (SImode, tmp));
	  ofs += 8;
	  i++;
	}

      while (i < nregs && GET_MODE (data_regs[i]) == SImode)
	{
	  emit_move_insn (adjust_address (orig_dst, SImode, ofs),
			  data_regs[i]);
	  ofs += 4;
	  i++;
	}
    }

  if (i < nregs && GET_MODE (data_regs[i]) == DImode)
    {
      /* Write out a remaining block of words using unaligned methods.  */

      for (words = 1; i + words < nregs; words++)
	if (GET_MODE (data_regs[i + words]) != DImode)
	  break;

      if (words == 1)
	alpha_expand_unaligned_store (orig_dst, data_regs[i], 8, ofs);
      else
        alpha_expand_unaligned_store_words (data_regs + i, orig_dst,
					    words, ofs);
     
      i += words;
      ofs += words * 8;
    }

  /* Due to the above, this won't be aligned.  */
  /* ??? If we have more than one of these, consider constructing full
     words in registers and using alpha_expand_unaligned_store_words.  */
  while (i < nregs && GET_MODE (data_regs[i]) == SImode)
    {
      alpha_expand_unaligned_store (orig_dst, data_regs[i], 4, ofs);
      ofs += 4;
      i++;
    }

  if (dst_align >= 16)
    while (i < nregs && GET_MODE (data_regs[i]) == HImode)
      {
	emit_move_insn (adjust_address (orig_dst, HImode, ofs), data_regs[i]);
	i++;
	ofs += 2;
      }
  else
    while (i < nregs && GET_MODE (data_regs[i]) == HImode)
      {
	alpha_expand_unaligned_store (orig_dst, data_regs[i], 2, ofs);
	i++;
	ofs += 2;
      }

  while (i < nregs && GET_MODE (data_regs[i]) == QImode)
    {
      emit_move_insn (adjust_address (orig_dst, QImode, ofs), data_regs[i]);
      i++;
      ofs += 1;
    }

 dst_done:

  if (i != nregs)
    abort ();

  return 1;
}

int
alpha_expand_block_clear (operands)
     rtx operands[];
{
  rtx bytes_rtx	= operands[1];
  rtx align_rtx = operands[2];
  HOST_WIDE_INT orig_bytes = INTVAL (bytes_rtx);
  HOST_WIDE_INT bytes = orig_bytes;
  HOST_WIDE_INT align = INTVAL (align_rtx) * BITS_PER_UNIT;
  HOST_WIDE_INT alignofs = 0;
  rtx orig_dst = operands[0];
  rtx tmp;
  int i, words, ofs = 0;
  
  if (orig_bytes <= 0)
    return 1;
  if (orig_bytes > MAX_MOVE_WORDS * UNITS_PER_WORD)
    return 0;

  /* Look for stricter alignment.  */
  tmp = XEXP (orig_dst, 0);
  if (GET_CODE (tmp) == REG)
    align = MAX (align, REGNO_POINTER_ALIGN (REGNO (tmp)));
  else if (GET_CODE (tmp) == PLUS
	   && GET_CODE (XEXP (tmp, 0)) == REG
	   && GET_CODE (XEXP (tmp, 1)) == CONST_INT)
    {
      HOST_WIDE_INT c = INTVAL (XEXP (tmp, 1));
      int a = REGNO_POINTER_ALIGN (REGNO (XEXP (tmp, 0)));

      if (a > align)
	{
          if (a >= 64)
	    align = a, alignofs = 8 - c % 8;
          else if (a >= 32)
	    align = a, alignofs = 4 - c % 4;
          else if (a >= 16)
	    align = a, alignofs = 2 - c % 2;
	}
    }
  else if (GET_CODE (tmp) == ADDRESSOF)
    {
      enum machine_mode mode;

      mode = mode_for_size (bytes * BITS_PER_UNIT, MODE_INT, 1);
      if (GET_MODE (XEXP (tmp, 0)) == mode)
	{
	  emit_move_insn (XEXP (tmp, 0), const0_rtx);
	  return 1;
	}

      /* No appropriate mode; fall back on memory.  */
      orig_dst = replace_equiv_address (orig_dst, copy_addr_to_reg (tmp));
      align = GET_MODE_BITSIZE (GET_MODE (XEXP (tmp, 0)));
    }

  /* Handle an unaligned prefix first.  */

  if (alignofs > 0)
    {
#if HOST_BITS_PER_WIDE_INT >= 64
      /* Given that alignofs is bounded by align, the only time BWX could
	 generate three stores is for a 7 byte fill.  Prefer two individual
	 stores over a load/mask/store sequence.  */
      if ((!TARGET_BWX || alignofs == 7)
	       && align >= 32
	       && !(alignofs == 4 && bytes >= 4))
	{
	  enum machine_mode mode = (align >= 64 ? DImode : SImode);
	  int inv_alignofs = (align >= 64 ? 8 : 4) - alignofs;
	  rtx mem, tmp;
	  HOST_WIDE_INT mask;

	  mem = adjust_address (orig_dst, mode, ofs - inv_alignofs);
	  set_mem_alias_set (mem, 0);

	  mask = ~(~(HOST_WIDE_INT)0 << (inv_alignofs * 8));
	  if (bytes < alignofs)
	    {
	      mask |= ~(HOST_WIDE_INT)0 << ((inv_alignofs + bytes) * 8);
	      ofs += bytes;
	      bytes = 0;
	    }
	  else
	    {
	      bytes -= alignofs;
	      ofs += alignofs;
	    }
	  alignofs = 0;

	  tmp = expand_binop (mode, and_optab, mem, GEN_INT (mask),
			      NULL_RTX, 1, OPTAB_WIDEN);

	  emit_move_insn (mem, tmp);
	}
#endif

      if (TARGET_BWX && (alignofs & 1) && bytes >= 1)
	{
	  emit_move_insn (adjust_address (orig_dst, QImode, ofs), const0_rtx);
	  bytes -= 1;
	  ofs += 1;
	  alignofs -= 1;
	}
      if (TARGET_BWX && align >= 16 && (alignofs & 3) == 2 && bytes >= 2)
	{
	  emit_move_insn (adjust_address (orig_dst, HImode, ofs), const0_rtx);
	  bytes -= 2;
	  ofs += 2;
	  alignofs -= 2;
	}
      if (alignofs == 4 && bytes >= 4)
	{
	  emit_move_insn (adjust_address (orig_dst, SImode, ofs), const0_rtx);
	  bytes -= 4;
	  ofs += 4;
	  alignofs = 0;
	}

      /* If we've not used the extra lead alignment information by now,
	 we won't be able to.  Downgrade align to match what's left over.  */
      if (alignofs > 0)
	{
	  alignofs = alignofs & -alignofs;
	  align = MIN (align, alignofs * BITS_PER_UNIT);
	}
    }

  /* Handle a block of contiguous long-words.  */

  if (align >= 64 && bytes >= 8)
    {
      words = bytes / 8;

      for (i = 0; i < words; ++i)
	emit_move_insn (adjust_address (orig_dst, DImode, ofs + i * 8),
			const0_rtx);

      bytes -= words * 8;
      ofs += words * 8;
    }

  /* If the block is large and appropriately aligned, emit a single
     store followed by a sequence of stq_u insns.  */

  if (align >= 32 && bytes > 16)
    {
      rtx orig_dsta;

      emit_move_insn (adjust_address (orig_dst, SImode, ofs), const0_rtx);
      bytes -= 4;
      ofs += 4;

      orig_dsta = XEXP (orig_dst, 0);
      if (GET_CODE (orig_dsta) == LO_SUM)
	orig_dsta = force_reg (Pmode, orig_dsta);

      words = bytes / 8;
      for (i = 0; i < words; ++i)
	{
	  rtx mem
	    = change_address (orig_dst, DImode,
			      gen_rtx_AND (DImode,
					   plus_constant (orig_dsta, ofs + i*8),
					   GEN_INT (-8)));
	  set_mem_alias_set (mem, 0);
	  emit_move_insn (mem, const0_rtx);
	}

      /* Depending on the alignment, the first stq_u may have overlapped
	 with the initial stl, which means that the last stq_u didn't
	 write as much as it would appear.  Leave those questionable bytes
	 unaccounted for.  */
      bytes -= words * 8 - 4;
      ofs += words * 8 - 4;
    }

  /* Handle a smaller block of aligned words.  */

  if ((align >= 64 && bytes == 4)
      || (align == 32 && bytes >= 4))
    {
      words = bytes / 4;

      for (i = 0; i < words; ++i)
	emit_move_insn (adjust_address (orig_dst, SImode, ofs + i * 4),
			const0_rtx);

      bytes -= words * 4;
      ofs += words * 4;
    }

  /* An unaligned block uses stq_u stores for as many as possible.  */

  if (bytes >= 8)
    {
      words = bytes / 8;

      alpha_expand_unaligned_store_words (NULL, orig_dst, words, ofs);

      bytes -= words * 8;
      ofs += words * 8;
    }

  /* Next clean up any trailing pieces.  */

#if HOST_BITS_PER_WIDE_INT >= 64
  /* Count the number of bits in BYTES for which aligned stores could
     be emitted.  */
  words = 0;
  for (i = (TARGET_BWX ? 1 : 4); i * BITS_PER_UNIT <= align ; i <<= 1)
    if (bytes & i)
      words += 1;

  /* If we have appropriate alignment (and it wouldn't take too many
     instructions otherwise), mask out the bytes we need.  */
  if (TARGET_BWX ? words > 2 : bytes > 0)
    {
      if (align >= 64)
	{
	  rtx mem, tmp;
	  HOST_WIDE_INT mask;

	  mem = adjust_address (orig_dst, DImode, ofs);
	  set_mem_alias_set (mem, 0);

	  mask = ~(HOST_WIDE_INT)0 << (bytes * 8);

	  tmp = expand_binop (DImode, and_optab, mem, GEN_INT (mask),
			      NULL_RTX, 1, OPTAB_WIDEN);

	  emit_move_insn (mem, tmp);
	  return 1;
	}
      else if (align >= 32 && bytes < 4)
	{
	  rtx mem, tmp;
	  HOST_WIDE_INT mask;

	  mem = adjust_address (orig_dst, SImode, ofs);
	  set_mem_alias_set (mem, 0);

	  mask = ~(HOST_WIDE_INT)0 << (bytes * 8);

	  tmp = expand_binop (SImode, and_optab, mem, GEN_INT (mask),
			      NULL_RTX, 1, OPTAB_WIDEN);

	  emit_move_insn (mem, tmp);
	  return 1;
	}
    }
#endif

  if (!TARGET_BWX && bytes >= 4)
    {
      alpha_expand_unaligned_store (orig_dst, const0_rtx, 4, ofs);
      bytes -= 4;
      ofs += 4;
    }

  if (bytes >= 2)
    {
      if (align >= 16)
	{
	  do {
	    emit_move_insn (adjust_address (orig_dst, HImode, ofs),
			    const0_rtx);
	    bytes -= 2;
	    ofs += 2;
	  } while (bytes >= 2);
	}
      else if (! TARGET_BWX)
	{
	  alpha_expand_unaligned_store (orig_dst, const0_rtx, 2, ofs);
	  bytes -= 2;
	  ofs += 2;
	}
    }

  while (bytes > 0)
    {
      emit_move_insn (adjust_address (orig_dst, QImode, ofs), const0_rtx);
      bytes -= 1;
      ofs += 1;
    }

  return 1;
}

/* Adjust the cost of a scheduling dependency.  Return the new cost of
   a dependency LINK or INSN on DEP_INSN.  COST is the current cost.  */

static int
alpha_adjust_cost (insn, link, dep_insn, cost)
     rtx insn;
     rtx link;
     rtx dep_insn;
     int cost;
{
  rtx set, set_src;
  enum attr_type insn_type, dep_insn_type;

  /* If the dependence is an anti-dependence, there is no cost.  For an
     output dependence, there is sometimes a cost, but it doesn't seem
     worth handling those few cases.  */

  if (REG_NOTE_KIND (link) != 0)
    return 0;

  /* If we can't recognize the insns, we can't really do anything.  */
  if (recog_memoized (insn) < 0 || recog_memoized (dep_insn) < 0)
    return cost;

  insn_type = get_attr_type (insn);
  dep_insn_type = get_attr_type (dep_insn);

  /* Bring in the user-defined memory latency.  */
  if (dep_insn_type == TYPE_ILD
      || dep_insn_type == TYPE_FLD
      || dep_insn_type == TYPE_LDSYM)
    cost += alpha_memory_latency-1;

  switch (alpha_cpu)
    {
    case PROCESSOR_EV4:
      /* On EV4, if INSN is a store insn and DEP_INSN is setting the data
	 being stored, we can sometimes lower the cost.  */

      if ((insn_type == TYPE_IST || insn_type == TYPE_FST)
	  && (set = single_set (dep_insn)) != 0
	  && GET_CODE (PATTERN (insn)) == SET
	  && rtx_equal_p (SET_DEST (set), SET_SRC (PATTERN (insn))))
	{
	  switch (dep_insn_type)
	    {
	    case TYPE_ILD:
	    case TYPE_FLD:
	      /* No savings here.  */
	      return cost;

	    case TYPE_IMUL:
	      /* In these cases, we save one cycle.  */
	      return cost - 1;

	    default:
	      /* In all other cases, we save two cycles.  */
	      return MAX (0, cost - 2);
	    }
	}

      /* Another case that needs adjustment is an arithmetic or logical
	 operation.  It's cost is usually one cycle, but we default it to
	 two in the MD file.  The only case that it is actually two is
	 for the address in loads, stores, and jumps.  */

      if (dep_insn_type == TYPE_IADD || dep_insn_type == TYPE_ILOG)
	{
	  switch (insn_type)
	    {
	    case TYPE_ILD:
	    case TYPE_IST:
	    case TYPE_FLD:
	    case TYPE_FST:
	    case TYPE_JSR:
	      return cost;
	    default:
	      return 1;
	    }
	}

      /* The final case is when a compare feeds into an integer branch;
	 the cost is only one cycle in that case.  */

      if (dep_insn_type == TYPE_ICMP && insn_type == TYPE_IBR)
	return 1;
      break;

    case PROCESSOR_EV5:
      /* And the lord DEC saith:  "A special bypass provides an effective
	 latency of 0 cycles for an ICMP or ILOG insn producing the test
	 operand of an IBR or ICMOV insn." */

      if ((dep_insn_type == TYPE_ICMP || dep_insn_type == TYPE_ILOG)
	  && (set = single_set (dep_insn)) != 0)
	{
	  /* A branch only has one input.  This must be it.  */
	  if (insn_type == TYPE_IBR)
	    return 0;
	  /* A conditional move has three, make sure it is the test.  */
	  if (insn_type == TYPE_ICMOV
	      && GET_CODE (set_src = PATTERN (insn)) == SET
	      && GET_CODE (set_src = SET_SRC (set_src)) == IF_THEN_ELSE
	      && rtx_equal_p (SET_DEST (set), XEXP (set_src, 0)))
	    return 0;
	}

      /* "The multiplier is unable to receive data from IEU bypass paths.
	 The instruction issues at the expected time, but its latency is
	 increased by the time it takes for the input data to become
	 available to the multiplier" -- which happens in pipeline stage
	 six, when results are comitted to the register file.  */

      if (insn_type == TYPE_IMUL)
	{
	  switch (dep_insn_type)
	    {
	    /* These insns produce their results in pipeline stage five.  */
	    case TYPE_ILD:
	    case TYPE_ICMOV:
	    case TYPE_IMUL:
	    case TYPE_MVI:
	      return cost + 1;

	    /* Other integer insns produce results in pipeline stage four.  */
	    default:
	      return cost + 2;
	    }
	}
      break;

    case PROCESSOR_EV6:
      /* There is additional latency to move the result of (most) FP 
         operations anywhere but the FP register file.  */

      if ((insn_type == TYPE_FST || insn_type == TYPE_FTOI)
	  && (dep_insn_type == TYPE_FADD ||
	      dep_insn_type == TYPE_FMUL ||
	      dep_insn_type == TYPE_FCMOV))
        return cost + 2;

      break;
    }

  /* Otherwise, return the default cost.  */
  return cost;
}

/* Function to initialize the issue rate used by the scheduler.  */
static int
alpha_issue_rate ()
{
  return (alpha_cpu == PROCESSOR_EV4 ? 2 : 4);
}

static int
alpha_variable_issue (dump, verbose, insn, cim)
     FILE *dump ATTRIBUTE_UNUSED;
     int verbose ATTRIBUTE_UNUSED;
     rtx insn;
     int cim;
{
    if (recog_memoized (insn) < 0 || get_attr_type (insn) == TYPE_MULTI)
      return 0;

    return cim - 1;
}


/* Register global variables and machine-specific functions with the
   garbage collector.  */

#if TARGET_ABI_UNICOSMK
static void
alpha_init_machine_status (p)
     struct function *p;
{
  p->machine =
    (struct machine_function *) xcalloc (1, sizeof (struct machine_function));

  p->machine->first_ciw = NULL_RTX;
  p->machine->last_ciw = NULL_RTX;
  p->machine->ciw_count = 0;
  p->machine->addr_list = NULL_RTX;
}

static void
alpha_mark_machine_status (p)
     struct function *p;
{
  struct machine_function *machine = p->machine;

  if (machine)
    {
      ggc_mark_rtx (machine->first_ciw);
      ggc_mark_rtx (machine->addr_list);
    }
}

static void
alpha_free_machine_status (p)
     struct function *p;
{
  free (p->machine);
  p->machine = NULL;
}
#endif /* TARGET_ABI_UNICOSMK */

/* Functions to save and restore alpha_return_addr_rtx.  */

/* Start the ball rolling with RETURN_ADDR_RTX.  */

rtx
alpha_return_addr (count, frame)
     int count;
     rtx frame ATTRIBUTE_UNUSED;
{
  if (count != 0)
    return const0_rtx;

  return get_hard_reg_initial_val (Pmode, REG_RA);
}

/* Return or create a pseudo containing the gp value for the current
   function.  Needed only if TARGET_LD_BUGGY_LDGP.  */

rtx
alpha_gp_save_rtx ()
{
  return get_hard_reg_initial_val (DImode, 29);
}

static int
alpha_ra_ever_killed ()
{
  rtx top;

#ifdef ASM_OUTPUT_MI_THUNK
  if (current_function_is_thunk)
    return 0;
#endif
  if (!has_hard_reg_initial_val (Pmode, REG_RA))
    return regs_ever_live[REG_RA];

  push_topmost_sequence ();
  top = get_insns ();
  pop_topmost_sequence ();

  return reg_set_between_p (gen_rtx_REG (Pmode, REG_RA), top, NULL_RTX);
}


/* Return the trap mode suffix applicable to the current
   instruction, or NULL.  */

static const char *
get_trap_mode_suffix ()
{
  enum attr_trap_suffix s = get_attr_trap_suffix (current_output_insn);

  switch (s)
    {
    case TRAP_SUFFIX_NONE:
      return NULL;

    case TRAP_SUFFIX_SU:
      if (alpha_fptm >= ALPHA_FPTM_SU)
	return "su";
      return NULL;

    case TRAP_SUFFIX_SUI:
      if (alpha_fptm >= ALPHA_FPTM_SUI)
	return "sui";
      return NULL;

    case TRAP_SUFFIX_V_SV:
      switch (alpha_fptm)
	{
	case ALPHA_FPTM_N:
	  return NULL;
	case ALPHA_FPTM_U:
	  return "v";
	case ALPHA_FPTM_SU:
	case ALPHA_FPTM_SUI:
	  return "sv";
	}
      break;

    case TRAP_SUFFIX_V_SV_SVI:
      switch (alpha_fptm)
	{
	case ALPHA_FPTM_N:
	  return NULL;
	case ALPHA_FPTM_U:
	  return "v";
	case ALPHA_FPTM_SU:
	  return "sv";
	case ALPHA_FPTM_SUI:
	  return "svi";
	}
      break;

    case TRAP_SUFFIX_U_SU_SUI:
      switch (alpha_fptm)
	{
	case ALPHA_FPTM_N:
	  return NULL;
	case ALPHA_FPTM_U:
	  return "u";
	case ALPHA_FPTM_SU:
	  return "su";
	case ALPHA_FPTM_SUI:
	  return "sui";
	}
      break;
    }
  abort ();
}

/* Return the rounding mode suffix applicable to the current
   instruction, or NULL.  */

static const char *
get_round_mode_suffix ()
{
  enum attr_round_suffix s = get_attr_round_suffix (current_output_insn);

  switch (s)
    {
    case ROUND_SUFFIX_NONE:
      return NULL;
    case ROUND_SUFFIX_NORMAL:
      switch (alpha_fprm)
	{
	case ALPHA_FPRM_NORM:
	  return NULL;
	case ALPHA_FPRM_MINF: 
	  return "m";
	case ALPHA_FPRM_CHOP:
	  return "c";
	case ALPHA_FPRM_DYN:
	  return "d";
	}
      break;

    case ROUND_SUFFIX_C:
      return "c";
    }
  abort ();
}

/* Print an operand.  Recognize special options, documented below.  */

void
print_operand (file, x, code)
    FILE *file;
    rtx x;
    int code;
{
  int i;

  switch (code)
    {
    case '~':
      /* Print the assembler name of the current function.  */
      assemble_name (file, alpha_fnname);
      break;

    case '/':
      {
	const char *trap = get_trap_mode_suffix ();
	const char *round = get_round_mode_suffix ();

	if (trap || round)
	  fprintf (file, (TARGET_AS_SLASH_BEFORE_SUFFIX ? "/%s%s" : "%s%s"),
		   (trap ? trap : ""), (round ? round : ""));
	break;
      }

    case ',':
      /* Generates single precision instruction suffix.  */
      fputc ((TARGET_FLOAT_VAX ? 'f' : 's'), file);
      break;

    case '-':
      /* Generates double precision instruction suffix.  */
      fputc ((TARGET_FLOAT_VAX ? 'g' : 't'), file);
      break;

    case '#':
      if (alpha_this_literal_sequence_number == 0)
	alpha_this_literal_sequence_number = alpha_next_sequence_number++;
      fprintf (file, "%d", alpha_this_literal_sequence_number);
      break;

    case '*':
      if (alpha_this_gpdisp_sequence_number == 0)
	alpha_this_gpdisp_sequence_number = alpha_next_sequence_number++;
      fprintf (file, "%d", alpha_this_gpdisp_sequence_number);
      break;

    case 'H':
      if (GET_CODE (x) == HIGH)
	output_addr_const (file, XEXP (x, 0));
      else
	output_operand_lossage ("invalid %%H value");
      break;

    case 'J':
      if (GET_CODE (x) == CONST_INT)
	{
	  if (INTVAL (x) != 0)
	    fprintf (file, "\t\t!lituse_jsr!%d", (int) INTVAL (x));
	}
      else
	output_operand_lossage ("invalid %%J value");
      break;

    case 'r':
      /* If this operand is the constant zero, write it as "$31".  */
      if (GET_CODE (x) == REG)
	fprintf (file, "%s", reg_names[REGNO (x)]);
      else if (x == CONST0_RTX (GET_MODE (x)))
	fprintf (file, "$31");
      else
	output_operand_lossage ("invalid %%r value");
      break;

    case 'R':
      /* Similar, but for floating-point.  */
      if (GET_CODE (x) == REG)
	fprintf (file, "%s", reg_names[REGNO (x)]);
      else if (x == CONST0_RTX (GET_MODE (x)))
	fprintf (file, "$f31");
      else
	output_operand_lossage ("invalid %%R value");
      break;

    case 'N':
      /* Write the 1's complement of a constant.  */
      if (GET_CODE (x) != CONST_INT)
	output_operand_lossage ("invalid %%N value");

      fprintf (file, HOST_WIDE_INT_PRINT_DEC, ~ INTVAL (x));
      break;

    case 'P':
      /* Write 1 << C, for a constant C.  */
      if (GET_CODE (x) != CONST_INT)
	output_operand_lossage ("invalid %%P value");

      fprintf (file, HOST_WIDE_INT_PRINT_DEC, (HOST_WIDE_INT) 1 << INTVAL (x));
      break;

    case 'h':
      /* Write the high-order 16 bits of a constant, sign-extended.  */
      if (GET_CODE (x) != CONST_INT)
	output_operand_lossage ("invalid %%h value");

      fprintf (file, HOST_WIDE_INT_PRINT_DEC, INTVAL (x) >> 16);
      break;

    case 'L':
      /* Write the low-order 16 bits of a constant, sign-extended.  */
      if (GET_CODE (x) != CONST_INT)
	output_operand_lossage ("invalid %%L value");

      fprintf (file, HOST_WIDE_INT_PRINT_DEC,
	       (INTVAL (x) & 0xffff) - 2 * (INTVAL (x) & 0x8000));
      break;

    case 'm':
      /* Write mask for ZAP insn.  */
      if (GET_CODE (x) == CONST_DOUBLE)
	{
	  HOST_WIDE_INT mask = 0;
	  HOST_WIDE_INT value;

	  value = CONST_DOUBLE_LOW (x);
	  for (i = 0; i < HOST_BITS_PER_WIDE_INT / HOST_BITS_PER_CHAR;
	       i++, value >>= 8)
	    if (value & 0xff)
	      mask |= (1 << i);

	  value = CONST_DOUBLE_HIGH (x);
	  for (i = 0; i < HOST_BITS_PER_WIDE_INT / HOST_BITS_PER_CHAR;
	       i++, value >>= 8)
	    if (value & 0xff)
	      mask |= (1 << (i + sizeof (int)));

	  fprintf (file, HOST_WIDE_INT_PRINT_DEC, mask & 0xff);
	}

      else if (GET_CODE (x) == CONST_INT)
	{
	  HOST_WIDE_INT mask = 0, value = INTVAL (x);

	  for (i = 0; i < 8; i++, value >>= 8)
	    if (value & 0xff)
	      mask |= (1 << i);

	  fprintf (file, HOST_WIDE_INT_PRINT_DEC, mask);
	}
      else
	output_operand_lossage ("invalid %%m value");
      break;

    case 'M':
      /* 'b', 'w', 'l', or 'q' as the value of the constant.  */
      if (GET_CODE (x) != CONST_INT
	  || (INTVAL (x) != 8 && INTVAL (x) != 16
	      && INTVAL (x) != 32 && INTVAL (x) != 64))
	output_operand_lossage ("invalid %%M value");

      fprintf (file, "%s",
	       (INTVAL (x) == 8 ? "b"
		: INTVAL (x) == 16 ? "w"
		: INTVAL (x) == 32 ? "l"
		: "q"));
      break;

    case 'U':
      /* Similar, except do it from the mask.  */
      if (GET_CODE (x) == CONST_INT && INTVAL (x) == 0xff)
	fprintf (file, "b");
      else if (GET_CODE (x) == CONST_INT && INTVAL (x) == 0xffff)
	fprintf (file, "w");
      else if (GET_CODE (x) == CONST_INT && INTVAL (x) == 0xffffffff)
	fprintf (file, "l");
#if HOST_BITS_PER_WIDE_INT == 32
      else if (GET_CODE (x) == CONST_DOUBLE
	       && CONST_DOUBLE_HIGH (x) == 0
	       && CONST_DOUBLE_LOW (x) == -1)
	fprintf (file, "l");
      else if (GET_CODE (x) == CONST_DOUBLE
	       && CONST_DOUBLE_HIGH (x) == -1
	       && CONST_DOUBLE_LOW (x) == -1)
	fprintf (file, "q");
#else
      else if (GET_CODE (x) == CONST_INT && INTVAL (x) == -1)
	fprintf (file, "q");
      else if (GET_CODE (x) == CONST_DOUBLE
	       && CONST_DOUBLE_HIGH (x) == 0
	       && CONST_DOUBLE_LOW (x) == -1)
	fprintf (file, "q");
#endif
      else
	output_operand_lossage ("invalid %%U value");
      break;

    case 's':
      /* Write the constant value divided by 8 for little-endian mode or
	 (56 - value) / 8 for big-endian mode.  */

      if (GET_CODE (x) != CONST_INT
	  || (unsigned HOST_WIDE_INT) INTVAL (x) >= (WORDS_BIG_ENDIAN
						     ? 56
						     : 64)  
	  || (INTVAL (x) & 7) != 0)
	output_operand_lossage ("invalid %%s value");

      fprintf (file, HOST_WIDE_INT_PRINT_DEC,
	       WORDS_BIG_ENDIAN
	       ? (56 - INTVAL (x)) / 8
	       : INTVAL (x) / 8);
      break;

    case 'S':
      /* Same, except compute (64 - c) / 8 */

      if (GET_CODE (x) != CONST_INT
	  && (unsigned HOST_WIDE_INT) INTVAL (x) >= 64
	  && (INTVAL (x) & 7) != 8)
	output_operand_lossage ("invalid %%s value");

      fprintf (file, HOST_WIDE_INT_PRINT_DEC, (64 - INTVAL (x)) / 8);
      break;

    case 't':
      {
        /* On Unicos/Mk systems: use a DEX expression if the symbol
	   clashes with a register name.  */
	int dex = unicosmk_need_dex (x);
	if (dex)
	  fprintf (file, "DEX(%d)", dex);
	else
	  output_addr_const (file, x);
      }
      break;

    case 'C': case 'D': case 'c': case 'd':
      /* Write out comparison name.  */
      {
	enum rtx_code c = GET_CODE (x);

        if (GET_RTX_CLASS (c) != '<')
	  output_operand_lossage ("invalid %%C value");

	else if (code == 'D')
	  c = reverse_condition (c);
	else if (code == 'c')
	  c = swap_condition (c);
	else if (code == 'd')
	  c = swap_condition (reverse_condition (c));

        if (c == LEU)
	  fprintf (file, "ule");
        else if (c == LTU)
	  fprintf (file, "ult");
	else if (c == UNORDERED)
	  fprintf (file, "un");
        else
	  fprintf (file, "%s", GET_RTX_NAME (c));
      }
      break;

    case 'E':
      /* Write the divide or modulus operator.  */
      switch (GET_CODE (x))
	{
	case DIV:
	  fprintf (file, "div%s", GET_MODE (x) == SImode ? "l" : "q");
	  break;
	case UDIV:
	  fprintf (file, "div%su", GET_MODE (x) == SImode ? "l" : "q");
	  break;
	case MOD:
	  fprintf (file, "rem%s", GET_MODE (x) == SImode ? "l" : "q");
	  break;
	case UMOD:
	  fprintf (file, "rem%su", GET_MODE (x) == SImode ? "l" : "q");
	  break;
	default:
	  output_operand_lossage ("invalid %%E value");
	  break;
	}
      break;

    case 'A':
      /* Write "_u" for unaligned access.  */
      if (GET_CODE (x) == MEM && GET_CODE (XEXP (x, 0)) == AND)
	fprintf (file, "_u");
      break;

    case 0:
      if (GET_CODE (x) == REG)
	fprintf (file, "%s", reg_names[REGNO (x)]);
      else if (GET_CODE (x) == MEM)
	output_address (XEXP (x, 0));
      else
	output_addr_const (file, x);
      break;

    default:
      output_operand_lossage ("invalid %%xn code");
    }
}

void
print_operand_address (file, addr)
    FILE *file;
     rtx addr;
{
  int basereg = 31;
  HOST_WIDE_INT offset = 0;

  if (GET_CODE (addr) == AND)
    addr = XEXP (addr, 0);

  if (GET_CODE (addr) == PLUS
      && GET_CODE (XEXP (addr, 1)) == CONST_INT)
    {
      offset = INTVAL (XEXP (addr, 1));
      addr = XEXP (addr, 0);
    }

  if (GET_CODE (addr) == LO_SUM)
    {
      output_addr_const (file, XEXP (addr, 1));
      if (offset)
	{
	  fputc ('+', file);
	  fprintf (file, HOST_WIDE_INT_PRINT_DEC, offset);
	}
      
      addr = XEXP (addr, 0);
      if (GET_CODE (addr) == REG)
	basereg = REGNO (addr);
      else if (GET_CODE (addr) == SUBREG
	       && GET_CODE (SUBREG_REG (addr)) == REG)
	basereg = subreg_regno (addr);
      else
	abort ();

      fprintf (file, "($%d)\t\t!%s", basereg,
	       (basereg == 29 ? "gprel" : "gprellow"));
      return;
    }

  if (GET_CODE (addr) == REG)
    basereg = REGNO (addr);
  else if (GET_CODE (addr) == SUBREG
	   && GET_CODE (SUBREG_REG (addr)) == REG)
    basereg = subreg_regno (addr);
  else if (GET_CODE (addr) == CONST_INT)
    offset = INTVAL (addr);
  else
    abort ();

  fprintf (file, HOST_WIDE_INT_PRINT_DEC, offset);
  fprintf (file, "($%d)", basereg);
}

/* Emit RTL insns to initialize the variable parts of a trampoline at
   TRAMP. FNADDR is an RTX for the address of the function's pure
   code.  CXT is an RTX for the static chain value for the function.

   The three offset parameters are for the individual template's
   layout.  A JMPOFS < 0 indicates that the trampoline does not 
   contain instructions at all.

   We assume here that a function will be called many more times than
   its address is taken (e.g., it might be passed to qsort), so we
   take the trouble to initialize the "hint" field in the JMP insn.
   Note that the hint field is PC (new) + 4 * bits 13:0.  */

void
alpha_initialize_trampoline (tramp, fnaddr, cxt, fnofs, cxtofs, jmpofs)
     rtx tramp, fnaddr, cxt;
     int fnofs, cxtofs, jmpofs;
{
  rtx temp, temp1, addr;
  /* VMS really uses DImode pointers in memory at this point.  */
  enum machine_mode mode = TARGET_ABI_OPEN_VMS ? Pmode : ptr_mode;

#ifdef POINTERS_EXTEND_UNSIGNED
  fnaddr = convert_memory_address (mode, fnaddr);
  cxt = convert_memory_address (mode, cxt);
#endif

  /* Store function address and CXT.  */
  addr = memory_address (mode, plus_constant (tramp, fnofs));
  emit_move_insn (gen_rtx_MEM (mode, addr), fnaddr);
  addr = memory_address (mode, plus_constant (tramp, cxtofs));
  emit_move_insn (gen_rtx_MEM (mode, addr), cxt);

  /* This has been disabled since the hint only has a 32k range, and in
     no existing OS is the stack within 32k of the text segment.  */
  if (0 && jmpofs >= 0)
    {
      /* Compute hint value.  */
      temp = force_operand (plus_constant (tramp, jmpofs+4), NULL_RTX);
      temp = expand_binop (DImode, sub_optab, fnaddr, temp, temp, 1,
			   OPTAB_WIDEN);
      temp = expand_shift (RSHIFT_EXPR, Pmode, temp,
		           build_int_2 (2, 0), NULL_RTX, 1);
      temp = expand_and (gen_lowpart (SImode, temp), GEN_INT (0x3fff), 0);

      /* Merge in the hint.  */
      addr = memory_address (SImode, plus_constant (tramp, jmpofs));
      temp1 = force_reg (SImode, gen_rtx_MEM (SImode, addr));
      temp1 = expand_and (temp1, GEN_INT (0xffffc000), NULL_RTX);
      temp1 = expand_binop (SImode, ior_optab, temp1, temp, temp1, 1,
			    OPTAB_WIDEN);
      emit_move_insn (gen_rtx_MEM (SImode, addr), temp1);
    }

#ifdef TRANSFER_FROM_TRAMPOLINE
  emit_library_call (gen_rtx_SYMBOL_REF (Pmode, "__enable_execute_stack"),
		     0, VOIDmode, 1, addr, Pmode);
#endif

  if (jmpofs >= 0)
    emit_insn (gen_imb ());
}

/* Determine where to put an argument to a function.
   Value is zero to push the argument on the stack,
   or a hard register in which to store the argument.

   MODE is the argument's machine mode.
   TYPE is the data type of the argument (as a tree).
    This is null for libcalls where that information may
    not be available.
   CUM is a variable of type CUMULATIVE_ARGS which gives info about
    the preceding args and about the function being called.
   NAMED is nonzero if this argument is a named parameter
    (otherwise it is an extra parameter matching an ellipsis).

   On Alpha the first 6 words of args are normally in registers
   and the rest are pushed.  */

rtx
function_arg (cum, mode, type, named)
     CUMULATIVE_ARGS cum;
     enum machine_mode mode;
     tree type;
     int named ATTRIBUTE_UNUSED;
{
  int basereg;
  int num_args;

  /* Set up defaults for FP operands passed in FP registers, and
     integral operands passed in integer registers.  */
  if (TARGET_FPREGS
      && (GET_MODE_CLASS (mode) == MODE_COMPLEX_FLOAT
	  || GET_MODE_CLASS (mode) == MODE_FLOAT))
    basereg = 32 + 16;
  else
    basereg = 16;

  /* ??? Irritatingly, the definition of CUMULATIVE_ARGS is different for
     the three platforms, so we can't avoid conditional compilation.  */
#if TARGET_ABI_OPEN_VMS
    {
      if (mode == VOIDmode)
	return alpha_arg_info_reg_val (cum);

      num_args = cum.num_args;
      if (num_args >= 6 || MUST_PASS_IN_STACK (mode, type))
	return NULL_RTX;
    }
#else
#if TARGET_ABI_UNICOSMK
    {
      int size;

      /* If this is the last argument, generate the call info word (CIW).  */
      /* ??? We don't include the caller's line number in the CIW because
	 I don't know how to determine it if debug infos are turned off.  */
      if (mode == VOIDmode)
	{
	  int i;
	  HOST_WIDE_INT lo;
	  HOST_WIDE_INT hi;
	  rtx ciw;

	  lo = 0;

	  for (i = 0; i < cum.num_reg_words && i < 5; i++)
	    if (cum.reg_args_type[i])
	      lo |= (1 << (7 - i));

	  if (cum.num_reg_words == 6 && cum.reg_args_type[5])
	    lo |= 7;
	  else
	    lo |= cum.num_reg_words;

#if HOST_BITS_PER_WIDE_INT == 32
	  hi = (cum.num_args << 20) | cum.num_arg_words;
#else
	  lo = lo | ((HOST_WIDE_INT) cum.num_args << 52)
	    | ((HOST_WIDE_INT) cum.num_arg_words << 32);
	  hi = 0;
#endif
	  ciw = immed_double_const (lo, hi, DImode);

	  return gen_rtx_UNSPEC (DImode, gen_rtvec (1, ciw),
				 UNSPEC_UMK_LOAD_CIW);
	}

      size = ALPHA_ARG_SIZE (mode, type, named);
      num_args = cum.num_reg_words;
      if (MUST_PASS_IN_STACK (mode, type)
	  || cum.num_reg_words + size > 6 || cum.force_stack)
	return NULL_RTX;
      else if (type && TYPE_MODE (type) == BLKmode)
	{
	  rtx reg1, reg2;

	  reg1 = gen_rtx_REG (DImode, num_args + 16);
	  reg1 = gen_rtx_EXPR_LIST (DImode, reg1, const0_rtx);

	  /* The argument fits in two registers. Note that we still need to
	     reserve a register for empty structures.  */
	  if (size == 0)
	    return NULL_RTX;
	  else if (size == 1)
	    return gen_rtx_PARALLEL (mode, gen_rtvec (1, reg1));
	  else
	    {
	      reg2 = gen_rtx_REG (DImode, num_args + 17);
	      reg2 = gen_rtx_EXPR_LIST (DImode, reg2, GEN_INT (8));
	      return gen_rtx_PARALLEL (mode, gen_rtvec (2, reg1, reg2));
	    }
	}
    }
#else
    {
      if (cum >= 6)
	return NULL_RTX;
      num_args = cum;

      /* VOID is passed as a special flag for "last argument".  */
      if (type == void_type_node)
	basereg = 16;
      else if (MUST_PASS_IN_STACK (mode, type))
	return NULL_RTX;
      else if (FUNCTION_ARG_PASS_BY_REFERENCE (cum, mode, type, named))
	basereg = 16;
    }
#endif /* TARGET_ABI_UNICOSMK */
#endif /* TARGET_ABI_OPEN_VMS */

  return gen_rtx_REG (mode, num_args + basereg);
}

tree
alpha_build_va_list ()
{
  tree base, ofs, record, type_decl;

  if (TARGET_ABI_OPEN_VMS || TARGET_ABI_UNICOSMK)
    return ptr_type_node;

  record = make_lang_type (RECORD_TYPE);
  type_decl = build_decl (TYPE_DECL, get_identifier ("__va_list_tag"), record);
  TREE_CHAIN (record) = type_decl;
  TYPE_NAME (record) = type_decl;

  /* C++? SET_IS_AGGR_TYPE (record, 1); */

  ofs = build_decl (FIELD_DECL, get_identifier ("__offset"),
		    integer_type_node);
  DECL_FIELD_CONTEXT (ofs) = record;

  base = build_decl (FIELD_DECL, get_identifier ("__base"),
		     ptr_type_node);
  DECL_FIELD_CONTEXT (base) = record;
  TREE_CHAIN (base) = ofs;

  TYPE_FIELDS (record) = base;
  layout_type (record);

  return record;
}

void
alpha_va_start (stdarg_p, valist, nextarg)
     int stdarg_p;
     tree valist;
     rtx nextarg ATTRIBUTE_UNUSED;
{
  HOST_WIDE_INT offset;
  tree t, offset_field, base_field;

  if (TREE_CODE (TREE_TYPE (valist)) == ERROR_MARK)
    return;

  if (TARGET_ABI_UNICOSMK)
    std_expand_builtin_va_start (stdarg_p, valist, nextarg);

  /* For Unix, SETUP_INCOMING_VARARGS moves the starting address base
     up by 48, storing fp arg registers in the first 48 bytes, and the
     integer arg registers in the next 48 bytes.  This is only done,
     however, if any integer registers need to be stored.

     If no integer registers need be stored, then we must subtract 48
     in order to account for the integer arg registers which are counted
     in argsize above, but which are not actually stored on the stack.  */

  if (NUM_ARGS <= 5 + stdarg_p)
    offset = TARGET_ABI_OPEN_VMS ? UNITS_PER_WORD : 6 * UNITS_PER_WORD;
  else
    offset = -6 * UNITS_PER_WORD;

  if (TARGET_ABI_OPEN_VMS)
    {
      nextarg = plus_constant (nextarg, offset);
      nextarg = plus_constant (nextarg, NUM_ARGS * UNITS_PER_WORD);
      t = build (MODIFY_EXPR, TREE_TYPE (valist), valist,
		 make_tree (ptr_type_node, nextarg));
      TREE_SIDE_EFFECTS (t) = 1;

      expand_expr (t, const0_rtx, VOIDmode, EXPAND_NORMAL);
    }
  else
    {
      base_field = TYPE_FIELDS (TREE_TYPE (valist));
      offset_field = TREE_CHAIN (base_field);

      base_field = build (COMPONENT_REF, TREE_TYPE (base_field),
			  valist, base_field);
      offset_field = build (COMPONENT_REF, TREE_TYPE (offset_field),
			    valist, offset_field);

      t = make_tree (ptr_type_node, virtual_incoming_args_rtx);
      t = build (PLUS_EXPR, ptr_type_node, t, build_int_2 (offset, 0));
      t = build (MODIFY_EXPR, TREE_TYPE (base_field), base_field, t);
      TREE_SIDE_EFFECTS (t) = 1;
      expand_expr (t, const0_rtx, VOIDmode, EXPAND_NORMAL);

      t = build_int_2 (NUM_ARGS * UNITS_PER_WORD, 0);
      t = build (MODIFY_EXPR, TREE_TYPE (offset_field), offset_field, t);
      TREE_SIDE_EFFECTS (t) = 1;
      expand_expr (t, const0_rtx, VOIDmode, EXPAND_NORMAL);
    }
}

rtx
alpha_va_arg (valist, type)
     tree valist, type;
{
  HOST_WIDE_INT tsize;
  rtx addr;
  tree t;
  tree offset_field, base_field, addr_tree, addend;
  tree wide_type, wide_ofs;
  int indirect = 0;

  if (TARGET_ABI_OPEN_VMS || TARGET_ABI_UNICOSMK)
    return std_expand_builtin_va_arg (valist, type);

  tsize = ((TREE_INT_CST_LOW (TYPE_SIZE (type)) / BITS_PER_UNIT + 7) / 8) * 8;

  base_field = TYPE_FIELDS (TREE_TYPE (valist));
  offset_field = TREE_CHAIN (base_field);

  base_field = build (COMPONENT_REF, TREE_TYPE (base_field),
		      valist, base_field);
  offset_field = build (COMPONENT_REF, TREE_TYPE (offset_field),
			valist, offset_field);

  wide_type = make_signed_type (64);
  wide_ofs = save_expr (build1 (CONVERT_EXPR, wide_type, offset_field));

  addend = wide_ofs;

  if (TYPE_MODE (type) == TFmode || TYPE_MODE (type) == TCmode)
    {
      indirect = 1;
      tsize = UNITS_PER_WORD;
    }
  else if (FLOAT_TYPE_P (type))
    {
      tree fpaddend, cond;

      fpaddend = fold (build (PLUS_EXPR, TREE_TYPE (addend),
			      addend, build_int_2 (-6*8, 0)));

      cond = fold (build (LT_EXPR, integer_type_node,
			  wide_ofs, build_int_2 (6*8, 0)));

      addend = fold (build (COND_EXPR, TREE_TYPE (addend), cond,
			    fpaddend, addend));
    }

  addr_tree = build (PLUS_EXPR, TREE_TYPE (base_field),
		     base_field, addend);

  addr = expand_expr (addr_tree, NULL_RTX, Pmode, EXPAND_NORMAL);
  addr = copy_to_reg (addr);

  t = build (MODIFY_EXPR, TREE_TYPE (offset_field), offset_field,
	     build (PLUS_EXPR, TREE_TYPE (offset_field), 
		    offset_field, build_int_2 (tsize, 0)));
  TREE_SIDE_EFFECTS (t) = 1;
  expand_expr (t, const0_rtx, VOIDmode, EXPAND_NORMAL);

  if (indirect)
    {
      addr = force_reg (Pmode, addr);
      addr = gen_rtx_MEM (Pmode, addr);
    }

  return addr;
}

/* This page contains routines that are used to determine what the function
   prologue and epilogue code will do and write them out.  */

/* Compute the size of the save area in the stack.  */

/* These variables are used for communication between the following functions.
   They indicate various things about the current function being compiled
   that are used to tell what kind of prologue, epilogue and procedure
   descriptior to generate.  */

/* Nonzero if we need a stack procedure.  */
static int alpha_is_stack_procedure;

/* Register number (either FP or SP) that is used to unwind the frame.  */
static int vms_unwind_regno;

/* Register number used to save FP.  We need not have one for RA since
   we don't modify it for register procedures.  This is only defined
   for register frame procedures.  */
static int vms_save_fp_regno;

/* Register number used to reference objects off our PV.  */
static int vms_base_regno;

/* Compute register masks for saved registers.  */

static void
alpha_sa_mask (imaskP, fmaskP)
    unsigned long *imaskP;
    unsigned long *fmaskP;
{
  unsigned long imask = 0;
  unsigned long fmask = 0;
  unsigned int i;

#ifdef ASM_OUTPUT_MI_THUNK
  if (!current_function_is_thunk)
#endif
    {
      if (TARGET_ABI_OPEN_VMS && alpha_is_stack_procedure)
	imask |= (1L << HARD_FRAME_POINTER_REGNUM);

      /* One for every register we have to save.  */
      for (i = 0; i < FIRST_PSEUDO_REGISTER; i++)
	if (! fixed_regs[i] && ! call_used_regs[i]
	    && regs_ever_live[i] && i != REG_RA
	    && (!TARGET_ABI_UNICOSMK || i != HARD_FRAME_POINTER_REGNUM))
	  {
	    if (i < 32)
	      imask |= (1L << i);
	    else
	      fmask |= (1L << (i - 32));
	  }

      /* We need to restore these for the handler.  */
      if (current_function_calls_eh_return)
	{
	  for (i = 0; ; ++i)
	    {
	      unsigned regno = EH_RETURN_DATA_REGNO (i);
	      if (regno == INVALID_REGNUM)
		break;
	      imask |= 1L << regno;
	    }
	}
     
      /* If any register spilled, then spill the return address also.  */
      /* ??? This is required by the Digital stack unwind specification
	 and isn't needed if we're doing Dwarf2 unwinding.  */
      if (imask || fmask || alpha_ra_ever_killed ())
	imask |= (1L << REG_RA);
    }

  *imaskP = imask;
  *fmaskP = fmask;
}

int
alpha_sa_size ()
{
  unsigned long mask[2];
  int sa_size = 0;
  int i, j;

  alpha_sa_mask (&mask[0], &mask[1]);

  if (TARGET_ABI_UNICOSMK)
    {
      if (mask[0] || mask[1])
	sa_size = 14;
    }
  else
    {
      for (j = 0; j < 2; ++j)
	for (i = 0; i < 32; ++i)
	  if ((mask[j] >> i) & 1)
	    sa_size++;
    }

  if (TARGET_ABI_UNICOSMK)
    {
      /* We might not need to generate a frame if we don't make any calls
	 (including calls to __T3E_MISMATCH if this is a vararg function),
	 don't have any local variables which require stack slots, don't
	 use alloca and have not determined that we need a frame for other
	 reasons.  */

      alpha_is_stack_procedure = (sa_size
				  || get_frame_size() != 0
				  || current_function_outgoing_args_size
				  || current_function_varargs
				  || current_function_stdarg
				  || current_function_calls_alloca
				  || frame_pointer_needed);

      /* Always reserve space for saving callee-saved registers if we
	 need a frame as required by the calling convention.  */
      if (alpha_is_stack_procedure)
        sa_size = 14;
    }
  else if (TARGET_ABI_OPEN_VMS)
    {
      /* Start by assuming we can use a register procedure if we don't
	 make any calls (REG_RA not used) or need to save any
	 registers and a stack procedure if we do.  */
      alpha_is_stack_procedure = ((mask[0] >> REG_RA) & 1);

      /* Don't reserve space for saving RA yet.  Do that later after we've
	 made the final decision on stack procedure vs register procedure.  */
      if (alpha_is_stack_procedure)
	sa_size--;

      /* Decide whether to refer to objects off our PV via FP or PV.
	 If we need FP for something else or if we receive a nonlocal
	 goto (which expects PV to contain the value), we must use PV.
	 Otherwise, start by assuming we can use FP.  */
      vms_base_regno = (frame_pointer_needed
			|| current_function_has_nonlocal_label
			|| alpha_is_stack_procedure
			|| current_function_outgoing_args_size
			? REG_PV : HARD_FRAME_POINTER_REGNUM);

      /* If we want to copy PV into FP, we need to find some register
	 in which to save FP.  */

      vms_save_fp_regno = -1;
      if (vms_base_regno == HARD_FRAME_POINTER_REGNUM)
	for (i = 0; i < 32; i++)
	  if (! fixed_regs[i] && call_used_regs[i] && ! regs_ever_live[i])
	    vms_save_fp_regno = i;

      if (vms_save_fp_regno == -1)
	vms_base_regno = REG_PV, alpha_is_stack_procedure = 1;

      /* Stack unwinding should be done via FP unless we use it for PV.  */
      vms_unwind_regno = (vms_base_regno == REG_PV
			  ? HARD_FRAME_POINTER_REGNUM : STACK_POINTER_REGNUM);

      /* If this is a stack procedure, allow space for saving FP and RA.  */
      if (alpha_is_stack_procedure)
	sa_size += 2;
    }
  else
    {
      /* Our size must be even (multiple of 16 bytes).  */
      if (sa_size & 1)
	sa_size++;
    }

  return sa_size * 8;
}

int
alpha_pv_save_size ()
{
  alpha_sa_size ();
  return alpha_is_stack_procedure ? 8 : 0;
}

int
alpha_using_fp ()
{
  alpha_sa_size ();
  return vms_unwind_regno == HARD_FRAME_POINTER_REGNUM;
}

#if TARGET_ABI_OPEN_VMS

const struct attribute_spec vms_attribute_table[] =
{
  /* { name, min_len, max_len, decl_req, type_req, fn_type_req, handler } */
  { "overlaid",   0, 0, true,  false, false, NULL },
  { "global",     0, 0, true,  false, false, NULL },
  { "initialize", 0, 0, true,  false, false, NULL },
  { NULL,         0, 0, false, false, false, NULL }
};

#endif

static int
find_lo_sum (px, data)
     rtx *px;
     void *data ATTRIBUTE_UNUSED;
{
  return GET_CODE (*px) == LO_SUM;
}

static int
alpha_does_function_need_gp ()
{
  rtx insn;

  /* The GP being variable is an OSF abi thing.  */
  if (! TARGET_ABI_OSF)
    return 0;

  if (TARGET_PROFILING_NEEDS_GP && current_function_profile)
    return 1;

#ifdef ASM_OUTPUT_MI_THUNK
  if (current_function_is_thunk)
    return 1;
#endif

  /* If we need a GP (we have a LDSYM insn or a CALL_INSN), load it first. 
     Even if we are a static function, we still need to do this in case
     our address is taken and passed to something like qsort.  */

  push_topmost_sequence ();
  insn = get_insns ();
  pop_topmost_sequence ();

  for (; insn; insn = NEXT_INSN (insn))
    if (INSN_P (insn)
	&& GET_CODE (PATTERN (insn)) != USE
	&& GET_CODE (PATTERN (insn)) != CLOBBER)
      {
	enum attr_type type = get_attr_type (insn);
	if (type == TYPE_LDSYM || type == TYPE_JSR)
	  return 1;
	if (TARGET_EXPLICIT_RELOCS
	    && for_each_rtx (&PATTERN (insn), find_lo_sum, NULL) > 0)
	  return 1;
      }

  return 0;
}

/* Write a version stamp.  Don't write anything if we are running as a
   cross-compiler.  Otherwise, use the versions in /usr/include/stamp.h.  */

#ifdef HAVE_STAMP_H
#include <stamp.h>
#endif

void
alpha_write_verstamp (file)
     FILE *file ATTRIBUTE_UNUSED;
{
#ifdef MS_STAMP
  fprintf (file, "\t.verstamp %d %d\n", MS_STAMP, LS_STAMP);
#endif
}

/* Helper function to set RTX_FRAME_RELATED_P on instructions, including
   sequences.  */

static rtx
set_frame_related_p ()
{
  rtx seq = gen_sequence ();
  end_sequence ();

  if (GET_CODE (seq) == SEQUENCE)
    {
      int i = XVECLEN (seq, 0);
      while (--i >= 0)
	RTX_FRAME_RELATED_P (XVECEXP (seq, 0, i)) = 1;
     return emit_insn (seq);
    }
  else
    {
      seq = emit_insn (seq);
      RTX_FRAME_RELATED_P (seq) = 1;
      return seq;
    }
}

#define FRP(exp)  (start_sequence (), exp, set_frame_related_p ())

/* Write function prologue.  */

/* On vms we have two kinds of functions:

   - stack frame (PROC_STACK)
	these are 'normal' functions with local vars and which are
	calling other functions
   - register frame (PROC_REGISTER)
	keeps all data in registers, needs no stack

   We must pass this to the assembler so it can generate the
   proper pdsc (procedure descriptor)
   This is done with the '.pdesc' command.

   On not-vms, we don't really differentiate between the two, as we can
   simply allocate stack without saving registers.  */

void
alpha_expand_prologue ()
{
  /* Registers to save.  */
  unsigned long imask = 0;
  unsigned long fmask = 0;
  /* Stack space needed for pushing registers clobbered by us.  */
  HOST_WIDE_INT sa_size;
  /* Complete stack size needed.  */
  HOST_WIDE_INT frame_size;
  /* Offset from base reg to register save area.  */
  HOST_WIDE_INT reg_offset;
  rtx sa_reg, mem;
  int i;

  sa_size = alpha_sa_size ();

  frame_size = get_frame_size ();
  if (TARGET_ABI_OPEN_VMS)
    frame_size = ALPHA_ROUND (sa_size 
			      + (alpha_is_stack_procedure ? 8 : 0)
			      + frame_size
			      + current_function_pretend_args_size);
  else if (TARGET_ABI_UNICOSMK)
    /* We have to allocate space for the DSIB if we generate a frame.  */
    frame_size = ALPHA_ROUND (sa_size
			      + (alpha_is_stack_procedure ? 48 : 0))
		 + ALPHA_ROUND (frame_size
				+ current_function_outgoing_args_size);
  else
    frame_size = (ALPHA_ROUND (current_function_outgoing_args_size)
		  + sa_size
		  + ALPHA_ROUND (frame_size
				 + current_function_pretend_args_size));

  if (TARGET_ABI_OPEN_VMS)
    reg_offset = 8;
  else
    reg_offset = ALPHA_ROUND (current_function_outgoing_args_size);

  alpha_sa_mask (&imask, &fmask);

  /* Emit an insn to reload GP, if needed.  */
  if (TARGET_ABI_OSF)
    {
      alpha_function_needs_gp = alpha_does_function_need_gp ();
      if (alpha_function_needs_gp)
	emit_insn (gen_prologue_ldgp ());
    }

  /* TARGET_PROFILING_NEEDS_GP actually implies that we need to insert
     the call to mcount ourselves, rather than having the linker do it
     magically in response to -pg.  Since _mcount has special linkage,
     don't represent the call as a call.  */
  if (TARGET_PROFILING_NEEDS_GP && current_function_profile)
    emit_insn (gen_prologue_mcount ());

  if (TARGET_ABI_UNICOSMK)
    unicosmk_gen_dsib (&imask);

  /* Adjust the stack by the frame size.  If the frame size is > 4096
     bytes, we need to be sure we probe somewhere in the first and last
     4096 bytes (we can probably get away without the latter test) and
     every 8192 bytes in between.  If the frame size is > 32768, we
     do this in a loop.  Otherwise, we generate the explicit probe
     instructions. 

     Note that we are only allowed to adjust sp once in the prologue.  */

  if (frame_size <= 32768)
    {
      if (frame_size > 4096)
	{
	  int probed = 4096;

	  do
	    emit_insn (gen_probe_stack (GEN_INT (TARGET_ABI_UNICOSMK
						 ? -probed + 64
						 : -probed)));
	  while ((probed += 8192) < frame_size);

	  /* We only have to do this probe if we aren't saving registers.  */
	  if (sa_size == 0 && probed + 4096 < frame_size)
	    emit_insn (gen_probe_stack (GEN_INT (-frame_size)));
	}

      if (frame_size != 0)
	FRP (emit_insn (gen_adddi3 (stack_pointer_rtx, stack_pointer_rtx,
				    GEN_INT (TARGET_ABI_UNICOSMK
					     ? -frame_size + 64
					     : -frame_size))));
    }
  else
    {
      /* Here we generate code to set R22 to SP + 4096 and set R23 to the
	 number of 8192 byte blocks to probe.  We then probe each block
	 in the loop and then set SP to the proper location.  If the
	 amount remaining is > 4096, we have to do one more probe if we
	 are not saving any registers.  */

      HOST_WIDE_INT blocks = (frame_size + 4096) / 8192;
      HOST_WIDE_INT leftover = frame_size + 4096 - blocks * 8192;
      rtx ptr = gen_rtx_REG (DImode, 22);
      rtx count = gen_rtx_REG (DImode, 23);
      rtx seq;

      emit_move_insn (count, GEN_INT (blocks));
      emit_insn (gen_adddi3 (ptr, stack_pointer_rtx,
			     GEN_INT (TARGET_ABI_UNICOSMK ? 4096 - 64 : 4096)));

      /* Because of the difficulty in emitting a new basic block this
	 late in the compilation, generate the loop as a single insn.  */
      emit_insn (gen_prologue_stack_probe_loop (count, ptr));

      if (leftover > 4096 && sa_size == 0)
	{
	  rtx last = gen_rtx_MEM (DImode, plus_constant (ptr, -leftover));
	  MEM_VOLATILE_P (last) = 1;
	  emit_move_insn (last, const0_rtx);
	}

      if (TARGET_ABI_WINDOWS_NT)
	{
	  /* For NT stack unwind (done by 'reverse execution'), it's
	     not OK to take the result of a loop, even though the value
	     is already in ptr, so we reload it via a single operation
	     and subtract it to sp. 

	     Yes, that's correct -- we have to reload the whole constant
	     into a temporary via ldah+lda then subtract from sp.  To
	     ensure we get ldah+lda, we use a special pattern.  */

	  HOST_WIDE_INT lo, hi;
	  lo = ((frame_size & 0xffff) ^ 0x8000) - 0x8000;
	  hi = frame_size - lo;

	  emit_move_insn (ptr, GEN_INT (hi));
	  emit_insn (gen_nt_lda (ptr, GEN_INT (lo)));
	  seq = emit_insn (gen_subdi3 (stack_pointer_rtx, stack_pointer_rtx,
				       ptr));
	}
      else
	{
	  seq = emit_insn (gen_adddi3 (stack_pointer_rtx, ptr,
				       GEN_INT (-leftover)));
	}

      /* This alternative is special, because the DWARF code cannot
         possibly intuit through the loop above.  So we invent this
         note it looks at instead.  */
      RTX_FRAME_RELATED_P (seq) = 1;
      REG_NOTES (seq)
        = gen_rtx_EXPR_LIST (REG_FRAME_RELATED_EXPR,
			     gen_rtx_SET (VOIDmode, stack_pointer_rtx,
			       gen_rtx_PLUS (Pmode, stack_pointer_rtx,
					     GEN_INT (TARGET_ABI_UNICOSMK
						      ? -frame_size + 64
						      : -frame_size))),
			     REG_NOTES (seq));
    }

  if (!TARGET_ABI_UNICOSMK)
    {
      /* Cope with very large offsets to the register save area.  */
      sa_reg = stack_pointer_rtx;
      if (reg_offset + sa_size > 0x8000)
	{
	  int low = ((reg_offset & 0xffff) ^ 0x8000) - 0x8000;
	  HOST_WIDE_INT bias;

	  if (low + sa_size <= 0x8000)
	    bias = reg_offset - low, reg_offset = low;
	  else 
	    bias = reg_offset, reg_offset = 0;

	  sa_reg = gen_rtx_REG (DImode, 24);
	  FRP (emit_insn (gen_adddi3 (sa_reg, stack_pointer_rtx,
				      GEN_INT (bias))));
	}
    
      /* Save regs in stack order.  Beginning with VMS PV.  */
      if (TARGET_ABI_OPEN_VMS && alpha_is_stack_procedure)
	{
	  mem = gen_rtx_MEM (DImode, stack_pointer_rtx);
	  set_mem_alias_set (mem, alpha_sr_alias_set);
	  FRP (emit_move_insn (mem, gen_rtx_REG (DImode, REG_PV)));
	}

      /* Save register RA next.  */
      if (imask & (1L << REG_RA))
	{
	  mem = gen_rtx_MEM (DImode, plus_constant (sa_reg, reg_offset));
	  set_mem_alias_set (mem, alpha_sr_alias_set);
	  FRP (emit_move_insn (mem, gen_rtx_REG (DImode, REG_RA)));
	  imask &= ~(1L << REG_RA);
	  reg_offset += 8;
	}

      /* Now save any other registers required to be saved.  */
      for (i = 0; i < 32; i++)
	if (imask & (1L << i))
	  {
	    mem = gen_rtx_MEM (DImode, plus_constant (sa_reg, reg_offset));
	    set_mem_alias_set (mem, alpha_sr_alias_set);
	    FRP (emit_move_insn (mem, gen_rtx_REG (DImode, i)));
	    reg_offset += 8;
	  }

      for (i = 0; i < 32; i++)
	if (fmask & (1L << i))
	  {
	    mem = gen_rtx_MEM (DFmode, plus_constant (sa_reg, reg_offset));
	    set_mem_alias_set (mem, alpha_sr_alias_set);
	    FRP (emit_move_insn (mem, gen_rtx_REG (DFmode, i+32)));
	    reg_offset += 8;
	  }
    }
  else if (TARGET_ABI_UNICOSMK && alpha_is_stack_procedure)
    {
      /* The standard frame on the T3E includes space for saving registers.
	 We just have to use it. We don't have to save the return address and
	 the old frame pointer here - they are saved in the DSIB.  */

      reg_offset = -56;
      for (i = 9; i < 15; i++)
	if (imask & (1L << i))
	  {
	    mem = gen_rtx_MEM (DImode, plus_constant(hard_frame_pointer_rtx,
						     reg_offset));
	    set_mem_alias_set (mem, alpha_sr_alias_set);
	    FRP (emit_move_insn (mem, gen_rtx_REG (DImode, i)));
	    reg_offset -= 8;
	  }
      for (i = 2; i < 10; i++)
	if (fmask & (1L << i))
	  {
	    mem = gen_rtx_MEM (DFmode, plus_constant (hard_frame_pointer_rtx,
						      reg_offset));
	    set_mem_alias_set (mem, alpha_sr_alias_set);
	    FRP (emit_move_insn (mem, gen_rtx_REG (DFmode, i+32)));
	    reg_offset -= 8;
	  }
    }

  if (TARGET_ABI_OPEN_VMS)
    {
      if (!alpha_is_stack_procedure)
	/* Register frame procedures save the fp.  */
	/* ??? Ought to have a dwarf2 save for this.  */
	emit_move_insn (gen_rtx_REG (DImode, vms_save_fp_regno),
			hard_frame_pointer_rtx);

      if (vms_base_regno != REG_PV)
	emit_insn (gen_force_movdi (gen_rtx_REG (DImode, vms_base_regno),
				    gen_rtx_REG (DImode, REG_PV)));

      if (vms_unwind_regno == HARD_FRAME_POINTER_REGNUM)
	FRP (emit_move_insn (hard_frame_pointer_rtx, stack_pointer_rtx));

      /* If we have to allocate space for outgoing args, do it now.  */
      if (current_function_outgoing_args_size != 0)
	FRP (emit_move_insn
	     (stack_pointer_rtx, 
	      plus_constant (hard_frame_pointer_rtx,
			     - (ALPHA_ROUND
				(current_function_outgoing_args_size)))));
    }
  else if (!TARGET_ABI_UNICOSMK)
    {
      /* If we need a frame pointer, set it from the stack pointer.  */
      if (frame_pointer_needed)
	{
	  if (TARGET_CAN_FAULT_IN_PROLOGUE)
	    FRP (emit_move_insn (hard_frame_pointer_rtx, stack_pointer_rtx));
	  else
	    /* This must always be the last instruction in the
	       prologue, thus we emit a special move + clobber.  */
	      FRP (emit_insn (gen_init_fp (hard_frame_pointer_rtx,
				           stack_pointer_rtx, sa_reg)));
	}
    }

  /* The ABIs for VMS and OSF/1 say that while we can schedule insns into
     the prologue, for exception handling reasons, we cannot do this for
     any insn that might fault.  We could prevent this for mems with a
     (clobber:BLK (scratch)), but this doesn't work for fp insns.  So we
     have to prevent all such scheduling with a blockage.

     Linux, on the other hand, never bothered to implement OSF/1's 
     exception handling, and so doesn't care about such things.  Anyone
     planning to use dwarf2 frame-unwind info can also omit the blockage.  */

  if (! TARGET_CAN_FAULT_IN_PROLOGUE)
    emit_insn (gen_blockage ());
}

/* Output the textual info surrounding the prologue.  */

void
alpha_start_function (file, fnname, decl)
     FILE *file;
     const char *fnname;
     tree decl ATTRIBUTE_UNUSED;
{
  unsigned long imask = 0;
  unsigned long fmask = 0;
  /* Stack space needed for pushing registers clobbered by us.  */
  HOST_WIDE_INT sa_size;
  /* Complete stack size needed.  */
  HOST_WIDE_INT frame_size;
  /* Offset from base reg to register save area.  */
  HOST_WIDE_INT reg_offset;
  char *entry_label = (char *) alloca (strlen (fnname) + 6);
  int i;

  /* Don't emit an extern directive for functions defined in the same file.  */
  if (TARGET_ABI_UNICOSMK)
    {
      tree name_tree;
      name_tree = get_identifier (fnname);
      TREE_ASM_WRITTEN (name_tree) = 1;
    }

  alpha_fnname = fnname;
  sa_size = alpha_sa_size ();

  frame_size = get_frame_size ();
  if (TARGET_ABI_OPEN_VMS)
    frame_size = ALPHA_ROUND (sa_size 
			      + (alpha_is_stack_procedure ? 8 : 0)
			      + frame_size
			      + current_function_pretend_args_size);
  else if (TARGET_ABI_UNICOSMK)
    frame_size = ALPHA_ROUND (sa_size
			      + (alpha_is_stack_procedure ? 48 : 0))
		 + ALPHA_ROUND (frame_size
			      + current_function_outgoing_args_size);
  else
    frame_size = (ALPHA_ROUND (current_function_outgoing_args_size)
		  + sa_size
		  + ALPHA_ROUND (frame_size
				 + current_function_pretend_args_size));

  if (TARGET_ABI_OPEN_VMS)
    reg_offset = 8;
  else
    reg_offset = ALPHA_ROUND (current_function_outgoing_args_size);

  alpha_sa_mask (&imask, &fmask);

  /* Ecoff can handle multiple .file directives, so put out file and lineno.
     We have to do that before the .ent directive as we cannot switch
     files within procedures with native ecoff because line numbers are
     linked to procedure descriptors.
     Outputting the lineno helps debugging of one line functions as they
     would otherwise get no line number at all. Please note that we would
     like to put out last_linenum from final.c, but it is not accessible.  */

  if (write_symbols == SDB_DEBUG)
    {
#ifdef ASM_OUTPUT_SOURCE_FILENAME
      ASM_OUTPUT_SOURCE_FILENAME (file,
				  DECL_SOURCE_FILE (current_function_decl));
#endif
#ifdef ASM_OUTPUT_SOURCE_LINE
      if (debug_info_level != DINFO_LEVEL_TERSE)
        ASM_OUTPUT_SOURCE_LINE (file,
				DECL_SOURCE_LINE (current_function_decl));
#endif
    }

  /* Issue function start and label.  */
  if (TARGET_ABI_OPEN_VMS
      || (!TARGET_ABI_UNICOSMK && !flag_inhibit_size_directive))
    {
      fputs ("\t.ent ", file);
      assemble_name (file, fnname);
      putc ('\n', file);

      /* If the function needs GP, we'll write the "..ng" label there.
	 Otherwise, do it here.  */
      if (TARGET_ABI_OSF && ! alpha_function_needs_gp)
	{
	  putc ('$', file);
	  assemble_name (file, fnname);
	  fputs ("..ng:\n", file);
	}
    }

  strcpy (entry_label, fnname);
  if (TARGET_ABI_OPEN_VMS)
    strcat (entry_label, "..en");

  /* For public functions, the label must be globalized by appending an
     additional colon.  */
  if (TARGET_ABI_UNICOSMK && TREE_PUBLIC (decl))
    strcat (entry_label, ":");

  ASM_OUTPUT_LABEL (file, entry_label);
  inside_function = TRUE;

  if (TARGET_ABI_OPEN_VMS)
    fprintf (file, "\t.base $%d\n", vms_base_regno);

  if (!TARGET_ABI_OPEN_VMS && !TARGET_ABI_UNICOSMK && TARGET_IEEE_CONFORMANT
      && !flag_inhibit_size_directive)
    {
      /* Set flags in procedure descriptor to request IEEE-conformant
	 math-library routines.  The value we set it to is PDSC_EXC_IEEE
	 (/usr/include/pdsc.h).  */
      fputs ("\t.eflag 48\n", file);
    }

  /* Set up offsets to alpha virtual arg/local debugging pointer.  */
  alpha_auto_offset = -frame_size + current_function_pretend_args_size;
  alpha_arg_offset = -frame_size + 48;

  /* Describe our frame.  If the frame size is larger than an integer,
     print it as zero to avoid an assembler error.  We won't be
     properly describing such a frame, but that's the best we can do.  */
  if (TARGET_ABI_UNICOSMK)
    ;
  else if (TARGET_ABI_OPEN_VMS)
    {
      fprintf (file, "\t.frame $%d,", vms_unwind_regno);
      fprintf (file, HOST_WIDE_INT_PRINT_DEC,
	       frame_size >= ((HOST_WIDE_INT) 1 << 31) ? 0 : frame_size);
      fputs (",$26,", file);
      fprintf (file, HOST_WIDE_INT_PRINT_DEC, reg_offset);
      fputs ("\n", file);
    }
  else if (!flag_inhibit_size_directive)
    {
      fprintf (file, "\t.frame $%d,",
	       (frame_pointer_needed
		? HARD_FRAME_POINTER_REGNUM : STACK_POINTER_REGNUM));
      fprintf (file, HOST_WIDE_INT_PRINT_DEC,
	       frame_size >= (1l << 31) ? 0 : frame_size);
      fprintf (file, ",$26,%d\n", current_function_pretend_args_size);
    }

  /* Describe which registers were spilled.  */
  if (TARGET_ABI_UNICOSMK)
    ;
  else if (TARGET_ABI_OPEN_VMS)
    {
      if (imask)
        /* ??? Does VMS care if mask contains ra?  The old code didn't
           set it, so I don't here.  */
	fprintf (file, "\t.mask 0x%lx,0\n", imask & ~(1L << REG_RA));
      if (fmask)
	fprintf (file, "\t.fmask 0x%lx,0\n", fmask);
      if (!alpha_is_stack_procedure)
	fprintf (file, "\t.fp_save $%d\n", vms_save_fp_regno);
    }
  else if (!flag_inhibit_size_directive)
    {
      if (imask)
	{
	  fprintf (file, "\t.mask 0x%lx,", imask);
	  fprintf (file, HOST_WIDE_INT_PRINT_DEC,
		   frame_size >= (1l << 31) ? 0 : reg_offset - frame_size);
	  putc ('\n', file);

	  for (i = 0; i < 32; ++i)
	    if (imask & (1L << i))
	      reg_offset += 8;
	}

      if (fmask)
	{
	  fprintf (file, "\t.fmask 0x%lx,", fmask);
	  fprintf (file, HOST_WIDE_INT_PRINT_DEC,
		   frame_size >= (1l << 31) ? 0 : reg_offset - frame_size);
	  putc ('\n', file);
	}
    }

#if TARGET_ABI_OPEN_VMS
  /* Ifdef'ed cause readonly_section and link_section are only
     available then.  */
  readonly_section ();
  fprintf (file, "\t.align 3\n");
  assemble_name (file, fnname); fputs ("..na:\n", file);
  fputs ("\t.ascii \"", file);
  assemble_name (file, fnname);
  fputs ("\\0\"\n", file);
      
  link_section ();
  fprintf (file, "\t.align 3\n");
  fputs ("\t.name ", file);
  assemble_name (file, fnname);
  fputs ("..na\n", file);
  ASM_OUTPUT_LABEL (file, fnname);
  fprintf (file, "\t.pdesc ");
  assemble_name (file, fnname);
  fprintf (file, "..en,%s\n", alpha_is_stack_procedure ? "stack" : "reg");
  alpha_need_linkage (fnname, 1);
  text_section ();
#endif
}

/* Emit the .prologue note at the scheduled end of the prologue.  */

static void
alpha_output_function_end_prologue (file)
     FILE *file;
{
  if (TARGET_ABI_UNICOSMK)
    ;
  else if (TARGET_ABI_OPEN_VMS)
    fputs ("\t.prologue\n", file);
  else if (TARGET_ABI_WINDOWS_NT)
    fputs ("\t.prologue 0\n", file);
  else if (!flag_inhibit_size_directive)
    fprintf (file, "\t.prologue %d\n", alpha_function_needs_gp);
}

/* Write function epilogue.  */

/* ??? At some point we will want to support full unwind, and so will 
   need to mark the epilogue as well.  At the moment, we just confuse
   dwarf2out.  */
#undef FRP
#define FRP(exp) exp

void
alpha_expand_epilogue ()
{
  /* Registers to save.  */
  unsigned long imask = 0;
  unsigned long fmask = 0;
  /* Stack space needed for pushing registers clobbered by us.  */
  HOST_WIDE_INT sa_size;
  /* Complete stack size needed.  */
  HOST_WIDE_INT frame_size;
  /* Offset from base reg to register save area.  */
  HOST_WIDE_INT reg_offset;
  int fp_is_frame_pointer, fp_offset;
  rtx sa_reg, sa_reg_exp = NULL;
  rtx sp_adj1, sp_adj2, mem;
  rtx eh_ofs;
  int i;

  sa_size = alpha_sa_size ();

  frame_size = get_frame_size ();
  if (TARGET_ABI_OPEN_VMS)
    frame_size = ALPHA_ROUND (sa_size 
			      + (alpha_is_stack_procedure ? 8 : 0)
			      + frame_size
			      + current_function_pretend_args_size);
  else if (TARGET_ABI_UNICOSMK)
    frame_size = ALPHA_ROUND (sa_size
			      + (alpha_is_stack_procedure ? 48 : 0))
		 + ALPHA_ROUND (frame_size
			      + current_function_outgoing_args_size);
  else
    frame_size = (ALPHA_ROUND (current_function_outgoing_args_size)
		  + sa_size
		  + ALPHA_ROUND (frame_size
				 + current_function_pretend_args_size));

  if (TARGET_ABI_OPEN_VMS)
    reg_offset = 8;
  else
    reg_offset = ALPHA_ROUND (current_function_outgoing_args_size);

  alpha_sa_mask (&imask, &fmask);

  fp_is_frame_pointer = ((TARGET_ABI_OPEN_VMS && alpha_is_stack_procedure)
			 || (!TARGET_ABI_OPEN_VMS && frame_pointer_needed));
  fp_offset = 0;
  sa_reg = stack_pointer_rtx;

  if (current_function_calls_eh_return)
    eh_ofs = EH_RETURN_STACKADJ_RTX;
  else
    eh_ofs = NULL_RTX;

  if (!TARGET_ABI_UNICOSMK && sa_size)
    {
      /* If we have a frame pointer, restore SP from it.  */
      if ((TARGET_ABI_OPEN_VMS
	   && vms_unwind_regno == HARD_FRAME_POINTER_REGNUM)
	  || (!TARGET_ABI_OPEN_VMS && frame_pointer_needed))
	FRP (emit_move_insn (stack_pointer_rtx, hard_frame_pointer_rtx));

      /* Cope with very large offsets to the register save area.  */
      if (reg_offset + sa_size > 0x8000)
	{
	  int low = ((reg_offset & 0xffff) ^ 0x8000) - 0x8000;
	  HOST_WIDE_INT bias;

	  if (low + sa_size <= 0x8000)
	    bias = reg_offset - low, reg_offset = low;
	  else 
	    bias = reg_offset, reg_offset = 0;

	  sa_reg = gen_rtx_REG (DImode, 22);
	  sa_reg_exp = plus_constant (stack_pointer_rtx, bias);

	  FRP (emit_move_insn (sa_reg, sa_reg_exp));
	}
	  
      /* Restore registers in order, excepting a true frame pointer.  */

      mem = gen_rtx_MEM (DImode, plus_constant (sa_reg, reg_offset));
      if (! eh_ofs)
        set_mem_alias_set (mem, alpha_sr_alias_set);
      FRP (emit_move_insn (gen_rtx_REG (DImode, REG_RA), mem));

      reg_offset += 8;
      imask &= ~(1L << REG_RA);

      for (i = 0; i < 32; ++i)
	if (imask & (1L << i))
	  {
	    if (i == HARD_FRAME_POINTER_REGNUM && fp_is_frame_pointer)
	      fp_offset = reg_offset;
	    else
	      {
		mem = gen_rtx_MEM (DImode, plus_constant(sa_reg, reg_offset));
		set_mem_alias_set (mem, alpha_sr_alias_set);
		FRP (emit_move_insn (gen_rtx_REG (DImode, i), mem));
	      }
	    reg_offset += 8;
	  }

      for (i = 0; i < 32; ++i)
	if (fmask & (1L << i))
	  {
	    mem = gen_rtx_MEM (DFmode, plus_constant(sa_reg, reg_offset));
	    set_mem_alias_set (mem, alpha_sr_alias_set);
	    FRP (emit_move_insn (gen_rtx_REG (DFmode, i+32), mem));
	    reg_offset += 8;
	  }
    }
  else if (TARGET_ABI_UNICOSMK && alpha_is_stack_procedure)
    {
      /* Restore callee-saved general-purpose registers.  */

      reg_offset = -56;

      for (i = 9; i < 15; i++)
	if (imask & (1L << i))
	  {
	    mem = gen_rtx_MEM (DImode, plus_constant(hard_frame_pointer_rtx,
						     reg_offset));
	    set_mem_alias_set (mem, alpha_sr_alias_set);
	    FRP (emit_move_insn (gen_rtx_REG (DImode, i), mem));
	    reg_offset -= 8;
	  }

      for (i = 2; i < 10; i++)
	if (fmask & (1L << i))
	  {
	    mem = gen_rtx_MEM (DFmode, plus_constant(hard_frame_pointer_rtx,
						     reg_offset));
	    set_mem_alias_set (mem, alpha_sr_alias_set);
	    FRP (emit_move_insn (gen_rtx_REG (DFmode, i+32), mem));
	    reg_offset -= 8;
	  }

      /* Restore the return address from the DSIB.  */

      mem = gen_rtx_MEM (DImode, plus_constant(hard_frame_pointer_rtx, -8));
      set_mem_alias_set (mem, alpha_sr_alias_set);
      FRP (emit_move_insn (gen_rtx_REG (DImode, REG_RA), mem));
    }

  if (frame_size || eh_ofs)
    {
      sp_adj1 = stack_pointer_rtx;

      if (eh_ofs)
	{
	  sp_adj1 = gen_rtx_REG (DImode, 23);
	  emit_move_insn (sp_adj1,
			  gen_rtx_PLUS (Pmode, stack_pointer_rtx, eh_ofs));
	}

      /* If the stack size is large, begin computation into a temporary
	 register so as not to interfere with a potential fp restore,
	 which must be consecutive with an SP restore.  */
      if (frame_size < 32768
	  && ! (TARGET_ABI_UNICOSMK && current_function_calls_alloca))
	sp_adj2 = GEN_INT (frame_size);
      else if (TARGET_ABI_UNICOSMK)
	{
	  sp_adj1 = gen_rtx_REG (DImode, 23);
	  FRP (emit_move_insn (sp_adj1, hard_frame_pointer_rtx));
	  sp_adj2 = const0_rtx;
	}
      else if (frame_size < 0x40007fffL)
	{
	  int low = ((frame_size & 0xffff) ^ 0x8000) - 0x8000;

	  sp_adj2 = plus_constant (sp_adj1, frame_size - low);
	  if (sa_reg_exp && rtx_equal_p (sa_reg_exp, sp_adj2))
	    sp_adj1 = sa_reg;
	  else
	    {
	      sp_adj1 = gen_rtx_REG (DImode, 23);
	      FRP (emit_move_insn (sp_adj1, sp_adj2));
	    }
	  sp_adj2 = GEN_INT (low);
	}
      else
	{
	  rtx tmp = gen_rtx_REG (DImode, 23);
	  FRP (sp_adj2 = alpha_emit_set_const (tmp, DImode, frame_size, 3));
	  if (!sp_adj2)
	    {
	      /* We can't drop new things to memory this late, afaik,
		 so build it up by pieces.  */
	      FRP (sp_adj2 = alpha_emit_set_long_const (tmp, frame_size,
							-(frame_size < 0)));
	      if (!sp_adj2)
		abort ();
	    }
	}

      /* From now on, things must be in order.  So emit blockages.  */

      /* Restore the frame pointer.  */
      if (TARGET_ABI_UNICOSMK)
	{
	  emit_insn (gen_blockage ());
	  mem = gen_rtx_MEM (DImode,
			     plus_constant (hard_frame_pointer_rtx, -16));
	  set_mem_alias_set (mem, alpha_sr_alias_set);
	  FRP (emit_move_insn (hard_frame_pointer_rtx, mem));
	}
      else if (fp_is_frame_pointer)
	{
	  emit_insn (gen_blockage ());
	  mem = gen_rtx_MEM (DImode, plus_constant (sa_reg, fp_offset));
	  set_mem_alias_set (mem, alpha_sr_alias_set);
	  FRP (emit_move_insn (hard_frame_pointer_rtx, mem));
	}
      else if (TARGET_ABI_OPEN_VMS)
	{
	  emit_insn (gen_blockage ());
	  FRP (emit_move_insn (hard_frame_pointer_rtx,
			       gen_rtx_REG (DImode, vms_save_fp_regno)));
	}

      /* Restore the stack pointer.  */
      emit_insn (gen_blockage ());
      if (sp_adj2 == const0_rtx)
	FRP (emit_move_insn (stack_pointer_rtx, sp_adj1));
      else
	FRP (emit_move_insn (stack_pointer_rtx,
			     gen_rtx_PLUS (DImode, sp_adj1, sp_adj2)));
    }
  else 
    {
      if (TARGET_ABI_OPEN_VMS && !alpha_is_stack_procedure)
        {
          emit_insn (gen_blockage ());
          FRP (emit_move_insn (hard_frame_pointer_rtx,
			       gen_rtx_REG (DImode, vms_save_fp_regno)));
        }
      else if (TARGET_ABI_UNICOSMK && !alpha_is_stack_procedure)
	{
	  /* Decrement the frame pointer if the function does not have a
	     frame.  */

	  emit_insn (gen_blockage ());
	  FRP (emit_insn (gen_adddi3 (hard_frame_pointer_rtx,
				      hard_frame_pointer_rtx, GEN_INT (-1))));
        }
    }
}

/* Output the rest of the textual info surrounding the epilogue.  */

void
alpha_end_function (file, fnname, decl)
     FILE *file;
     const char *fnname;
     tree decl ATTRIBUTE_UNUSED;
{
  /* End the function.  */
  if (!TARGET_ABI_UNICOSMK && !flag_inhibit_size_directive)
    {
      fputs ("\t.end ", file);
      assemble_name (file, fnname);
      putc ('\n', file);
    }
  inside_function = FALSE;

  /* Show that we know this function if it is called again. 

     Don't do this for global functions in object files destined for a
     shared library because the function may be overridden by the application
     or other libraries.  Similarly, don't do this for weak functions.

     Don't do this for functions not defined in the .text section, as
     otherwise it's not unlikely that the destination is out of range
     for a direct branch.  */

  if (!DECL_WEAK (current_function_decl)
      && (!flag_pic || !TREE_PUBLIC (current_function_decl))
      && decl_in_text_section (current_function_decl))
    SYMBOL_REF_FLAG (XEXP (DECL_RTL (current_function_decl), 0)) = 1;

  /* Output jump tables and the static subroutine information block.  */
  if (TARGET_ABI_UNICOSMK)
    {
      unicosmk_output_ssib (file, fnname);
      unicosmk_output_deferred_case_vectors (file);
    }
}

/* Debugging support.  */

#include "gstab.h"

/* Count the number of sdb related labels are generated (to find block
   start and end boundaries).  */

int sdb_label_count = 0;

/* Next label # for each statement.  */

static int sym_lineno = 0;

/* Count the number of .file directives, so that .loc is up to date.  */

static int num_source_filenames = 0;

/* Name of the file containing the current function.  */

static const char *current_function_file = "";

/* Offsets to alpha virtual arg/local debugging pointers.  */

long alpha_arg_offset;
long alpha_auto_offset;

/* Emit a new filename to a stream.  */

void
alpha_output_filename (stream, name)
     FILE *stream;
     const char *name;
{
  static int first_time = TRUE;
  char ltext_label_name[100];

  if (first_time)
    {
      first_time = FALSE;
      ++num_source_filenames;
      current_function_file = name;
      fprintf (stream, "\t.file\t%d ", num_source_filenames);
      output_quoted_string (stream, name);
      fprintf (stream, "\n");
      if (!TARGET_GAS && write_symbols == DBX_DEBUG)
	fprintf (stream, "\t#@stabs\n");
    }

  else if (write_symbols == DBX_DEBUG)
    {
      ASM_GENERATE_INTERNAL_LABEL (ltext_label_name, "Ltext", 0);
      fprintf (stream, "%s", ASM_STABS_OP);
      output_quoted_string (stream, name);
      fprintf (stream, ",%d,0,0,%s\n", N_SOL, &ltext_label_name[1]);
    }

  else if (name != current_function_file
	   && strcmp (name, current_function_file) != 0)
    {
      if (inside_function && ! TARGET_GAS)
	fprintf (stream, "\t#.file\t%d ", num_source_filenames);
      else
	{
	  ++num_source_filenames;
	  current_function_file = name;
	  fprintf (stream, "\t.file\t%d ", num_source_filenames);
	}

      output_quoted_string (stream, name);
      fprintf (stream, "\n");
    }
}

/* Emit a linenumber to a stream.  */

void
alpha_output_lineno (stream, line)
     FILE *stream;
     int line;
{
  if (write_symbols == DBX_DEBUG)
    {
      /* mips-tfile doesn't understand .stabd directives.  */
      ++sym_lineno;
      fprintf (stream, "$LM%d:\n%s%d,0,%d,$LM%d\n",
	       sym_lineno, ASM_STABN_OP, N_SLINE, line, sym_lineno);
    }
  else
    fprintf (stream, "\n\t.loc\t%d %d\n", num_source_filenames, line);
}

/* Structure to show the current status of registers and memory.  */

struct shadow_summary
{
  struct {
    unsigned int i     : 31;	/* Mask of int regs */
    unsigned int fp    : 31;	/* Mask of fp regs */
    unsigned int mem   :  1;	/* mem == imem | fpmem */
  } used, defd;
};

static void summarize_insn PARAMS ((rtx, struct shadow_summary *, int));
static void alpha_handle_trap_shadows PARAMS ((rtx));

/* Summary the effects of expression X on the machine.  Update SUM, a pointer
   to the summary structure.  SET is nonzero if the insn is setting the
   object, otherwise zero.  */

static void
summarize_insn (x, sum, set)
     rtx x;
     struct shadow_summary *sum;
     int set;
{
  const char *format_ptr;
  int i, j;

  if (x == 0)
    return;

  switch (GET_CODE (x))
    {
      /* ??? Note that this case would be incorrect if the Alpha had a
	 ZERO_EXTRACT in SET_DEST.  */
    case SET:
      summarize_insn (SET_SRC (x), sum, 0);
      summarize_insn (SET_DEST (x), sum, 1);
      break;

    case CLOBBER:
      summarize_insn (XEXP (x, 0), sum, 1);
      break;

    case USE:
      summarize_insn (XEXP (x, 0), sum, 0);
      break;

    case ASM_OPERANDS:
      for (i = ASM_OPERANDS_INPUT_LENGTH (x) - 1; i >= 0; i--)
	summarize_insn (ASM_OPERANDS_INPUT (x, i), sum, 0);
      break;

    case PARALLEL:
      for (i = XVECLEN (x, 0) - 1; i >= 0; i--)
	summarize_insn (XVECEXP (x, 0, i), sum, 0);
      break;

    case SUBREG:
      summarize_insn (SUBREG_REG (x), sum, 0);
      break;

    case REG:
      {
	int regno = REGNO (x);
	unsigned long mask = ((unsigned long) 1) << (regno % 32);

	if (regno == 31 || regno == 63)
	  break;

	if (set)
	  {
	    if (regno < 32)
	      sum->defd.i |= mask;
	    else
	      sum->defd.fp |= mask;
	  }
	else
	  {
	    if (regno < 32)
	      sum->used.i  |= mask;
	    else
	      sum->used.fp |= mask;
	  }
	}
      break;

    case MEM:
      if (set)
	sum->defd.mem = 1;
      else
	sum->used.mem = 1;

      /* Find the regs used in memory address computation: */
      summarize_insn (XEXP (x, 0), sum, 0);
      break;

    case CONST_INT:   case CONST_DOUBLE:
    case SYMBOL_REF:  case LABEL_REF:     case CONST:
    case SCRATCH:     case ASM_INPUT:
      break;

      /* Handle common unary and binary ops for efficiency.  */
    case COMPARE:  case PLUS:    case MINUS:   case MULT:      case DIV:
    case MOD:      case UDIV:    case UMOD:    case AND:       case IOR:
    case XOR:      case ASHIFT:  case ROTATE:  case ASHIFTRT:  case LSHIFTRT:
    case ROTATERT: case SMIN:    case SMAX:    case UMIN:      case UMAX:
    case NE:       case EQ:      case GE:      case GT:        case LE:
    case LT:       case GEU:     case GTU:     case LEU:       case LTU:
      summarize_insn (XEXP (x, 0), sum, 0);
      summarize_insn (XEXP (x, 1), sum, 0);
      break;

    case NEG:  case NOT:  case SIGN_EXTEND:  case ZERO_EXTEND:
    case TRUNCATE:  case FLOAT_EXTEND:  case FLOAT_TRUNCATE:  case FLOAT:
    case FIX:  case UNSIGNED_FLOAT:  case UNSIGNED_FIX:  case ABS:
    case SQRT:  case FFS: 
      summarize_insn (XEXP (x, 0), sum, 0);
      break;

    default:
      format_ptr = GET_RTX_FORMAT (GET_CODE (x));
      for (i = GET_RTX_LENGTH (GET_CODE (x)) - 1; i >= 0; i--)
	switch (format_ptr[i])
	  {
	  case 'e':
	    summarize_insn (XEXP (x, i), sum, 0);
	    break;

	  case 'E':
	    for (j = XVECLEN (x, i) - 1; j >= 0; j--)
	      summarize_insn (XVECEXP (x, i, j), sum, 0);
	    break;

	  case 'i':
	    break;

	  default:
	    abort ();
	  }
    }
}

/* Ensure a sufficient number of `trapb' insns are in the code when
   the user requests code with a trap precision of functions or
   instructions.

   In naive mode, when the user requests a trap-precision of
   "instruction", a trapb is needed after every instruction that may
   generate a trap.  This ensures that the code is resumption safe but
   it is also slow.

   When optimizations are turned on, we delay issuing a trapb as long
   as possible.  In this context, a trap shadow is the sequence of
   instructions that starts with a (potentially) trap generating
   instruction and extends to the next trapb or call_pal instruction
   (but GCC never generates call_pal by itself).  We can delay (and
   therefore sometimes omit) a trapb subject to the following
   conditions:

   (a) On entry to the trap shadow, if any Alpha register or memory
   location contains a value that is used as an operand value by some
   instruction in the trap shadow (live on entry), then no instruction
   in the trap shadow may modify the register or memory location.

   (b) Within the trap shadow, the computation of the base register
   for a memory load or store instruction may not involve using the
   result of an instruction that might generate an UNPREDICTABLE
   result.

   (c) Within the trap shadow, no register may be used more than once
   as a destination register.  (This is to make life easier for the
   trap-handler.)

   (d) The trap shadow may not include any branch instructions.  */

static void
alpha_handle_trap_shadows (insns)
     rtx insns;
{
  struct shadow_summary shadow;
  int trap_pending, exception_nesting;
  rtx i, n;

  trap_pending = 0;
  exception_nesting = 0;
  shadow.used.i = 0;
  shadow.used.fp = 0;
  shadow.used.mem = 0;
  shadow.defd = shadow.used;
  
  for (i = insns; i ; i = NEXT_INSN (i))
    {
      if (GET_CODE (i) == NOTE)
	{
	  switch (NOTE_LINE_NUMBER (i))
	    {
	    case NOTE_INSN_EH_REGION_BEG:
	      exception_nesting++;
	      if (trap_pending)
		goto close_shadow;
	      break;

	    case NOTE_INSN_EH_REGION_END:
	      exception_nesting--;
	      if (trap_pending)
		goto close_shadow;
	      break;

	    case NOTE_INSN_EPILOGUE_BEG:
	      if (trap_pending && alpha_tp >= ALPHA_TP_FUNC)
		goto close_shadow;
	      break;
	    }
	}
      else if (trap_pending)
	{
	  if (alpha_tp == ALPHA_TP_FUNC)
	    {
	      if (GET_CODE (i) == JUMP_INSN
		  && GET_CODE (PATTERN (i)) == RETURN)
		goto close_shadow;
	    }
	  else if (alpha_tp == ALPHA_TP_INSN)
	    {
	      if (optimize > 0)
		{
		  struct shadow_summary sum;

		  sum.used.i = 0;
		  sum.used.fp = 0;
		  sum.used.mem = 0;
		  sum.defd = sum.used;

		  switch (GET_CODE (i))
		    {
		    case INSN:
		      /* Annoyingly, get_attr_trap will abort on these.  */
		      if (GET_CODE (PATTERN (i)) == USE
			  || GET_CODE (PATTERN (i)) == CLOBBER)
			break;

		      summarize_insn (PATTERN (i), &sum, 0);

		      if ((sum.defd.i & shadow.defd.i)
			  || (sum.defd.fp & shadow.defd.fp))
			{
			  /* (c) would be violated */
			  goto close_shadow;
			}

		      /* Combine shadow with summary of current insn: */
		      shadow.used.i   |= sum.used.i;
		      shadow.used.fp  |= sum.used.fp;
		      shadow.used.mem |= sum.used.mem;
		      shadow.defd.i   |= sum.defd.i;
		      shadow.defd.fp  |= sum.defd.fp;
		      shadow.defd.mem |= sum.defd.mem;

		      if ((sum.defd.i & shadow.used.i)
			  || (sum.defd.fp & shadow.used.fp)
			  || (sum.defd.mem & shadow.used.mem))
			{
			  /* (a) would be violated (also takes care of (b))  */
			  if (get_attr_trap (i) == TRAP_YES
			      && ((sum.defd.i & sum.used.i)
				  || (sum.defd.fp & sum.used.fp)))
			    abort ();

			  goto close_shadow;
			}
		      break;

		    case JUMP_INSN:
		    case CALL_INSN:
		    case CODE_LABEL:
		      goto close_shadow;

		    default:
		      abort ();
		    }
		}
	      else
		{
		close_shadow:
		  n = emit_insn_before (gen_trapb (), i);
		  PUT_MODE (n, TImode);
		  PUT_MODE (i, TImode);
		  trap_pending = 0;
		  shadow.used.i = 0;
		  shadow.used.fp = 0;
		  shadow.used.mem = 0;
		  shadow.defd = shadow.used;
		}
	    }
	}

      if ((exception_nesting > 0 || alpha_tp >= ALPHA_TP_FUNC)
	  && GET_CODE (i) == INSN
	  && GET_CODE (PATTERN (i)) != USE
	  && GET_CODE (PATTERN (i)) != CLOBBER
	  && get_attr_trap (i) == TRAP_YES)
	{
	  if (optimize && !trap_pending)
	    summarize_insn (PATTERN (i), &shadow, 0);
	  trap_pending = 1;
	}
    }
}

/* Alpha can only issue instruction groups simultaneously if they are
   suitibly aligned.  This is very processor-specific.  */

enum alphaev4_pipe {
  EV4_STOP = 0,
  EV4_IB0 = 1,
  EV4_IB1 = 2,
  EV4_IBX = 4
};

enum alphaev5_pipe {
  EV5_STOP = 0,
  EV5_NONE = 1,
  EV5_E01 = 2,
  EV5_E0 = 4,
  EV5_E1 = 8,
  EV5_FAM = 16,
  EV5_FA = 32,
  EV5_FM = 64
};

static enum alphaev4_pipe alphaev4_insn_pipe PARAMS ((rtx));
static enum alphaev5_pipe alphaev5_insn_pipe PARAMS ((rtx));
static rtx alphaev4_next_group PARAMS ((rtx, int *, int *));
static rtx alphaev5_next_group PARAMS ((rtx, int *, int *));
static rtx alphaev4_next_nop PARAMS ((int *));
static rtx alphaev5_next_nop PARAMS ((int *));

static void alpha_align_insns
  PARAMS ((rtx, unsigned int, rtx (*)(rtx, int *, int *), rtx (*)(int *)));

static enum alphaev4_pipe
alphaev4_insn_pipe (insn)
     rtx insn;
{
  if (recog_memoized (insn) < 0)
    return EV4_STOP;
  if (get_attr_length (insn) != 4)
    return EV4_STOP;

  switch (get_attr_type (insn))
    {
    case TYPE_ILD:
    case TYPE_FLD:
      return EV4_IBX;

    case TYPE_LDSYM:
    case TYPE_IADD:
    case TYPE_ILOG:
    case TYPE_ICMOV:
    case TYPE_ICMP:
    case TYPE_IST:
    case TYPE_FST:
    case TYPE_SHIFT:
    case TYPE_IMUL:
    case TYPE_FBR:
      return EV4_IB0;

    case TYPE_MISC:
    case TYPE_IBR:
    case TYPE_JSR:
    case TYPE_FCPYS:
    case TYPE_FCMOV:
    case TYPE_FADD:
    case TYPE_FDIV:
    case TYPE_FMUL:
      return EV4_IB1;

    default:
      abort ();
    }
}

static enum alphaev5_pipe
alphaev5_insn_pipe (insn)
     rtx insn;
{
  if (recog_memoized (insn) < 0)
    return EV5_STOP;
  if (get_attr_length (insn) != 4)
    return EV5_STOP;

  switch (get_attr_type (insn))
    {
    case TYPE_ILD:
    case TYPE_FLD:
    case TYPE_LDSYM:
    case TYPE_IADD:
    case TYPE_ILOG:
    case TYPE_ICMOV:
    case TYPE_ICMP:
      return EV5_E01;

    case TYPE_IST:
    case TYPE_FST:
    case TYPE_SHIFT:
    case TYPE_IMUL:
    case TYPE_MISC:
    case TYPE_MVI:
      return EV5_E0;

    case TYPE_IBR:
    case TYPE_JSR:
      return EV5_E1;

    case TYPE_FCPYS:
      return EV5_FAM;

    case TYPE_FBR:
    case TYPE_FCMOV:
    case TYPE_FADD:
    case TYPE_FDIV:
      return EV5_FA;

    case TYPE_FMUL:
      return EV5_FM;

    default:
      abort();
    }
}

/* IN_USE is a mask of the slots currently filled within the insn group. 
   The mask bits come from alphaev4_pipe above.  If EV4_IBX is set, then
   the insn in EV4_IB0 can be swapped by the hardware into EV4_IB1. 

   LEN is, of course, the length of the group in bytes.  */

static rtx
alphaev4_next_group (insn, pin_use, plen)
     rtx insn;
     int *pin_use, *plen;
{
  int len, in_use;

  len = in_use = 0;

  if (! INSN_P (insn)
      || GET_CODE (PATTERN (insn)) == CLOBBER
      || GET_CODE (PATTERN (insn)) == USE)
    goto next_and_done;

  while (1)
    {
      enum alphaev4_pipe pipe;

      pipe = alphaev4_insn_pipe (insn);
      switch (pipe)
	{
	case EV4_STOP:
	  /* Force complex instructions to start new groups.  */
	  if (in_use)
	    goto done;

	  /* If this is a completely unrecognized insn, its an asm.
	     We don't know how long it is, so record length as -1 to
	     signal a needed realignment.  */
	  if (recog_memoized (insn) < 0)
	    len = -1;
	  else
	    len = get_attr_length (insn);
	  goto next_and_done;

	case EV4_IBX:
	  if (in_use & EV4_IB0)
	    {
	      if (in_use & EV4_IB1)
		goto done;
	      in_use |= EV4_IB1;
	    }
	  else
	    in_use |= EV4_IB0 | EV4_IBX;
	  break;

	case EV4_IB0:
	  if (in_use & EV4_IB0)
	    {
	      if (!(in_use & EV4_IBX) || (in_use & EV4_IB1))
		goto done;
	      in_use |= EV4_IB1;
	    }
	  in_use |= EV4_IB0;
	  break;

	case EV4_IB1:
	  if (in_use & EV4_IB1)
	    goto done;
	  in_use |= EV4_IB1;
	  break;

	default:
	  abort();
	}
      len += 4;
      
      /* Haifa doesn't do well scheduling branches.  */
      if (GET_CODE (insn) == JUMP_INSN)
	goto next_and_done;

    next:
      insn = next_nonnote_insn (insn);

      if (!insn || ! INSN_P (insn))
	goto done;

      /* Let Haifa tell us where it thinks insn group boundaries are.  */
      if (GET_MODE (insn) == TImode)
	goto done;

      if (GET_CODE (insn) == CLOBBER || GET_CODE (insn) == USE)
	goto next;
    }

 next_and_done:
  insn = next_nonnote_insn (insn);

 done:
  *plen = len;
  *pin_use = in_use;
  return insn;
}

/* IN_USE is a mask of the slots currently filled within the insn group. 
   The mask bits come from alphaev5_pipe above.  If EV5_E01 is set, then
   the insn in EV5_E0 can be swapped by the hardware into EV5_E1. 

   LEN is, of course, the length of the group in bytes.  */

static rtx
alphaev5_next_group (insn, pin_use, plen)
     rtx insn;
     int *pin_use, *plen;
{
  int len, in_use;

  len = in_use = 0;

  if (! INSN_P (insn)
      || GET_CODE (PATTERN (insn)) == CLOBBER
      || GET_CODE (PATTERN (insn)) == USE)
    goto next_and_done;

  while (1)
    {
      enum alphaev5_pipe pipe;

      pipe = alphaev5_insn_pipe (insn);
      switch (pipe)
	{
	case EV5_STOP:
	  /* Force complex instructions to start new groups.  */
	  if (in_use)
	    goto done;

	  /* If this is a completely unrecognized insn, its an asm.
	     We don't know how long it is, so record length as -1 to
	     signal a needed realignment.  */
	  if (recog_memoized (insn) < 0)
	    len = -1;
	  else
	    len = get_attr_length (insn);
	  goto next_and_done;

	/* ??? Most of the places below, we would like to abort, as 
	   it would indicate an error either in Haifa, or in the 
	   scheduling description.  Unfortunately, Haifa never 
	   schedules the last instruction of the BB, so we don't
	   have an accurate TI bit to go off.  */
	case EV5_E01:
	  if (in_use & EV5_E0)
	    {
	      if (in_use & EV5_E1)
		goto done;
	      in_use |= EV5_E1;
	    }
	  else
	    in_use |= EV5_E0 | EV5_E01;
	  break;

	case EV5_E0:
	  if (in_use & EV5_E0)
	    {
	      if (!(in_use & EV5_E01) || (in_use & EV5_E1))
		goto done;
	      in_use |= EV5_E1;
	    }
	  in_use |= EV5_E0;
	  break;

	case EV5_E1:
	  if (in_use & EV5_E1)
	    goto done;
	  in_use |= EV5_E1;
	  break;

	case EV5_FAM:
	  if (in_use & EV5_FA)
	    {
	      if (in_use & EV5_FM)
		goto done;
	      in_use |= EV5_FM;
	    }
	  else
	    in_use |= EV5_FA | EV5_FAM;
	  break;

	case EV5_FA:
	  if (in_use & EV5_FA)
	    goto done;
	  in_use |= EV5_FA;
	  break;

	case EV5_FM:
	  if (in_use & EV5_FM)
	    goto done;
	  in_use |= EV5_FM;
	  break;

	case EV5_NONE:
	  break;

	default:
	  abort();
	}
      len += 4;
      
      /* Haifa doesn't do well scheduling branches.  */
      /* ??? If this is predicted not-taken, slotting continues, except
	 that no more IBR, FBR, or JSR insns may be slotted.  */
      if (GET_CODE (insn) == JUMP_INSN)
	goto next_and_done;

    next:
      insn = next_nonnote_insn (insn);

      if (!insn || ! INSN_P (insn))
	goto done;

      /* Let Haifa tell us where it thinks insn group boundaries are.  */
      if (GET_MODE (insn) == TImode)
	goto done;

      if (GET_CODE (insn) == CLOBBER || GET_CODE (insn) == USE)
	goto next;
    }

 next_and_done:
  insn = next_nonnote_insn (insn);

 done:
  *plen = len;
  *pin_use = in_use;
  return insn;
}

static rtx
alphaev4_next_nop (pin_use)
     int *pin_use;
{
  int in_use = *pin_use;
  rtx nop;

  if (!(in_use & EV4_IB0))
    {
      in_use |= EV4_IB0;
      nop = gen_nop ();
    }
  else if ((in_use & (EV4_IBX|EV4_IB1)) == EV4_IBX)
    {
      in_use |= EV4_IB1;
      nop = gen_nop ();
    }
  else if (TARGET_FP && !(in_use & EV4_IB1))
    {
      in_use |= EV4_IB1;
      nop = gen_fnop ();
    }
  else
    nop = gen_unop ();

  *pin_use = in_use;
  return nop;
}

static rtx
alphaev5_next_nop (pin_use)
     int *pin_use;
{
  int in_use = *pin_use;
  rtx nop;

  if (!(in_use & EV5_E1))
    {
      in_use |= EV5_E1;
      nop = gen_nop ();
    }
  else if (TARGET_FP && !(in_use & EV5_FA))
    {
      in_use |= EV5_FA;
      nop = gen_fnop ();
    }
  else if (TARGET_FP && !(in_use & EV5_FM))
    {
      in_use |= EV5_FM;
      nop = gen_fnop ();
    }
  else
    nop = gen_unop ();

  *pin_use = in_use;
  return nop;
}

/* The instruction group alignment main loop.  */

static void
alpha_align_insns (insns, max_align, next_group, next_nop)
     rtx insns;
     unsigned int max_align;
     rtx (*next_group) PARAMS ((rtx, int *, int *));
     rtx (*next_nop) PARAMS ((int *));
{
  /* ALIGN is the known alignment for the insn group.  */
  unsigned int align;
  /* OFS is the offset of the current insn in the insn group.  */
  int ofs;
  int prev_in_use, in_use, len;
  rtx i, next;

  /* Let shorten branches care for assigning alignments to code labels.  */
  shorten_branches (insns);

  if (align_functions < 4)
    align = 4;
  else if ((unsigned int) align_functions < max_align)
    align = align_functions;
  else
    align = max_align;

  ofs = prev_in_use = 0;
  i = insns;
  if (GET_CODE (i) == NOTE)
    i = next_nonnote_insn (i);

  while (i)
    {
      next = (*next_group) (i, &in_use, &len);

      /* When we see a label, resync alignment etc.  */
      if (GET_CODE (i) == CODE_LABEL)
	{
	  unsigned int new_align = 1 << label_to_alignment (i);

	  if (new_align >= align)
	    {
	      align = new_align < max_align ? new_align : max_align;
	      ofs = 0;
	    }

	  else if (ofs & (new_align-1))
	    ofs = (ofs | (new_align-1)) + 1;
	  if (len != 0)
	    abort();
	}

      /* Handle complex instructions special.  */
      else if (in_use == 0)
	{
	  /* Asms will have length < 0.  This is a signal that we have
	     lost alignment knowledge.  Assume, however, that the asm
	     will not mis-align instructions.  */
	  if (len < 0)
	    {
	      ofs = 0;
	      align = 4;
	      len = 0;
	    }
	}

      /* If the known alignment is smaller than the recognized insn group,
	 realign the output.  */
      else if ((int) align < len)
	{
	  unsigned int new_log_align = len > 8 ? 4 : 3;
	  rtx prev, where;

	  where = prev = prev_nonnote_insn (i);
	  if (!where || GET_CODE (where) != CODE_LABEL)
	    where = i;

	  /* Can't realign between a call and its gp reload.  */
	  if (! (TARGET_EXPLICIT_RELOCS
		 && prev && GET_CODE (prev) == CALL_INSN))
	    {
	      emit_insn_before (gen_realign (GEN_INT (new_log_align)), where);
	      align = 1 << new_log_align;
	      ofs = 0;
	    }
	}

      /* If the group won't fit in the same INT16 as the previous,
	 we need to add padding to keep the group together.  Rather
	 than simply leaving the insn filling to the assembler, we
	 can make use of the knowledge of what sorts of instructions
	 were issued in the previous group to make sure that all of
	 the added nops are really free.  */
      else if (ofs + len > (int) align)
	{
	  int nop_count = (align - ofs) / 4;
	  rtx where;

	  /* Insert nops before labels, branches, and calls to truely merge
	     the execution of the nops with the previous instruction group.  */
	  where = prev_nonnote_insn (i);
	  if (where)
	    {
	      if (GET_CODE (where) == CODE_LABEL)
		{
		  rtx where2 = prev_nonnote_insn (where);
		  if (where2 && GET_CODE (where2) == JUMP_INSN)
		    where = where2;
		}
	      else if (GET_CODE (where) == INSN)
		where = i;
	    }
	  else
	    where = i;

	  do 
	    emit_insn_before ((*next_nop)(&prev_in_use), where);
	  while (--nop_count);
	  ofs = 0;
	}

      ofs = (ofs + len) & (align - 1);
      prev_in_use = in_use;
      i = next;
    }
}

/* Machine dependent reorg pass.  */

void
alpha_reorg (insns)
     rtx insns;
{
  if (alpha_tp != ALPHA_TP_PROG || flag_exceptions)
    alpha_handle_trap_shadows (insns);

  /* Due to the number of extra trapb insns, don't bother fixing up
     alignment when trap precision is instruction.  Moreover, we can
     only do our job when sched2 is run.  */
  if (optimize && !optimize_size
      && alpha_tp != ALPHA_TP_INSN
      && flag_schedule_insns_after_reload)
    {
      if (alpha_cpu == PROCESSOR_EV4)
	alpha_align_insns (insns, 8, alphaev4_next_group, alphaev4_next_nop);
      else if (alpha_cpu == PROCESSOR_EV5)
	alpha_align_insns (insns, 16, alphaev5_next_group, alphaev5_next_nop);
    }
}

/* Check a floating-point value for validity for a particular machine mode.  */

static const char * const float_strings[] =
{
  /* These are for FLOAT_VAX.  */
   "1.70141173319264430e+38", /* 2^127 (2^24 - 1) / 2^24 */
  "-1.70141173319264430e+38",
   "2.93873587705571877e-39", /* 2^-128 */
  "-2.93873587705571877e-39",
  /* These are for the default broken IEEE mode, which traps
     on infinity or denormal numbers.  */
   "3.402823466385288598117e+38", /* 2^128 (1 - 2^-24) */
  "-3.402823466385288598117e+38",
   "1.1754943508222875079687e-38", /* 2^-126 */
  "-1.1754943508222875079687e-38",
};

static REAL_VALUE_TYPE float_values[8];
static int inited_float_values = 0;

int
check_float_value (mode, d, overflow)
     enum machine_mode mode;
     REAL_VALUE_TYPE *d;
     int overflow ATTRIBUTE_UNUSED;
{

  if (TARGET_IEEE || TARGET_IEEE_CONFORMANT || TARGET_IEEE_WITH_INEXACT)
    return 0;

  if (inited_float_values == 0)
    {
      int i;
      for (i = 0; i < 8; i++)
	float_values[i] = REAL_VALUE_ATOF (float_strings[i], DFmode);

      inited_float_values = 1;
    }

  if (mode == SFmode)
    {
      REAL_VALUE_TYPE r;
      REAL_VALUE_TYPE *fvptr;

      if (TARGET_FLOAT_VAX)
	fvptr = &float_values[0];
      else
	fvptr = &float_values[4];

      memcpy (&r, d, sizeof (REAL_VALUE_TYPE));
      if (REAL_VALUES_LESS (fvptr[0], r))
	{
	  memcpy (d, &fvptr[0], sizeof (REAL_VALUE_TYPE));
	  return 1;
	}
      else if (REAL_VALUES_LESS (r, fvptr[1]))
	{
	  memcpy (d, &fvptr[1], sizeof (REAL_VALUE_TYPE));
	  return 1;
	}
      else if (REAL_VALUES_LESS (dconst0, r)
		&& REAL_VALUES_LESS (r, fvptr[2]))
	{
	  memcpy (d, &dconst0, sizeof (REAL_VALUE_TYPE));
	  return 1;
	}
      else if (REAL_VALUES_LESS (r, dconst0)
		&& REAL_VALUES_LESS (fvptr[3], r))
	{
	  memcpy (d, &dconst0, sizeof (REAL_VALUE_TYPE));
	  return 1;
	}
    }

  return 0;
}

#if TARGET_ABI_OPEN_VMS

/* Return the VMS argument type corresponding to MODE.  */

enum avms_arg_type
alpha_arg_type (mode)
     enum machine_mode mode;
{
  switch (mode)
    {
    case SFmode:
      return TARGET_FLOAT_VAX ? FF : FS;
    case DFmode:
      return TARGET_FLOAT_VAX ? FD : FT;
    default:
      return I64;
    }
}

/* Return an rtx for an integer representing the VMS Argument Information
   register value.  */

rtx
alpha_arg_info_reg_val (cum)
     CUMULATIVE_ARGS cum;
{
  unsigned HOST_WIDE_INT regval = cum.num_args;
  int i;

  for (i = 0; i < 6; i++)
    regval |= ((int) cum.atypes[i]) << (i * 3 + 8);

  return GEN_INT (regval);
}

#include <splay-tree.h>

/* Structure to collect function names for final output
   in link section.  */

enum links_kind {KIND_UNUSED, KIND_LOCAL, KIND_EXTERN};

struct alpha_links
{
  rtx linkage;
  enum links_kind kind;
};

static splay_tree alpha_links;

static int mark_alpha_links_node	PARAMS ((splay_tree_node, void *));
static void mark_alpha_links		PARAMS ((void *));
static int alpha_write_one_linkage	PARAMS ((splay_tree_node, void *));

/* Protect alpha_links from garbage collection.  */

static int
mark_alpha_links_node (node, data)
     splay_tree_node node;
     void *data ATTRIBUTE_UNUSED;
{
  struct alpha_links *links = (struct alpha_links *) node->value;
  ggc_mark_rtx (links->linkage);
  return 0;
}

static void
mark_alpha_links (ptr)
     void *ptr;
{
  splay_tree tree = *(splay_tree *) ptr;
  splay_tree_foreach (tree, mark_alpha_links_node, NULL);
}

/* Make (or fake) .linkage entry for function call.

   IS_LOCAL is 0 if name is used in call, 1 if name is used in definition.

   Return an SYMBOL_REF rtx for the linkage.  */

rtx
alpha_need_linkage (name, is_local)
    const char *name;
    int is_local;
{
  splay_tree_node node;
  struct alpha_links *al;

  if (name[0] == '*')
    name++;

  if (alpha_links)
    {
      /* Is this name already defined?  */

      node = splay_tree_lookup (alpha_links, (splay_tree_key) name);
      if (node)
	{
	  al = (struct alpha_links *) node->value;
	  if (is_local)
	    {
	      /* Defined here but external assumed.  */
	      if (al->kind == KIND_EXTERN)
		al->kind = KIND_LOCAL;
	    }
	  else
	    {
	      /* Used here but unused assumed.  */
	      if (al->kind == KIND_UNUSED)
		al->kind = KIND_LOCAL;
	    }
	  return al->linkage;
	}
    }
  else
    {
      alpha_links = splay_tree_new ((splay_tree_compare_fn) strcmp, 
				    (splay_tree_delete_key_fn) free,
				    (splay_tree_delete_key_fn) free);
      ggc_add_root (&alpha_links, 1, 1, mark_alpha_links);
    }

  al = (struct alpha_links *) xmalloc (sizeof (struct alpha_links));
  name = xstrdup (name);

  /* Assume external if no definition.  */
  al->kind = (is_local ? KIND_UNUSED : KIND_EXTERN);

  /* Ensure we have an IDENTIFIER so assemble_name can mark it used.  */
  get_identifier (name);

  /* Construct a SYMBOL_REF for us to call.  */
  {
    size_t name_len = strlen (name);
    char *linksym = alloca (name_len + 6);
    linksym[0] = '$';
    memcpy (linksym + 1, name, name_len);
    memcpy (linksym + 1 + name_len, "..lk", 5);
    al->linkage = gen_rtx_SYMBOL_REF (Pmode,
				      ggc_alloc_string (linksym, name_len + 5));
  }

  splay_tree_insert (alpha_links, (splay_tree_key) name,
		     (splay_tree_value) al);

  return al->linkage;
}

static int
alpha_write_one_linkage (node, data)
     splay_tree_node node;
     void *data;
{
  const char *const name = (const char *) node->key;
  struct alpha_links *links = (struct alpha_links *) node->value;
  FILE *stream = (FILE *) data;

  if (links->kind == KIND_UNUSED
      || ! TREE_SYMBOL_REFERENCED (get_identifier (name)))
    return 0;

  fprintf (stream, "$%s..lk:\n", name);
  if (links->kind == KIND_LOCAL)
    {
      /* Local and used, build linkage pair.  */
      fprintf (stream, "\t.quad %s..en\n", name);
      fprintf (stream, "\t.quad %s\n", name);
    }
  else
    {
      /* External and used, request linkage pair.  */
      fprintf (stream, "\t.linkage %s\n", name);
    }

  return 0;
}

void
alpha_write_linkage (stream)
    FILE *stream;
{
  if (alpha_links)
    {
      readonly_section ();
      fprintf (stream, "\t.align 3\n");
      splay_tree_foreach (alpha_links, alpha_write_one_linkage, stream);
    }
}

/* Given a decl, a section name, and whether the decl initializer
   has relocs, choose attributes for the section.  */

#define SECTION_VMS_OVERLAY	SECTION_FORGET
#define SECTION_VMS_GLOBAL SECTION_MACH_DEP
#define SECTION_VMS_INITIALIZE (SECTION_VMS_GLOBAL << 1)

static unsigned int
vms_section_type_flags (decl, name, reloc)
     tree decl;
     const char *name;
     int reloc;
{
  unsigned int flags = default_section_type_flags (decl, name, reloc);

  if (decl && DECL_ATTRIBUTES (decl)
      && lookup_attribute ("overlaid", DECL_ATTRIBUTES (decl)))
    flags |= SECTION_VMS_OVERLAY;
  if (decl && DECL_ATTRIBUTES (decl)
      && lookup_attribute ("global", DECL_ATTRIBUTES (decl)))
    flags |= SECTION_VMS_GLOBAL;
  if (decl && DECL_ATTRIBUTES (decl)
      && lookup_attribute ("initialize", DECL_ATTRIBUTES (decl)))
    flags |= SECTION_VMS_INITIALIZE;

  return flags;
}

/* Switch to an arbitrary section NAME with attributes as specified
   by FLAGS.  ALIGN specifies any known alignment requirements for
   the section; 0 if the default should be used.  */

static void
vms_asm_named_section (name, flags)
     const char *name;
     unsigned int flags;
{
  fputc ('\n', asm_out_file);
  fprintf (asm_out_file, ".section\t%s", name);

  if (flags & SECTION_VMS_OVERLAY)
    fprintf (asm_out_file, ",OVR");
  if (flags & SECTION_VMS_GLOBAL)
    fprintf (asm_out_file, ",GBL");
  if (flags & SECTION_VMS_INITIALIZE)
    fprintf (asm_out_file, ",NOMOD");
  if (flags & SECTION_DEBUG)
    fprintf (asm_out_file, ",NOWRT");

  fputc ('\n', asm_out_file);
}

/* Record an element in the table of global constructors.  SYMBOL is
   a SYMBOL_REF of the function to be called; PRIORITY is a number
   between 0 and MAX_INIT_PRIORITY.  

   Differs from default_ctors_section_asm_out_constructor in that the
   width of the .ctors entry is always 64 bits, rather than the 32 bits
   used by a normal pointer.  */

static void
vms_asm_out_constructor (symbol, priority)
     rtx symbol;
     int priority ATTRIBUTE_UNUSED;
{
  ctors_section ();
  assemble_align (BITS_PER_WORD);
  assemble_integer (symbol, UNITS_PER_WORD, BITS_PER_WORD, 1);
}

static void
vms_asm_out_destructor (symbol, priority)
     rtx symbol;
     int priority ATTRIBUTE_UNUSED;
{
  dtors_section ();
  assemble_align (BITS_PER_WORD);
  assemble_integer (symbol, UNITS_PER_WORD, BITS_PER_WORD, 1);
}
#else

rtx
alpha_need_linkage (name, is_local)
     const char *name ATTRIBUTE_UNUSED;
     int is_local ATTRIBUTE_UNUSED;
{
  return NULL_RTX;
}

#endif /* TARGET_ABI_OPEN_VMS */

#if TARGET_ABI_UNICOSMK

static void unicosmk_output_module_name PARAMS ((FILE *));
static void unicosmk_output_default_externs PARAMS ((FILE *));
static void unicosmk_output_dex PARAMS ((FILE *));
static void unicosmk_output_externs PARAMS ((FILE *));
static void unicosmk_output_addr_vec PARAMS ((FILE *, rtx));
static const char *unicosmk_ssib_name PARAMS ((void));
static int unicosmk_special_name PARAMS ((const char *));

/* Define the offset between two registers, one to be eliminated, and the
   other its replacement, at the start of a routine.  */

int
unicosmk_initial_elimination_offset (from, to)
      int from;
      int to;
{
  int fixed_size;
  
  fixed_size = alpha_sa_size();
  if (fixed_size != 0)
    fixed_size += 48;

  if (from == FRAME_POINTER_REGNUM && to == HARD_FRAME_POINTER_REGNUM)
    return -fixed_size; 
  else if (from == ARG_POINTER_REGNUM && to == HARD_FRAME_POINTER_REGNUM)
    return 0;
  else if (from == FRAME_POINTER_REGNUM && to == STACK_POINTER_REGNUM)
    return (ALPHA_ROUND (current_function_outgoing_args_size)
	    + ALPHA_ROUND (get_frame_size()));
  else if (from == ARG_POINTER_REGNUM && to == STACK_POINTER_REGNUM)
    return (ALPHA_ROUND (fixed_size)
	    + ALPHA_ROUND (get_frame_size() 
			   + current_function_outgoing_args_size));
  else
    abort ();
}

/* Output the module name for .ident and .end directives. We have to strip
   directories and add make sure that the module name starts with a letter
   or '$'.  */

static void
unicosmk_output_module_name (file)
      FILE *file;
{
  const char *name;

  /* Strip directories.  */

  name = strrchr (main_input_filename, '/');
  if (name)
    ++name;
  else
    name = main_input_filename;

  /* CAM only accepts module names that start with a letter or '$'. We
     prefix the module name with a '$' if necessary.  */

  if (!ISALPHA (*name))
    fprintf (file, "$%s", name);
  else
    fputs (name, file);
}

/* Output text that to appear at the beginning of an assembler file.  */

void 
unicosmk_asm_file_start (file)
      FILE *file;
{
  int i;

  fputs ("\t.ident\t", file);
  unicosmk_output_module_name (file);
  fputs ("\n\n", file);

  /* The Unicos/Mk assembler uses different register names. Instead of trying
     to support them, we simply use micro definitions.  */

  /* CAM has different register names: rN for the integer register N and fN
     for the floating-point register N. Instead of trying to use these in
     alpha.md, we define the symbols $N and $fN to refer to the appropriate
     register.  */

  for (i = 0; i < 32; ++i)
    fprintf (file, "$%d <- r%d\n", i, i);

  for (i = 0; i < 32; ++i)
    fprintf (file, "$f%d <- f%d\n", i, i);

  putc ('\n', file);

  /* The .align directive fill unused space with zeroes which does not work
     in code sections. We define the macro 'gcc@code@align' which uses nops
     instead. Note that it assumes that code sections always have the
     biggest possible alignment since . refers to the current offset from
     the beginning of the section.  */

  fputs ("\t.macro gcc@code@align n\n", file);
  fputs ("gcc@n@bytes = 1 << n\n", file);
  fputs ("gcc@here = . % gcc@n@bytes\n", file);
  fputs ("\t.if ne, gcc@here, 0\n", file);
  fputs ("\t.repeat (gcc@n@bytes - gcc@here) / 4\n", file);
  fputs ("\tbis r31,r31,r31\n", file);
  fputs ("\t.endr\n", file);
  fputs ("\t.endif\n", file);
  fputs ("\t.endm gcc@code@align\n\n", file);

  /* Output extern declarations which should always be visible.  */
  unicosmk_output_default_externs (file);

  /* Open a dummy section. We always need to be inside a section for the
     section-switching code to work correctly.
     ??? This should be a module id or something like that. I still have to
     figure out what the rules for those are.  */
  fputs ("\n\t.psect\t$SG00000,data\n", file);
}

/* Output text to appear at the end of an assembler file. This includes all
   pending extern declarations and DEX expressions.  */

void
unicosmk_asm_file_end (file)
      FILE *file;
{
  fputs ("\t.endp\n\n", file);

  /* Output all pending externs.  */

  unicosmk_output_externs (file);

  /* Output dex definitions used for functions whose names conflict with 
     register names.  */

  unicosmk_output_dex (file);

  fputs ("\t.end\t", file);
  unicosmk_output_module_name (file);
  putc ('\n', file);
}

/* Output the definition of a common variable.  */

void
unicosmk_output_common (file, name, size, align)
      FILE *file;
      const char *name;
      int size;
      int align;
{
  tree name_tree;
  printf ("T3E__: common %s\n", name);

  common_section ();
  fputs("\t.endp\n\n\t.psect ", file);
  assemble_name(file, name);
  fprintf(file, ",%d,common\n", floor_log2 (align / BITS_PER_UNIT));
  fprintf(file, "\t.byte\t0:%d\n", size);

  /* Mark the symbol as defined in this module.  */
  name_tree = get_identifier (name);
  TREE_ASM_WRITTEN (name_tree) = 1;
}

#define SECTION_PUBLIC SECTION_MACH_DEP
#define SECTION_MAIN (SECTION_PUBLIC << 1)
static int current_section_align;

static unsigned int
unicosmk_section_type_flags (decl, name, reloc)
     tree decl;
     const char *name;
     int reloc ATTRIBUTE_UNUSED;
{
  unsigned int flags = default_section_type_flags (decl, name, reloc);

  if (!decl)
    return flags;

  if (TREE_CODE (decl) == FUNCTION_DECL)
    {
      current_section_align = floor_log2 (FUNCTION_BOUNDARY / BITS_PER_UNIT);
      if (align_functions_log > current_section_align)
	current_section_align = align_functions_log;

      if (! strcmp (IDENTIFIER_POINTER (DECL_ASSEMBLER_NAME (decl)), "main"))
	flags |= SECTION_MAIN;
    }
  else
    current_section_align = floor_log2 (DECL_ALIGN (decl) / BITS_PER_UNIT);

  if (TREE_PUBLIC (decl))
    flags |= SECTION_PUBLIC;

  return flags;
}

/* Generate a section name for decl and associate it with the
   declaration.  */

void
unicosmk_unique_section (decl, reloc)
      tree decl;
      int reloc ATTRIBUTE_UNUSED;
{
  const char *name;
  int len;

  if (!decl) 
    abort ();

  name = IDENTIFIER_POINTER (DECL_ASSEMBLER_NAME (decl));
  STRIP_NAME_ENCODING (name, name);
  len = strlen (name);

  if (TREE_CODE (decl) == FUNCTION_DECL)
    {
      char *string;

      /* It is essential that we prefix the section name here because 
	 otherwise the section names generated for constructors and 
	 destructors confuse collect2.  */

      string = alloca (len + 6);
      sprintf (string, "code@%s", name);
      DECL_SECTION_NAME (decl) = build_string (len + 5, string);
    }
  else if (TREE_PUBLIC (decl))
    DECL_SECTION_NAME (decl) = build_string (len, name);
  else
    {
      char *string;

      string = alloca (len + 6);
      sprintf (string, "data@%s", name);
      DECL_SECTION_NAME (decl) = build_string (len + 5, string);
    }
}

/* Switch to an arbitrary section NAME with attributes as specified
   by FLAGS.  ALIGN specifies any known alignment requirements for
   the section; 0 if the default should be used.  */

static void
unicosmk_asm_named_section (name, flags)
     const char *name;
     unsigned int flags;
{
  const char *kind;

  /* Close the previous section.  */

  fputs ("\t.endp\n\n", asm_out_file);

  /* Find out what kind of section we are opening.  */

  if (flags & SECTION_MAIN)
    fputs ("\t.start\tmain\n", asm_out_file);

  if (flags & SECTION_CODE)
    kind = "code";
  else if (flags & SECTION_PUBLIC)
    kind = "common";
  else
    kind = "data";

  if (current_section_align != 0)
    fprintf (asm_out_file, "\t.psect\t%s,%d,%s\n", name,
	     current_section_align, kind);
  else
    fprintf (asm_out_file, "\t.psect\t%s,%s\n", name, kind);
}

static void
unicosmk_insert_attributes (decl, attr_ptr)
     tree decl;
     tree *attr_ptr ATTRIBUTE_UNUSED;
{
  if (DECL_P (decl)
      && (TREE_PUBLIC (decl) || TREE_CODE (decl) == FUNCTION_DECL))
    UNIQUE_SECTION (decl, 0);
}

/* Output an alignment directive. We have to use the macro 'gcc@code@align'
   in code sections because .align fill unused space with zeroes.  */
      
void
unicosmk_output_align (file, align)
      FILE *file;
      int align;
{
  if (inside_function)
    fprintf (file, "\tgcc@code@align\t%d\n", align);
  else
    fprintf (file, "\t.align\t%d\n", align);
}

/* Add a case vector to the current function's list of deferred case
   vectors. Case vectors have to be put into a separate section because CAM
   does not allow data definitions in code sections.  */

void
unicosmk_defer_case_vector (lab, vec)
      rtx lab;
      rtx vec;
{
  struct machine_function *machine = cfun->machine;
  
  vec = gen_rtx_EXPR_LIST (VOIDmode, lab, vec);
  machine->addr_list = gen_rtx_EXPR_LIST (VOIDmode, vec,
					  machine->addr_list); 
}

/* Output a case vector.  */

static void
unicosmk_output_addr_vec (file, vec)
      FILE *file;
      rtx vec;
{
  rtx lab  = XEXP (vec, 0);
  rtx body = XEXP (vec, 1);
  int vlen = XVECLEN (body, 0);
  int idx;

  ASM_OUTPUT_INTERNAL_LABEL (file, "L", CODE_LABEL_NUMBER (lab));

  for (idx = 0; idx < vlen; idx++)
    {
      ASM_OUTPUT_ADDR_VEC_ELT
        (file, CODE_LABEL_NUMBER (XEXP (XVECEXP (body, 0, idx), 0)));
    }
}

/* Output current function's deferred case vectors.  */

static void
unicosmk_output_deferred_case_vectors (file)
      FILE *file;
{
  struct machine_function *machine = cfun->machine;
  rtx t;

  if (machine->addr_list == NULL_RTX)
    return;

  data_section ();
  for (t = machine->addr_list; t; t = XEXP (t, 1))
    unicosmk_output_addr_vec (file, XEXP (t, 0));
}

/* Set up the dynamic subprogram information block (DSIB) and update the 
   frame pointer register ($15) for subroutines which have a frame. If the 
   subroutine doesn't have a frame, simply increment $15.  */

static void
unicosmk_gen_dsib (imaskP)
      unsigned long * imaskP;
{
  if (alpha_is_stack_procedure)
    {
      const char *ssib_name;
      rtx mem;

      /* Allocate 64 bytes for the DSIB.  */

      FRP (emit_insn (gen_adddi3 (stack_pointer_rtx, stack_pointer_rtx,
                                  GEN_INT (-64))));
      emit_insn (gen_blockage ());

      /* Save the return address.  */

      mem = gen_rtx_MEM (DImode, plus_constant (stack_pointer_rtx, 56));
      set_mem_alias_set (mem, alpha_sr_alias_set);
      FRP (emit_move_insn (mem, gen_rtx_REG (DImode, REG_RA)));
      (*imaskP) &= ~(1L << REG_RA);

      /* Save the old frame pointer.  */

      mem = gen_rtx_MEM (DImode, plus_constant (stack_pointer_rtx, 48));
      set_mem_alias_set (mem, alpha_sr_alias_set);
      FRP (emit_move_insn (mem, hard_frame_pointer_rtx));
      (*imaskP) &= ~(1L << HARD_FRAME_POINTER_REGNUM);

      emit_insn (gen_blockage ());

      /* Store the SSIB pointer.  */

      ssib_name = ggc_strdup (unicosmk_ssib_name ());
      mem = gen_rtx_MEM (DImode, plus_constant (stack_pointer_rtx, 32));
      set_mem_alias_set (mem, alpha_sr_alias_set);

      FRP (emit_move_insn (gen_rtx_REG (DImode, 5),
                           gen_rtx_SYMBOL_REF (Pmode, ssib_name)));
      FRP (emit_move_insn (mem, gen_rtx_REG (DImode, 5)));

      /* Save the CIW index.  */

      mem = gen_rtx_MEM (DImode, plus_constant (stack_pointer_rtx, 24));
      set_mem_alias_set (mem, alpha_sr_alias_set);
      FRP (emit_move_insn (mem, gen_rtx_REG (DImode, 25)));

      emit_insn (gen_blockage ());

      /* Set the new frame pointer.  */

      FRP (emit_insn (gen_adddi3 (hard_frame_pointer_rtx,
                                  stack_pointer_rtx, GEN_INT (64))));

    }
  else
    {
      /* Increment the frame pointer register to indicate that we do not
         have a frame.  */

      FRP (emit_insn (gen_adddi3 (hard_frame_pointer_rtx,
                                  hard_frame_pointer_rtx, GEN_INT (1))));
    }
}

#define SSIB_PREFIX "__SSIB_"
#define SSIB_PREFIX_LEN 7

/* Generate the name of the SSIB section for the current function.  */

static const char *
unicosmk_ssib_name ()
{
  /* This is ok since CAM won't be able to deal with names longer than that 
     anyway.  */

  static char name[256];

  rtx x;
  const char *fnname;
  int len;

  x = DECL_RTL (cfun->decl);
  if (GET_CODE (x) != MEM)
    abort ();
  x = XEXP (x, 0);
  if (GET_CODE (x) != SYMBOL_REF)
    abort ();
  fnname = XSTR (x, 0);
  STRIP_NAME_ENCODING (fnname, fnname);

  len = strlen (fnname);
  if (len + SSIB_PREFIX_LEN > 255)
    len = 255 - SSIB_PREFIX_LEN;

  strcpy (name, SSIB_PREFIX);
  strncpy (name + SSIB_PREFIX_LEN, fnname, len);
  name[len + SSIB_PREFIX_LEN] = 0;

  return name;
}

/* Output the static subroutine information block for the current
   function.  */

static void
unicosmk_output_ssib (file, fnname)
      FILE *file;
      const char *fnname;
{
  int len;
  int i;
  rtx x;
  rtx ciw;
  struct machine_function *machine = cfun->machine;

  ssib_section ();
  fprintf (file, "\t.endp\n\n\t.psect\t%s%s,data\n", user_label_prefix,
	   unicosmk_ssib_name ());

  /* Some required stuff and the function name length.  */

  len = strlen (fnname);
  fprintf (file, "\t.quad\t^X20008%2.2X28\n", len);

  /* Saved registers
     ??? We don't do that yet.  */

  fputs ("\t.quad\t0\n", file);

  /* Function address.  */

  fputs ("\t.quad\t", file);
  assemble_name (file, fnname);
  putc ('\n', file);

  fputs ("\t.quad\t0\n", file);
  fputs ("\t.quad\t0\n", file);

  /* Function name.
     ??? We do it the same way Cray CC does it but this could be
     simplified.  */

  for( i = 0; i < len; i++ )
    fprintf (file, "\t.byte\t%d\n", (int)(fnname[i]));
  if( (len % 8) == 0 )
    fputs ("\t.quad\t0\n", file);
  else
    fprintf (file, "\t.bits\t%d : 0\n", (8 - (len % 8))*8);

  /* All call information words used in the function.  */

  for (x = machine->first_ciw; x; x = XEXP (x, 1))
    {
      ciw = XEXP (x, 0);
      fprintf (file, "\t.quad\t");
#if HOST_BITS_PER_WIDE_INT == 32
      fprintf (file, HOST_WIDE_INT_PRINT_DOUBLE_HEX,
	       CONST_DOUBLE_HIGH (ciw), CONST_DOUBLE_LOW (ciw));
#else
      fprintf (file, HOST_WIDE_INT_PRINT_HEX, INTVAL (ciw));
#endif
      fprintf (file, "\n");
    }
}

/* Add a call information word (CIW) to the list of the current function's
   CIWs and return its index.

   X is a CONST_INT or CONST_DOUBLE representing the CIW.  */

rtx
unicosmk_add_call_info_word (x)
      rtx x;
{
  rtx node;
  struct machine_function *machine = cfun->machine;

  node = gen_rtx_EXPR_LIST (VOIDmode, x, NULL_RTX);
  if (machine->first_ciw == NULL_RTX)
    machine->first_ciw = node;
  else
    XEXP (machine->last_ciw, 1) = node;

  machine->last_ciw = node;
  ++machine->ciw_count;

  return GEN_INT (machine->ciw_count
		  + strlen (current_function_name)/8 + 5);
}

static char unicosmk_section_buf[100];

char *
unicosmk_text_section ()
{
  static int count = 0;
  sprintf (unicosmk_section_buf, "\t.endp\n\n\t.psect\tgcc@text___%d,code", 
				 count++);
  return unicosmk_section_buf;
}

char *
unicosmk_data_section ()
{
  static int count = 1;
  sprintf (unicosmk_section_buf, "\t.endp\n\n\t.psect\tgcc@data___%d,data", 
				 count++);
  return unicosmk_section_buf;
}

/* The Cray assembler doesn't accept extern declarations for symbols which
   are defined in the same file. We have to keep track of all global
   symbols which are referenced and/or defined in a source file and output
   extern declarations for those which are referenced but not defined at
   the end of file.  */

/* List of identifiers for which an extern declaration might have to be
   emitted.  */

struct unicosmk_extern_list
{
  struct unicosmk_extern_list *next;
  const char *name;
};

static struct unicosmk_extern_list *unicosmk_extern_head = 0;

/* Output extern declarations which are required for every asm file.  */

static void
unicosmk_output_default_externs (file)
	FILE *file;
{
  static const char *const externs[] =
    { "__T3E_MISMATCH" };

  int i;
  int n;

  n = ARRAY_SIZE (externs);

  for (i = 0; i < n; i++)
    fprintf (file, "\t.extern\t%s\n", externs[i]);
}

/* Output extern declarations for global symbols which are have been
   referenced but not defined.  */

static void
unicosmk_output_externs (file)
      FILE *file;
{
  struct unicosmk_extern_list *p;
  const char *real_name;
  int len;
  tree name_tree;

  len = strlen (user_label_prefix);
  for (p = unicosmk_extern_head; p != 0; p = p->next)
    {
      /* We have to strip the encoding and possibly remove user_label_prefix 
	 from the identifier in order to handle -fleading-underscore and
	 explicit asm names correctly (cf. gcc.dg/asm-names-1.c).  */
      STRIP_NAME_ENCODING (real_name, p->name);
      if (len && p->name[0] == '*'
	  && !memcmp (real_name, user_label_prefix, len))
	real_name += len;
	
      name_tree = get_identifier (real_name);
      if (! TREE_ASM_WRITTEN (name_tree))
	{
	  TREE_ASM_WRITTEN (name_tree) = 1;
	  fputs ("\t.extern\t", file);
	  assemble_name (file, p->name);
	  putc ('\n', file);
	}
    }
}
      
/* Record an extern.  */

void
unicosmk_add_extern (name)
     const char *name;
{
  struct unicosmk_extern_list *p;

  p = (struct unicosmk_extern_list *)
       permalloc (sizeof (struct unicosmk_extern_list));
  p->next = unicosmk_extern_head;
  p->name = name;
  unicosmk_extern_head = p;
}

/* The Cray assembler generates incorrect code if identifiers which
   conflict with register names are used as instruction operands. We have
   to replace such identifiers with DEX expressions.  */

/* Structure to collect identifiers which have been replaced by DEX
   expressions.  */

struct unicosmk_dex {
  struct unicosmk_dex *next;
  const char *name;
};

/* List of identifiers which have been replaced by DEX expressions. The DEX 
   number is determined by the position in the list.  */

static struct unicosmk_dex *unicosmk_dex_list = NULL; 

/* The number of elements in the DEX list.  */

static int unicosmk_dex_count = 0;

/* Check if NAME must be replaced by a DEX expression.  */

static int
unicosmk_special_name (name)
      const char *name;
{
  if (name[0] == '*')
    ++name;

  if (name[0] == '$')
    ++name;

  if (name[0] != 'r' && name[0] != 'f' && name[0] != 'R' && name[0] != 'F')
    return 0;

  switch (name[1])
    {
    case '1':  case '2':
      return (name[2] == '\0' || (ISDIGIT (name[2]) && name[3] == '\0'));

    case '3':
      return (name[2] == '\0'
	       || ((name[2] == '0' || name[2] == '1') && name[3] == '\0'));

    default:
      return (ISDIGIT (name[1]) && name[2] == '\0');
    }
}

/* Return the DEX number if X must be replaced by a DEX expression and 0
   otherwise.  */

static int
unicosmk_need_dex (x)
      rtx x;
{
  struct unicosmk_dex *dex;
  const char *name;
  int i;
  
  if (GET_CODE (x) != SYMBOL_REF)
    return 0;

  name = XSTR (x,0);
  if (! unicosmk_special_name (name))
    return 0;

  i = unicosmk_dex_count;
  for (dex = unicosmk_dex_list; dex; dex = dex->next)
    {
      if (! strcmp (name, dex->name))
        return i;
      --i;
    }
      
  dex = (struct unicosmk_dex *) permalloc (sizeof (struct unicosmk_dex));
  dex->name = name;
  dex->next = unicosmk_dex_list;
  unicosmk_dex_list = dex;

  ++unicosmk_dex_count;
  return unicosmk_dex_count;
}

/* Output the DEX definitions for this file.  */

static void
unicosmk_output_dex (file)
      FILE *file;
{
  struct unicosmk_dex *dex;
  int i;

  if (unicosmk_dex_list == NULL)
    return;

  fprintf (file, "\t.dexstart\n");

  i = unicosmk_dex_count;
  for (dex = unicosmk_dex_list; dex; dex = dex->next)
    {
      fprintf (file, "\tDEX (%d) = ", i);
      assemble_name (file, dex->name);
      putc ('\n', file);
      --i;
    }
  
  fprintf (file, "\t.dexend\n");
}

#else

static void
unicosmk_output_deferred_case_vectors (file)
      FILE *file ATTRIBUTE_UNUSED;
{}

static void
unicosmk_gen_dsib (imaskP)
      unsigned long * imaskP ATTRIBUTE_UNUSED;
{}

static void
unicosmk_output_ssib (file, fnname)
      FILE * file ATTRIBUTE_UNUSED;
      const char * fnname ATTRIBUTE_UNUSED;
{}

rtx
unicosmk_add_call_info_word (x)
     rtx x ATTRIBUTE_UNUSED;
{
  return NULL_RTX;
}

static int
unicosmk_need_dex (x)
      rtx x ATTRIBUTE_UNUSED;
{
  return 0;
}

#endif /* TARGET_ABI_UNICOSMK */
