/* lang-options.h file for Fortran
   Copyright (C) 1995, 1996, 1997, 1998, 1999, 2000 Free Software Foundation, Inc.
   Contributed by James Craig Burley.

This file is part of GNU Fortran.

GNU Fortran is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU Fortran is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Fortran; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA.

*/

/* This is the contribution to the `documented_lang_options' array in
   toplev.c for g77.  */

#ifdef __STDC__	/* To be consistent with lang-specs.h.  Maybe avoid
		   overflowing some old compiler's tables, etc. */

DEFINE_LANG_NAME ("Fortran")

  { "-fversion", 
    N_("Print g77-specific compiler version info, run internal tests") },
/*"-fident",*/
/*"-fno-ident",*/
  { "-ff66", 
    N_("Program is written in typical FORTRAN 66 dialect") },
  { "-fno-f66", "" },
  { "-ff77", 
    N_("Program is written in typical Unix f77 dialect") },
  { "-fno-f77", 
    N_("Program does not use Unix-f77 dialectal features") },
  { "-ff90", 
    N_("Program is written in Fortran-90-ish dialect") },
  { "-fno-f90", "" },
  { "-fautomatic", "" },
  { "-fno-automatic", 
    N_("Treat local vars and COMMON blocks as if they were named in SAVE statements") },
  { "-fdollar-ok", 
    N_("Allow $ in symbol names") },
  { "-fno-dollar-ok", "" },
  { "-ff2c", "" },
  { "-fno-f2c", 
    N_("f2c-compatible code need not be generated") },
  { "-ff2c-library", "" },
  { "-fno-f2c-library", 
    N_("Unsupported; do not generate libf2c-calling code") },
  { "-fflatten-arrays", 
    N_("Unsupported; affects code-generation of arrays") },
  { "-fno-flatten-arrays", "" },
  { "-ffree-form", 
    N_("Program is written in Fortran-90-ish free form") },
  { "-fno-free-form", "" },
  { "-ffixed-form", "" },
  { "-fno-fixed-form", "" },
  { "-fpedantic", 
    N_("Warn about use of (only a few for now) Fortran extensions") },
  { "-fno-pedantic", "" },
  { "-fvxt", 
    N_("Program is written in VXT (Digital-like) FORTRAN") },
  { "-fno-vxt", "" },
  { "-fno-ugly", 
    N_("Disallow all ugly features") },
  { "-fugly-args", "" },
  { "-fno-ugly-args", 
    N_("Hollerith and typeless constants not passed as arguments") },
  { "-fugly-assign", 
    N_("Allow ordinary copying of ASSIGN'ed vars") },
  { "-fno-ugly-assign", "" },
  { "-fugly-assumed", 
    N_("Dummy array dimensioned to (1) is assumed-size") },
  { "-fno-ugly-assumed", "" },
  { "-fugly-comma", 
    N_("Trailing comma in procedure call denotes null argument") },
  { "-fno-ugly-comma", "" },
  { "-fugly-complex", 
    N_("Allow REAL(Z) and AIMAG(Z) given DOUBLE COMPLEX Z") },
  { "-fno-ugly-complex", "" },
  { "-fugly-init", "" },
  { "-fno-ugly-init", 
    N_("Initialization via DATA and PARAMETER is type-compatible") },
  { "-fugly-logint", 
    N_("Allow INTEGER and LOGICAL interchangeability") },
  { "-fno-ugly-logint", "" },
  { "-fxyzzy", 
    N_("Print internal debugging-related info") },
  { "-fno-xyzzy", "" },
  { "-finit-local-zero", 
    N_("Initialize local vars and arrays to zero") },
  { "-fno-init-local-zero", "" },
  { "-fbackslash", "" },
  { "-fno-backslash", 
    N_("Backslashes in character/hollerith constants not special (C-style)") },
  { "-femulate-complex", 
    N_("Have front end emulate COMPLEX arithmetic to avoid bugs") },
  { "-fno-emulate-complex", "" },
  { "-funderscoring", "" },
  { "-fno-underscoring", 
    N_("Disable the appending of underscores to externals") },
  { "-fsecond-underscore", "" },
  { "-fno-second-underscore", 
    N_("Never append a second underscore to externals") },
  { "-fintrin-case-initcap", 
    N_("Intrinsics spelled as e.g. SqRt") },
  { "-fintrin-case-upper", 
    N_("Intrinsics in uppercase") },
  { "-fintrin-case-lower", "" },
  { "-fintrin-case-any", 
    N_("Intrinsics letters in arbitrary cases") },
  { "-fmatch-case-initcap", 
    N_("Language keywords spelled as e.g. IOStat") },
  { "-fmatch-case-upper", 
    N_("Language keywords in uppercase") },
  { "-fmatch-case-lower", "" },
  { "-fmatch-case-any", 
    N_("Language keyword letters in arbitrary cases") },
  { "-fsource-case-upper", 
    N_("Internally convert most source to uppercase") },
  { "-fsource-case-lower", "" },
  { "-fsource-case-preserve", 
    N_("Internally preserve source case") },
  { "-fsymbol-case-initcap", 
    N_("Symbol names spelled in mixed case") },
  { "-fsymbol-case-upper", 
    N_("Symbol names in uppercase") },
  { "-fsymbol-case-lower", 
    N_("Symbol names in lowercase") },
  { "-fsymbol-case-any", "" },
  { "-fcase-strict-upper", 
    N_("Program written in uppercase") },
  { "-fcase-strict-lower", 
    N_("Program written in lowercase") },
  { "-fcase-initcap", 
    N_("Program written in strict mixed-case") },
  { "-fcase-upper", 
    N_("Compile as if program written in uppercase") },
  { "-fcase-lower", 
    N_("Compile as if program written in lowercase") },
  { "-fcase-preserve", 
    N_("Preserve all spelling (case) used in program") },
  { "-fbadu77-intrinsics-delete", 
    N_("Delete libU77 intrinsics with bad interfaces") },
  { "-fbadu77-intrinsics-disable", 
    N_("Disable libU77 intrinsics with bad interfaces") },
  { "-fbadu77-intrinsics-enable", "" },
  { "-fbadu77-intrinsics-hide", 
    N_("Hide libU77 intrinsics with bad interfaces") },
  { "-ff2c-intrinsics-delete", 
    N_("Delete non-FORTRAN-77 intrinsics f2c supports") },
  { "-ff2c-intrinsics-disable", 
    N_("Disable non-FORTRAN-77 intrinsics f2c supports") },
  { "-ff2c-intrinsics-enable", "" },
  { "-ff2c-intrinsics-hide", 
    N_("Hide non-FORTRAN-77 intrinsics f2c supports") },
  { "-ff90-intrinsics-delete", 
    N_("Delete non-FORTRAN-77 intrinsics F90 supports") },
  { "-ff90-intrinsics-disable", 
    N_("Disable non-FORTRAN-77 intrinsics F90 supports") },
  { "-ff90-intrinsics-enable", "" },
  { "-ff90-intrinsics-hide", 
    N_("Hide non-FORTRAN-77 intrinsics F90 supports") },
  { "-fgnu-intrinsics-delete", 
    N_("Delete non-FORTRAN-77 intrinsics g77 supports") },
  { "-fgnu-intrinsics-disable", 
    N_("Disable non-FORTRAN 77 intrinsics F90 supports") },
  { "-fgnu-intrinsics-enable", "" },
  { "-fgnu-intrinsics-hide", 
    N_("Hide non-FORTRAN 77 intrinsics F90 supports") },
  { "-fmil-intrinsics-delete", 
    N_("Delete MIL-STD 1753 intrinsics") },
  { "-fmil-intrinsics-disable", 
    N_("Disable MIL-STD 1753 intrinsics") },
  { "-fmil-intrinsics-enable", "" },
  { "-fmil-intrinsics-hide", 
    N_("Hide MIL-STD 1753 intrinsics") },
  { "-funix-intrinsics-delete", 
    N_("Delete libU77 intrinsics") },
  { "-funix-intrinsics-disable", 
    N_("Disable libU77 intrinsics") },
  { "-funix-intrinsics-enable", "" },
  { "-funix-intrinsics-hide", 
    N_("Hide libU77 intrinsics") },
  { "-fvxt-intrinsics-delete", 
    N_("Delete non-FORTRAN-77 intrinsics VXT FORTRAN supports") },
  { "-fvxt-intrinsics-disable", 
    N_("Disable non-FORTRAN-77 intrinsics VXT FORTRAN supports") },
  { "-fvxt-intrinsics-enable", "" },
  { "-fvxt-intrinsics-hide", 
    N_("Hide non-FORTRAN-77 intrinsics VXT FORTRAN supports") },
  { "-fzeros", 
    N_("Treat initial values of 0 like non-zero values") },
  { "-fno-zeros", "" },
  { "-fdebug-kludge", 
    N_("Emit special debugging information for COMMON and EQUIVALENCE (disabled)") },
  { "-fno-debug-kludge", "" },
  { "-fonetrip", 
    N_("Take at least one trip through each iterative DO loop") },
  { "-fno-onetrip", "" },
  { "-fsilent", "" },
  { "-fno-silent", 
    N_("Print names of program units as they are compiled") },
  { "-fglobals", "" },
  { "-fno-globals", 
    N_("Disable fatal diagnostics about inter-procedural problems") },
  { "-ftypeless-boz", 
    N_("Make prefix-radix non-decimal constants be typeless") },
  { "-fno-typeless-boz", "" },
  { "-fbounds-check", 
    N_("Generate code to check subscript and substring bounds") },
  { "-fno-bounds-check", "" },
  { "-ffortran-bounds-check",
    N_("Fortran-specific form of -fbounds-check") },
  { "-fno-fortran-bounds-check", "" },
  { "-Wglobals", "" },
  { "-Wno-globals", 
    N_("Disable warnings about inter-procedural problems") },
/*"-Wimplicit",*/
/*"-Wno-implicit",*/
  { "-Wsurprising", 
    N_("Warn about constructs with surprising meanings") },
  { "-Wno-surprising", "" },
/*"-Wall",*/
/* Prefix options.  */
  { "-I", 
    N_("Add a directory for INCLUDE searching") },
  { "-ffixed-line-length-", 
    N_("Set the maximum line length") },

#endif
