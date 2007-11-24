/* PPC64 ELF support for BFD.
   Copyright 2003 Free Software Foundation, Inc.

This file is part of BFD, the Binary File Descriptor library.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#ifndef _ELF_PPC64_H
#define _ELF_PPC64_H

#include "elf/reloc-macros.h"

/* Relocations.  */
START_RELOC_NUMBERS (elf_ppc64_reloc_type)
  RELOC_NUMBER (R_PPC64_NONE,		     0)
  RELOC_NUMBER (R_PPC64_ADDR32,		     1)
  RELOC_NUMBER (R_PPC64_ADDR24,		     2)
  RELOC_NUMBER (R_PPC64_ADDR16,		     3)
  RELOC_NUMBER (R_PPC64_ADDR16_LO,	     4)
  RELOC_NUMBER (R_PPC64_ADDR16_HI,	     5)
  RELOC_NUMBER (R_PPC64_ADDR16_HA,	     6)
  RELOC_NUMBER (R_PPC64_ADDR14,		     7)
  RELOC_NUMBER (R_PPC64_ADDR14_BRTAKEN,	     8)
  RELOC_NUMBER (R_PPC64_ADDR14_BRNTAKEN,     9)
  RELOC_NUMBER (R_PPC64_REL24,		    10)
  RELOC_NUMBER (R_PPC64_REL14,		    11)
  RELOC_NUMBER (R_PPC64_REL14_BRTAKEN,	    12)
  RELOC_NUMBER (R_PPC64_REL14_BRNTAKEN,	    13)
  RELOC_NUMBER (R_PPC64_GOT16,		    14)
  RELOC_NUMBER (R_PPC64_GOT16_LO,	    15)
  RELOC_NUMBER (R_PPC64_GOT16_HI,	    16)
  RELOC_NUMBER (R_PPC64_GOT16_HA,	    17)
  /* 18 unused.  32-bit reloc is R_PPC_PLTREL24.  */
  RELOC_NUMBER (R_PPC64_COPY,		    19)
  RELOC_NUMBER (R_PPC64_GLOB_DAT,	    20)
  RELOC_NUMBER (R_PPC64_JMP_SLOT,	    21)
  RELOC_NUMBER (R_PPC64_RELATIVE,	    22)
  /* 23 unused.  32-bit reloc is R_PPC_LOCAL24PC.  */
  RELOC_NUMBER (R_PPC64_UADDR32,	    24)
  RELOC_NUMBER (R_PPC64_UADDR16,	    25)
  RELOC_NUMBER (R_PPC64_REL32,		    26)
  RELOC_NUMBER (R_PPC64_PLT32,		    27)
  RELOC_NUMBER (R_PPC64_PLTREL32,	    28)
  RELOC_NUMBER (R_PPC64_PLT16_LO,	    29)
  RELOC_NUMBER (R_PPC64_PLT16_HI,	    30)
  RELOC_NUMBER (R_PPC64_PLT16_HA,	    31)
  /* 32 unused.  32-bit reloc is R_PPC_SDAREL16.  */
  RELOC_NUMBER (R_PPC64_SECTOFF,	    33)
  RELOC_NUMBER (R_PPC64_SECTOFF_LO,	    34)
  RELOC_NUMBER (R_PPC64_SECTOFF_HI,	    35)
  RELOC_NUMBER (R_PPC64_SECTOFF_HA,	    36)
  RELOC_NUMBER (R_PPC64_REL30,		    37)
  RELOC_NUMBER (R_PPC64_ADDR64,		    38)
  RELOC_NUMBER (R_PPC64_ADDR16_HIGHER,	    39)
  RELOC_NUMBER (R_PPC64_ADDR16_HIGHERA,	    40)
  RELOC_NUMBER (R_PPC64_ADDR16_HIGHEST,	    41)
  RELOC_NUMBER (R_PPC64_ADDR16_HIGHESTA,    42)
  RELOC_NUMBER (R_PPC64_UADDR64,	    43)
  RELOC_NUMBER (R_PPC64_REL64,		    44)
  RELOC_NUMBER (R_PPC64_PLT64,		    45)
  RELOC_NUMBER (R_PPC64_PLTREL64,	    46)
  RELOC_NUMBER (R_PPC64_TOC16,		    47)
  RELOC_NUMBER (R_PPC64_TOC16_LO,	    48)
  RELOC_NUMBER (R_PPC64_TOC16_HI,	    49)
  RELOC_NUMBER (R_PPC64_TOC16_HA,	    50)
  RELOC_NUMBER (R_PPC64_TOC,		    51)
  RELOC_NUMBER (R_PPC64_PLTGOT16,	    52)
  RELOC_NUMBER (R_PPC64_PLTGOT16_LO,	    53)
  RELOC_NUMBER (R_PPC64_PLTGOT16_HI,	    54)
  RELOC_NUMBER (R_PPC64_PLTGOT16_HA,	    55)

  /* The following relocs were added in the 64-bit PowerPC ELF ABI
     revision 1.2. */
  RELOC_NUMBER (R_PPC64_ADDR16_DS,	    56)
  RELOC_NUMBER (R_PPC64_ADDR16_LO_DS,	    57)
  RELOC_NUMBER (R_PPC64_GOT16_DS,	    58)
  RELOC_NUMBER (R_PPC64_GOT16_LO_DS,	    59)
  RELOC_NUMBER (R_PPC64_PLT16_LO_DS,	    60)
  RELOC_NUMBER (R_PPC64_SECTOFF_DS,	    61)
  RELOC_NUMBER (R_PPC64_SECTOFF_LO_DS,	    62)
  RELOC_NUMBER (R_PPC64_TOC16_DS,	    63)
  RELOC_NUMBER (R_PPC64_TOC16_LO_DS,	    64)
  RELOC_NUMBER (R_PPC64_PLTGOT16_DS,	    65)
  RELOC_NUMBER (R_PPC64_PLTGOT16_LO_DS,	    66)

  /* Relocs added to support TLS.  PowerPC64 ELF ABI revision 1.5.  */
  RELOC_NUMBER (R_PPC64_TLS,		    67)
  RELOC_NUMBER (R_PPC64_DTPMOD64,	    68)
  RELOC_NUMBER (R_PPC64_TPREL16,	    69)
  RELOC_NUMBER (R_PPC64_TPREL16_LO,	    70)
  RELOC_NUMBER (R_PPC64_TPREL16_HI,	    71)
  RELOC_NUMBER (R_PPC64_TPREL16_HA,	    72)
  RELOC_NUMBER (R_PPC64_TPREL64,	    73)
  RELOC_NUMBER (R_PPC64_DTPREL16,	    74)
  RELOC_NUMBER (R_PPC64_DTPREL16_LO,	    75)
  RELOC_NUMBER (R_PPC64_DTPREL16_HI,	    76)
  RELOC_NUMBER (R_PPC64_DTPREL16_HA,	    77)
  RELOC_NUMBER (R_PPC64_DTPREL64,	    78)
  RELOC_NUMBER (R_PPC64_GOT_TLSGD16,	    79)
  RELOC_NUMBER (R_PPC64_GOT_TLSGD16_LO,	    80)
  RELOC_NUMBER (R_PPC64_GOT_TLSGD16_HI,	    81)
  RELOC_NUMBER (R_PPC64_GOT_TLSGD16_HA,	    82)
  RELOC_NUMBER (R_PPC64_GOT_TLSLD16,	    83)
  RELOC_NUMBER (R_PPC64_GOT_TLSLD16_LO,	    84)
  RELOC_NUMBER (R_PPC64_GOT_TLSLD16_HI,	    85)
  RELOC_NUMBER (R_PPC64_GOT_TLSLD16_HA,	    86)
  RELOC_NUMBER (R_PPC64_GOT_TPREL16_DS,	    87)
  RELOC_NUMBER (R_PPC64_GOT_TPREL16_LO_DS,  88)
  RELOC_NUMBER (R_PPC64_GOT_TPREL16_HI,	    89)
  RELOC_NUMBER (R_PPC64_GOT_TPREL16_HA,	    90)
  RELOC_NUMBER (R_PPC64_GOT_DTPREL16_DS,    91)
  RELOC_NUMBER (R_PPC64_GOT_DTPREL16_LO_DS, 92)
  RELOC_NUMBER (R_PPC64_GOT_DTPREL16_HI,    93)
  RELOC_NUMBER (R_PPC64_GOT_DTPREL16_HA,    94)
  RELOC_NUMBER (R_PPC64_TPREL16_DS,	    95)
  RELOC_NUMBER (R_PPC64_TPREL16_LO_DS,	    96)
  RELOC_NUMBER (R_PPC64_TPREL16_HIGHER,	    97)
  RELOC_NUMBER (R_PPC64_TPREL16_HIGHERA,    98)
  RELOC_NUMBER (R_PPC64_TPREL16_HIGHEST,    99)
  RELOC_NUMBER (R_PPC64_TPREL16_HIGHESTA,  100)
  RELOC_NUMBER (R_PPC64_DTPREL16_DS,	   101)
  RELOC_NUMBER (R_PPC64_DTPREL16_LO_DS,	   102)
  RELOC_NUMBER (R_PPC64_DTPREL16_HIGHER,   103)
  RELOC_NUMBER (R_PPC64_DTPREL16_HIGHERA,  104)
  RELOC_NUMBER (R_PPC64_DTPREL16_HIGHEST,  105)
  RELOC_NUMBER (R_PPC64_DTPREL16_HIGHESTA, 106)

  /* These are GNU extensions to enable C++ vtable garbage collection.  */
  RELOC_NUMBER (R_PPC64_GNU_VTINHERIT,	   253)
  RELOC_NUMBER (R_PPC64_GNU_VTENTRY,	   254)

END_RELOC_NUMBERS (R_PPC64_max)

#define IS_PPC64_TLS_RELOC(R) \
  ((R) >= R_PPC64_TLS && (R) <= R_PPC64_DTPREL16_HIGHESTA)

/* Specify the start of the .glink section.  */
#define DT_PPC64_GLINK		DT_LOPROC

/* Specify the start and size of the .opd section.  */
#define DT_PPC64_OPD		(DT_LOPROC + 1)
#define DT_PPC64_OPDSZ		(DT_LOPROC + 2)

#endif /* _ELF_PPC64_H */
