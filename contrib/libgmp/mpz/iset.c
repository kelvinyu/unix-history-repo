/* mpz_init_set (src_integer) -- Make a new multiple precision number with
   a value copied from SRC_INTEGER.

Copyright (C) 1991, 1993, 1994, 1996 Free Software Foundation, Inc.

This file is part of the GNU MP Library.

The GNU MP Library is free software; you can redistribute it and/or modify
it under the terms of the GNU Library General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at your
option) any later version.

The GNU MP Library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
License for more details.

You should have received a copy of the GNU Library General Public License
along with the GNU MP Library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
MA 02111-1307, USA. */

#include "gmp.h"
#include "gmp-impl.h"

void
#if __STDC__
mpz_init_set (mpz_ptr w, mpz_srcptr u)
#else
mpz_init_set (w, u)
     mpz_ptr w;
     mpz_srcptr u;
#endif
{
  mp_ptr wp, up;
  mp_size_t usize, size;

  usize = u->_mp_size;
  size = ABS (usize);

  w->_mp_alloc = MAX (size, 1);
  w->_mp_d = (mp_ptr) (*_mp_allocate_func) (w->_mp_alloc * BYTES_PER_MP_LIMB);

  wp = w->_mp_d;
  up = u->_mp_d;

  MPN_COPY (wp, up, size);
  w->_mp_size = usize;
}
