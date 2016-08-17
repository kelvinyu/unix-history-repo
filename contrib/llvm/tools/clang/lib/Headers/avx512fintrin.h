/*===---- avx512fintrin.h - AVX512F intrinsics -----------------------------===
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *===-----------------------------------------------------------------------===
 */
#ifndef __IMMINTRIN_H
#error "Never use <avx512fintrin.h> directly; include <immintrin.h> instead."
#endif

#ifndef __AVX512FINTRIN_H
#define __AVX512FINTRIN_H

typedef char __v64qi __attribute__((__vector_size__(64)));
typedef short __v32hi __attribute__((__vector_size__(64)));
typedef double __v8df __attribute__((__vector_size__(64)));
typedef float __v16sf __attribute__((__vector_size__(64)));
typedef long long __v8di __attribute__((__vector_size__(64)));
typedef int __v16si __attribute__((__vector_size__(64)));

/* Unsigned types */
typedef unsigned char __v64qu __attribute__((__vector_size__(64)));
typedef unsigned short __v32hu __attribute__((__vector_size__(64)));
typedef unsigned long long __v8du __attribute__((__vector_size__(64)));
typedef unsigned int __v16su __attribute__((__vector_size__(64)));

typedef float __m512 __attribute__((__vector_size__(64)));
typedef double __m512d __attribute__((__vector_size__(64)));
typedef long long __m512i __attribute__((__vector_size__(64)));

typedef unsigned char __mmask8;
typedef unsigned short __mmask16;

/* Rounding mode macros.  */
#define _MM_FROUND_TO_NEAREST_INT   0x00
#define _MM_FROUND_TO_NEG_INF       0x01
#define _MM_FROUND_TO_POS_INF       0x02
#define _MM_FROUND_TO_ZERO          0x03
#define _MM_FROUND_CUR_DIRECTION    0x04

typedef enum
{
  _MM_PERM_AAAA = 0x00, _MM_PERM_AAAB = 0x01, _MM_PERM_AAAC = 0x02,
  _MM_PERM_AAAD = 0x03, _MM_PERM_AABA = 0x04, _MM_PERM_AABB = 0x05,
  _MM_PERM_AABC = 0x06, _MM_PERM_AABD = 0x07, _MM_PERM_AACA = 0x08,
  _MM_PERM_AACB = 0x09, _MM_PERM_AACC = 0x0A, _MM_PERM_AACD = 0x0B,
  _MM_PERM_AADA = 0x0C, _MM_PERM_AADB = 0x0D, _MM_PERM_AADC = 0x0E,
  _MM_PERM_AADD = 0x0F, _MM_PERM_ABAA = 0x10, _MM_PERM_ABAB = 0x11,
  _MM_PERM_ABAC = 0x12, _MM_PERM_ABAD = 0x13, _MM_PERM_ABBA = 0x14,
  _MM_PERM_ABBB = 0x15, _MM_PERM_ABBC = 0x16, _MM_PERM_ABBD = 0x17,
  _MM_PERM_ABCA = 0x18, _MM_PERM_ABCB = 0x19, _MM_PERM_ABCC = 0x1A,
  _MM_PERM_ABCD = 0x1B, _MM_PERM_ABDA = 0x1C, _MM_PERM_ABDB = 0x1D,
  _MM_PERM_ABDC = 0x1E, _MM_PERM_ABDD = 0x1F, _MM_PERM_ACAA = 0x20,
  _MM_PERM_ACAB = 0x21, _MM_PERM_ACAC = 0x22, _MM_PERM_ACAD = 0x23,
  _MM_PERM_ACBA = 0x24, _MM_PERM_ACBB = 0x25, _MM_PERM_ACBC = 0x26,
  _MM_PERM_ACBD = 0x27, _MM_PERM_ACCA = 0x28, _MM_PERM_ACCB = 0x29,
  _MM_PERM_ACCC = 0x2A, _MM_PERM_ACCD = 0x2B, _MM_PERM_ACDA = 0x2C,
  _MM_PERM_ACDB = 0x2D, _MM_PERM_ACDC = 0x2E, _MM_PERM_ACDD = 0x2F,
  _MM_PERM_ADAA = 0x30, _MM_PERM_ADAB = 0x31, _MM_PERM_ADAC = 0x32,
  _MM_PERM_ADAD = 0x33, _MM_PERM_ADBA = 0x34, _MM_PERM_ADBB = 0x35,
  _MM_PERM_ADBC = 0x36, _MM_PERM_ADBD = 0x37, _MM_PERM_ADCA = 0x38,
  _MM_PERM_ADCB = 0x39, _MM_PERM_ADCC = 0x3A, _MM_PERM_ADCD = 0x3B,
  _MM_PERM_ADDA = 0x3C, _MM_PERM_ADDB = 0x3D, _MM_PERM_ADDC = 0x3E,
  _MM_PERM_ADDD = 0x3F, _MM_PERM_BAAA = 0x40, _MM_PERM_BAAB = 0x41,
  _MM_PERM_BAAC = 0x42, _MM_PERM_BAAD = 0x43, _MM_PERM_BABA = 0x44,
  _MM_PERM_BABB = 0x45, _MM_PERM_BABC = 0x46, _MM_PERM_BABD = 0x47,
  _MM_PERM_BACA = 0x48, _MM_PERM_BACB = 0x49, _MM_PERM_BACC = 0x4A,
  _MM_PERM_BACD = 0x4B, _MM_PERM_BADA = 0x4C, _MM_PERM_BADB = 0x4D,
  _MM_PERM_BADC = 0x4E, _MM_PERM_BADD = 0x4F, _MM_PERM_BBAA = 0x50,
  _MM_PERM_BBAB = 0x51, _MM_PERM_BBAC = 0x52, _MM_PERM_BBAD = 0x53,
  _MM_PERM_BBBA = 0x54, _MM_PERM_BBBB = 0x55, _MM_PERM_BBBC = 0x56,
  _MM_PERM_BBBD = 0x57, _MM_PERM_BBCA = 0x58, _MM_PERM_BBCB = 0x59,
  _MM_PERM_BBCC = 0x5A, _MM_PERM_BBCD = 0x5B, _MM_PERM_BBDA = 0x5C,
  _MM_PERM_BBDB = 0x5D, _MM_PERM_BBDC = 0x5E, _MM_PERM_BBDD = 0x5F,
  _MM_PERM_BCAA = 0x60, _MM_PERM_BCAB = 0x61, _MM_PERM_BCAC = 0x62,
  _MM_PERM_BCAD = 0x63, _MM_PERM_BCBA = 0x64, _MM_PERM_BCBB = 0x65,
  _MM_PERM_BCBC = 0x66, _MM_PERM_BCBD = 0x67, _MM_PERM_BCCA = 0x68,
  _MM_PERM_BCCB = 0x69, _MM_PERM_BCCC = 0x6A, _MM_PERM_BCCD = 0x6B,
  _MM_PERM_BCDA = 0x6C, _MM_PERM_BCDB = 0x6D, _MM_PERM_BCDC = 0x6E,
  _MM_PERM_BCDD = 0x6F, _MM_PERM_BDAA = 0x70, _MM_PERM_BDAB = 0x71,
  _MM_PERM_BDAC = 0x72, _MM_PERM_BDAD = 0x73, _MM_PERM_BDBA = 0x74,
  _MM_PERM_BDBB = 0x75, _MM_PERM_BDBC = 0x76, _MM_PERM_BDBD = 0x77,
  _MM_PERM_BDCA = 0x78, _MM_PERM_BDCB = 0x79, _MM_PERM_BDCC = 0x7A,
  _MM_PERM_BDCD = 0x7B, _MM_PERM_BDDA = 0x7C, _MM_PERM_BDDB = 0x7D,
  _MM_PERM_BDDC = 0x7E, _MM_PERM_BDDD = 0x7F, _MM_PERM_CAAA = 0x80,
  _MM_PERM_CAAB = 0x81, _MM_PERM_CAAC = 0x82, _MM_PERM_CAAD = 0x83,
  _MM_PERM_CABA = 0x84, _MM_PERM_CABB = 0x85, _MM_PERM_CABC = 0x86,
  _MM_PERM_CABD = 0x87, _MM_PERM_CACA = 0x88, _MM_PERM_CACB = 0x89,
  _MM_PERM_CACC = 0x8A, _MM_PERM_CACD = 0x8B, _MM_PERM_CADA = 0x8C,
  _MM_PERM_CADB = 0x8D, _MM_PERM_CADC = 0x8E, _MM_PERM_CADD = 0x8F,
  _MM_PERM_CBAA = 0x90, _MM_PERM_CBAB = 0x91, _MM_PERM_CBAC = 0x92,
  _MM_PERM_CBAD = 0x93, _MM_PERM_CBBA = 0x94, _MM_PERM_CBBB = 0x95,
  _MM_PERM_CBBC = 0x96, _MM_PERM_CBBD = 0x97, _MM_PERM_CBCA = 0x98,
  _MM_PERM_CBCB = 0x99, _MM_PERM_CBCC = 0x9A, _MM_PERM_CBCD = 0x9B,
  _MM_PERM_CBDA = 0x9C, _MM_PERM_CBDB = 0x9D, _MM_PERM_CBDC = 0x9E,
  _MM_PERM_CBDD = 0x9F, _MM_PERM_CCAA = 0xA0, _MM_PERM_CCAB = 0xA1,
  _MM_PERM_CCAC = 0xA2, _MM_PERM_CCAD = 0xA3, _MM_PERM_CCBA = 0xA4,
  _MM_PERM_CCBB = 0xA5, _MM_PERM_CCBC = 0xA6, _MM_PERM_CCBD = 0xA7,
  _MM_PERM_CCCA = 0xA8, _MM_PERM_CCCB = 0xA9, _MM_PERM_CCCC = 0xAA,
  _MM_PERM_CCCD = 0xAB, _MM_PERM_CCDA = 0xAC, _MM_PERM_CCDB = 0xAD,
  _MM_PERM_CCDC = 0xAE, _MM_PERM_CCDD = 0xAF, _MM_PERM_CDAA = 0xB0,
  _MM_PERM_CDAB = 0xB1, _MM_PERM_CDAC = 0xB2, _MM_PERM_CDAD = 0xB3,
  _MM_PERM_CDBA = 0xB4, _MM_PERM_CDBB = 0xB5, _MM_PERM_CDBC = 0xB6,
  _MM_PERM_CDBD = 0xB7, _MM_PERM_CDCA = 0xB8, _MM_PERM_CDCB = 0xB9,
  _MM_PERM_CDCC = 0xBA, _MM_PERM_CDCD = 0xBB, _MM_PERM_CDDA = 0xBC,
  _MM_PERM_CDDB = 0xBD, _MM_PERM_CDDC = 0xBE, _MM_PERM_CDDD = 0xBF,
  _MM_PERM_DAAA = 0xC0, _MM_PERM_DAAB = 0xC1, _MM_PERM_DAAC = 0xC2,
  _MM_PERM_DAAD = 0xC3, _MM_PERM_DABA = 0xC4, _MM_PERM_DABB = 0xC5,
  _MM_PERM_DABC = 0xC6, _MM_PERM_DABD = 0xC7, _MM_PERM_DACA = 0xC8,
  _MM_PERM_DACB = 0xC9, _MM_PERM_DACC = 0xCA, _MM_PERM_DACD = 0xCB,
  _MM_PERM_DADA = 0xCC, _MM_PERM_DADB = 0xCD, _MM_PERM_DADC = 0xCE,
  _MM_PERM_DADD = 0xCF, _MM_PERM_DBAA = 0xD0, _MM_PERM_DBAB = 0xD1,
  _MM_PERM_DBAC = 0xD2, _MM_PERM_DBAD = 0xD3, _MM_PERM_DBBA = 0xD4,
  _MM_PERM_DBBB = 0xD5, _MM_PERM_DBBC = 0xD6, _MM_PERM_DBBD = 0xD7,
  _MM_PERM_DBCA = 0xD8, _MM_PERM_DBCB = 0xD9, _MM_PERM_DBCC = 0xDA,
  _MM_PERM_DBCD = 0xDB, _MM_PERM_DBDA = 0xDC, _MM_PERM_DBDB = 0xDD,
  _MM_PERM_DBDC = 0xDE, _MM_PERM_DBDD = 0xDF, _MM_PERM_DCAA = 0xE0,
  _MM_PERM_DCAB = 0xE1, _MM_PERM_DCAC = 0xE2, _MM_PERM_DCAD = 0xE3,
  _MM_PERM_DCBA = 0xE4, _MM_PERM_DCBB = 0xE5, _MM_PERM_DCBC = 0xE6,
  _MM_PERM_DCBD = 0xE7, _MM_PERM_DCCA = 0xE8, _MM_PERM_DCCB = 0xE9,
  _MM_PERM_DCCC = 0xEA, _MM_PERM_DCCD = 0xEB, _MM_PERM_DCDA = 0xEC,
  _MM_PERM_DCDB = 0xED, _MM_PERM_DCDC = 0xEE, _MM_PERM_DCDD = 0xEF,
  _MM_PERM_DDAA = 0xF0, _MM_PERM_DDAB = 0xF1, _MM_PERM_DDAC = 0xF2,
  _MM_PERM_DDAD = 0xF3, _MM_PERM_DDBA = 0xF4, _MM_PERM_DDBB = 0xF5,
  _MM_PERM_DDBC = 0xF6, _MM_PERM_DDBD = 0xF7, _MM_PERM_DDCA = 0xF8,
  _MM_PERM_DDCB = 0xF9, _MM_PERM_DDCC = 0xFA, _MM_PERM_DDCD = 0xFB,
  _MM_PERM_DDDA = 0xFC, _MM_PERM_DDDB = 0xFD, _MM_PERM_DDDC = 0xFE,
  _MM_PERM_DDDD = 0xFF
} _MM_PERM_ENUM;

typedef enum
{
  _MM_MANT_NORM_1_2,    /* interval [1, 2)      */
  _MM_MANT_NORM_p5_2,   /* interval [0.5, 2)    */
  _MM_MANT_NORM_p5_1,   /* interval [0.5, 1)    */
  _MM_MANT_NORM_p75_1p5   /* interval [0.75, 1.5) */
} _MM_MANTISSA_NORM_ENUM;

typedef enum
{
  _MM_MANT_SIGN_src,    /* sign = sign(SRC)     */
  _MM_MANT_SIGN_zero,   /* sign = 0             */
  _MM_MANT_SIGN_nan   /* DEST = NaN if sign(SRC) = 1 */
} _MM_MANTISSA_SIGN_ENUM;

/* Define the default attributes for the functions in this file. */
#define __DEFAULT_FN_ATTRS __attribute__((__always_inline__, __nodebug__, __target__("avx512f")))

/* Create vectors with repeated elements */

static  __inline __m512i __DEFAULT_FN_ATTRS
_mm512_setzero_si512(void)
{
  return (__m512i)(__v8di){ 0, 0, 0, 0, 0, 0, 0, 0 };
}

#define _mm512_setzero_epi32 _mm512_setzero_si512

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_undefined_pd(void)
{
  return (__m512d)__builtin_ia32_undef512();
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_undefined(void)
{
  return (__m512)__builtin_ia32_undef512();
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_undefined_ps(void)
{
  return (__m512)__builtin_ia32_undef512();
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_undefined_epi32(void)
{
  return (__m512i)__builtin_ia32_undef512();
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_broadcastd_epi32 (__m128i __A)
{
  return (__m512i)__builtin_shufflevector((__v4si) __A,
                                          (__v4si)_mm_undefined_si128(),
                                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_broadcastd_epi32 (__m512i __O, __mmask16 __M, __m128i __A)
{
  return (__m512i)__builtin_ia32_selectd_512(__M,
                                             (__v16si) _mm512_broadcastd_epi32(__A),
                                             (__v16si) __O);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_broadcastd_epi32 (__mmask16 __M, __m128i __A)
{
  return (__m512i)__builtin_ia32_selectd_512(__M,
                                             (__v16si) _mm512_broadcastd_epi32(__A),
                                             (__v16si) _mm512_setzero_si512());
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_broadcastq_epi64 (__m128i __A)
{
  return (__m512i)__builtin_shufflevector((__v2di) __A,
                                          (__v2di) _mm_undefined_si128(),
                                          0, 0, 0, 0, 0, 0, 0, 0);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_broadcastq_epi64 (__m512i __O, __mmask8 __M, __m128i __A)
{
  return (__m512i)__builtin_ia32_selectq_512(__M,
                                             (__v8di) _mm512_broadcastq_epi64(__A),
                                             (__v8di) __O);

}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_broadcastq_epi64 (__mmask8 __M, __m128i __A)
{
  return (__m512i)__builtin_ia32_selectq_512(__M,
                                             (__v8di) _mm512_broadcastq_epi64(__A),
                                             (__v8di) _mm512_setzero_si512());
}

static __inline __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_set1_epi32(__mmask16 __M, int __A)
{
  return (__m512i) __builtin_ia32_pbroadcastd512_gpr_mask (__A,
                 (__v16si)
                 _mm512_setzero_si512 (),
                 __M);
}

static __inline __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_set1_epi64(__mmask8 __M, long long __A)
{
#ifdef __x86_64__
  return (__m512i) __builtin_ia32_pbroadcastq512_gpr_mask (__A,
                 (__v8di)
                 _mm512_setzero_si512 (),
                 __M);
#else
  return (__m512i) __builtin_ia32_pbroadcastq512_mem_mask (__A,
                 (__v8di)
                 _mm512_setzero_si512 (),
                 __M);
#endif
}

static __inline __m512 __DEFAULT_FN_ATTRS
_mm512_setzero_ps(void)
{
  return (__m512){ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                   0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
}

#define _mm512_setzero _mm512_setzero_ps

static  __inline __m512d __DEFAULT_FN_ATTRS
_mm512_setzero_pd(void)
{
  return (__m512d){ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
}

static __inline __m512 __DEFAULT_FN_ATTRS
_mm512_set1_ps(float __w)
{
  return (__m512){ __w, __w, __w, __w, __w, __w, __w, __w,
                   __w, __w, __w, __w, __w, __w, __w, __w  };
}

static __inline __m512d __DEFAULT_FN_ATTRS
_mm512_set1_pd(double __w)
{
  return (__m512d){ __w, __w, __w, __w, __w, __w, __w, __w };
}

static __inline __m512i __DEFAULT_FN_ATTRS
_mm512_set1_epi8(char __w)
{
  return (__m512i)(__v64qi){ __w, __w, __w, __w, __w, __w, __w, __w,
                             __w, __w, __w, __w, __w, __w, __w, __w,
                             __w, __w, __w, __w, __w, __w, __w, __w,
                             __w, __w, __w, __w, __w, __w, __w, __w,
                             __w, __w, __w, __w, __w, __w, __w, __w,
                             __w, __w, __w, __w, __w, __w, __w, __w,
                             __w, __w, __w, __w, __w, __w, __w, __w,
                             __w, __w, __w, __w, __w, __w, __w, __w  };
}

static __inline __m512i __DEFAULT_FN_ATTRS
_mm512_set1_epi16(short __w)
{
  return (__m512i)(__v32hi){ __w, __w, __w, __w, __w, __w, __w, __w,
                             __w, __w, __w, __w, __w, __w, __w, __w,
                             __w, __w, __w, __w, __w, __w, __w, __w,
                             __w, __w, __w, __w, __w, __w, __w, __w };
}

static __inline __m512i __DEFAULT_FN_ATTRS
_mm512_set1_epi32(int __s)
{
  return (__m512i)(__v16si){ __s, __s, __s, __s, __s, __s, __s, __s,
                             __s, __s, __s, __s, __s, __s, __s, __s };
}

static __inline __m512i __DEFAULT_FN_ATTRS
_mm512_set1_epi64(long long __d)
{
  return (__m512i)(__v8di){ __d, __d, __d, __d, __d, __d, __d, __d };
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_broadcastss_ps(__m128 __A)
{
  return (__m512)__builtin_shufflevector((__v4sf) __A,
                                         (__v4sf)_mm_undefined_ps(),
                                         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

static __inline __m512i __DEFAULT_FN_ATTRS
_mm512_set4_epi32 (int __A, int __B, int __C, int __D)
{
  return  (__m512i)(__v16si)
   { __D, __C, __B, __A, __D, __C, __B, __A,
     __D, __C, __B, __A, __D, __C, __B, __A };
}

static __inline __m512i __DEFAULT_FN_ATTRS
_mm512_set4_epi64 (long long __A, long long __B, long long __C,
       long long __D)
{
  return  (__m512i) (__v8di)
   { __D, __C, __B, __A, __D, __C, __B, __A };
}

static __inline __m512d __DEFAULT_FN_ATTRS
_mm512_set4_pd (double __A, double __B, double __C, double __D)
{
  return  (__m512d)
   { __D, __C, __B, __A, __D, __C, __B, __A };
}

static __inline __m512 __DEFAULT_FN_ATTRS
_mm512_set4_ps (float __A, float __B, float __C, float __D)
{
  return  (__m512)
   { __D, __C, __B, __A, __D, __C, __B, __A,
     __D, __C, __B, __A, __D, __C, __B, __A };
}

#define _mm512_setr4_epi32(e0,e1,e2,e3)               \
  _mm512_set4_epi32((e3),(e2),(e1),(e0))

#define _mm512_setr4_epi64(e0,e1,e2,e3)               \
  _mm512_set4_epi64((e3),(e2),(e1),(e0))

#define _mm512_setr4_pd(e0,e1,e2,e3)                \
  _mm512_set4_pd((e3),(e2),(e1),(e0))

#define _mm512_setr4_ps(e0,e1,e2,e3)                \
  _mm512_set4_ps((e3),(e2),(e1),(e0))

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_broadcastsd_pd(__m128d __A)
{
  return (__m512d)__builtin_shufflevector((__v2df) __A,
                                          (__v2df) _mm_undefined_pd(),
                                          0, 0, 0, 0, 0, 0, 0, 0);
}

/* Cast between vector types */

static __inline __m512d __DEFAULT_FN_ATTRS
_mm512_castpd256_pd512(__m256d __a)
{
  return __builtin_shufflevector(__a, __a, 0, 1, 2, 3, -1, -1, -1, -1);
}

static __inline __m512 __DEFAULT_FN_ATTRS
_mm512_castps256_ps512(__m256 __a)
{
  return __builtin_shufflevector(__a, __a, 0,  1,  2,  3,  4,  5,  6,  7,
                                          -1, -1, -1, -1, -1, -1, -1, -1);
}

static __inline __m128d __DEFAULT_FN_ATTRS
_mm512_castpd512_pd128(__m512d __a)
{
  return __builtin_shufflevector(__a, __a, 0, 1);
}

static __inline __m256d __DEFAULT_FN_ATTRS
_mm512_castpd512_pd256 (__m512d __A)
{
  return __builtin_shufflevector(__A, __A, 0, 1, 2, 3);
}

static __inline __m128 __DEFAULT_FN_ATTRS
_mm512_castps512_ps128(__m512 __a)
{
  return __builtin_shufflevector(__a, __a, 0, 1, 2, 3);
}

static __inline __m256 __DEFAULT_FN_ATTRS
_mm512_castps512_ps256 (__m512 __A)
{
  return __builtin_shufflevector(__A, __A, 0, 1, 2, 3, 4, 5, 6, 7);
}

static __inline __m512 __DEFAULT_FN_ATTRS
_mm512_castpd_ps (__m512d __A)
{
  return (__m512) (__A);
}

static __inline __m512i __DEFAULT_FN_ATTRS
_mm512_castpd_si512 (__m512d __A)
{
  return (__m512i) (__A);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_castpd128_pd512 (__m128d __A)
{
  return __builtin_shufflevector( __A, __A, 0, 1, -1, -1, -1, -1, -1, -1);
}

static __inline __m512d __DEFAULT_FN_ATTRS
_mm512_castps_pd (__m512 __A)
{
  return (__m512d) (__A);
}

static __inline __m512i __DEFAULT_FN_ATTRS
_mm512_castps_si512 (__m512 __A)
{
  return (__m512i) (__A);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_castps128_ps512 (__m128 __A)
{
    return  __builtin_shufflevector( __A, __A, 0, 1, 2, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_castsi128_si512 (__m128i __A)
{
   return  __builtin_shufflevector( __A, __A, 0, 1, -1, -1, -1, -1, -1, -1);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_castsi256_si512 (__m256i __A)
{
   return  __builtin_shufflevector( __A, __A, 0, 1, 2, 3, -1, -1, -1, -1);
}

static __inline __m512 __DEFAULT_FN_ATTRS
_mm512_castsi512_ps (__m512i __A)
{
  return (__m512) (__A);
}

static __inline __m512d __DEFAULT_FN_ATTRS
_mm512_castsi512_pd (__m512i __A)
{
  return (__m512d) (__A);
}

static __inline __m128i __DEFAULT_FN_ATTRS
_mm512_castsi512_si128 (__m512i __A)
{
  return (__m128i)__builtin_shufflevector(__A, __A , 0, 1);
}

static __inline __m256i __DEFAULT_FN_ATTRS
_mm512_castsi512_si256 (__m512i __A)
{
  return (__m256i)__builtin_shufflevector(__A, __A , 0, 1, 2, 3);
}

/* Bitwise operators */
static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_and_epi32(__m512i __a, __m512i __b)
{
  return (__m512i)((__v16su)__a & (__v16su)__b);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_and_epi32(__m512i __src, __mmask16 __k, __m512i __a, __m512i __b)
{
  return (__m512i)__builtin_ia32_selectd_512((__mmask16)__k,
                (__v16si) _mm512_and_epi32(__a, __b),
                (__v16si) __src);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_and_epi32(__mmask16 __k, __m512i __a, __m512i __b)
{
  return (__m512i) _mm512_mask_and_epi32(_mm512_setzero_si512 (),
                                         __k, __a, __b);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_and_epi64(__m512i __a, __m512i __b)
{
  return (__m512i)((__v8du)__a & (__v8du)__b);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_and_epi64(__m512i __src, __mmask8 __k, __m512i __a, __m512i __b)
{
    return (__m512i) __builtin_ia32_selectq_512 ((__mmask8) __k,
                (__v8di) _mm512_and_epi64(__a, __b),
                (__v8di) __src);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_and_epi64(__mmask8 __k, __m512i __a, __m512i __b)
{
  return (__m512i) _mm512_mask_and_epi64(_mm512_setzero_si512 (),
                                         __k, __a, __b);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_andnot_si512 (__m512i __A, __m512i __B)
{
  return (__m512i)(~(__v8du)(__A) & (__v8du)__B);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_andnot_epi32 (__m512i __A, __m512i __B)
{
  return (__m512i)(~(__v16su)(__A) & (__v16su)__B);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_andnot_epi32(__m512i __W, __mmask16 __U, __m512i __A, __m512i __B)
{
  return (__m512i)__builtin_ia32_selectd_512((__mmask16)__U,
                                         (__v16si)_mm512_andnot_epi32(__A, __B),
                                         (__v16si)__W);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_andnot_epi32(__mmask16 __U, __m512i __A, __m512i __B)
{
  return (__m512i)_mm512_mask_andnot_epi32(_mm512_setzero_si512(),
                                           __U, __A, __B);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_andnot_epi64(__m512i __A, __m512i __B)
{
  return (__m512i)(~(__v8du)(__A) & (__v8du)__B);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_andnot_epi64(__m512i __W, __mmask8 __U, __m512i __A, __m512i __B)
{
  return (__m512i)__builtin_ia32_selectq_512((__mmask8)__U,
                                          (__v8di)_mm512_andnot_epi64(__A, __B),
                                          (__v8di)__W);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_andnot_epi64(__mmask8 __U, __m512i __A, __m512i __B)
{
  return (__m512i)_mm512_mask_andnot_epi64(_mm512_setzero_si512(),
                                           __U, __A, __B);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_or_epi32(__m512i __a, __m512i __b)
{
  return (__m512i)((__v16su)__a | (__v16su)__b);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_or_epi32(__m512i __src, __mmask16 __k, __m512i __a, __m512i __b)
{
  return (__m512i)__builtin_ia32_selectd_512((__mmask16)__k,
                                             (__v16si)_mm512_or_epi32(__a, __b),
                                             (__v16si)__src);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_or_epi32(__mmask16 __k, __m512i __a, __m512i __b)
{
  return (__m512i)_mm512_mask_or_epi32(_mm512_setzero_si512(), __k, __a, __b);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_or_epi64(__m512i __a, __m512i __b)
{
  return (__m512i)((__v8du)__a | (__v8du)__b);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_or_epi64(__m512i __src, __mmask8 __k, __m512i __a, __m512i __b)
{
  return (__m512i)__builtin_ia32_selectq_512((__mmask8)__k,
                                             (__v8di)_mm512_or_epi64(__a, __b),
                                             (__v8di)__src);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_or_epi64(__mmask8 __k, __m512i __a, __m512i __b)
{
  return (__m512i)_mm512_mask_or_epi64(_mm512_setzero_si512(), __k, __a, __b);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_xor_epi32(__m512i __a, __m512i __b)
{
  return (__m512i)((__v16su)__a ^ (__v16su)__b);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_xor_epi32(__m512i __src, __mmask16 __k, __m512i __a, __m512i __b)
{
  return (__m512i)__builtin_ia32_selectd_512((__mmask16)__k,
                                            (__v16si)_mm512_xor_epi32(__a, __b),
                                            (__v16si)__src);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_xor_epi32(__mmask16 __k, __m512i __a, __m512i __b)
{
  return (__m512i)_mm512_mask_xor_epi32(_mm512_setzero_si512(), __k, __a, __b);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_xor_epi64(__m512i __a, __m512i __b)
{
  return (__m512i)((__v8du)__a ^ (__v8du)__b);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_xor_epi64(__m512i __src, __mmask8 __k, __m512i __a, __m512i __b)
{
  return (__m512i)__builtin_ia32_selectq_512((__mmask8)__k,
                                             (__v8di)_mm512_xor_epi64(__a, __b),
                                             (__v8di)__src);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_xor_epi64(__mmask8 __k, __m512i __a, __m512i __b)
{
  return (__m512i)_mm512_mask_xor_epi64(_mm512_setzero_si512(), __k, __a, __b);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_and_si512(__m512i __a, __m512i __b)
{
  return (__m512i)((__v8du)__a & (__v8du)__b);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_or_si512(__m512i __a, __m512i __b)
{
  return (__m512i)((__v8du)__a | (__v8du)__b);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_xor_si512(__m512i __a, __m512i __b)
{
  return (__m512i)((__v8du)__a ^ (__v8du)__b);
}

/* Arithmetic */

static __inline __m512d __DEFAULT_FN_ATTRS
_mm512_add_pd(__m512d __a, __m512d __b)
{
  return (__m512d)((__v8df)__a + (__v8df)__b);
}

static __inline __m512 __DEFAULT_FN_ATTRS
_mm512_add_ps(__m512 __a, __m512 __b)
{
  return (__m512)((__v16sf)__a + (__v16sf)__b);
}

static __inline __m512d __DEFAULT_FN_ATTRS
_mm512_mul_pd(__m512d __a, __m512d __b)
{
  return (__m512d)((__v8df)__a * (__v8df)__b);
}

static __inline __m512 __DEFAULT_FN_ATTRS
_mm512_mul_ps(__m512 __a, __m512 __b)
{
  return (__m512)((__v16sf)__a * (__v16sf)__b);
}

static __inline __m512d __DEFAULT_FN_ATTRS
_mm512_sub_pd(__m512d __a, __m512d __b)
{
  return (__m512d)((__v8df)__a - (__v8df)__b);
}

static __inline __m512 __DEFAULT_FN_ATTRS
_mm512_sub_ps(__m512 __a, __m512 __b)
{
  return (__m512)((__v16sf)__a - (__v16sf)__b);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_add_epi64 (__m512i __A, __m512i __B)
{
  return (__m512i) ((__v8du) __A + (__v8du) __B);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_add_epi64 (__m512i __W, __mmask8 __U, __m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_paddq512_mask ((__v8di) __A,
             (__v8di) __B,
             (__v8di) __W,
             (__mmask8) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_add_epi64 (__mmask8 __U, __m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_paddq512_mask ((__v8di) __A,
             (__v8di) __B,
             (__v8di)
             _mm512_setzero_si512 (),
             (__mmask8) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_sub_epi64 (__m512i __A, __m512i __B)
{
  return (__m512i) ((__v8du) __A - (__v8du) __B);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_sub_epi64 (__m512i __W, __mmask8 __U, __m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_psubq512_mask ((__v8di) __A,
             (__v8di) __B,
             (__v8di) __W,
             (__mmask8) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_sub_epi64 (__mmask8 __U, __m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_psubq512_mask ((__v8di) __A,
             (__v8di) __B,
             (__v8di)
             _mm512_setzero_si512 (),
             (__mmask8) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_add_epi32 (__m512i __A, __m512i __B)
{
  return (__m512i) ((__v16su) __A + (__v16su) __B);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_add_epi32 (__m512i __W, __mmask16 __U, __m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_paddd512_mask ((__v16si) __A,
             (__v16si) __B,
             (__v16si) __W,
             (__mmask16) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_add_epi32 (__mmask16 __U, __m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_paddd512_mask ((__v16si) __A,
             (__v16si) __B,
             (__v16si)
             _mm512_setzero_si512 (),
             (__mmask16) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_sub_epi32 (__m512i __A, __m512i __B)
{
  return (__m512i) ((__v16su) __A - (__v16su) __B);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_sub_epi32 (__m512i __W, __mmask16 __U, __m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_psubd512_mask ((__v16si) __A,
             (__v16si) __B,
             (__v16si) __W,
             (__mmask16) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_sub_epi32 (__mmask16 __U, __m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_psubd512_mask ((__v16si) __A,
             (__v16si) __B,
             (__v16si)
             _mm512_setzero_si512 (),
             (__mmask16) __U);
}

#define _mm512_mask_max_round_pd(W, U, A, B, R) __extension__ ({ \
  (__m512d)__builtin_ia32_maxpd512_mask((__v8df)(__m512d)(A), \
                                        (__v8df)(__m512d)(B), \
                                        (__v8df)(__m512d)(W), (__mmask8)(U), \
                                        (int)(R)); })

#define _mm512_maskz_max_round_pd(U, A, B, R) __extension__ ({ \
  (__m512d)__builtin_ia32_maxpd512_mask((__v8df)(__m512d)(A), \
                                        (__v8df)(__m512d)(B), \
                                        (__v8df)_mm512_setzero_pd(), \
                                        (__mmask8)(U), (int)(R)); })

#define _mm512_max_round_pd(A, B, R) __extension__ ({ \
  (__m512d)__builtin_ia32_maxpd512_mask((__v8df)(__m512d)(A), \
                                        (__v8df)(__m512d)(B), \
                                        (__v8df)_mm512_undefined_pd(), \
                                        (__mmask8)-1, (int)(R)); })

static  __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_max_pd(__m512d __A, __m512d __B)
{
  return (__m512d) __builtin_ia32_maxpd512_mask ((__v8df) __A,
             (__v8df) __B,
             (__v8df)
             _mm512_setzero_pd (),
             (__mmask8) -1,
             _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_mask_max_pd (__m512d __W, __mmask8 __U, __m512d __A, __m512d __B)
{
  return (__m512d) __builtin_ia32_maxpd512_mask ((__v8df) __A,
                  (__v8df) __B,
                  (__v8df) __W,
                  (__mmask8) __U,
                  _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_maskz_max_pd (__mmask8 __U, __m512d __A, __m512d __B)
{
  return (__m512d) __builtin_ia32_maxpd512_mask ((__v8df) __A,
                  (__v8df) __B,
                  (__v8df)
                  _mm512_setzero_pd (),
                  (__mmask8) __U,
                  _MM_FROUND_CUR_DIRECTION);
}

#define _mm512_mask_max_round_ps(W, U, A, B, R) __extension__ ({ \
  (__m512)__builtin_ia32_maxps512_mask((__v16sf)(__m512)(A), \
                                       (__v16sf)(__m512)(B), \
                                       (__v16sf)(__m512)(W), (__mmask16)(U), \
                                       (int)(R)); })

#define _mm512_maskz_max_round_ps(U, A, B, R) __extension__ ({ \
  (__m512)__builtin_ia32_maxps512_mask((__v16sf)(__m512)(A), \
                                       (__v16sf)(__m512)(B), \
                                       (__v16sf)_mm512_setzero_ps(), \
                                       (__mmask16)(U), (int)(R)); })

#define _mm512_max_round_ps(A, B, R) __extension__ ({ \
  (__m512)__builtin_ia32_maxps512_mask((__v16sf)(__m512)(A), \
                                       (__v16sf)(__m512)(B), \
                                       (__v16sf)_mm512_undefined_ps(), \
                                       (__mmask16)-1, (int)(R)); })

static  __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_max_ps(__m512 __A, __m512 __B)
{
  return (__m512) __builtin_ia32_maxps512_mask ((__v16sf) __A,
            (__v16sf) __B,
            (__v16sf)
            _mm512_setzero_ps (),
            (__mmask16) -1,
            _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_mask_max_ps (__m512 __W, __mmask16 __U, __m512 __A, __m512 __B)
{
  return (__m512) __builtin_ia32_maxps512_mask ((__v16sf) __A,
                 (__v16sf) __B,
                 (__v16sf) __W,
                 (__mmask16) __U,
                 _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_maskz_max_ps (__mmask16 __U, __m512 __A, __m512 __B)
{
  return (__m512) __builtin_ia32_maxps512_mask ((__v16sf) __A,
                 (__v16sf) __B,
                 (__v16sf)
                 _mm512_setzero_ps (),
                 (__mmask16) __U,
                 _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_mask_max_ss(__m128 __W, __mmask8 __U,__m128 __A, __m128 __B) {
  return (__m128) __builtin_ia32_maxss_round_mask ((__v4sf) __A,
                (__v4sf) __B,
                (__v4sf) __W,
                (__mmask8) __U,
                _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_maskz_max_ss(__mmask8 __U,__m128 __A, __m128 __B) {
  return (__m128) __builtin_ia32_maxss_round_mask ((__v4sf) __A,
                (__v4sf) __B,
                (__v4sf)  _mm_setzero_ps (),
                (__mmask8) __U,
                _MM_FROUND_CUR_DIRECTION);
}

#define _mm_max_round_ss(A, B, R) __extension__ ({ \
  (__m128)__builtin_ia32_maxss_round_mask((__v4sf)(__m128)(A), \
                                          (__v4sf)(__m128)(B), \
                                          (__v4sf)_mm_setzero_ps(), \
                                          (__mmask8)-1, (int)(R)); })

#define _mm_mask_max_round_ss(W, U, A, B, R) __extension__ ({ \
  (__m128)__builtin_ia32_maxss_round_mask((__v4sf)(__m128)(A), \
                                          (__v4sf)(__m128)(B), \
                                          (__v4sf)(__m128)(W), (__mmask8)(U), \
                                          (int)(R)); })

#define _mm_maskz_max_round_ss(U, A, B, R) __extension__ ({ \
  (__m128)__builtin_ia32_maxss_round_mask((__v4sf)(__m128)(A), \
                                          (__v4sf)(__m128)(B), \
                                          (__v4sf)_mm_setzero_ps(), \
                                          (__mmask8)(U), (int)(R)); })

static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_mask_max_sd(__m128d __W, __mmask8 __U,__m128d __A, __m128d __B) {
  return (__m128d) __builtin_ia32_maxsd_round_mask ((__v2df) __A,
                (__v2df) __B,
                (__v2df) __W,
                (__mmask8) __U,
                _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_maskz_max_sd(__mmask8 __U,__m128d __A, __m128d __B) {
  return (__m128d) __builtin_ia32_maxsd_round_mask ((__v2df) __A,
                (__v2df) __B,
                (__v2df)  _mm_setzero_pd (),
                (__mmask8) __U,
                _MM_FROUND_CUR_DIRECTION);
}

#define _mm_max_round_sd(A, B, R) __extension__ ({ \
  (__m128d)__builtin_ia32_maxsd_round_mask((__v2df)(__m128d)(A), \
                                           (__v2df)(__m128d)(B), \
                                           (__v2df)_mm_setzero_pd(), \
                                           (__mmask8)-1, (int)(R)); })

#define _mm_mask_max_round_sd(W, U, A, B, R) __extension__ ({ \
  (__m128d)__builtin_ia32_maxsd_round_mask((__v2df)(__m128d)(A), \
                                           (__v2df)(__m128d)(B), \
                                           (__v2df)(__m128d)(W), \
                                           (__mmask8)(U), (int)(R)); })

#define _mm_maskz_max_round_sd(U, A, B, R) __extension__ ({ \
  (__m128d)__builtin_ia32_maxsd_round_mask((__v2df)(__m128d)(A), \
                                           (__v2df)(__m128d)(B), \
                                           (__v2df)_mm_setzero_pd(), \
                                           (__mmask8)(U), (int)(R)); })

static __inline __m512i
__DEFAULT_FN_ATTRS
_mm512_max_epi32(__m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_pmaxsd512_mask ((__v16si) __A,
              (__v16si) __B,
              (__v16si)
              _mm512_setzero_si512 (),
              (__mmask16) -1);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_max_epi32 (__m512i __W, __mmask16 __M, __m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_pmaxsd512_mask ((__v16si) __A,
                   (__v16si) __B,
                   (__v16si) __W, __M);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_max_epi32 (__mmask16 __M, __m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_pmaxsd512_mask ((__v16si) __A,
                   (__v16si) __B,
                   (__v16si)
                   _mm512_setzero_si512 (),
                   __M);
}

static __inline __m512i __DEFAULT_FN_ATTRS
_mm512_max_epu32(__m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_pmaxud512_mask ((__v16si) __A,
              (__v16si) __B,
              (__v16si)
              _mm512_setzero_si512 (),
              (__mmask16) -1);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_max_epu32 (__m512i __W, __mmask16 __M, __m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_pmaxud512_mask ((__v16si) __A,
                   (__v16si) __B,
                   (__v16si) __W, __M);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_max_epu32 (__mmask16 __M, __m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_pmaxud512_mask ((__v16si) __A,
                   (__v16si) __B,
                   (__v16si)
                   _mm512_setzero_si512 (),
                   __M);
}

static __inline __m512i __DEFAULT_FN_ATTRS
_mm512_max_epi64(__m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_pmaxsq512_mask ((__v8di) __A,
              (__v8di) __B,
              (__v8di)
              _mm512_setzero_si512 (),
              (__mmask8) -1);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_max_epi64 (__m512i __W, __mmask8 __M, __m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_pmaxsq512_mask ((__v8di) __A,
                   (__v8di) __B,
                   (__v8di) __W, __M);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_max_epi64 (__mmask8 __M, __m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_pmaxsq512_mask ((__v8di) __A,
                   (__v8di) __B,
                   (__v8di)
                   _mm512_setzero_si512 (),
                   __M);
}

static __inline __m512i __DEFAULT_FN_ATTRS
_mm512_max_epu64(__m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_pmaxuq512_mask ((__v8di) __A,
              (__v8di) __B,
              (__v8di)
              _mm512_setzero_si512 (),
              (__mmask8) -1);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_max_epu64 (__m512i __W, __mmask8 __M, __m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_pmaxuq512_mask ((__v8di) __A,
                   (__v8di) __B,
                   (__v8di) __W, __M);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_max_epu64 (__mmask8 __M, __m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_pmaxuq512_mask ((__v8di) __A,
                   (__v8di) __B,
                   (__v8di)
                   _mm512_setzero_si512 (),
                   __M);
}

#define _mm512_mask_min_round_pd(W, U, A, B, R) __extension__ ({ \
  (__m512d)__builtin_ia32_minpd512_mask((__v8df)(__m512d)(A), \
                                        (__v8df)(__m512d)(B), \
                                        (__v8df)(__m512d)(W), (__mmask8)(U), \
                                        (int)(R)); })

#define _mm512_maskz_min_round_pd(U, A, B, R) __extension__ ({ \
  (__m512d)__builtin_ia32_minpd512_mask((__v8df)(__m512d)(A), \
                                        (__v8df)(__m512d)(B), \
                                        (__v8df)_mm512_setzero_pd(), \
                                        (__mmask8)(U), (int)(R)); })

#define _mm512_min_round_pd(A, B, R) __extension__ ({ \
  (__m512d)__builtin_ia32_minpd512_mask((__v8df)(__m512d)(A), \
                                        (__v8df)(__m512d)(B), \
                                        (__v8df)_mm512_undefined_pd(), \
                                        (__mmask8)-1, (int)(R)); })

static  __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_min_pd(__m512d __A, __m512d __B)
{
  return (__m512d) __builtin_ia32_minpd512_mask ((__v8df) __A,
             (__v8df) __B,
             (__v8df)
             _mm512_setzero_pd (),
             (__mmask8) -1,
             _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_mask_min_pd (__m512d __W, __mmask8 __U, __m512d __A, __m512d __B)
{
  return (__m512d) __builtin_ia32_minpd512_mask ((__v8df) __A,
                  (__v8df) __B,
                  (__v8df) __W,
                  (__mmask8) __U,
                  _MM_FROUND_CUR_DIRECTION);
}

#define _mm512_mask_min_round_ps(W, U, A, B, R) __extension__ ({ \
  (__m512)__builtin_ia32_minps512_mask((__v16sf)(__m512)(A), \
                                       (__v16sf)(__m512)(B), \
                                       (__v16sf)(__m512)(W), (__mmask16)(U), \
                                       (int)(R)); })

#define _mm512_maskz_min_round_ps(U, A, B, R) __extension__ ({ \
  (__m512)__builtin_ia32_minps512_mask((__v16sf)(__m512)(A), \
                                       (__v16sf)(__m512)(B), \
                                       (__v16sf)_mm512_setzero_ps(), \
                                       (__mmask16)(U), (int)(R)); })

#define _mm512_min_round_ps(A, B, R) __extension__ ({ \
  (__m512)__builtin_ia32_minps512_mask((__v16sf)(__m512)(A), \
                                       (__v16sf)(__m512)(B), \
                                       (__v16sf)_mm512_undefined_ps(), \
                                       (__mmask16)-1, (int)(R)); })

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_maskz_min_pd (__mmask8 __U, __m512d __A, __m512d __B)
{
  return (__m512d) __builtin_ia32_minpd512_mask ((__v8df) __A,
                  (__v8df) __B,
                  (__v8df)
                  _mm512_setzero_pd (),
                  (__mmask8) __U,
                  _MM_FROUND_CUR_DIRECTION);
}

static  __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_min_ps(__m512 __A, __m512 __B)
{
  return (__m512) __builtin_ia32_minps512_mask ((__v16sf) __A,
            (__v16sf) __B,
            (__v16sf)
            _mm512_setzero_ps (),
            (__mmask16) -1,
            _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_mask_min_ps (__m512 __W, __mmask16 __U, __m512 __A, __m512 __B)
{
  return (__m512) __builtin_ia32_minps512_mask ((__v16sf) __A,
                 (__v16sf) __B,
                 (__v16sf) __W,
                 (__mmask16) __U,
                 _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_maskz_min_ps (__mmask16 __U, __m512 __A, __m512 __B)
{
  return (__m512) __builtin_ia32_minps512_mask ((__v16sf) __A,
                 (__v16sf) __B,
                 (__v16sf)
                 _mm512_setzero_ps (),
                 (__mmask16) __U,
                 _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_mask_min_ss(__m128 __W, __mmask8 __U,__m128 __A, __m128 __B) {
  return (__m128) __builtin_ia32_minss_round_mask ((__v4sf) __A,
                (__v4sf) __B,
                (__v4sf) __W,
                (__mmask8) __U,
                _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_maskz_min_ss(__mmask8 __U,__m128 __A, __m128 __B) {
  return (__m128) __builtin_ia32_minss_round_mask ((__v4sf) __A,
                (__v4sf) __B,
                (__v4sf)  _mm_setzero_ps (),
                (__mmask8) __U,
                _MM_FROUND_CUR_DIRECTION);
}

#define _mm_min_round_ss(A, B, R) __extension__ ({ \
  (__m128)__builtin_ia32_minss_round_mask((__v4sf)(__m128)(A), \
                                          (__v4sf)(__m128)(B), \
                                          (__v4sf)_mm_setzero_ps(), \
                                          (__mmask8)-1, (int)(R)); })

#define _mm_mask_min_round_ss(W, U, A, B, R) __extension__ ({ \
  (__m128)__builtin_ia32_minss_round_mask((__v4sf)(__m128)(A), \
                                          (__v4sf)(__m128)(B), \
                                          (__v4sf)(__m128)(W), (__mmask8)(U), \
                                          (int)(R)); })

#define _mm_maskz_min_round_ss(U, A, B, R) __extension__ ({ \
  (__m128)__builtin_ia32_minss_round_mask((__v4sf)(__m128)(A), \
                                          (__v4sf)(__m128)(B), \
                                          (__v4sf)_mm_setzero_ps(), \
                                          (__mmask8)(U), (int)(R)); })

static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_mask_min_sd(__m128d __W, __mmask8 __U,__m128d __A, __m128d __B) {
  return (__m128d) __builtin_ia32_minsd_round_mask ((__v2df) __A,
                (__v2df) __B,
                (__v2df) __W,
                (__mmask8) __U,
                _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_maskz_min_sd(__mmask8 __U,__m128d __A, __m128d __B) {
  return (__m128d) __builtin_ia32_minsd_round_mask ((__v2df) __A,
                (__v2df) __B,
                (__v2df)  _mm_setzero_pd (),
                (__mmask8) __U,
                _MM_FROUND_CUR_DIRECTION);
}

#define _mm_min_round_sd(A, B, R) __extension__ ({ \
  (__m128d)__builtin_ia32_minsd_round_mask((__v2df)(__m128d)(A), \
                                           (__v2df)(__m128d)(B), \
                                           (__v2df)_mm_setzero_pd(), \
                                           (__mmask8)-1, (int)(R)); })

#define _mm_mask_min_round_sd(W, U, A, B, R) __extension__ ({ \
  (__m128d)__builtin_ia32_minsd_round_mask((__v2df)(__m128d)(A), \
                                           (__v2df)(__m128d)(B), \
                                           (__v2df)(__m128d)(W), \
                                           (__mmask8)(U), (int)(R)); })

#define _mm_maskz_min_round_sd(U, A, B, R) __extension__ ({ \
  (__m128d)__builtin_ia32_minsd_round_mask((__v2df)(__m128d)(A), \
                                           (__v2df)(__m128d)(B), \
                                           (__v2df)_mm_setzero_pd(), \
                                           (__mmask8)(U), (int)(R)); })

static __inline __m512i
__DEFAULT_FN_ATTRS
_mm512_min_epi32(__m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_pminsd512_mask ((__v16si) __A,
              (__v16si) __B,
              (__v16si)
              _mm512_setzero_si512 (),
              (__mmask16) -1);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_min_epi32 (__m512i __W, __mmask16 __M, __m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_pminsd512_mask ((__v16si) __A,
                   (__v16si) __B,
                   (__v16si) __W, __M);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_min_epi32 (__mmask16 __M, __m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_pminsd512_mask ((__v16si) __A,
                   (__v16si) __B,
                   (__v16si)
                   _mm512_setzero_si512 (),
                   __M);
}

static __inline __m512i __DEFAULT_FN_ATTRS
_mm512_min_epu32(__m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_pminud512_mask ((__v16si) __A,
              (__v16si) __B,
              (__v16si)
              _mm512_setzero_si512 (),
              (__mmask16) -1);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_min_epu32 (__m512i __W, __mmask16 __M, __m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_pminud512_mask ((__v16si) __A,
                   (__v16si) __B,
                   (__v16si) __W, __M);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_min_epu32 (__mmask16 __M, __m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_pminud512_mask ((__v16si) __A,
                   (__v16si) __B,
                   (__v16si)
                   _mm512_setzero_si512 (),
                   __M);
}

static __inline __m512i __DEFAULT_FN_ATTRS
_mm512_min_epi64(__m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_pminsq512_mask ((__v8di) __A,
              (__v8di) __B,
              (__v8di)
              _mm512_setzero_si512 (),
              (__mmask8) -1);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_min_epi64 (__m512i __W, __mmask8 __M, __m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_pminsq512_mask ((__v8di) __A,
                   (__v8di) __B,
                   (__v8di) __W, __M);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_min_epi64 (__mmask8 __M, __m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_pminsq512_mask ((__v8di) __A,
                   (__v8di) __B,
                   (__v8di)
                   _mm512_setzero_si512 (),
                   __M);
}

static __inline __m512i __DEFAULT_FN_ATTRS
_mm512_min_epu64(__m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_pminuq512_mask ((__v8di) __A,
              (__v8di) __B,
              (__v8di)
              _mm512_setzero_si512 (),
              (__mmask8) -1);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_min_epu64 (__m512i __W, __mmask8 __M, __m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_pminuq512_mask ((__v8di) __A,
                   (__v8di) __B,
                   (__v8di) __W, __M);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_min_epu64 (__mmask8 __M, __m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_pminuq512_mask ((__v8di) __A,
                   (__v8di) __B,
                   (__v8di)
                   _mm512_setzero_si512 (),
                   __M);
}

static __inline __m512i __DEFAULT_FN_ATTRS
_mm512_mul_epi32(__m512i __X, __m512i __Y)
{
  return (__m512i) __builtin_ia32_pmuldq512_mask ((__v16si) __X,
              (__v16si) __Y,
              (__v8di)
              _mm512_setzero_si512 (),
              (__mmask8) -1);
}

static __inline __m512i __DEFAULT_FN_ATTRS
_mm512_mask_mul_epi32 (__m512i __W, __mmask8 __M, __m512i __X, __m512i __Y)
{
  return (__m512i) __builtin_ia32_pmuldq512_mask ((__v16si) __X,
              (__v16si) __Y,
              (__v8di) __W, __M);
}

static __inline __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_mul_epi32 (__mmask8 __M, __m512i __X, __m512i __Y)
{
  return (__m512i) __builtin_ia32_pmuldq512_mask ((__v16si) __X,
              (__v16si) __Y,
              (__v8di)
              _mm512_setzero_si512 (),
              __M);
}

static __inline __m512i __DEFAULT_FN_ATTRS
_mm512_mul_epu32(__m512i __X, __m512i __Y)
{
  return (__m512i) __builtin_ia32_pmuludq512_mask ((__v16si) __X,
               (__v16si) __Y,
               (__v8di)
               _mm512_setzero_si512 (),
               (__mmask8) -1);
}

static __inline __m512i __DEFAULT_FN_ATTRS
_mm512_mask_mul_epu32 (__m512i __W, __mmask8 __M, __m512i __X, __m512i __Y)
{
  return (__m512i) __builtin_ia32_pmuludq512_mask ((__v16si) __X,
               (__v16si) __Y,
               (__v8di) __W, __M);
}

static __inline __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_mul_epu32 (__mmask8 __M, __m512i __X, __m512i __Y)
{
  return (__m512i) __builtin_ia32_pmuludq512_mask ((__v16si) __X,
               (__v16si) __Y,
               (__v8di)
               _mm512_setzero_si512 (),
               __M);
}

static __inline __m512i __DEFAULT_FN_ATTRS
_mm512_mullo_epi32 (__m512i __A, __m512i __B)
{
  return (__m512i) ((__v16su) __A * (__v16su) __B);
}

static __inline __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_mullo_epi32 (__mmask16 __M, __m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_pmulld512_mask ((__v16si) __A,
              (__v16si) __B,
              (__v16si)
              _mm512_setzero_si512 (),
              __M);
}

static __inline __m512i __DEFAULT_FN_ATTRS
_mm512_mask_mullo_epi32 (__m512i __W, __mmask16 __M, __m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_pmulld512_mask ((__v16si) __A,
              (__v16si) __B,
              (__v16si) __W, __M);
}

#define _mm512_mask_sqrt_round_pd(W, U, A, R) __extension__ ({ \
  (__m512d)__builtin_ia32_sqrtpd512_mask((__v8df)(__m512d)(A), \
                                         (__v8df)(__m512d)(W), (__mmask8)(U), \
                                         (int)(R)); })

#define _mm512_maskz_sqrt_round_pd(U, A, R) __extension__ ({ \
  (__m512d)__builtin_ia32_sqrtpd512_mask((__v8df)(__m512d)(A), \
                                         (__v8df)_mm512_setzero_pd(), \
                                         (__mmask8)(U), (int)(R)); })

#define _mm512_sqrt_round_pd(A, R) __extension__ ({ \
  (__m512d)__builtin_ia32_sqrtpd512_mask((__v8df)(__m512d)(A), \
                                         (__v8df)_mm512_undefined_pd(), \
                                         (__mmask8)-1, (int)(R)); })

static  __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_sqrt_pd(__m512d __a)
{
  return (__m512d)__builtin_ia32_sqrtpd512_mask((__v8df)__a,
                                                (__v8df) _mm512_setzero_pd (),
                                                (__mmask8) -1,
                                                _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_mask_sqrt_pd (__m512d __W, __mmask8 __U, __m512d __A)
{
  return (__m512d) __builtin_ia32_sqrtpd512_mask ((__v8df) __A,
                   (__v8df) __W,
                   (__mmask8) __U,
                   _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_maskz_sqrt_pd (__mmask8 __U, __m512d __A)
{
  return (__m512d) __builtin_ia32_sqrtpd512_mask ((__v8df) __A,
                   (__v8df)
                   _mm512_setzero_pd (),
                   (__mmask8) __U,
                   _MM_FROUND_CUR_DIRECTION);
}

#define _mm512_mask_sqrt_round_ps(W, U, A, R) __extension__ ({ \
  (__m512)__builtin_ia32_sqrtps512_mask((__v16sf)(__m512)(A), \
                                        (__v16sf)(__m512)(W), (__mmask16)(U), \
                                        (int)(R)); })

#define _mm512_maskz_sqrt_round_ps(U, A, R) __extension__ ({ \
  (__m512)__builtin_ia32_sqrtps512_mask((__v16sf)(__m512)(A), \
                                        (__v16sf)_mm512_setzero_ps(), \
                                        (__mmask16)(U), (int)(R)); })

#define _mm512_sqrt_round_ps(A, R) __extension__ ({ \
  (__m512)__builtin_ia32_sqrtps512_mask((__v16sf)(__m512)(A), \
                                        (__v16sf)_mm512_undefined_ps(), \
                                        (__mmask16)-1, (int)(R)); })

static  __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_sqrt_ps(__m512 __a)
{
  return (__m512)__builtin_ia32_sqrtps512_mask((__v16sf)__a,
                                               (__v16sf) _mm512_setzero_ps (),
                                               (__mmask16) -1,
                                               _MM_FROUND_CUR_DIRECTION);
}

static  __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_mask_sqrt_ps(__m512 __W, __mmask16 __U, __m512 __A)
{
  return (__m512)__builtin_ia32_sqrtps512_mask((__v16sf)__A,
                                               (__v16sf) __W,
                                               (__mmask16) __U,
                                               _MM_FROUND_CUR_DIRECTION);
}

static  __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_maskz_sqrt_ps( __mmask16 __U, __m512 __A)
{
  return (__m512)__builtin_ia32_sqrtps512_mask((__v16sf)__A,
                                               (__v16sf) _mm512_setzero_ps (),
                                               (__mmask16) __U,
                                               _MM_FROUND_CUR_DIRECTION);
}

static  __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_rsqrt14_pd(__m512d __A)
{
  return (__m512d) __builtin_ia32_rsqrt14pd512_mask ((__v8df) __A,
                 (__v8df)
                 _mm512_setzero_pd (),
                 (__mmask8) -1);}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_mask_rsqrt14_pd (__m512d __W, __mmask8 __U, __m512d __A)
{
  return (__m512d) __builtin_ia32_rsqrt14pd512_mask ((__v8df) __A,
                  (__v8df) __W,
                  (__mmask8) __U);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_maskz_rsqrt14_pd (__mmask8 __U, __m512d __A)
{
  return (__m512d) __builtin_ia32_rsqrt14pd512_mask ((__v8df) __A,
                  (__v8df)
                  _mm512_setzero_pd (),
                  (__mmask8) __U);
}

static  __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_rsqrt14_ps(__m512 __A)
{
  return (__m512) __builtin_ia32_rsqrt14ps512_mask ((__v16sf) __A,
                (__v16sf)
                _mm512_setzero_ps (),
                (__mmask16) -1);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_mask_rsqrt14_ps (__m512 __W, __mmask16 __U, __m512 __A)
{
  return (__m512) __builtin_ia32_rsqrt14ps512_mask ((__v16sf) __A,
                 (__v16sf) __W,
                 (__mmask16) __U);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_maskz_rsqrt14_ps (__mmask16 __U, __m512 __A)
{
  return (__m512) __builtin_ia32_rsqrt14ps512_mask ((__v16sf) __A,
                 (__v16sf)
                 _mm512_setzero_ps (),
                 (__mmask16) __U);
}

static  __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_rsqrt14_ss(__m128 __A, __m128 __B)
{
  return (__m128) __builtin_ia32_rsqrt14ss_mask ((__v4sf) __A,
             (__v4sf) __B,
             (__v4sf)
             _mm_setzero_ps (),
             (__mmask8) -1);
}

static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_mask_rsqrt14_ss (__m128 __W, __mmask8 __U, __m128 __A, __m128 __B)
{
 return (__m128) __builtin_ia32_rsqrt14ss_mask ((__v4sf) __A,
          (__v4sf) __B,
          (__v4sf) __W,
          (__mmask8) __U);
}

static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_maskz_rsqrt14_ss (__mmask8 __U, __m128 __A, __m128 __B)
{
 return (__m128) __builtin_ia32_rsqrt14ss_mask ((__v4sf) __A,
          (__v4sf) __B,
          (__v4sf) _mm_setzero_ps (),
          (__mmask8) __U);
}

static  __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_rsqrt14_sd(__m128d __A, __m128d __B)
{
  return (__m128d) __builtin_ia32_rsqrt14sd_mask ((__v2df) __A,
              (__v2df) __B,
              (__v2df)
              _mm_setzero_pd (),
              (__mmask8) -1);
}

static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_mask_rsqrt14_sd (__m128d __W, __mmask8 __U, __m128d __A, __m128d __B)
{
 return (__m128d) __builtin_ia32_rsqrt14sd_mask ( (__v2df) __A,
          (__v2df) __B,
          (__v2df) __W,
          (__mmask8) __U);
}

static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_maskz_rsqrt14_sd (__mmask8 __U, __m128d __A, __m128d __B)
{
 return (__m128d) __builtin_ia32_rsqrt14sd_mask ( (__v2df) __A,
          (__v2df) __B,
          (__v2df) _mm_setzero_pd (),
          (__mmask8) __U);
}

static  __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_rcp14_pd(__m512d __A)
{
  return (__m512d) __builtin_ia32_rcp14pd512_mask ((__v8df) __A,
               (__v8df)
               _mm512_setzero_pd (),
               (__mmask8) -1);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_mask_rcp14_pd (__m512d __W, __mmask8 __U, __m512d __A)
{
  return (__m512d) __builtin_ia32_rcp14pd512_mask ((__v8df) __A,
                (__v8df) __W,
                (__mmask8) __U);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_maskz_rcp14_pd (__mmask8 __U, __m512d __A)
{
  return (__m512d) __builtin_ia32_rcp14pd512_mask ((__v8df) __A,
                (__v8df)
                _mm512_setzero_pd (),
                (__mmask8) __U);
}

static  __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_rcp14_ps(__m512 __A)
{
  return (__m512) __builtin_ia32_rcp14ps512_mask ((__v16sf) __A,
              (__v16sf)
              _mm512_setzero_ps (),
              (__mmask16) -1);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_mask_rcp14_ps (__m512 __W, __mmask16 __U, __m512 __A)
{
  return (__m512) __builtin_ia32_rcp14ps512_mask ((__v16sf) __A,
                   (__v16sf) __W,
                   (__mmask16) __U);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_maskz_rcp14_ps (__mmask16 __U, __m512 __A)
{
  return (__m512) __builtin_ia32_rcp14ps512_mask ((__v16sf) __A,
                   (__v16sf)
                   _mm512_setzero_ps (),
                   (__mmask16) __U);
}

static  __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_rcp14_ss(__m128 __A, __m128 __B)
{
  return (__m128) __builtin_ia32_rcp14ss_mask ((__v4sf) __A,
                 (__v4sf) __B,
                 (__v4sf)
                 _mm_setzero_ps (),
                 (__mmask8) -1);
}

static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_mask_rcp14_ss (__m128 __W, __mmask8 __U, __m128 __A, __m128 __B)
{
 return (__m128) __builtin_ia32_rcp14ss_mask ((__v4sf) __A,
          (__v4sf) __B,
          (__v4sf) __W,
          (__mmask8) __U);
}

static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_maskz_rcp14_ss (__mmask8 __U, __m128 __A, __m128 __B)
{
 return (__m128) __builtin_ia32_rcp14ss_mask ((__v4sf) __A,
          (__v4sf) __B,
          (__v4sf) _mm_setzero_ps (),
          (__mmask8) __U);
}

static  __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_rcp14_sd(__m128d __A, __m128d __B)
{
  return (__m128d) __builtin_ia32_rcp14sd_mask ((__v2df) __A,
            (__v2df) __B,
            (__v2df)
            _mm_setzero_pd (),
            (__mmask8) -1);
}

static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_mask_rcp14_sd (__m128d __W, __mmask8 __U, __m128d __A, __m128d __B)
{
 return (__m128d) __builtin_ia32_rcp14sd_mask ( (__v2df) __A,
          (__v2df) __B,
          (__v2df) __W,
          (__mmask8) __U);
}

static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_maskz_rcp14_sd (__mmask8 __U, __m128d __A, __m128d __B)
{
 return (__m128d) __builtin_ia32_rcp14sd_mask ( (__v2df) __A,
          (__v2df) __B,
          (__v2df) _mm_setzero_pd (),
          (__mmask8) __U);
}

static __inline __m512 __DEFAULT_FN_ATTRS
_mm512_floor_ps(__m512 __A)
{
  return (__m512) __builtin_ia32_rndscaleps_mask ((__v16sf) __A,
                                                  _MM_FROUND_FLOOR,
                                                  (__v16sf) __A, -1,
                                                  _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_mask_floor_ps (__m512 __W, __mmask16 __U, __m512 __A)
{
  return (__m512) __builtin_ia32_rndscaleps_mask ((__v16sf) __A,
                   _MM_FROUND_FLOOR,
                   (__v16sf) __W, __U,
                   _MM_FROUND_CUR_DIRECTION);
}

static __inline __m512d __DEFAULT_FN_ATTRS
_mm512_floor_pd(__m512d __A)
{
  return (__m512d) __builtin_ia32_rndscalepd_mask ((__v8df) __A,
                                                   _MM_FROUND_FLOOR,
                                                   (__v8df) __A, -1,
                                                   _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_mask_floor_pd (__m512d __W, __mmask8 __U, __m512d __A)
{
  return (__m512d) __builtin_ia32_rndscalepd_mask ((__v8df) __A,
                _MM_FROUND_FLOOR,
                (__v8df) __W, __U,
                _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_mask_ceil_ps (__m512 __W, __mmask16 __U, __m512 __A)
{
  return (__m512) __builtin_ia32_rndscaleps_mask ((__v16sf) __A,
                   _MM_FROUND_CEIL,
                   (__v16sf) __W, __U,
                   _MM_FROUND_CUR_DIRECTION);
}

static __inline __m512 __DEFAULT_FN_ATTRS
_mm512_ceil_ps(__m512 __A)
{
  return (__m512) __builtin_ia32_rndscaleps_mask ((__v16sf) __A,
                                                  _MM_FROUND_CEIL,
                                                  (__v16sf) __A, -1,
                                                  _MM_FROUND_CUR_DIRECTION);
}

static __inline __m512d __DEFAULT_FN_ATTRS
_mm512_ceil_pd(__m512d __A)
{
  return (__m512d) __builtin_ia32_rndscalepd_mask ((__v8df) __A,
                                                   _MM_FROUND_CEIL,
                                                   (__v8df) __A, -1,
                                                   _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_mask_ceil_pd (__m512d __W, __mmask8 __U, __m512d __A)
{
  return (__m512d) __builtin_ia32_rndscalepd_mask ((__v8df) __A,
                _MM_FROUND_CEIL,
                (__v8df) __W, __U,
                _MM_FROUND_CUR_DIRECTION);
}

static __inline __m512i __DEFAULT_FN_ATTRS
_mm512_abs_epi64(__m512i __A)
{
  return (__m512i) __builtin_ia32_pabsq512_mask ((__v8di) __A,
             (__v8di)
             _mm512_setzero_si512 (),
             (__mmask8) -1);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_abs_epi64 (__m512i __W, __mmask8 __U, __m512i __A)
{
  return (__m512i) __builtin_ia32_pabsq512_mask ((__v8di) __A,
                  (__v8di) __W,
                  (__mmask8) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_abs_epi64 (__mmask8 __U, __m512i __A)
{
  return (__m512i) __builtin_ia32_pabsq512_mask ((__v8di) __A,
                  (__v8di)
                  _mm512_setzero_si512 (),
                  (__mmask8) __U);
}

static __inline __m512i __DEFAULT_FN_ATTRS
_mm512_abs_epi32(__m512i __A)
{
  return (__m512i) __builtin_ia32_pabsd512_mask ((__v16si) __A,
             (__v16si)
             _mm512_setzero_si512 (),
             (__mmask16) -1);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_abs_epi32 (__m512i __W, __mmask16 __U, __m512i __A)
{
  return (__m512i) __builtin_ia32_pabsd512_mask ((__v16si) __A,
                  (__v16si) __W,
                  (__mmask16) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_abs_epi32 (__mmask16 __U, __m512i __A)
{
  return (__m512i) __builtin_ia32_pabsd512_mask ((__v16si) __A,
                  (__v16si)
                  _mm512_setzero_si512 (),
                  (__mmask16) __U);
}

static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_mask_add_ss(__m128 __W, __mmask8 __U,__m128 __A, __m128 __B) {
  return (__m128) __builtin_ia32_addss_round_mask ((__v4sf) __A,
                (__v4sf) __B,
                (__v4sf) __W,
                (__mmask8) __U,
                _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_maskz_add_ss(__mmask8 __U,__m128 __A, __m128 __B) {
  return (__m128) __builtin_ia32_addss_round_mask ((__v4sf) __A,
                (__v4sf) __B,
                (__v4sf)  _mm_setzero_ps (),
                (__mmask8) __U,
                _MM_FROUND_CUR_DIRECTION);
}

#define _mm_add_round_ss(A, B, R) __extension__ ({ \
  (__m128)__builtin_ia32_addss_round_mask((__v4sf)(__m128)(A), \
                                          (__v4sf)(__m128)(B), \
                                          (__v4sf)_mm_setzero_ps(), \
                                          (__mmask8)-1, (int)(R)); })

#define _mm_mask_add_round_ss(W, U, A, B, R) __extension__ ({ \
  (__m128)__builtin_ia32_addss_round_mask((__v4sf)(__m128)(A), \
                                          (__v4sf)(__m128)(B), \
                                          (__v4sf)(__m128)(W), (__mmask8)(U), \
                                          (int)(R)); })

#define _mm_maskz_add_round_ss(U, A, B, R) __extension__ ({ \
  (__m128)__builtin_ia32_addss_round_mask((__v4sf)(__m128)(A), \
                                          (__v4sf)(__m128)(B), \
                                          (__v4sf)_mm_setzero_ps(), \
                                          (__mmask8)(U), (int)(R)); })

static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_mask_add_sd(__m128d __W, __mmask8 __U,__m128d __A, __m128d __B) {
  return (__m128d) __builtin_ia32_addsd_round_mask ((__v2df) __A,
                (__v2df) __B,
                (__v2df) __W,
                (__mmask8) __U,
                _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_maskz_add_sd(__mmask8 __U,__m128d __A, __m128d __B) {
  return (__m128d) __builtin_ia32_addsd_round_mask ((__v2df) __A,
                (__v2df) __B,
                (__v2df)  _mm_setzero_pd (),
                (__mmask8) __U,
                _MM_FROUND_CUR_DIRECTION);
}
#define _mm_add_round_sd(A, B, R) __extension__ ({ \
  (__m128d)__builtin_ia32_addsd_round_mask((__v2df)(__m128d)(A), \
                                           (__v2df)(__m128d)(B), \
                                           (__v2df)_mm_setzero_pd(), \
                                           (__mmask8)-1, (int)(R)); })

#define _mm_mask_add_round_sd(W, U, A, B, R) __extension__ ({ \
  (__m128d)__builtin_ia32_addsd_round_mask((__v2df)(__m128d)(A), \
                                           (__v2df)(__m128d)(B), \
                                           (__v2df)(__m128d)(W), \
                                           (__mmask8)(U), (int)(R)); })

#define _mm_maskz_add_round_sd(U, A, B, R) __extension__ ({ \
  (__m128d)__builtin_ia32_addsd_round_mask((__v2df)(__m128d)(A), \
                                           (__v2df)(__m128d)(B), \
                                           (__v2df)_mm_setzero_pd(), \
                                           (__mmask8)(U), (int)(R)); })

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_mask_add_pd(__m512d __W, __mmask8 __U, __m512d __A, __m512d __B) {
  return (__m512d) __builtin_ia32_addpd512_mask ((__v8df) __A,
             (__v8df) __B,
             (__v8df) __W,
             (__mmask8) __U,
             _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_maskz_add_pd(__mmask8 __U, __m512d __A, __m512d __B) {
  return (__m512d) __builtin_ia32_addpd512_mask ((__v8df) __A,
             (__v8df) __B,
             (__v8df) _mm512_setzero_pd (),
             (__mmask8) __U,
             _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_mask_add_ps(__m512 __W, __mmask16 __U, __m512 __A, __m512 __B) {
  return (__m512) __builtin_ia32_addps512_mask ((__v16sf) __A,
            (__v16sf) __B,
            (__v16sf) __W,
            (__mmask16) __U,
            _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_maskz_add_ps(__mmask16 __U, __m512 __A, __m512 __B) {
  return (__m512) __builtin_ia32_addps512_mask ((__v16sf) __A,
            (__v16sf) __B,
            (__v16sf) _mm512_setzero_ps (),
            (__mmask16) __U,
            _MM_FROUND_CUR_DIRECTION);
}

#define _mm512_add_round_pd(A, B, R) __extension__ ({ \
  (__m512d)__builtin_ia32_addpd512_mask((__v8df)(__m512d)(A), \
                                        (__v8df)(__m512d)(B), \
                                        (__v8df)_mm512_setzero_pd(), \
                                        (__mmask8)-1, (int)(R)); })

#define _mm512_mask_add_round_pd(W, U, A, B, R) __extension__ ({ \
  (__m512d)__builtin_ia32_addpd512_mask((__v8df)(__m512d)(A), \
                                        (__v8df)(__m512d)(B), \
                                        (__v8df)(__m512d)(W), (__mmask8)(U), \
                                        (int)(R)); })

#define _mm512_maskz_add_round_pd(U, A, B, R) __extension__ ({ \
  (__m512d)__builtin_ia32_addpd512_mask((__v8df)(__m512d)(A), \
                                        (__v8df)(__m512d)(B), \
                                        (__v8df)_mm512_setzero_pd(), \
                                        (__mmask8)(U), (int)(R)); })

#define _mm512_add_round_ps(A, B, R) __extension__ ({ \
  (__m512)__builtin_ia32_addps512_mask((__v16sf)(__m512)(A), \
                                       (__v16sf)(__m512)(B), \
                                       (__v16sf)_mm512_setzero_ps(), \
                                       (__mmask16)-1, (int)(R)); })

#define _mm512_mask_add_round_ps(W, U, A, B, R) __extension__ ({ \
  (__m512)__builtin_ia32_addps512_mask((__v16sf)(__m512)(A), \
                                       (__v16sf)(__m512)(B), \
                                       (__v16sf)(__m512)(W), (__mmask16)(U), \
                                       (int)(R)); })

#define _mm512_maskz_add_round_ps(U, A, B, R) __extension__ ({ \
  (__m512)__builtin_ia32_addps512_mask((__v16sf)(__m512)(A), \
                                       (__v16sf)(__m512)(B), \
                                       (__v16sf)_mm512_setzero_ps(), \
                                       (__mmask16)(U), (int)(R)); })

static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_mask_sub_ss(__m128 __W, __mmask8 __U,__m128 __A, __m128 __B) {
  return (__m128) __builtin_ia32_subss_round_mask ((__v4sf) __A,
                (__v4sf) __B,
                (__v4sf) __W,
                (__mmask8) __U,
                _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_maskz_sub_ss(__mmask8 __U,__m128 __A, __m128 __B) {
  return (__m128) __builtin_ia32_subss_round_mask ((__v4sf) __A,
                (__v4sf) __B,
                (__v4sf)  _mm_setzero_ps (),
                (__mmask8) __U,
                _MM_FROUND_CUR_DIRECTION);
}
#define _mm_sub_round_ss(A, B, R) __extension__ ({ \
  (__m128)__builtin_ia32_subss_round_mask((__v4sf)(__m128)(A), \
                                          (__v4sf)(__m128)(B), \
                                          (__v4sf)_mm_setzero_ps(), \
                                          (__mmask8)-1, (int)(R)); })

#define _mm_mask_sub_round_ss(W, U, A, B, R) __extension__ ({ \
  (__m128)__builtin_ia32_subss_round_mask((__v4sf)(__m128)(A), \
                                          (__v4sf)(__m128)(B), \
                                          (__v4sf)(__m128)(W), (__mmask8)(U), \
                                          (int)(R)); })

#define _mm_maskz_sub_round_ss(U, A, B, R) __extension__ ({ \
  (__m128)__builtin_ia32_subss_round_mask((__v4sf)(__m128)(A), \
                                          (__v4sf)(__m128)(B), \
                                          (__v4sf)_mm_setzero_ps(), \
                                          (__mmask8)(U), (int)(R)); })

static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_mask_sub_sd(__m128d __W, __mmask8 __U,__m128d __A, __m128d __B) {
  return (__m128d) __builtin_ia32_subsd_round_mask ((__v2df) __A,
                (__v2df) __B,
                (__v2df) __W,
                (__mmask8) __U,
                _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_maskz_sub_sd(__mmask8 __U,__m128d __A, __m128d __B) {
  return (__m128d) __builtin_ia32_subsd_round_mask ((__v2df) __A,
                (__v2df) __B,
                (__v2df)  _mm_setzero_pd (),
                (__mmask8) __U,
                _MM_FROUND_CUR_DIRECTION);
}

#define _mm_sub_round_sd(A, B, R) __extension__ ({ \
  (__m128d)__builtin_ia32_subsd_round_mask((__v2df)(__m128d)(A), \
                                           (__v2df)(__m128d)(B), \
                                           (__v2df)_mm_setzero_pd(), \
                                           (__mmask8)-1, (int)(R)); })

#define _mm_mask_sub_round_sd(W, U, A, B, R) __extension__ ({ \
  (__m128d)__builtin_ia32_subsd_round_mask((__v2df)(__m128d)(A), \
                                           (__v2df)(__m128d)(B), \
                                           (__v2df)(__m128d)(W), \
                                           (__mmask8)(U), (int)(R)); })

#define _mm_maskz_sub_round_sd(U, A, B, R) __extension__ ({ \
  (__m128d)__builtin_ia32_subsd_round_mask((__v2df)(__m128d)(A), \
                                           (__v2df)(__m128d)(B), \
                                           (__v2df)_mm_setzero_pd(), \
                                           (__mmask8)(U), (int)(R)); })

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_mask_sub_pd(__m512d __W, __mmask8 __U, __m512d __A, __m512d __B) {
  return (__m512d) __builtin_ia32_subpd512_mask ((__v8df) __A,
             (__v8df) __B,
             (__v8df) __W,
             (__mmask8) __U,
             _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_maskz_sub_pd(__mmask8 __U, __m512d __A, __m512d __B) {
  return (__m512d) __builtin_ia32_subpd512_mask ((__v8df) __A,
             (__v8df) __B,
             (__v8df)
             _mm512_setzero_pd (),
             (__mmask8) __U,
             _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_mask_sub_ps(__m512 __W, __mmask16 __U, __m512 __A, __m512 __B) {
  return (__m512) __builtin_ia32_subps512_mask ((__v16sf) __A,
            (__v16sf) __B,
            (__v16sf) __W,
            (__mmask16) __U,
            _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_maskz_sub_ps(__mmask16 __U, __m512 __A, __m512 __B) {
  return (__m512) __builtin_ia32_subps512_mask ((__v16sf) __A,
            (__v16sf) __B,
            (__v16sf)
            _mm512_setzero_ps (),
            (__mmask16) __U,
            _MM_FROUND_CUR_DIRECTION);
}

#define _mm512_sub_round_pd(A, B, R) __extension__ ({ \
  (__m512d)__builtin_ia32_subpd512_mask((__v8df)(__m512d)(A), \
                                        (__v8df)(__m512d)(B), \
                                        (__v8df)_mm512_setzero_pd(), \
                                        (__mmask8)-1, (int)(R)); })

#define _mm512_mask_sub_round_pd(W, U, A, B, R) __extension__ ({ \
  (__m512d)__builtin_ia32_subpd512_mask((__v8df)(__m512d)(A), \
                                        (__v8df)(__m512d)(B), \
                                        (__v8df)(__m512d)(W), (__mmask8)(U), \
                                        (int)(R)); })

#define _mm512_maskz_sub_round_pd(U, A, B, R) __extension__ ({ \
  (__m512d)__builtin_ia32_subpd512_mask((__v8df)(__m512d)(A), \
                                        (__v8df)(__m512d)(B), \
                                        (__v8df)_mm512_setzero_pd(), \
                                        (__mmask8)(U), (int)(R)); })

#define _mm512_sub_round_ps(A, B, R) __extension__ ({ \
  (__m512)__builtin_ia32_subps512_mask((__v16sf)(__m512)(A), \
                                       (__v16sf)(__m512)(B), \
                                       (__v16sf)_mm512_setzero_ps(), \
                                       (__mmask16)-1, (int)(R)); })

#define _mm512_mask_sub_round_ps(W, U, A, B, R)  __extension__ ({ \
  (__m512)__builtin_ia32_subps512_mask((__v16sf)(__m512)(A), \
                                       (__v16sf)(__m512)(B), \
                                       (__v16sf)(__m512)(W), (__mmask16)(U), \
                                       (int)(R)); });

#define _mm512_maskz_sub_round_ps(U, A, B, R)  __extension__ ({ \
  (__m512)__builtin_ia32_subps512_mask((__v16sf)(__m512)(A), \
                                       (__v16sf)(__m512)(B), \
                                       (__v16sf)_mm512_setzero_ps(), \
                                       (__mmask16)(U), (int)(R)); });

static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_mask_mul_ss(__m128 __W, __mmask8 __U,__m128 __A, __m128 __B) {
  return (__m128) __builtin_ia32_mulss_round_mask ((__v4sf) __A,
                (__v4sf) __B,
                (__v4sf) __W,
                (__mmask8) __U,
                _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_maskz_mul_ss(__mmask8 __U,__m128 __A, __m128 __B) {
  return (__m128) __builtin_ia32_mulss_round_mask ((__v4sf) __A,
                (__v4sf) __B,
                (__v4sf)  _mm_setzero_ps (),
                (__mmask8) __U,
                _MM_FROUND_CUR_DIRECTION);
}
#define _mm_mul_round_ss(A, B, R) __extension__ ({ \
  (__m128)__builtin_ia32_mulss_round_mask((__v4sf)(__m128)(A), \
                                          (__v4sf)(__m128)(B), \
                                          (__v4sf)_mm_setzero_ps(), \
                                          (__mmask8)-1, (int)(R)); })

#define _mm_mask_mul_round_ss(W, U, A, B, R) __extension__ ({ \
  (__m128)__builtin_ia32_mulss_round_mask((__v4sf)(__m128)(A), \
                                          (__v4sf)(__m128)(B), \
                                          (__v4sf)(__m128)(W), (__mmask8)(U), \
                                          (int)(R)); })

#define _mm_maskz_mul_round_ss(U, A, B, R) __extension__ ({ \
  (__m128)__builtin_ia32_mulss_round_mask((__v4sf)(__m128)(A), \
                                          (__v4sf)(__m128)(B), \
                                          (__v4sf)_mm_setzero_ps(), \
                                          (__mmask8)(U), (int)(R)); })

static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_mask_mul_sd(__m128d __W, __mmask8 __U,__m128d __A, __m128d __B) {
  return (__m128d) __builtin_ia32_mulsd_round_mask ((__v2df) __A,
                (__v2df) __B,
                (__v2df) __W,
                (__mmask8) __U,
                _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_maskz_mul_sd(__mmask8 __U,__m128d __A, __m128d __B) {
  return (__m128d) __builtin_ia32_mulsd_round_mask ((__v2df) __A,
                (__v2df) __B,
                (__v2df)  _mm_setzero_pd (),
                (__mmask8) __U,
                _MM_FROUND_CUR_DIRECTION);
}

#define _mm_mul_round_sd(A, B, R) __extension__ ({ \
  (__m128d)__builtin_ia32_mulsd_round_mask((__v2df)(__m128d)(A), \
                                           (__v2df)(__m128d)(B), \
                                           (__v2df)_mm_setzero_pd(), \
                                           (__mmask8)-1, (int)(R)); })

#define _mm_mask_mul_round_sd(W, U, A, B, R) __extension__ ({ \
  (__m128d)__builtin_ia32_mulsd_round_mask((__v2df)(__m128d)(A), \
                                           (__v2df)(__m128d)(B), \
                                           (__v2df)(__m128d)(W), \
                                           (__mmask8)(U), (int)(R)); })

#define _mm_maskz_mul_round_sd(U, A, B, R) __extension__ ({ \
  (__m128d)__builtin_ia32_mulsd_round_mask((__v2df)(__m128d)(A), \
                                           (__v2df)(__m128d)(B), \
                                           (__v2df)_mm_setzero_pd(), \
                                           (__mmask8)(U), (int)(R)); })

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_mask_mul_pd(__m512d __W, __mmask8 __U, __m512d __A, __m512d __B) {
  return (__m512d) __builtin_ia32_mulpd512_mask ((__v8df) __A,
             (__v8df) __B,
             (__v8df) __W,
             (__mmask8) __U,
             _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_maskz_mul_pd(__mmask8 __U, __m512d __A, __m512d __B) {
  return (__m512d) __builtin_ia32_mulpd512_mask ((__v8df) __A,
             (__v8df) __B,
             (__v8df)
             _mm512_setzero_pd (),
             (__mmask8) __U,
             _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_mask_mul_ps(__m512 __W, __mmask16 __U, __m512 __A, __m512 __B) {
  return (__m512) __builtin_ia32_mulps512_mask ((__v16sf) __A,
            (__v16sf) __B,
            (__v16sf) __W,
            (__mmask16) __U,
            _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_maskz_mul_ps(__mmask16 __U, __m512 __A, __m512 __B) {
  return (__m512) __builtin_ia32_mulps512_mask ((__v16sf) __A,
            (__v16sf) __B,
            (__v16sf)
            _mm512_setzero_ps (),
            (__mmask16) __U,
            _MM_FROUND_CUR_DIRECTION);
}

#define _mm512_mul_round_pd(A, B, R) __extension__ ({ \
  (__m512d)__builtin_ia32_mulpd512_mask((__v8df)(__m512d)(A), \
                                        (__v8df)(__m512d)(B), \
                                        (__v8df)_mm512_setzero_pd(), \
                                        (__mmask8)-1, (int)(R)); })

#define _mm512_mask_mul_round_pd(W, U, A, B, R) __extension__ ({ \
  (__m512d)__builtin_ia32_mulpd512_mask((__v8df)(__m512d)(A), \
                                        (__v8df)(__m512d)(B), \
                                        (__v8df)(__m512d)(W), (__mmask8)(U), \
                                        (int)(R)); })

#define _mm512_maskz_mul_round_pd(U, A, B, R) __extension__ ({ \
  (__m512d)__builtin_ia32_mulpd512_mask((__v8df)(__m512d)(A), \
                                        (__v8df)(__m512d)(B), \
                                        (__v8df)_mm512_setzero_pd(), \
                                        (__mmask8)(U), (int)(R)); })

#define _mm512_mul_round_ps(A, B, R) __extension__ ({ \
  (__m512)__builtin_ia32_mulps512_mask((__v16sf)(__m512)(A), \
                                       (__v16sf)(__m512)(B), \
                                       (__v16sf)_mm512_setzero_ps(), \
                                       (__mmask16)-1, (int)(R)); })

#define _mm512_mask_mul_round_ps(W, U, A, B, R)  __extension__ ({ \
  (__m512)__builtin_ia32_mulps512_mask((__v16sf)(__m512)(A), \
                                       (__v16sf)(__m512)(B), \
                                       (__v16sf)(__m512)(W), (__mmask16)(U), \
                                       (int)(R)); });

#define _mm512_maskz_mul_round_ps(U, A, B, R)  __extension__ ({ \
  (__m512)__builtin_ia32_mulps512_mask((__v16sf)(__m512)(A), \
                                       (__v16sf)(__m512)(B), \
                                       (__v16sf)_mm512_setzero_ps(), \
                                       (__mmask16)(U), (int)(R)); });

static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_mask_div_ss(__m128 __W, __mmask8 __U,__m128 __A, __m128 __B) {
  return (__m128) __builtin_ia32_divss_round_mask ((__v4sf) __A,
                (__v4sf) __B,
                (__v4sf) __W,
                (__mmask8) __U,
                _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_maskz_div_ss(__mmask8 __U,__m128 __A, __m128 __B) {
  return (__m128) __builtin_ia32_divss_round_mask ((__v4sf) __A,
                (__v4sf) __B,
                (__v4sf)  _mm_setzero_ps (),
                (__mmask8) __U,
                _MM_FROUND_CUR_DIRECTION);
}

#define _mm_div_round_ss(A, B, R) __extension__ ({ \
  (__m128)__builtin_ia32_divss_round_mask((__v4sf)(__m128)(A), \
                                          (__v4sf)(__m128)(B), \
                                          (__v4sf)_mm_setzero_ps(), \
                                          (__mmask8)-1, (int)(R)); })

#define _mm_mask_div_round_ss(W, U, A, B, R) __extension__ ({ \
  (__m128)__builtin_ia32_divss_round_mask((__v4sf)(__m128)(A), \
                                          (__v4sf)(__m128)(B), \
                                          (__v4sf)(__m128)(W), (__mmask8)(U), \
                                          (int)(R)); })

#define _mm_maskz_div_round_ss(U, A, B, R) __extension__ ({ \
  (__m128)__builtin_ia32_divss_round_mask((__v4sf)(__m128)(A), \
                                          (__v4sf)(__m128)(B), \
                                          (__v4sf)_mm_setzero_ps(), \
                                          (__mmask8)(U), (int)(R)); })

static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_mask_div_sd(__m128d __W, __mmask8 __U,__m128d __A, __m128d __B) {
  return (__m128d) __builtin_ia32_divsd_round_mask ((__v2df) __A,
                (__v2df) __B,
                (__v2df) __W,
                (__mmask8) __U,
                _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_maskz_div_sd(__mmask8 __U,__m128d __A, __m128d __B) {
  return (__m128d) __builtin_ia32_divsd_round_mask ((__v2df) __A,
                (__v2df) __B,
                (__v2df)  _mm_setzero_pd (),
                (__mmask8) __U,
                _MM_FROUND_CUR_DIRECTION);
}

#define _mm_div_round_sd(A, B, R) __extension__ ({ \
  (__m128d)__builtin_ia32_divsd_round_mask((__v2df)(__m128d)(A), \
                                           (__v2df)(__m128d)(B), \
                                           (__v2df)_mm_setzero_pd(), \
                                           (__mmask8)-1, (int)(R)); })

#define _mm_mask_div_round_sd(W, U, A, B, R) __extension__ ({ \
  (__m128d)__builtin_ia32_divsd_round_mask((__v2df)(__m128d)(A), \
                                           (__v2df)(__m128d)(B), \
                                           (__v2df)(__m128d)(W), \
                                           (__mmask8)(U), (int)(R)); })

#define _mm_maskz_div_round_sd(U, A, B, R) __extension__ ({ \
  (__m128d)__builtin_ia32_divsd_round_mask((__v2df)(__m128d)(A), \
                                           (__v2df)(__m128d)(B), \
                                           (__v2df)_mm_setzero_pd(), \
                                           (__mmask8)(U), (int)(R)); })

static __inline __m512d __DEFAULT_FN_ATTRS
_mm512_div_pd(__m512d __a, __m512d __b)
{
  return (__m512d)((__v8df)__a/(__v8df)__b);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_mask_div_pd(__m512d __W, __mmask8 __U, __m512d __A, __m512d __B) {
  return (__m512d) __builtin_ia32_divpd512_mask ((__v8df) __A,
             (__v8df) __B,
             (__v8df) __W,
             (__mmask8) __U,
             _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_maskz_div_pd(__mmask8 __U, __m512d __A, __m512d __B) {
  return (__m512d) __builtin_ia32_divpd512_mask ((__v8df) __A,
             (__v8df) __B,
             (__v8df)
             _mm512_setzero_pd (),
             (__mmask8) __U,
             _MM_FROUND_CUR_DIRECTION);
}

static __inline __m512 __DEFAULT_FN_ATTRS
_mm512_div_ps(__m512 __a, __m512 __b)
{
  return (__m512)((__v16sf)__a/(__v16sf)__b);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_mask_div_ps(__m512 __W, __mmask16 __U, __m512 __A, __m512 __B) {
  return (__m512) __builtin_ia32_divps512_mask ((__v16sf) __A,
            (__v16sf) __B,
            (__v16sf) __W,
            (__mmask16) __U,
            _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_maskz_div_ps(__mmask16 __U, __m512 __A, __m512 __B) {
  return (__m512) __builtin_ia32_divps512_mask ((__v16sf) __A,
            (__v16sf) __B,
            (__v16sf)
            _mm512_setzero_ps (),
            (__mmask16) __U,
            _MM_FROUND_CUR_DIRECTION);
}

#define _mm512_div_round_pd(A, B, R) __extension__ ({ \
  (__m512d)__builtin_ia32_divpd512_mask((__v8df)(__m512d)(A), \
                                        (__v8df)(__m512d)(B), \
                                        (__v8df)_mm512_setzero_pd(), \
                                        (__mmask8)-1, (int)(R)); })

#define _mm512_mask_div_round_pd(W, U, A, B, R) __extension__ ({ \
  (__m512d)__builtin_ia32_divpd512_mask((__v8df)(__m512d)(A), \
                                        (__v8df)(__m512d)(B), \
                                        (__v8df)(__m512d)(W), (__mmask8)(U), \
                                        (int)(R)); })

#define _mm512_maskz_div_round_pd(U, A, B, R) __extension__ ({ \
  (__m512d)__builtin_ia32_divpd512_mask((__v8df)(__m512d)(A), \
                                        (__v8df)(__m512d)(B), \
                                        (__v8df)_mm512_setzero_pd(), \
                                        (__mmask8)(U), (int)(R)); })

#define _mm512_div_round_ps(A, B, R) __extension__ ({ \
  (__m512)__builtin_ia32_divps512_mask((__v16sf)(__m512)(A), \
                                       (__v16sf)(__m512)(B), \
                                       (__v16sf)_mm512_setzero_ps(), \
                                       (__mmask16)-1, (int)(R)); })

#define _mm512_mask_div_round_ps(W, U, A, B, R)  __extension__ ({ \
  (__m512)__builtin_ia32_divps512_mask((__v16sf)(__m512)(A), \
                                       (__v16sf)(__m512)(B), \
                                       (__v16sf)(__m512)(W), (__mmask16)(U), \
                                       (int)(R)); });

#define _mm512_maskz_div_round_ps(U, A, B, R)  __extension__ ({ \
  (__m512)__builtin_ia32_divps512_mask((__v16sf)(__m512)(A), \
                                       (__v16sf)(__m512)(B), \
                                       (__v16sf)_mm512_setzero_ps(), \
                                       (__mmask16)(U), (int)(R)); });

#define _mm512_roundscale_ps(A, B) __extension__ ({ \
  (__m512)__builtin_ia32_rndscaleps_mask((__v16sf)(__m512)(A), (int)(B), \
                                         (__v16sf)(__m512)(A), (__mmask16)-1, \
                                         _MM_FROUND_CUR_DIRECTION); })

#define _mm512_mask_roundscale_ps(A, B, C, imm) __extension__ ({\
  (__m512)__builtin_ia32_rndscaleps_mask((__v16sf)(__m512)(C), (int)(imm), \
                                         (__v16sf)(__m512)(A), (__mmask16)(B), \
                                         _MM_FROUND_CUR_DIRECTION); })

#define _mm512_maskz_roundscale_ps(A, B, imm) __extension__ ({\
  (__m512)__builtin_ia32_rndscaleps_mask((__v16sf)(__m512)(B), (int)(imm), \
                                         (__v16sf)_mm512_setzero_ps(), \
                                         (__mmask16)(A), \
                                         _MM_FROUND_CUR_DIRECTION); })

#define _mm512_mask_roundscale_round_ps(A, B, C, imm, R) __extension__ ({ \
  (__m512)__builtin_ia32_rndscaleps_mask((__v16sf)(__m512)(C), (int)(imm), \
                                         (__v16sf)(__m512)(A), (__mmask16)(B), \
                                         (int)(R)); })

#define _mm512_maskz_roundscale_round_ps(A, B, imm, R) __extension__ ({ \
  (__m512)__builtin_ia32_rndscaleps_mask((__v16sf)(__m512)(B), (int)(imm), \
                                         (__v16sf)_mm512_setzero_ps(), \
                                         (__mmask16)(A), (int)(R)); })

#define _mm512_roundscale_round_ps(A, imm, R) __extension__ ({ \
  (__m512)__builtin_ia32_rndscaleps_mask((__v16sf)(__m512)(A), (int)(imm), \
                                         (__v16sf)_mm512_undefined_ps(), \
                                         (__mmask16)-1, (int)(R)); })

#define _mm512_roundscale_pd(A, B) __extension__ ({ \
  (__m512d)__builtin_ia32_rndscalepd_mask((__v8df)(__m512d)(A), (int)(B), \
                                          (__v8df)(__m512d)(A), (__mmask8)-1, \
                                          _MM_FROUND_CUR_DIRECTION); })

#define _mm512_mask_roundscale_pd(A, B, C, imm) __extension__ ({\
  (__m512d)__builtin_ia32_rndscalepd_mask((__v8df)(__m512d)(C), (int)(imm), \
                                          (__v8df)(__m512d)(A), (__mmask8)(B), \
                                          _MM_FROUND_CUR_DIRECTION); })

#define _mm512_maskz_roundscale_pd(A, B, imm) __extension__ ({\
  (__m512d)__builtin_ia32_rndscalepd_mask((__v8df)(__m512d)(B), (int)(imm), \
                                          (__v8df)_mm512_setzero_pd(), \
                                          (__mmask8)(A), \
                                          _MM_FROUND_CUR_DIRECTION); })

#define _mm512_mask_roundscale_round_pd(A, B, C, imm, R) __extension__ ({ \
  (__m512d)__builtin_ia32_rndscalepd_mask((__v8df)(__m512d)(C), (int)(imm), \
                                          (__v8df)(__m512d)(A), (__mmask8)(B), \
                                          (int)(R)); })

#define _mm512_maskz_roundscale_round_pd(A, B, imm, R) __extension__ ({ \
  (__m512d)__builtin_ia32_rndscalepd_mask((__v8df)(__m512d)(B), (int)(imm), \
                                          (__v8df)_mm512_setzero_pd(), \
                                          (__mmask8)(A), (int)(R)); })

#define _mm512_roundscale_round_pd(A, imm, R) __extension__ ({ \
  (__m512d)__builtin_ia32_rndscalepd_mask((__v8df)(__m512d)(A), (int)(imm), \
                                          (__v8df)_mm512_undefined_pd(), \
                                          (__mmask8)-1, (int)(R)); })

#define _mm512_fmadd_round_pd(A, B, C, R) __extension__ ({ \
  (__m512d)__builtin_ia32_vfmaddpd512_mask((__v8df)(__m512d)(A), \
                                           (__v8df)(__m512d)(B), \
                                           (__v8df)(__m512d)(C), (__mmask8)-1, \
                                           (int)(R)); })


#define _mm512_mask_fmadd_round_pd(A, U, B, C, R) __extension__ ({ \
  (__m512d)__builtin_ia32_vfmaddpd512_mask((__v8df)(__m512d)(A), \
                                           (__v8df)(__m512d)(B), \
                                           (__v8df)(__m512d)(C), \
                                           (__mmask8)(U), (int)(R)); })


#define _mm512_mask3_fmadd_round_pd(A, B, C, U, R) __extension__ ({ \
  (__m512d)__builtin_ia32_vfmaddpd512_mask3((__v8df)(__m512d)(A), \
                                            (__v8df)(__m512d)(B), \
                                            (__v8df)(__m512d)(C), \
                                            (__mmask8)(U), (int)(R)); })


#define _mm512_maskz_fmadd_round_pd(U, A, B, C, R) __extension__ ({ \
  (__m512d)__builtin_ia32_vfmaddpd512_maskz((__v8df)(__m512d)(A), \
                                            (__v8df)(__m512d)(B), \
                                            (__v8df)(__m512d)(C), \
                                            (__mmask8)(U), (int)(R)); })


#define _mm512_fmsub_round_pd(A, B, C, R) __extension__ ({ \
  (__m512d)__builtin_ia32_vfmaddpd512_mask((__v8df)(__m512d)(A), \
                                           (__v8df)(__m512d)(B), \
                                           -(__v8df)(__m512d)(C), \
                                           (__mmask8)-1, (int)(R)); })


#define _mm512_mask_fmsub_round_pd(A, U, B, C, R) __extension__ ({ \
  (__m512d)__builtin_ia32_vfmaddpd512_mask((__v8df)(__m512d)(A), \
                                           (__v8df)(__m512d)(B), \
                                           -(__v8df)(__m512d)(C), \
                                           (__mmask8)(U), (int)(R)); })


#define _mm512_maskz_fmsub_round_pd(U, A, B, C, R) __extension__ ({ \
  (__m512d)__builtin_ia32_vfmaddpd512_maskz((__v8df)(__m512d)(A), \
                                            (__v8df)(__m512d)(B), \
                                            -(__v8df)(__m512d)(C), \
                                            (__mmask8)(U), (int)(R)); })


#define _mm512_fnmadd_round_pd(A, B, C, R) __extension__ ({ \
  (__m512d)__builtin_ia32_vfmaddpd512_mask(-(__v8df)(__m512d)(A), \
                                           (__v8df)(__m512d)(B), \
                                           (__v8df)(__m512d)(C), (__mmask8)-1, \
                                           (int)(R)); })


#define _mm512_mask3_fnmadd_round_pd(A, B, C, U, R) __extension__ ({ \
  (__m512d)__builtin_ia32_vfmaddpd512_mask3(-(__v8df)(__m512d)(A), \
                                            (__v8df)(__m512d)(B), \
                                            (__v8df)(__m512d)(C), \
                                            (__mmask8)(U), (int)(R)); })


#define _mm512_maskz_fnmadd_round_pd(U, A, B, C, R) __extension__ ({ \
  (__m512d)__builtin_ia32_vfmaddpd512_maskz(-(__v8df)(__m512d)(A), \
                                            (__v8df)(__m512d)(B), \
                                            (__v8df)(__m512d)(C), \
                                            (__mmask8)(U), (int)(R)); })


#define _mm512_fnmsub_round_pd(A, B, C, R) __extension__ ({ \
  (__m512d)__builtin_ia32_vfmaddpd512_mask(-(__v8df)(__m512d)(A), \
                                           (__v8df)(__m512d)(B), \
                                           -(__v8df)(__m512d)(C), \
                                           (__mmask8)-1, (int)(R)); })


#define _mm512_maskz_fnmsub_round_pd(U, A, B, C, R) __extension__ ({ \
  (__m512d)__builtin_ia32_vfmaddpd512_maskz(-(__v8df)(__m512d)(A), \
                                            (__v8df)(__m512d)(B), \
                                            -(__v8df)(__m512d)(C), \
                                            (__mmask8)(U), (int)(R)); })


static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_fmadd_pd(__m512d __A, __m512d __B, __m512d __C)
{
  return (__m512d) __builtin_ia32_vfmaddpd512_mask ((__v8df) __A,
                                                    (__v8df) __B,
                                                    (__v8df) __C,
                                                    (__mmask8) -1,
                                                    _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_mask_fmadd_pd(__m512d __A, __mmask8 __U, __m512d __B, __m512d __C)
{
  return (__m512d) __builtin_ia32_vfmaddpd512_mask ((__v8df) __A,
                                                    (__v8df) __B,
                                                    (__v8df) __C,
                                                    (__mmask8) __U,
                                                    _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_mask3_fmadd_pd(__m512d __A, __m512d __B, __m512d __C, __mmask8 __U)
{
  return (__m512d) __builtin_ia32_vfmaddpd512_mask3 ((__v8df) __A,
                                                     (__v8df) __B,
                                                     (__v8df) __C,
                                                     (__mmask8) __U,
                                                     _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_maskz_fmadd_pd(__mmask8 __U, __m512d __A, __m512d __B, __m512d __C)
{
  return (__m512d) __builtin_ia32_vfmaddpd512_maskz ((__v8df) __A,
                                                     (__v8df) __B,
                                                     (__v8df) __C,
                                                     (__mmask8) __U,
                                                     _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_fmsub_pd(__m512d __A, __m512d __B, __m512d __C)
{
  return (__m512d) __builtin_ia32_vfmaddpd512_mask ((__v8df) __A,
                                                    (__v8df) __B,
                                                    -(__v8df) __C,
                                                    (__mmask8) -1,
                                                    _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_mask_fmsub_pd(__m512d __A, __mmask8 __U, __m512d __B, __m512d __C)
{
  return (__m512d) __builtin_ia32_vfmaddpd512_mask ((__v8df) __A,
                                                    (__v8df) __B,
                                                    -(__v8df) __C,
                                                    (__mmask8) __U,
                                                    _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_maskz_fmsub_pd(__mmask8 __U, __m512d __A, __m512d __B, __m512d __C)
{
  return (__m512d) __builtin_ia32_vfmaddpd512_maskz ((__v8df) __A,
                                                     (__v8df) __B,
                                                     -(__v8df) __C,
                                                     (__mmask8) __U,
                                                     _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_fnmadd_pd(__m512d __A, __m512d __B, __m512d __C)
{
  return (__m512d) __builtin_ia32_vfmaddpd512_mask (-(__v8df) __A,
                                                    (__v8df) __B,
                                                    (__v8df) __C,
                                                    (__mmask8) -1,
                                                    _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_mask3_fnmadd_pd(__m512d __A, __m512d __B, __m512d __C, __mmask8 __U)
{
  return (__m512d) __builtin_ia32_vfmaddpd512_mask3 (-(__v8df) __A,
                                                     (__v8df) __B,
                                                     (__v8df) __C,
                                                     (__mmask8) __U,
                                                     _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_maskz_fnmadd_pd(__mmask8 __U, __m512d __A, __m512d __B, __m512d __C)
{
  return (__m512d) __builtin_ia32_vfmaddpd512_maskz (-(__v8df) __A,
                                                     (__v8df) __B,
                                                     (__v8df) __C,
                                                     (__mmask8) __U,
                                                     _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_fnmsub_pd(__m512d __A, __m512d __B, __m512d __C)
{
  return (__m512d) __builtin_ia32_vfmaddpd512_mask (-(__v8df) __A,
                                                    (__v8df) __B,
                                                    -(__v8df) __C,
                                                    (__mmask8) -1,
                                                    _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_maskz_fnmsub_pd(__mmask8 __U, __m512d __A, __m512d __B, __m512d __C)
{
  return (__m512d) __builtin_ia32_vfmaddpd512_maskz (-(__v8df) __A,
                                                     (__v8df) __B,
                                                     -(__v8df) __C,
                                                     (__mmask8) __U,
                                                     _MM_FROUND_CUR_DIRECTION);
}

#define _mm512_fmadd_round_ps(A, B, C, R) __extension__ ({ \
  (__m512)__builtin_ia32_vfmaddps512_mask((__v16sf)(__m512)(A), \
                                          (__v16sf)(__m512)(B), \
                                          (__v16sf)(__m512)(C), (__mmask16)-1, \
                                          (int)(R)); })


#define _mm512_mask_fmadd_round_ps(A, U, B, C, R) __extension__ ({ \
  (__m512)__builtin_ia32_vfmaddps512_mask((__v16sf)(__m512)(A), \
                                          (__v16sf)(__m512)(B), \
                                          (__v16sf)(__m512)(C), \
                                          (__mmask16)(U), (int)(R)); })


#define _mm512_mask3_fmadd_round_ps(A, B, C, U, R) __extension__ ({ \
  (__m512)__builtin_ia32_vfmaddps512_mask3((__v16sf)(__m512)(A), \
                                           (__v16sf)(__m512)(B), \
                                           (__v16sf)(__m512)(C), \
                                           (__mmask16)(U), (int)(R)); })


#define _mm512_maskz_fmadd_round_ps(U, A, B, C, R) __extension__ ({ \
  (__m512)__builtin_ia32_vfmaddps512_maskz((__v16sf)(__m512)(A), \
                                           (__v16sf)(__m512)(B), \
                                           (__v16sf)(__m512)(C), \
                                           (__mmask16)(U), (int)(R)); })


#define _mm512_fmsub_round_ps(A, B, C, R) __extension__ ({ \
  (__m512)__builtin_ia32_vfmaddps512_mask((__v16sf)(__m512)(A), \
                                          (__v16sf)(__m512)(B), \
                                          -(__v16sf)(__m512)(C), \
                                          (__mmask16)-1, (int)(R)); })


#define _mm512_mask_fmsub_round_ps(A, U, B, C, R) __extension__ ({ \
  (__m512)__builtin_ia32_vfmaddps512_mask((__v16sf)(__m512)(A), \
                                          (__v16sf)(__m512)(B), \
                                          -(__v16sf)(__m512)(C), \
                                          (__mmask16)(U), (int)(R)); })


#define _mm512_maskz_fmsub_round_ps(U, A, B, C, R) __extension__ ({ \
  (__m512)__builtin_ia32_vfmaddps512_maskz((__v16sf)(__m512)(A), \
                                           (__v16sf)(__m512)(B), \
                                           -(__v16sf)(__m512)(C), \
                                           (__mmask16)(U), (int)(R)); })


#define _mm512_fnmadd_round_ps(A, B, C, R) __extension__ ({ \
  (__m512)__builtin_ia32_vfmaddps512_mask(-(__v16sf)(__m512)(A), \
                                          (__v16sf)(__m512)(B), \
                                          (__v16sf)(__m512)(C), (__mmask16)-1, \
                                          (int)(R)); })


#define _mm512_mask3_fnmadd_round_ps(A, B, C, U, R) __extension__ ({ \
  (__m512)__builtin_ia32_vfmaddps512_mask3(-(__v16sf)(__m512)(A), \
                                           (__v16sf)(__m512)(B), \
                                           (__v16sf)(__m512)(C), \
                                           (__mmask16)(U), (int)(R)); })


#define _mm512_maskz_fnmadd_round_ps(U, A, B, C, R) __extension__ ({ \
  (__m512)__builtin_ia32_vfmaddps512_maskz(-(__v16sf)(__m512)(A), \
                                           (__v16sf)(__m512)(B), \
                                           (__v16sf)(__m512)(C), \
                                           (__mmask16)(U), (int)(R)); })


#define _mm512_fnmsub_round_ps(A, B, C, R) __extension__ ({ \
  (__m512)__builtin_ia32_vfmaddps512_mask(-(__v16sf)(__m512)(A), \
                                          (__v16sf)(__m512)(B), \
                                          -(__v16sf)(__m512)(C), \
                                          (__mmask16)-1, (int)(R)); })


#define _mm512_maskz_fnmsub_round_ps(U, A, B, C, R) __extension__ ({ \
  (__m512)__builtin_ia32_vfmaddps512_maskz(-(__v16sf)(__m512)(A), \
                                           (__v16sf)(__m512)(B), \
                                           -(__v16sf)(__m512)(C), \
                                           (__mmask16)(U), (int)(R)); })


static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_fmadd_ps(__m512 __A, __m512 __B, __m512 __C)
{
  return (__m512) __builtin_ia32_vfmaddps512_mask ((__v16sf) __A,
                                                   (__v16sf) __B,
                                                   (__v16sf) __C,
                                                   (__mmask16) -1,
                                                   _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_mask_fmadd_ps(__m512 __A, __mmask16 __U, __m512 __B, __m512 __C)
{
  return (__m512) __builtin_ia32_vfmaddps512_mask ((__v16sf) __A,
                                                   (__v16sf) __B,
                                                   (__v16sf) __C,
                                                   (__mmask16) __U,
                                                   _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_mask3_fmadd_ps(__m512 __A, __m512 __B, __m512 __C, __mmask16 __U)
{
  return (__m512) __builtin_ia32_vfmaddps512_mask3 ((__v16sf) __A,
                                                    (__v16sf) __B,
                                                    (__v16sf) __C,
                                                    (__mmask16) __U,
                                                    _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_maskz_fmadd_ps(__mmask16 __U, __m512 __A, __m512 __B, __m512 __C)
{
  return (__m512) __builtin_ia32_vfmaddps512_maskz ((__v16sf) __A,
                                                    (__v16sf) __B,
                                                    (__v16sf) __C,
                                                    (__mmask16) __U,
                                                    _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_fmsub_ps(__m512 __A, __m512 __B, __m512 __C)
{
  return (__m512) __builtin_ia32_vfmaddps512_mask ((__v16sf) __A,
                                                   (__v16sf) __B,
                                                   -(__v16sf) __C,
                                                   (__mmask16) -1,
                                                   _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_mask_fmsub_ps(__m512 __A, __mmask16 __U, __m512 __B, __m512 __C)
{
  return (__m512) __builtin_ia32_vfmaddps512_mask ((__v16sf) __A,
                                                   (__v16sf) __B,
                                                   -(__v16sf) __C,
                                                   (__mmask16) __U,
                                                   _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_maskz_fmsub_ps(__mmask16 __U, __m512 __A, __m512 __B, __m512 __C)
{
  return (__m512) __builtin_ia32_vfmaddps512_maskz ((__v16sf) __A,
                                                    (__v16sf) __B,
                                                    -(__v16sf) __C,
                                                    (__mmask16) __U,
                                                    _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_fnmadd_ps(__m512 __A, __m512 __B, __m512 __C)
{
  return (__m512) __builtin_ia32_vfmaddps512_mask (-(__v16sf) __A,
                                                   (__v16sf) __B,
                                                   (__v16sf) __C,
                                                   (__mmask16) -1,
                                                   _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_mask3_fnmadd_ps(__m512 __A, __m512 __B, __m512 __C, __mmask16 __U)
{
  return (__m512) __builtin_ia32_vfmaddps512_mask3 (-(__v16sf) __A,
                                                    (__v16sf) __B,
                                                    (__v16sf) __C,
                                                    (__mmask16) __U,
                                                    _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_maskz_fnmadd_ps(__mmask16 __U, __m512 __A, __m512 __B, __m512 __C)
{
  return (__m512) __builtin_ia32_vfmaddps512_maskz (-(__v16sf) __A,
                                                    (__v16sf) __B,
                                                    (__v16sf) __C,
                                                    (__mmask16) __U,
                                                    _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_fnmsub_ps(__m512 __A, __m512 __B, __m512 __C)
{
  return (__m512) __builtin_ia32_vfmaddps512_mask (-(__v16sf) __A,
                                                   (__v16sf) __B,
                                                   -(__v16sf) __C,
                                                   (__mmask16) -1,
                                                   _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_maskz_fnmsub_ps(__mmask16 __U, __m512 __A, __m512 __B, __m512 __C)
{
  return (__m512) __builtin_ia32_vfmaddps512_maskz (-(__v16sf) __A,
                                                    (__v16sf) __B,
                                                    -(__v16sf) __C,
                                                    (__mmask16) __U,
                                                    _MM_FROUND_CUR_DIRECTION);
}

#define _mm512_fmaddsub_round_pd(A, B, C, R) __extension__ ({ \
  (__m512d)__builtin_ia32_vfmaddsubpd512_mask((__v8df)(__m512d)(A), \
                                              (__v8df)(__m512d)(B), \
                                              (__v8df)(__m512d)(C), \
                                              (__mmask8)-1, (int)(R)); })


#define _mm512_mask_fmaddsub_round_pd(A, U, B, C, R) __extension__ ({ \
  (__m512d)__builtin_ia32_vfmaddsubpd512_mask((__v8df)(__m512d)(A), \
                                              (__v8df)(__m512d)(B), \
                                              (__v8df)(__m512d)(C), \
                                              (__mmask8)(U), (int)(R)); })


#define _mm512_mask3_fmaddsub_round_pd(A, B, C, U, R) __extension__ ({ \
  (__m512d)__builtin_ia32_vfmaddsubpd512_mask3((__v8df)(__m512d)(A), \
                                               (__v8df)(__m512d)(B), \
                                               (__v8df)(__m512d)(C), \
                                               (__mmask8)(U), (int)(R)); })


#define _mm512_maskz_fmaddsub_round_pd(U, A, B, C, R) __extension__ ({ \
  (__m512d)__builtin_ia32_vfmaddsubpd512_maskz((__v8df)(__m512d)(A), \
                                               (__v8df)(__m512d)(B), \
                                               (__v8df)(__m512d)(C), \
                                               (__mmask8)(U), (int)(R)); })


#define _mm512_fmsubadd_round_pd(A, B, C, R) __extension__ ({ \
  (__m512d)__builtin_ia32_vfmaddsubpd512_mask((__v8df)(__m512d)(A), \
                                              (__v8df)(__m512d)(B), \
                                              -(__v8df)(__m512d)(C), \
                                              (__mmask8)-1, (int)(R)); })


#define _mm512_mask_fmsubadd_round_pd(A, U, B, C, R) __extension__ ({ \
  (__m512d)__builtin_ia32_vfmaddsubpd512_mask((__v8df)(__m512d)(A), \
                                              (__v8df)(__m512d)(B), \
                                              -(__v8df)(__m512d)(C), \
                                              (__mmask8)(U), (int)(R)); })


#define _mm512_maskz_fmsubadd_round_pd(U, A, B, C, R) __extension__ ({ \
  (__m512d)__builtin_ia32_vfmaddsubpd512_maskz((__v8df)(__m512d)(A), \
                                               (__v8df)(__m512d)(B), \
                                               -(__v8df)(__m512d)(C), \
                                               (__mmask8)(U), (int)(R)); })


static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_fmaddsub_pd(__m512d __A, __m512d __B, __m512d __C)
{
  return (__m512d) __builtin_ia32_vfmaddsubpd512_mask ((__v8df) __A,
                                                       (__v8df) __B,
                                                       (__v8df) __C,
                                                       (__mmask8) -1,
                                                       _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_mask_fmaddsub_pd(__m512d __A, __mmask8 __U, __m512d __B, __m512d __C)
{
  return (__m512d) __builtin_ia32_vfmaddsubpd512_mask ((__v8df) __A,
                                                       (__v8df) __B,
                                                       (__v8df) __C,
                                                       (__mmask8) __U,
                                                       _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_mask3_fmaddsub_pd(__m512d __A, __m512d __B, __m512d __C, __mmask8 __U)
{
  return (__m512d) __builtin_ia32_vfmaddsubpd512_mask3 ((__v8df) __A,
                                                        (__v8df) __B,
                                                        (__v8df) __C,
                                                        (__mmask8) __U,
                                                        _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_maskz_fmaddsub_pd(__mmask8 __U, __m512d __A, __m512d __B, __m512d __C)
{
  return (__m512d) __builtin_ia32_vfmaddsubpd512_maskz ((__v8df) __A,
                                                        (__v8df) __B,
                                                        (__v8df) __C,
                                                        (__mmask8) __U,
                                                        _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_fmsubadd_pd(__m512d __A, __m512d __B, __m512d __C)
{
  return (__m512d) __builtin_ia32_vfmaddsubpd512_mask ((__v8df) __A,
                                                       (__v8df) __B,
                                                       -(__v8df) __C,
                                                       (__mmask8) -1,
                                                       _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_mask_fmsubadd_pd(__m512d __A, __mmask8 __U, __m512d __B, __m512d __C)
{
  return (__m512d) __builtin_ia32_vfmaddsubpd512_mask ((__v8df) __A,
                                                       (__v8df) __B,
                                                       -(__v8df) __C,
                                                       (__mmask8) __U,
                                                       _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_maskz_fmsubadd_pd(__mmask8 __U, __m512d __A, __m512d __B, __m512d __C)
{
  return (__m512d) __builtin_ia32_vfmaddsubpd512_maskz ((__v8df) __A,
                                                        (__v8df) __B,
                                                        -(__v8df) __C,
                                                        (__mmask8) __U,
                                                        _MM_FROUND_CUR_DIRECTION);
}

#define _mm512_fmaddsub_round_ps(A, B, C, R) __extension__ ({ \
  (__m512)__builtin_ia32_vfmaddsubps512_mask((__v16sf)(__m512)(A), \
                                             (__v16sf)(__m512)(B), \
                                             (__v16sf)(__m512)(C), \
                                             (__mmask16)-1, (int)(R)); })


#define _mm512_mask_fmaddsub_round_ps(A, U, B, C, R) __extension__ ({ \
  (__m512)__builtin_ia32_vfmaddsubps512_mask((__v16sf)(__m512)(A), \
                                             (__v16sf)(__m512)(B), \
                                             (__v16sf)(__m512)(C), \
                                             (__mmask16)(U), (int)(R)); })


#define _mm512_mask3_fmaddsub_round_ps(A, B, C, U, R) __extension__ ({ \
  (__m512)__builtin_ia32_vfmaddsubps512_mask3((__v16sf)(__m512)(A), \
                                              (__v16sf)(__m512)(B), \
                                              (__v16sf)(__m512)(C), \
                                              (__mmask16)(U), (int)(R)); })


#define _mm512_maskz_fmaddsub_round_ps(U, A, B, C, R) __extension__ ({ \
  (__m512)__builtin_ia32_vfmaddsubps512_maskz((__v16sf)(__m512)(A), \
                                              (__v16sf)(__m512)(B), \
                                              (__v16sf)(__m512)(C), \
                                              (__mmask16)(U), (int)(R)); })


#define _mm512_fmsubadd_round_ps(A, B, C, R) __extension__ ({ \
  (__m512)__builtin_ia32_vfmaddsubps512_mask((__v16sf)(__m512)(A), \
                                             (__v16sf)(__m512)(B), \
                                             -(__v16sf)(__m512)(C), \
                                             (__mmask16)-1, (int)(R)); })


#define _mm512_mask_fmsubadd_round_ps(A, U, B, C, R) __extension__ ({ \
  (__m512)__builtin_ia32_vfmaddsubps512_mask((__v16sf)(__m512)(A), \
                                             (__v16sf)(__m512)(B), \
                                             -(__v16sf)(__m512)(C), \
                                             (__mmask16)(U), (int)(R)); })


#define _mm512_maskz_fmsubadd_round_ps(U, A, B, C, R) __extension__ ({ \
  (__m512)__builtin_ia32_vfmaddsubps512_maskz((__v16sf)(__m512)(A), \
                                              (__v16sf)(__m512)(B), \
                                              -(__v16sf)(__m512)(C), \
                                              (__mmask16)(U), (int)(R)); })


static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_fmaddsub_ps(__m512 __A, __m512 __B, __m512 __C)
{
  return (__m512) __builtin_ia32_vfmaddsubps512_mask ((__v16sf) __A,
                                                      (__v16sf) __B,
                                                      (__v16sf) __C,
                                                      (__mmask16) -1,
                                                      _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_mask_fmaddsub_ps(__m512 __A, __mmask16 __U, __m512 __B, __m512 __C)
{
  return (__m512) __builtin_ia32_vfmaddsubps512_mask ((__v16sf) __A,
                                                      (__v16sf) __B,
                                                      (__v16sf) __C,
                                                      (__mmask16) __U,
                                                      _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_mask3_fmaddsub_ps(__m512 __A, __m512 __B, __m512 __C, __mmask16 __U)
{
  return (__m512) __builtin_ia32_vfmaddsubps512_mask3 ((__v16sf) __A,
                                                       (__v16sf) __B,
                                                       (__v16sf) __C,
                                                       (__mmask16) __U,
                                                       _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_maskz_fmaddsub_ps(__mmask16 __U, __m512 __A, __m512 __B, __m512 __C)
{
  return (__m512) __builtin_ia32_vfmaddsubps512_maskz ((__v16sf) __A,
                                                       (__v16sf) __B,
                                                       (__v16sf) __C,
                                                       (__mmask16) __U,
                                                       _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_fmsubadd_ps(__m512 __A, __m512 __B, __m512 __C)
{
  return (__m512) __builtin_ia32_vfmaddsubps512_mask ((__v16sf) __A,
                                                      (__v16sf) __B,
                                                      -(__v16sf) __C,
                                                      (__mmask16) -1,
                                                      _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_mask_fmsubadd_ps(__m512 __A, __mmask16 __U, __m512 __B, __m512 __C)
{
  return (__m512) __builtin_ia32_vfmaddsubps512_mask ((__v16sf) __A,
                                                      (__v16sf) __B,
                                                      -(__v16sf) __C,
                                                      (__mmask16) __U,
                                                      _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_maskz_fmsubadd_ps(__mmask16 __U, __m512 __A, __m512 __B, __m512 __C)
{
  return (__m512) __builtin_ia32_vfmaddsubps512_maskz ((__v16sf) __A,
                                                       (__v16sf) __B,
                                                       -(__v16sf) __C,
                                                       (__mmask16) __U,
                                                       _MM_FROUND_CUR_DIRECTION);
}

#define _mm512_mask3_fmsub_round_pd(A, B, C, U, R) __extension__ ({ \
  (__m512d)__builtin_ia32_vfmsubpd512_mask3((__v8df)(__m512d)(A), \
                                            (__v8df)(__m512d)(B), \
                                            (__v8df)(__m512d)(C), \
                                            (__mmask8)(U), (int)(R)); })


static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_mask3_fmsub_pd(__m512d __A, __m512d __B, __m512d __C, __mmask8 __U)
{
  return (__m512d) __builtin_ia32_vfmsubpd512_mask3 ((__v8df) __A,
                                                     (__v8df) __B,
                                                     (__v8df) __C,
                                                     (__mmask8) __U,
                                                     _MM_FROUND_CUR_DIRECTION);
}

#define _mm512_mask3_fmsub_round_ps(A, B, C, U, R) __extension__ ({ \
  (__m512)__builtin_ia32_vfmsubps512_mask3((__v16sf)(__m512)(A), \
                                           (__v16sf)(__m512)(B), \
                                           (__v16sf)(__m512)(C), \
                                           (__mmask16)(U), (int)(R)); })


static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_mask3_fmsub_ps(__m512 __A, __m512 __B, __m512 __C, __mmask16 __U)
{
  return (__m512) __builtin_ia32_vfmsubps512_mask3 ((__v16sf) __A,
                                                    (__v16sf) __B,
                                                    (__v16sf) __C,
                                                    (__mmask16) __U,
                                                    _MM_FROUND_CUR_DIRECTION);
}

#define _mm512_mask3_fmsubadd_round_pd(A, B, C, U, R) __extension__ ({ \
  (__m512d)__builtin_ia32_vfmsubaddpd512_mask3((__v8df)(__m512d)(A), \
                                               (__v8df)(__m512d)(B), \
                                               (__v8df)(__m512d)(C), \
                                               (__mmask8)(U), (int)(R)); })


static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_mask3_fmsubadd_pd(__m512d __A, __m512d __B, __m512d __C, __mmask8 __U)
{
  return (__m512d) __builtin_ia32_vfmsubaddpd512_mask3 ((__v8df) __A,
                                                        (__v8df) __B,
                                                        (__v8df) __C,
                                                        (__mmask8) __U,
                                                        _MM_FROUND_CUR_DIRECTION);
}

#define _mm512_mask3_fmsubadd_round_ps(A, B, C, U, R) __extension__ ({ \
  (__m512)__builtin_ia32_vfmsubaddps512_mask3((__v16sf)(__m512)(A), \
                                              (__v16sf)(__m512)(B), \
                                              (__v16sf)(__m512)(C), \
                                              (__mmask16)(U), (int)(R)); })


static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_mask3_fmsubadd_ps(__m512 __A, __m512 __B, __m512 __C, __mmask16 __U)
{
  return (__m512) __builtin_ia32_vfmsubaddps512_mask3 ((__v16sf) __A,
                                                       (__v16sf) __B,
                                                       (__v16sf) __C,
                                                       (__mmask16) __U,
                                                       _MM_FROUND_CUR_DIRECTION);
}

#define _mm512_mask_fnmadd_round_pd(A, U, B, C, R) __extension__ ({ \
  (__m512d)__builtin_ia32_vfnmaddpd512_mask((__v8df)(__m512d)(A), \
                                            (__v8df)(__m512d)(B), \
                                            (__v8df)(__m512d)(C), \
                                            (__mmask8)(U), (int)(R)); })


static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_mask_fnmadd_pd(__m512d __A, __mmask8 __U, __m512d __B, __m512d __C)
{
  return (__m512d) __builtin_ia32_vfnmaddpd512_mask ((__v8df) __A,
                                                     (__v8df) __B,
                                                     (__v8df) __C,
                                                     (__mmask8) __U,
                                                     _MM_FROUND_CUR_DIRECTION);
}

#define _mm512_mask_fnmadd_round_ps(A, U, B, C, R) __extension__ ({ \
  (__m512)__builtin_ia32_vfnmaddps512_mask((__v16sf)(__m512)(A), \
                                           (__v16sf)(__m512)(B), \
                                           (__v16sf)(__m512)(C), \
                                           (__mmask16)(U), (int)(R)); })


static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_mask_fnmadd_ps(__m512 __A, __mmask16 __U, __m512 __B, __m512 __C)
{
  return (__m512) __builtin_ia32_vfnmaddps512_mask ((__v16sf) __A,
                                                    (__v16sf) __B,
                                                    (__v16sf) __C,
                                                    (__mmask16) __U,
                                                    _MM_FROUND_CUR_DIRECTION);
}

#define _mm512_mask_fnmsub_round_pd(A, U, B, C, R) __extension__ ({ \
  (__m512d)__builtin_ia32_vfnmsubpd512_mask((__v8df)(__m512d)(A), \
                                            (__v8df)(__m512d)(B), \
                                            (__v8df)(__m512d)(C), \
                                            (__mmask8)(U), (int)(R)); })


#define _mm512_mask3_fnmsub_round_pd(A, B, C, U, R) __extension__ ({ \
  (__m512d)__builtin_ia32_vfnmsubpd512_mask3((__v8df)(__m512d)(A), \
                                             (__v8df)(__m512d)(B), \
                                             (__v8df)(__m512d)(C), \
                                             (__mmask8)(U), (int)(R)); })


static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_mask_fnmsub_pd(__m512d __A, __mmask8 __U, __m512d __B, __m512d __C)
{
  return (__m512d) __builtin_ia32_vfnmsubpd512_mask ((__v8df) __A,
                                                     (__v8df) __B,
                                                     (__v8df) __C,
                                                     (__mmask8) __U,
                                                     _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_mask3_fnmsub_pd(__m512d __A, __m512d __B, __m512d __C, __mmask8 __U)
{
  return (__m512d) __builtin_ia32_vfnmsubpd512_mask3 ((__v8df) __A,
                                                      (__v8df) __B,
                                                      (__v8df) __C,
                                                      (__mmask8) __U,
                                                      _MM_FROUND_CUR_DIRECTION);
}

#define _mm512_mask_fnmsub_round_ps(A, U, B, C, R) __extension__ ({ \
  (__m512)__builtin_ia32_vfnmsubps512_mask((__v16sf)(__m512)(A), \
                                           (__v16sf)(__m512)(B), \
                                           (__v16sf)(__m512)(C), \
                                           (__mmask16)(U), (int)(R)); })


#define _mm512_mask3_fnmsub_round_ps(A, B, C, U, R) __extension__ ({ \
  (__m512)__builtin_ia32_vfnmsubps512_mask3((__v16sf)(__m512)(A), \
                                            (__v16sf)(__m512)(B), \
                                            (__v16sf)(__m512)(C), \
                                            (__mmask16)(U), (int)(R)); })


static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_mask_fnmsub_ps(__m512 __A, __mmask16 __U, __m512 __B, __m512 __C)
{
  return (__m512) __builtin_ia32_vfnmsubps512_mask ((__v16sf) __A,
                                                    (__v16sf) __B,
                                                    (__v16sf) __C,
                                                    (__mmask16) __U,
                                                    _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_mask3_fnmsub_ps(__m512 __A, __m512 __B, __m512 __C, __mmask16 __U)
{
  return (__m512) __builtin_ia32_vfnmsubps512_mask3 ((__v16sf) __A,
                                                     (__v16sf) __B,
                                                     (__v16sf) __C,
                                                     (__mmask16) __U,
                                                     _MM_FROUND_CUR_DIRECTION);
}



/* Vector permutations */

static __inline __m512i __DEFAULT_FN_ATTRS
_mm512_permutex2var_epi32(__m512i __A, __m512i __I, __m512i __B)
{
  return (__m512i) __builtin_ia32_vpermt2vard512_mask ((__v16si) __I
                                                       /* idx */ ,
                                                       (__v16si) __A,
                                                       (__v16si) __B,
                                                       (__mmask16) -1);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_permutex2var_epi32 (__m512i __A, __mmask16 __U,
                                __m512i __I, __m512i __B)
{
  return (__m512i) __builtin_ia32_vpermt2vard512_mask ((__v16si) __I
                                                        /* idx */ ,
                                                        (__v16si) __A,
                                                        (__v16si) __B,
                                                        (__mmask16) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_permutex2var_epi32 (__mmask16 __U, __m512i __A,
                                 __m512i __I, __m512i __B)
{
  return (__m512i) __builtin_ia32_vpermt2vard512_maskz ((__v16si) __I
                                                        /* idx */ ,
                                                        (__v16si) __A,
                                                        (__v16si) __B,
                                                        (__mmask16) __U);
}

static __inline __m512i __DEFAULT_FN_ATTRS
_mm512_permutex2var_epi64(__m512i __A, __m512i __I, __m512i __B)
{
  return (__m512i) __builtin_ia32_vpermt2varq512_mask ((__v8di) __I
                                                       /* idx */ ,
                                                       (__v8di) __A,
                                                       (__v8di) __B,
                                                       (__mmask8) -1);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_permutex2var_epi64 (__m512i __A, __mmask8 __U, __m512i __I,
                                __m512i __B)
{
  return (__m512i) __builtin_ia32_vpermt2varq512_mask ((__v8di) __I
                                                       /* idx */ ,
                                                       (__v8di) __A,
                                                       (__v8di) __B,
                                                       (__mmask8) __U);
}


static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_permutex2var_epi64 (__mmask8 __U, __m512i __A,
         __m512i __I, __m512i __B)
{
  return (__m512i) __builtin_ia32_vpermt2varq512_maskz ((__v8di) __I
                                                        /* idx */ ,
                                                        (__v8di) __A,
                                                        (__v8di) __B,
                                                        (__mmask8) __U);
}

#define _mm512_alignr_epi64(A, B, I) __extension__ ({ \
  (__m512i)__builtin_ia32_alignq512_mask((__v8di)(__m512i)(A), \
                                         (__v8di)(__m512i)(B), (int)(I), \
                                         (__v8di)_mm512_setzero_si512(), \
                                         (__mmask8)-1); })

#define _mm512_mask_alignr_epi64(W, U, A, B, imm) __extension__({\
  (__m512i)__builtin_ia32_alignq512_mask((__v8di)(__m512i)(A), \
                                         (__v8di)(__m512i)(B), (int)(imm), \
                                         (__v8di)(__m512i)(W), \
                                         (__mmask8)(U)); })

#define _mm512_maskz_alignr_epi64(U, A, B, imm) __extension__({\
  (__m512i)__builtin_ia32_alignq512_mask((__v8di)(__m512i)(A), \
                                         (__v8di)(__m512i)(B), (int)(imm), \
                                         (__v8di)_mm512_setzero_si512(), \
                                         (__mmask8)(U)); })

#define _mm512_alignr_epi32(A, B, I) __extension__ ({ \
  (__m512i)__builtin_ia32_alignd512_mask((__v16si)(__m512i)(A), \
                                         (__v16si)(__m512i)(B), (int)(I), \
                                         (__v16si)_mm512_setzero_si512(), \
                                         (__mmask16)-1); })

#define _mm512_mask_alignr_epi32(W, U, A, B, imm) __extension__ ({\
  (__m512i)__builtin_ia32_alignd512_mask((__v16si)(__m512i)(A), \
                                         (__v16si)(__m512i)(B), (int)(imm), \
                                         (__v16si)(__m512i)(W), \
                                         (__mmask16)(U)); })

#define _mm512_maskz_alignr_epi32(U, A, B, imm) __extension__({\
  (__m512i)__builtin_ia32_alignd512_mask((__v16si)(__m512i)(A), \
                                         (__v16si)(__m512i)(B), (int)(imm), \
                                         (__v16si)_mm512_setzero_si512(), \
                                         (__mmask16)(U)); })
/* Vector Extract */

#define _mm512_extractf64x4_pd(A, I) __extension__ ({                    \
  (__m256d)__builtin_ia32_extractf64x4_mask((__v8df)(__m512d)(A), (int)(I), \
                                            (__v4df)_mm256_setzero_si256(), \
                                            (__mmask8)-1); })

#define _mm512_mask_extractf64x4_pd(W, U, A, imm) __extension__ ({\
  (__m256d)__builtin_ia32_extractf64x4_mask((__v8df)(__m512d)(A), (int)(imm), \
                                            (__v4df)(__m256d)(W), \
                                            (__mmask8)(U)); })

#define _mm512_maskz_extractf64x4_pd(U, A, imm) __extension__ ({\
  (__m256d)__builtin_ia32_extractf64x4_mask((__v8df)(__m512d)(A), (int)(imm), \
                                            (__v4df)_mm256_setzero_pd(), \
                                            (__mmask8)(U)); })

#define _mm512_extractf32x4_ps(A, I) __extension__ ({                    \
  (__m128)__builtin_ia32_extractf32x4_mask((__v16sf)(__m512)(A), (int)(I), \
                                           (__v4sf)_mm_setzero_ps(), \
                                           (__mmask8)-1); })

#define _mm512_mask_extractf32x4_ps(W, U, A, imm) __extension__ ({\
  (__m128)__builtin_ia32_extractf32x4_mask((__v16sf)(__m512)(A), (int)(imm), \
                                           (__v4sf)(__m128)(W), \
                                           (__mmask8)(U)); })

#define _mm512_maskz_extractf32x4_ps(U, A, imm) __extension__ ({\
  (__m128)__builtin_ia32_extractf32x4_mask((__v16sf)(__m512)(A), (int)(imm), \
                                           (__v4sf)_mm_setzero_ps(), \
                                           (__mmask8)(U)); })
/* Vector Blend */

static __inline __m512d __DEFAULT_FN_ATTRS
_mm512_mask_blend_pd(__mmask8 __U, __m512d __A, __m512d __W)
{
  return (__m512d) __builtin_ia32_selectpd_512 ((__mmask8) __U,
                 (__v8df) __W,
                 (__v8df) __A);
}

static __inline __m512 __DEFAULT_FN_ATTRS
_mm512_mask_blend_ps(__mmask16 __U, __m512 __A, __m512 __W)
{
  return (__m512) __builtin_ia32_selectps_512 ((__mmask16) __U,
                (__v16sf) __W,
                (__v16sf) __A);
}

static __inline __m512i __DEFAULT_FN_ATTRS
_mm512_mask_blend_epi64(__mmask8 __U, __m512i __A, __m512i __W)
{
  return (__m512i) __builtin_ia32_selectq_512 ((__mmask8) __U,
                (__v8di) __W,
                (__v8di) __A);
}

static __inline __m512i __DEFAULT_FN_ATTRS
_mm512_mask_blend_epi32(__mmask16 __U, __m512i __A, __m512i __W)
{
  return (__m512i) __builtin_ia32_selectd_512 ((__mmask16) __U,
                (__v16si) __W,
                (__v16si) __A);
}

/* Compare */

#define _mm512_cmp_round_ps_mask(A, B, P, R) __extension__ ({ \
  (__mmask16)__builtin_ia32_cmpps512_mask((__v16sf)(__m512)(A), \
                                          (__v16sf)(__m512)(B), (int)(P), \
                                          (__mmask16)-1, (int)(R)); })

#define _mm512_mask_cmp_round_ps_mask(U, A, B, P, R) __extension__ ({ \
  (__mmask16)__builtin_ia32_cmpps512_mask((__v16sf)(__m512)(A), \
                                          (__v16sf)(__m512)(B), (int)(P), \
                                          (__mmask16)(U), (int)(R)); })

#define _mm512_cmp_ps_mask(A, B, P) \
  _mm512_cmp_round_ps_mask((A), (B), (P), _MM_FROUND_CUR_DIRECTION)

#define _mm512_mask_cmp_ps_mask(U, A, B, P) \
  _mm512_mask_cmp_round_ps_mask((U), (A), (B), (P), _MM_FROUND_CUR_DIRECTION)

#define _mm512_cmp_round_pd_mask(A, B, P, R) __extension__ ({ \
  (__mmask8)__builtin_ia32_cmppd512_mask((__v8df)(__m512d)(A), \
                                         (__v8df)(__m512d)(B), (int)(P), \
                                         (__mmask8)-1, (int)(R)); })

#define _mm512_mask_cmp_round_pd_mask(U, A, B, P, R) __extension__ ({ \
  (__mmask8)__builtin_ia32_cmppd512_mask((__v8df)(__m512d)(A), \
                                         (__v8df)(__m512d)(B), (int)(P), \
                                         (__mmask8)(U), (int)(R)); })

#define _mm512_cmp_pd_mask(A, B, P) \
  _mm512_cmp_round_pd_mask((A), (B), (P), _MM_FROUND_CUR_DIRECTION)

#define _mm512_mask_cmp_pd_mask(U, A, B, P) \
  _mm512_mask_cmp_round_pd_mask((U), (A), (B), (P), _MM_FROUND_CUR_DIRECTION)

/* Conversion */

#define _mm512_cvtt_roundps_epu32(A, R) __extension__ ({ \
  (__m512i)__builtin_ia32_cvttps2udq512_mask((__v16sf)(__m512)(A), \
                                             (__v16si)_mm512_undefined_epi32(), \
                                             (__mmask16)-1, (int)(R)); })

#define _mm512_mask_cvtt_roundps_epu32(W, U, A, R) __extension__ ({ \
  (__m512i)__builtin_ia32_cvttps2udq512_mask((__v16sf)(__m512)(A), \
                                             (__v16si)(__m512i)(W), \
                                             (__mmask16)(U), (int)(R)); })

#define _mm512_maskz_cvtt_roundps_epu32(U, A, R) __extension__ ({ \
  (__m512i)__builtin_ia32_cvttps2udq512_mask((__v16sf)(__m512)(A), \
                                             (__v16si)_mm512_setzero_si512(), \
                                             (__mmask16)(U), (int)(R)); })


static __inline __m512i __DEFAULT_FN_ATTRS
_mm512_cvttps_epu32(__m512 __A)
{
  return (__m512i) __builtin_ia32_cvttps2udq512_mask ((__v16sf) __A,
                  (__v16si)
                  _mm512_setzero_si512 (),
                  (__mmask16) -1,
                  _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_cvttps_epu32 (__m512i __W, __mmask16 __U, __m512 __A)
{
  return (__m512i) __builtin_ia32_cvttps2udq512_mask ((__v16sf) __A,
                   (__v16si) __W,
                   (__mmask16) __U,
                   _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_cvttps_epu32 (__mmask16 __U, __m512 __A)
{
  return (__m512i) __builtin_ia32_cvttps2udq512_mask ((__v16sf) __A,
                   (__v16si) _mm512_setzero_si512 (),
                   (__mmask16) __U,
                   _MM_FROUND_CUR_DIRECTION);
}

#define _mm512_cvt_roundepi32_ps(A, R) __extension__ ({ \
  (__m512)__builtin_ia32_cvtdq2ps512_mask((__v16si)(__m512i)(A), \
                                          (__v16sf)_mm512_setzero_ps(), \
                                          (__mmask16)-1, (int)(R)); })

#define _mm512_mask_cvt_roundepi32_ps(W, U, A, R) __extension__ ({ \
  (__m512)__builtin_ia32_cvtdq2ps512_mask((__v16si)(__m512i)(A), \
                                          (__v16sf)(__m512)(W), \
                                          (__mmask16)(U), (int)(R)); })

#define _mm512_maskz_cvt_roundepi32_ps(U, A, R) __extension__ ({ \
  (__m512)__builtin_ia32_cvtdq2ps512_mask((__v16si)(__m512i)(A), \
                                          (__v16sf)_mm512_setzero_ps(), \
                                          (__mmask16)(U), (int)(R)); })

#define _mm512_cvt_roundepu32_ps(A, R) __extension__ ({ \
  (__m512)__builtin_ia32_cvtudq2ps512_mask((__v16si)(__m512i)(A), \
                                           (__v16sf)_mm512_setzero_ps(), \
                                           (__mmask16)-1, (int)(R)); })

#define _mm512_mask_cvt_roundepu32_ps(W, U, A, R) __extension__ ({ \
  (__m512)__builtin_ia32_cvtudq2ps512_mask((__v16si)(__m512i)(A), \
                                           (__v16sf)(__m512)(W), \
                                           (__mmask16)(U), (int)(R)); })

#define _mm512_maskz_cvt_roundepu32_ps(U, A, R) __extension__ ({ \
  (__m512)__builtin_ia32_cvtudq2ps512_mask((__v16si)(__m512i)(A), \
                                           (__v16sf)_mm512_setzero_ps(), \
                                           (__mmask16)(U), (int)(R)); })

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_cvtepu32_ps (__m512i __A)
{
  return (__m512) __builtin_ia32_cvtudq2ps512_mask ((__v16si) __A,
                 (__v16sf) _mm512_undefined_ps (),
                 (__mmask16) -1,
                 _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_mask_cvtepu32_ps (__m512 __W, __mmask16 __U, __m512i __A)
{
  return (__m512) __builtin_ia32_cvtudq2ps512_mask ((__v16si) __A,
                 (__v16sf) __W,
                 (__mmask16) __U,
                 _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_maskz_cvtepu32_ps (__mmask16 __U, __m512i __A)
{
  return (__m512) __builtin_ia32_cvtudq2ps512_mask ((__v16si) __A,
                 (__v16sf) _mm512_setzero_ps (),
                 (__mmask16) __U,
                 _MM_FROUND_CUR_DIRECTION);
}

static __inline __m512d __DEFAULT_FN_ATTRS
_mm512_cvtepi32_pd(__m256i __A)
{
  return (__m512d) __builtin_ia32_cvtdq2pd512_mask ((__v8si) __A,
                (__v8df)
                _mm512_setzero_pd (),
                (__mmask8) -1);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_mask_cvtepi32_pd (__m512d __W, __mmask8 __U, __m256i __A)
{
  return (__m512d) __builtin_ia32_cvtdq2pd512_mask ((__v8si) __A,
                (__v8df) __W,
                (__mmask8) __U);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_maskz_cvtepi32_pd (__mmask8 __U, __m256i __A)
{
  return (__m512d) __builtin_ia32_cvtdq2pd512_mask ((__v8si) __A,
                (__v8df) _mm512_setzero_pd (),
                (__mmask8) __U);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_cvtepi32_ps (__m512i __A)
{
  return (__m512) __builtin_ia32_cvtdq2ps512_mask ((__v16si) __A,
                (__v16sf) _mm512_undefined_ps (),
                (__mmask16) -1,
                _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_mask_cvtepi32_ps (__m512 __W, __mmask16 __U, __m512i __A)
{
  return (__m512) __builtin_ia32_cvtdq2ps512_mask ((__v16si) __A,
                (__v16sf) __W,
                (__mmask16) __U,
                _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_maskz_cvtepi32_ps (__mmask16 __U, __m512i __A)
{
  return (__m512) __builtin_ia32_cvtdq2ps512_mask ((__v16si) __A,
                (__v16sf) _mm512_setzero_ps (),
                (__mmask16) __U,
                _MM_FROUND_CUR_DIRECTION);
}

static __inline __m512d __DEFAULT_FN_ATTRS
_mm512_cvtepu32_pd(__m256i __A)
{
  return (__m512d) __builtin_ia32_cvtudq2pd512_mask ((__v8si) __A,
                (__v8df)
                _mm512_setzero_pd (),
                (__mmask8) -1);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_mask_cvtepu32_pd (__m512d __W, __mmask8 __U, __m256i __A)
{
  return (__m512d) __builtin_ia32_cvtudq2pd512_mask ((__v8si) __A,
                  (__v8df) __W,
                  (__mmask8) __U);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_maskz_cvtepu32_pd (__mmask8 __U, __m256i __A)
{
  return (__m512d) __builtin_ia32_cvtudq2pd512_mask ((__v8si) __A,
                  (__v8df) _mm512_setzero_pd (),
                  (__mmask8) __U);
}

#define _mm512_cvt_roundpd_ps(A, R) __extension__ ({ \
  (__m256)__builtin_ia32_cvtpd2ps512_mask((__v8df)(__m512d)(A), \
                                          (__v8sf)_mm256_setzero_ps(), \
                                          (__mmask8)-1, (int)(R)); })

#define _mm512_mask_cvt_roundpd_ps(W, U, A, R) __extension__ ({ \
  (__m256)__builtin_ia32_cvtpd2ps512_mask((__v8df)(__m512d)(A), \
                                          (__v8sf)(__m256)(W), (__mmask8)(U), \
                                          (int)(R)); })

#define _mm512_maskz_cvt_roundpd_ps(U, A, R) __extension__ ({ \
  (__m256)__builtin_ia32_cvtpd2ps512_mask((__v8df)(__m512d)(A), \
                                          (__v8sf)_mm256_setzero_ps(), \
                                          (__mmask8)(U), (int)(R)); })

static __inline__ __m256 __DEFAULT_FN_ATTRS
_mm512_cvtpd_ps (__m512d __A)
{
  return (__m256) __builtin_ia32_cvtpd2ps512_mask ((__v8df) __A,
                (__v8sf) _mm256_undefined_ps (),
                (__mmask8) -1,
                _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m256 __DEFAULT_FN_ATTRS
_mm512_mask_cvtpd_ps (__m256 __W, __mmask8 __U, __m512d __A)
{
  return (__m256) __builtin_ia32_cvtpd2ps512_mask ((__v8df) __A,
                (__v8sf) __W,
                (__mmask8) __U,
                _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m256 __DEFAULT_FN_ATTRS
_mm512_maskz_cvtpd_ps (__mmask8 __U, __m512d __A)
{
  return (__m256) __builtin_ia32_cvtpd2ps512_mask ((__v8df) __A,
                (__v8sf) _mm256_setzero_ps (),
                (__mmask8) __U,
                _MM_FROUND_CUR_DIRECTION);
}

#define _mm512_cvt_roundps_ph(A, I) __extension__ ({ \
  (__m256i)__builtin_ia32_vcvtps2ph512_mask((__v16sf)(__m512)(A), (int)(I), \
                                            (__v16hi)_mm256_undefined_si256(), \
                                            (__mmask16)-1); })

#define _mm512_mask_cvt_roundps_ph(U, W, A, I) __extension__ ({ \
  (__m256i)__builtin_ia32_vcvtps2ph512_mask((__v16sf)(__m512)(A), (int)(I), \
                                            (__v16hi)(__m256i)(U), \
                                            (__mmask16)(W)); })

#define _mm512_maskz_cvt_roundps_ph(W, A, I) __extension__ ({ \
  (__m256i)__builtin_ia32_vcvtps2ph512_mask((__v16sf)(__m512)(A), (int)(I), \
                                            (__v16hi)_mm256_setzero_si256(), \
                                            (__mmask16)(W)); })

#define _mm512_cvtps_ph(A, I) __extension__ ({ \
  (__m256i)__builtin_ia32_vcvtps2ph512_mask((__v16sf)(__m512)(A), (int)(I), \
                                            (__v16hi)_mm256_setzero_si256(), \
                                            (__mmask16)-1); })

#define _mm512_mask_cvtps_ph(U, W, A, I) __extension__ ({ \
  (__m256i)__builtin_ia32_vcvtps2ph512_mask((__v16sf)(__m512)(A), (int)(I), \
                                            (__v16hi)(__m256i)(U), \
                                            (__mmask16)(W)); })

#define _mm512_maskz_cvtps_ph(W, A, I) __extension__ ({\
  (__m256i)__builtin_ia32_vcvtps2ph512_mask((__v16sf)(__m512)(A), (int)(I), \
                                            (__v16hi)_mm256_setzero_si256(), \
                                            (__mmask16)(W)); })

#define _mm512_cvt_roundph_ps(A, R) __extension__ ({ \
  (__m512)__builtin_ia32_vcvtph2ps512_mask((__v16hi)(__m256i)(A), \
                                           (__v16sf)_mm512_undefined_ps(), \
                                           (__mmask16)-1, (int)(R)); })

#define _mm512_mask_cvt_roundph_ps(W, U, A, R) __extension__ ({ \
  (__m512)__builtin_ia32_vcvtph2ps512_mask((__v16hi)(__m256i)(A), \
                                           (__v16sf)(__m512)(W), \
                                           (__mmask16)(U), (int)(R)); })

#define _mm512_maskz_cvt_roundph_ps(U, A, R) __extension__ ({ \
  (__m512)__builtin_ia32_vcvtph2ps512_mask((__v16hi)(__m256i)(A), \
                                           (__v16sf)_mm512_setzero_ps(), \
                                           (__mmask16)(U), (int)(R)); })


static  __inline __m512 __DEFAULT_FN_ATTRS
_mm512_cvtph_ps(__m256i __A)
{
  return (__m512) __builtin_ia32_vcvtph2ps512_mask ((__v16hi) __A,
                (__v16sf)
                _mm512_setzero_ps (),
                (__mmask16) -1,
                _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_mask_cvtph_ps (__m512 __W, __mmask16 __U, __m256i __A)
{
  return (__m512) __builtin_ia32_vcvtph2ps512_mask ((__v16hi) __A,
                 (__v16sf) __W,
                 (__mmask16) __U,
                 _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_maskz_cvtph_ps (__mmask16 __U, __m256i __A)
{
  return (__m512) __builtin_ia32_vcvtph2ps512_mask ((__v16hi) __A,
                 (__v16sf) _mm512_setzero_ps (),
                 (__mmask16) __U,
                 _MM_FROUND_CUR_DIRECTION);
}

#define _mm512_cvtt_roundpd_epi32(A, R) __extension__ ({ \
  (__m256i)__builtin_ia32_cvttpd2dq512_mask((__v8df)(__m512d)(A), \
                                            (__v8si)_mm256_setzero_si256(), \
                                            (__mmask8)-1, (int)(R)); })

#define _mm512_mask_cvtt_roundpd_epi32(W, U, A, R) __extension__ ({ \
  (__m256i)__builtin_ia32_cvttpd2dq512_mask((__v8df)(__m512d)(A), \
                                            (__v8si)(__m256i)(W), \
                                            (__mmask8)(U), (int)(R)); })

#define _mm512_maskz_cvtt_roundpd_epi32(U, A, R) __extension__ ({ \
  (__m256i)__builtin_ia32_cvttpd2dq512_mask((__v8df)(__m512d)(A), \
                                            (__v8si)_mm256_setzero_si256(), \
                                            (__mmask8)(U), (int)(R)); })

static __inline __m256i __DEFAULT_FN_ATTRS
_mm512_cvttpd_epi32(__m512d __a)
{
  return (__m256i)__builtin_ia32_cvttpd2dq512_mask((__v8df) __a,
                                                   (__v8si)_mm256_setzero_si256(),
                                                   (__mmask8) -1,
                                                    _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS
_mm512_mask_cvttpd_epi32 (__m256i __W, __mmask8 __U, __m512d __A)
{
  return (__m256i) __builtin_ia32_cvttpd2dq512_mask ((__v8df) __A,
                  (__v8si) __W,
                  (__mmask8) __U,
                  _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS
_mm512_maskz_cvttpd_epi32 (__mmask8 __U, __m512d __A)
{
  return (__m256i) __builtin_ia32_cvttpd2dq512_mask ((__v8df) __A,
                  (__v8si) _mm256_setzero_si256 (),
                  (__mmask8) __U,
                  _MM_FROUND_CUR_DIRECTION);
}

#define _mm512_cvtt_roundps_epi32(A, R) __extension__ ({ \
  (__m512i)__builtin_ia32_cvttps2dq512_mask((__v16sf)(__m512)(A), \
                                            (__v16si)_mm512_setzero_si512(), \
                                            (__mmask16)-1, (int)(R)); })

#define _mm512_mask_cvtt_roundps_epi32(W, U, A, R) __extension__ ({ \
  (__m512i)__builtin_ia32_cvttps2dq512_mask((__v16sf)(__m512)(A), \
                                            (__v16si)(__m512i)(W), \
                                            (__mmask16)(U), (int)(R)); })

#define _mm512_maskz_cvtt_roundps_epi32(U, A, R) __extension__ ({ \
  (__m512i)__builtin_ia32_cvttps2dq512_mask((__v16sf)(__m512)(A), \
                                            (__v16si)_mm512_setzero_si512(), \
                                            (__mmask16)(U), (int)(R)); })

static __inline __m512i __DEFAULT_FN_ATTRS
_mm512_cvttps_epi32(__m512 __a)
{
  return (__m512i)
    __builtin_ia32_cvttps2dq512_mask((__v16sf) __a,
                                     (__v16si) _mm512_setzero_si512 (),
                                     (__mmask16) -1, _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_cvttps_epi32 (__m512i __W, __mmask16 __U, __m512 __A)
{
  return (__m512i) __builtin_ia32_cvttps2dq512_mask ((__v16sf) __A,
                  (__v16si) __W,
                  (__mmask16) __U,
                  _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_cvttps_epi32 (__mmask16 __U, __m512 __A)
{
  return (__m512i) __builtin_ia32_cvttps2dq512_mask ((__v16sf) __A,
                  (__v16si) _mm512_setzero_si512 (),
                  (__mmask16) __U,
                  _MM_FROUND_CUR_DIRECTION);
}

#define _mm512_cvt_roundps_epi32(A, R) __extension__ ({ \
  (__m512i)__builtin_ia32_cvtps2dq512_mask((__v16sf)(__m512)(A), \
                                           (__v16si)_mm512_setzero_si512(), \
                                           (__mmask16)-1, (int)(R)); })

#define _mm512_mask_cvt_roundps_epi32(W, U, A, R) __extension__ ({ \
  (__m512i)__builtin_ia32_cvtps2dq512_mask((__v16sf)(__m512)(A), \
                                           (__v16si)(__m512i)(W), \
                                           (__mmask16)(U), (int)(R)); })

#define _mm512_maskz_cvt_roundps_epi32(U, A, R) __extension__ ({ \
  (__m512i)__builtin_ia32_cvtps2dq512_mask((__v16sf)(__m512)(A), \
                                           (__v16si)_mm512_setzero_si512(), \
                                           (__mmask16)(U), (int)(R)); })

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_cvtps_epi32 (__m512 __A)
{
  return (__m512i) __builtin_ia32_cvtps2dq512_mask ((__v16sf) __A,
                 (__v16si) _mm512_undefined_epi32 (),
                 (__mmask16) -1,
                 _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_cvtps_epi32 (__m512i __W, __mmask16 __U, __m512 __A)
{
  return (__m512i) __builtin_ia32_cvtps2dq512_mask ((__v16sf) __A,
                 (__v16si) __W,
                 (__mmask16) __U,
                 _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_cvtps_epi32 (__mmask16 __U, __m512 __A)
{
  return (__m512i) __builtin_ia32_cvtps2dq512_mask ((__v16sf) __A,
                 (__v16si)
                 _mm512_setzero_si512 (),
                 (__mmask16) __U,
                 _MM_FROUND_CUR_DIRECTION);
}

#define _mm512_cvt_roundpd_epi32(A, R) __extension__ ({ \
  (__m256i)__builtin_ia32_cvtpd2dq512_mask((__v8df)(__m512d)(A), \
                                           (__v8si)_mm256_setzero_si256(), \
                                           (__mmask8)-1, (int)(R)); })

#define _mm512_mask_cvt_roundpd_epi32(W, U, A, R) __extension__ ({ \
  (__m256i)__builtin_ia32_cvtpd2dq512_mask((__v8df)(__m512d)(A), \
                                           (__v8si)(__m256i)(W), \
                                           (__mmask8)(U), (int)(R)); })

#define _mm512_maskz_cvt_roundpd_epi32(U, A, R) __extension__ ({ \
  (__m256i)__builtin_ia32_cvtpd2dq512_mask((__v8df)(__m512d)(A), \
                                           (__v8si)_mm256_setzero_si256(), \
                                           (__mmask8)(U), (int)(R)); })

static __inline__ __m256i __DEFAULT_FN_ATTRS
_mm512_cvtpd_epi32 (__m512d __A)
{
  return (__m256i) __builtin_ia32_cvtpd2dq512_mask ((__v8df) __A,
                 (__v8si)
                 _mm256_undefined_si256 (),
                 (__mmask8) -1,
                 _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS
_mm512_mask_cvtpd_epi32 (__m256i __W, __mmask8 __U, __m512d __A)
{
  return (__m256i) __builtin_ia32_cvtpd2dq512_mask ((__v8df) __A,
                 (__v8si) __W,
                 (__mmask8) __U,
                 _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS
_mm512_maskz_cvtpd_epi32 (__mmask8 __U, __m512d __A)
{
  return (__m256i) __builtin_ia32_cvtpd2dq512_mask ((__v8df) __A,
                 (__v8si)
                 _mm256_setzero_si256 (),
                 (__mmask8) __U,
                 _MM_FROUND_CUR_DIRECTION);
}

#define _mm512_cvt_roundps_epu32(A, R) __extension__ ({ \
  (__m512i)__builtin_ia32_cvtps2udq512_mask((__v16sf)(__m512)(A), \
                                            (__v16si)_mm512_setzero_si512(), \
                                            (__mmask16)-1, (int)(R)); })

#define _mm512_mask_cvt_roundps_epu32(W, U, A, R) __extension__ ({ \
  (__m512i)__builtin_ia32_cvtps2udq512_mask((__v16sf)(__m512)(A), \
                                            (__v16si)(__m512i)(W), \
                                            (__mmask16)(U), (int)(R)); })

#define _mm512_maskz_cvt_roundps_epu32(U, A, R) __extension__ ({ \
  (__m512i)__builtin_ia32_cvtps2udq512_mask((__v16sf)(__m512)(A), \
                                            (__v16si)_mm512_setzero_si512(), \
                                            (__mmask16)(U), (int)(R)); })

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_cvtps_epu32 ( __m512 __A)
{
  return (__m512i) __builtin_ia32_cvtps2udq512_mask ((__v16sf) __A,\
                  (__v16si)\
                  _mm512_undefined_epi32 (),\
                  (__mmask16) -1,\
                  _MM_FROUND_CUR_DIRECTION);\
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_cvtps_epu32 (__m512i __W, __mmask16 __U, __m512 __A)
{
  return (__m512i) __builtin_ia32_cvtps2udq512_mask ((__v16sf) __A,
                  (__v16si) __W,
                  (__mmask16) __U,
                  _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_cvtps_epu32 ( __mmask16 __U, __m512 __A)
{
  return (__m512i) __builtin_ia32_cvtps2udq512_mask ((__v16sf) __A,
                  (__v16si) 
                  _mm512_setzero_si512 (),
                  (__mmask16) __U ,
                  _MM_FROUND_CUR_DIRECTION);
}

#define _mm512_cvt_roundpd_epu32(A, R) __extension__ ({ \
  (__m256i)__builtin_ia32_cvtpd2udq512_mask((__v8df)(__m512d)(A), \
                                            (__v8si)_mm256_setzero_si256(), \
                                            (__mmask8)-1, (int)(R)); })

#define _mm512_mask_cvt_roundpd_epu32(W, U, A, R) __extension__ ({ \
  (__m256i)__builtin_ia32_cvtpd2udq512_mask((__v8df)(__m512d)(A), \
                                            (__v8si)(W), \
                                            (__mmask8)(U), (int)(R)); })

#define _mm512_maskz_cvt_roundpd_epu32(U, A, R) __extension__ ({ \
  (__m256i)__builtin_ia32_cvtpd2udq512_mask((__v8df)(__m512d)(A), \
                                            (__v8si)_mm256_setzero_si256(), \
                                            (__mmask8)(U), (int)(R)); })

static __inline__ __m256i __DEFAULT_FN_ATTRS
_mm512_cvtpd_epu32 (__m512d __A)
{
  return (__m256i) __builtin_ia32_cvtpd2udq512_mask ((__v8df) __A,
                  (__v8si)
                  _mm256_undefined_si256 (),
                  (__mmask8) -1,
                  _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS
_mm512_mask_cvtpd_epu32 (__m256i __W, __mmask8 __U, __m512d __A)
{
  return (__m256i) __builtin_ia32_cvtpd2udq512_mask ((__v8df) __A,
                  (__v8si) __W,
                  (__mmask8) __U,
                  _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS
_mm512_maskz_cvtpd_epu32 (__mmask8 __U, __m512d __A)
{
  return (__m256i) __builtin_ia32_cvtpd2udq512_mask ((__v8df) __A,
                  (__v8si)
                  _mm256_setzero_si256 (),
                  (__mmask8) __U,
                  _MM_FROUND_CUR_DIRECTION);
}

/* Unpack and Interleave */

static __inline __m512d __DEFAULT_FN_ATTRS
_mm512_unpackhi_pd(__m512d __a, __m512d __b)
{
  return (__m512d)__builtin_shufflevector((__v8df)__a, (__v8df)__b,
                                          1, 9, 1+2, 9+2, 1+4, 9+4, 1+6, 9+6);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_mask_unpackhi_pd(__m512d __W, __mmask8 __U, __m512d __A, __m512d __B)
{
  return (__m512d)__builtin_ia32_selectpd_512((__mmask8) __U,
                                           (__v8df)_mm512_unpackhi_pd(__A, __B),
                                           (__v8df)__W);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_maskz_unpackhi_pd(__mmask8 __U, __m512d __A, __m512d __B)
{
  return (__m512d)__builtin_ia32_selectpd_512((__mmask8) __U,
                                           (__v8df)_mm512_unpackhi_pd(__A, __B),
                                           (__v8df)_mm512_setzero_pd());
}

static __inline __m512d __DEFAULT_FN_ATTRS
_mm512_unpacklo_pd(__m512d __a, __m512d __b)
{
  return (__m512d)__builtin_shufflevector((__v8df)__a, (__v8df)__b,
                                          0, 8, 0+2, 8+2, 0+4, 8+4, 0+6, 8+6);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_mask_unpacklo_pd(__m512d __W, __mmask8 __U, __m512d __A, __m512d __B)
{
  return (__m512d)__builtin_ia32_selectpd_512((__mmask8) __U,
                                           (__v8df)_mm512_unpacklo_pd(__A, __B),
                                           (__v8df)__W);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_maskz_unpacklo_pd (__mmask8 __U, __m512d __A, __m512d __B)
{
  return (__m512d)__builtin_ia32_selectpd_512((__mmask8) __U,
                                           (__v8df)_mm512_unpacklo_pd(__A, __B),
                                           (__v8df)_mm512_setzero_pd());
}

static __inline __m512 __DEFAULT_FN_ATTRS
_mm512_unpackhi_ps(__m512 __a, __m512 __b)
{
  return (__m512)__builtin_shufflevector((__v16sf)__a, (__v16sf)__b,
                                         2,    18,    3,    19,
                                         2+4,  18+4,  3+4,  19+4,
                                         2+8,  18+8,  3+8,  19+8,
                                         2+12, 18+12, 3+12, 19+12);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_mask_unpackhi_ps(__m512 __W, __mmask16 __U, __m512 __A, __m512 __B)
{
  return (__m512)__builtin_ia32_selectps_512((__mmask16) __U,
                                          (__v16sf)_mm512_unpackhi_ps(__A, __B),
                                          (__v16sf)__W);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_maskz_unpackhi_ps (__mmask16 __U, __m512 __A, __m512 __B)
{
  return (__m512)__builtin_ia32_selectps_512((__mmask16) __U,
                                          (__v16sf)_mm512_unpackhi_ps(__A, __B),
                                          (__v16sf)_mm512_setzero_ps());
}

static __inline __m512 __DEFAULT_FN_ATTRS
_mm512_unpacklo_ps(__m512 __a, __m512 __b)
{
  return (__m512)__builtin_shufflevector((__v16sf)__a, (__v16sf)__b,
                                         0,    16,    1,    17,
                                         0+4,  16+4,  1+4,  17+4,
                                         0+8,  16+8,  1+8,  17+8,
                                         0+12, 16+12, 1+12, 17+12);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_mask_unpacklo_ps(__m512 __W, __mmask16 __U, __m512 __A, __m512 __B)
{
  return (__m512)__builtin_ia32_selectps_512((__mmask16) __U,
                                          (__v16sf)_mm512_unpacklo_ps(__A, __B),
                                          (__v16sf)__W);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_maskz_unpacklo_ps (__mmask16 __U, __m512 __A, __m512 __B)
{
  return (__m512)__builtin_ia32_selectps_512((__mmask16) __U,
                                          (__v16sf)_mm512_unpacklo_ps(__A, __B),
                                          (__v16sf)_mm512_setzero_ps());
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_unpackhi_epi32(__m512i __A, __m512i __B)
{
  return (__m512i)__builtin_shufflevector((__v16si)__A, (__v16si)__B,
                                          2,    18,    3,    19,
                                          2+4,  18+4,  3+4,  19+4,
                                          2+8,  18+8,  3+8,  19+8,
                                          2+12, 18+12, 3+12, 19+12);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_unpackhi_epi32(__m512i __W, __mmask16 __U, __m512i __A, __m512i __B)
{
  return (__m512i)__builtin_ia32_selectd_512((__mmask16) __U,
                                       (__v16si)_mm512_unpackhi_epi32(__A, __B),
                                       (__v16si)__W);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_unpackhi_epi32(__mmask16 __U, __m512i __A, __m512i __B)
{
  return (__m512i)__builtin_ia32_selectd_512((__mmask16) __U,
                                       (__v16si)_mm512_unpackhi_epi32(__A, __B),
                                       (__v16si)_mm512_setzero_si512());
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_unpacklo_epi32(__m512i __A, __m512i __B)
{
  return (__m512i)__builtin_shufflevector((__v16si)__A, (__v16si)__B,
                                          0,    16,    1,    17,
                                          0+4,  16+4,  1+4,  17+4,
                                          0+8,  16+8,  1+8,  17+8,
                                          0+12, 16+12, 1+12, 17+12);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_unpacklo_epi32(__m512i __W, __mmask16 __U, __m512i __A, __m512i __B)
{
  return (__m512i)__builtin_ia32_selectd_512((__mmask16) __U,
                                       (__v16si)_mm512_unpacklo_epi32(__A, __B),
                                       (__v16si)__W);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_unpacklo_epi32(__mmask16 __U, __m512i __A, __m512i __B)
{
  return (__m512i)__builtin_ia32_selectd_512((__mmask16) __U,
                                       (__v16si)_mm512_unpacklo_epi32(__A, __B),
                                       (__v16si)_mm512_setzero_si512());
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_unpackhi_epi64(__m512i __A, __m512i __B)
{
  return (__m512i)__builtin_shufflevector((__v8di)__A, (__v8di)__B,
                                          1, 9, 1+2, 9+2, 1+4, 9+4, 1+6, 9+6);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_unpackhi_epi64(__m512i __W, __mmask8 __U, __m512i __A, __m512i __B)
{
  return (__m512i)__builtin_ia32_selectq_512((__mmask8) __U,
                                        (__v8di)_mm512_unpackhi_epi64(__A, __B),
                                        (__v8di)__W);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_unpackhi_epi64(__mmask8 __U, __m512i __A, __m512i __B)
{
  return (__m512i)__builtin_ia32_selectq_512((__mmask8) __U,
                                        (__v8di)_mm512_unpackhi_epi64(__A, __B),
                                        (__v8di)_mm512_setzero_si512());
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_unpacklo_epi64 (__m512i __A, __m512i __B)
{
  return (__m512i)__builtin_shufflevector((__v8di)__A, (__v8di)__B,
                                          0, 8, 0+2, 8+2, 0+4, 8+4, 0+6, 8+6);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_unpacklo_epi64 (__m512i __W, __mmask8 __U, __m512i __A, __m512i __B)
{
  return (__m512i)__builtin_ia32_selectq_512((__mmask8) __U,
                                        (__v8di)_mm512_unpacklo_epi64(__A, __B),
                                        (__v8di)__W);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_unpacklo_epi64 (__mmask8 __U, __m512i __A, __m512i __B)
{
  return (__m512i)__builtin_ia32_selectq_512((__mmask8) __U,
                                        (__v8di)_mm512_unpacklo_epi64(__A, __B),
                                        (__v8di)_mm512_setzero_si512());
}

/* Bit Test */

static __inline __mmask16 __DEFAULT_FN_ATTRS
_mm512_test_epi32_mask(__m512i __A, __m512i __B)
{
  return (__mmask16) __builtin_ia32_ptestmd512 ((__v16si) __A,
            (__v16si) __B,
            (__mmask16) -1);
}

static __inline__ __mmask16 __DEFAULT_FN_ATTRS
_mm512_mask_test_epi32_mask (__mmask16 __U, __m512i __A, __m512i __B)
{
  return (__mmask16) __builtin_ia32_ptestmd512 ((__v16si) __A,
                 (__v16si) __B, __U);
}

static __inline __mmask8 __DEFAULT_FN_ATTRS
_mm512_test_epi64_mask(__m512i __A, __m512i __B)
{
  return (__mmask8) __builtin_ia32_ptestmq512 ((__v8di) __A,
                 (__v8di) __B,
                 (__mmask8) -1);
}

static __inline__ __mmask8 __DEFAULT_FN_ATTRS
_mm512_mask_test_epi64_mask (__mmask8 __U, __m512i __A, __m512i __B)
{
  return (__mmask8) __builtin_ia32_ptestmq512 ((__v8di) __A, (__v8di) __B, __U);
}


/* SIMD load ops */

static __inline __m512i __DEFAULT_FN_ATTRS
_mm512_loadu_si512 (void const *__P)
{
  return (__m512i) __builtin_ia32_loaddqusi512_mask ((const int *) __P,
                  (__v16si)
                  _mm512_setzero_si512 (),
                  (__mmask16) -1);
}

static __inline __m512i __DEFAULT_FN_ATTRS
_mm512_mask_loadu_epi32 (__m512i __W, __mmask16 __U, void const *__P)
{
  return (__m512i) __builtin_ia32_loaddqusi512_mask ((const int *) __P,
                  (__v16si) __W,
                  (__mmask16) __U);
}


static __inline __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_loadu_epi32(__mmask16 __U, void const *__P)
{
  return (__m512i) __builtin_ia32_loaddqusi512_mask ((const int *)__P,
                                                     (__v16si)
                                                     _mm512_setzero_si512 (),
                                                     (__mmask16) __U);
}

static __inline __m512i __DEFAULT_FN_ATTRS
_mm512_mask_loadu_epi64 (__m512i __W, __mmask8 __U, void const *__P)
{
  return (__m512i) __builtin_ia32_loaddqudi512_mask ((const long long *) __P,
                  (__v8di) __W,
                  (__mmask8) __U);
}

static __inline __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_loadu_epi64(__mmask8 __U, void const *__P)
{
  return (__m512i) __builtin_ia32_loaddqudi512_mask ((const long long *)__P,
                                                     (__v8di)
                                                     _mm512_setzero_si512 (),
                                                     (__mmask8) __U);
}

static __inline __m512 __DEFAULT_FN_ATTRS
_mm512_mask_loadu_ps (__m512 __W, __mmask16 __U, void const *__P)
{
  return (__m512) __builtin_ia32_loadups512_mask ((const float *) __P,
                   (__v16sf) __W,
                   (__mmask16) __U);
}

static __inline __m512 __DEFAULT_FN_ATTRS
_mm512_maskz_loadu_ps(__mmask16 __U, void const *__P)
{
  return (__m512) __builtin_ia32_loadups512_mask ((const float *)__P,
                                                  (__v16sf)
                                                  _mm512_setzero_ps (),
                                                  (__mmask16) __U);
}

static __inline __m512d __DEFAULT_FN_ATTRS
_mm512_mask_loadu_pd (__m512d __W, __mmask8 __U, void const *__P)
{
  return (__m512d) __builtin_ia32_loadupd512_mask ((const double *) __P,
                (__v8df) __W,
                (__mmask8) __U);
}

static __inline __m512d __DEFAULT_FN_ATTRS
_mm512_maskz_loadu_pd(__mmask8 __U, void const *__P)
{
  return (__m512d) __builtin_ia32_loadupd512_mask ((const double *)__P,
                                                   (__v8df)
                                                   _mm512_setzero_pd (),
                                                   (__mmask8) __U);
}

static __inline __m512d __DEFAULT_FN_ATTRS
_mm512_loadu_pd(double const *__p)
{
  struct __loadu_pd {
    __m512d __v;
  } __attribute__((__packed__, __may_alias__));
  return ((struct __loadu_pd*)__p)->__v;
}

static __inline __m512 __DEFAULT_FN_ATTRS
_mm512_loadu_ps(float const *__p)
{
  struct __loadu_ps {
    __m512 __v;
  } __attribute__((__packed__, __may_alias__));
  return ((struct __loadu_ps*)__p)->__v;
}

static __inline __m512 __DEFAULT_FN_ATTRS
_mm512_load_ps(float const *__p)
{
  return (__m512) __builtin_ia32_loadaps512_mask ((const __v16sf *)__p,
                                                  (__v16sf)
                                                  _mm512_setzero_ps (),
                                                  (__mmask16) -1);
}

static __inline __m512 __DEFAULT_FN_ATTRS
_mm512_mask_load_ps (__m512 __W, __mmask16 __U, void const *__P)
{
  return (__m512) __builtin_ia32_loadaps512_mask ((const __v16sf *) __P,
                   (__v16sf) __W,
                   (__mmask16) __U);
}

static __inline __m512 __DEFAULT_FN_ATTRS
_mm512_maskz_load_ps(__mmask16 __U, void const *__P)
{
  return (__m512) __builtin_ia32_loadaps512_mask ((const __v16sf *)__P,
                                                  (__v16sf)
                                                  _mm512_setzero_ps (),
                                                  (__mmask16) __U);
}

static __inline __m512d __DEFAULT_FN_ATTRS
_mm512_load_pd(double const *__p)
{
  return (__m512d) __builtin_ia32_loadapd512_mask ((const __v8df *)__p,
                                                   (__v8df)
                                                   _mm512_setzero_pd (),
                                                   (__mmask8) -1);
}

static __inline __m512d __DEFAULT_FN_ATTRS
_mm512_mask_load_pd (__m512d __W, __mmask8 __U, void const *__P)
{
  return (__m512d) __builtin_ia32_loadapd512_mask ((const __v8df *) __P,
                          (__v8df) __W,
                          (__mmask8) __U);
}

static __inline __m512d __DEFAULT_FN_ATTRS
_mm512_maskz_load_pd(__mmask8 __U, void const *__P)
{
  return (__m512d) __builtin_ia32_loadapd512_mask ((const __v8df *)__P,
                                                   (__v8df)
                                                   _mm512_setzero_pd (),
                                                   (__mmask8) __U);
}

static __inline __m512i __DEFAULT_FN_ATTRS
_mm512_load_si512 (void const *__P)
{
  return *(__m512i *) __P;
}

static __inline __m512i __DEFAULT_FN_ATTRS
_mm512_load_epi32 (void const *__P)
{
  return *(__m512i *) __P;
}

static __inline __m512i __DEFAULT_FN_ATTRS
_mm512_load_epi64 (void const *__P)
{
  return *(__m512i *) __P;
}

/* SIMD store ops */

static __inline void __DEFAULT_FN_ATTRS
_mm512_mask_storeu_epi64(void *__P, __mmask8 __U, __m512i __A)
{
  __builtin_ia32_storedqudi512_mask ((long long *)__P, (__v8di) __A,
                                     (__mmask8) __U);
}

static __inline void __DEFAULT_FN_ATTRS
_mm512_storeu_si512 (void *__P, __m512i __A)
{
  __builtin_ia32_storedqusi512_mask ((int *) __P, (__v16si) __A,
            (__mmask16) -1);
}

static __inline void __DEFAULT_FN_ATTRS
_mm512_mask_storeu_epi32(void *__P, __mmask16 __U, __m512i __A)
{
  __builtin_ia32_storedqusi512_mask ((int *)__P, (__v16si) __A,
                                     (__mmask16) __U);
}

static __inline void __DEFAULT_FN_ATTRS
_mm512_mask_storeu_pd(void *__P, __mmask8 __U, __m512d __A)
{
  __builtin_ia32_storeupd512_mask ((double *)__P, (__v8df) __A, (__mmask8) __U);
}

static __inline void __DEFAULT_FN_ATTRS
_mm512_storeu_pd(void *__P, __m512d __A)
{
  __builtin_ia32_storeupd512_mask((double *)__P, (__v8df)__A, (__mmask8)-1);
}

static __inline void __DEFAULT_FN_ATTRS
_mm512_mask_storeu_ps(void *__P, __mmask16 __U, __m512 __A)
{
  __builtin_ia32_storeups512_mask ((float *)__P, (__v16sf) __A,
                                   (__mmask16) __U);
}

static __inline void __DEFAULT_FN_ATTRS
_mm512_storeu_ps(void *__P, __m512 __A)
{
  __builtin_ia32_storeups512_mask((float *)__P, (__v16sf)__A, (__mmask16)-1);
}

static __inline void __DEFAULT_FN_ATTRS
_mm512_mask_store_pd(void *__P, __mmask8 __U, __m512d __A)
{
  __builtin_ia32_storeapd512_mask ((__v8df *)__P, (__v8df) __A, (__mmask8) __U);
}

static __inline void __DEFAULT_FN_ATTRS
_mm512_store_pd(void *__P, __m512d __A)
{
  *(__m512d*)__P = __A;
}

static __inline void __DEFAULT_FN_ATTRS
_mm512_mask_store_ps(void *__P, __mmask16 __U, __m512 __A)
{
  __builtin_ia32_storeaps512_mask ((__v16sf *)__P, (__v16sf) __A,
                                   (__mmask16) __U);
}

static __inline void __DEFAULT_FN_ATTRS
_mm512_store_ps(void *__P, __m512 __A)
{
  *(__m512*)__P = __A;
}

static __inline void __DEFAULT_FN_ATTRS
_mm512_store_si512 (void *__P, __m512i __A)
{
  *(__m512i *) __P = __A;
}

static __inline void __DEFAULT_FN_ATTRS
_mm512_store_epi32 (void *__P, __m512i __A)
{
  *(__m512i *) __P = __A;
}

static __inline void __DEFAULT_FN_ATTRS
_mm512_store_epi64 (void *__P, __m512i __A)
{
  *(__m512i *) __P = __A;
}

/* Mask ops */

static __inline __mmask16 __DEFAULT_FN_ATTRS
_mm512_knot(__mmask16 __M)
{
  return __builtin_ia32_knothi(__M);
}

/* Integer compare */

static __inline__ __mmask16 __DEFAULT_FN_ATTRS
_mm512_cmpeq_epi32_mask(__m512i __a, __m512i __b) {
  return (__mmask16)__builtin_ia32_pcmpeqd512_mask((__v16si)__a, (__v16si)__b,
                                                   (__mmask16)-1);
}

static __inline__ __mmask16 __DEFAULT_FN_ATTRS
_mm512_mask_cmpeq_epi32_mask(__mmask16 __u, __m512i __a, __m512i __b) {
  return (__mmask16)__builtin_ia32_pcmpeqd512_mask((__v16si)__a, (__v16si)__b,
                                                   __u);
}

static __inline__ __mmask16 __DEFAULT_FN_ATTRS
_mm512_cmpeq_epu32_mask(__m512i __a, __m512i __b) {
  return (__mmask16)__builtin_ia32_ucmpd512_mask((__v16si)__a, (__v16si)__b, 0,
                                                 (__mmask16)-1);
}

static __inline__ __mmask16 __DEFAULT_FN_ATTRS
_mm512_mask_cmpeq_epu32_mask(__mmask16 __u, __m512i __a, __m512i __b) {
  return (__mmask16)__builtin_ia32_ucmpd512_mask((__v16si)__a, (__v16si)__b, 0,
                                                 __u);
}

static __inline__ __mmask8 __DEFAULT_FN_ATTRS
_mm512_mask_cmpeq_epi64_mask(__mmask8 __u, __m512i __a, __m512i __b) {
  return (__mmask8)__builtin_ia32_pcmpeqq512_mask((__v8di)__a, (__v8di)__b,
                                                  __u);
}

static __inline__ __mmask8 __DEFAULT_FN_ATTRS
_mm512_cmpeq_epi64_mask(__m512i __a, __m512i __b) {
  return (__mmask8)__builtin_ia32_pcmpeqq512_mask((__v8di)__a, (__v8di)__b,
                                                  (__mmask8)-1);
}

static __inline__ __mmask8 __DEFAULT_FN_ATTRS
_mm512_cmpeq_epu64_mask(__m512i __a, __m512i __b) {
  return (__mmask8)__builtin_ia32_ucmpq512_mask((__v8di)__a, (__v8di)__b, 0,
                                                (__mmask8)-1);
}

static __inline__ __mmask8 __DEFAULT_FN_ATTRS
_mm512_mask_cmpeq_epu64_mask(__mmask8 __u, __m512i __a, __m512i __b) {
  return (__mmask8)__builtin_ia32_ucmpq512_mask((__v8di)__a, (__v8di)__b, 0,
                                                __u);
}

static __inline__ __mmask16 __DEFAULT_FN_ATTRS
_mm512_cmpge_epi32_mask(__m512i __a, __m512i __b) {
  return (__mmask16)__builtin_ia32_cmpd512_mask((__v16si)__a, (__v16si)__b, 5,
                                                (__mmask16)-1);
}

static __inline__ __mmask16 __DEFAULT_FN_ATTRS
_mm512_mask_cmpge_epi32_mask(__mmask16 __u, __m512i __a, __m512i __b) {
  return (__mmask16)__builtin_ia32_cmpd512_mask((__v16si)__a, (__v16si)__b, 5,
                                                __u);
}

static __inline__ __mmask16 __DEFAULT_FN_ATTRS
_mm512_cmpge_epu32_mask(__m512i __a, __m512i __b) {
  return (__mmask16)__builtin_ia32_ucmpd512_mask((__v16si)__a, (__v16si)__b, 5,
                                                 (__mmask16)-1);
}

static __inline__ __mmask16 __DEFAULT_FN_ATTRS
_mm512_mask_cmpge_epu32_mask(__mmask16 __u, __m512i __a, __m512i __b) {
  return (__mmask16)__builtin_ia32_ucmpd512_mask((__v16si)__a, (__v16si)__b, 5,
                                                 __u);
}

static __inline__ __mmask8 __DEFAULT_FN_ATTRS
_mm512_cmpge_epi64_mask(__m512i __a, __m512i __b) {
  return (__mmask8)__builtin_ia32_cmpq512_mask((__v8di)__a, (__v8di)__b, 5,
                                               (__mmask8)-1);
}

static __inline__ __mmask8 __DEFAULT_FN_ATTRS
_mm512_mask_cmpge_epi64_mask(__mmask8 __u, __m512i __a, __m512i __b) {
  return (__mmask8)__builtin_ia32_cmpq512_mask((__v8di)__a, (__v8di)__b, 5,
                                               __u);
}

static __inline__ __mmask8 __DEFAULT_FN_ATTRS
_mm512_cmpge_epu64_mask(__m512i __a, __m512i __b) {
  return (__mmask8)__builtin_ia32_ucmpq512_mask((__v8di)__a, (__v8di)__b, 5,
                                                (__mmask8)-1);
}

static __inline__ __mmask8 __DEFAULT_FN_ATTRS
_mm512_mask_cmpge_epu64_mask(__mmask8 __u, __m512i __a, __m512i __b) {
  return (__mmask8)__builtin_ia32_ucmpq512_mask((__v8di)__a, (__v8di)__b, 5,
                                                __u);
}

static __inline__ __mmask16 __DEFAULT_FN_ATTRS
_mm512_cmpgt_epi32_mask(__m512i __a, __m512i __b) {
  return (__mmask16)__builtin_ia32_pcmpgtd512_mask((__v16si)__a, (__v16si)__b,
                                                   (__mmask16)-1);
}

static __inline__ __mmask16 __DEFAULT_FN_ATTRS
_mm512_mask_cmpgt_epi32_mask(__mmask16 __u, __m512i __a, __m512i __b) {
  return (__mmask16)__builtin_ia32_pcmpgtd512_mask((__v16si)__a, (__v16si)__b,
                                                   __u);
}

static __inline__ __mmask16 __DEFAULT_FN_ATTRS
_mm512_cmpgt_epu32_mask(__m512i __a, __m512i __b) {
  return (__mmask16)__builtin_ia32_ucmpd512_mask((__v16si)__a, (__v16si)__b, 6,
                                                 (__mmask16)-1);
}

static __inline__ __mmask16 __DEFAULT_FN_ATTRS
_mm512_mask_cmpgt_epu32_mask(__mmask16 __u, __m512i __a, __m512i __b) {
  return (__mmask16)__builtin_ia32_ucmpd512_mask((__v16si)__a, (__v16si)__b, 6,
                                                 __u);
}

static __inline__ __mmask8 __DEFAULT_FN_ATTRS
_mm512_mask_cmpgt_epi64_mask(__mmask8 __u, __m512i __a, __m512i __b) {
  return (__mmask8)__builtin_ia32_pcmpgtq512_mask((__v8di)__a, (__v8di)__b,
                                                  __u);
}

static __inline__ __mmask8 __DEFAULT_FN_ATTRS
_mm512_cmpgt_epi64_mask(__m512i __a, __m512i __b) {
  return (__mmask8)__builtin_ia32_pcmpgtq512_mask((__v8di)__a, (__v8di)__b,
                                                  (__mmask8)-1);
}

static __inline__ __mmask8 __DEFAULT_FN_ATTRS
_mm512_cmpgt_epu64_mask(__m512i __a, __m512i __b) {
  return (__mmask8)__builtin_ia32_ucmpq512_mask((__v8di)__a, (__v8di)__b, 6,
                                                (__mmask8)-1);
}

static __inline__ __mmask8 __DEFAULT_FN_ATTRS
_mm512_mask_cmpgt_epu64_mask(__mmask8 __u, __m512i __a, __m512i __b) {
  return (__mmask8)__builtin_ia32_ucmpq512_mask((__v8di)__a, (__v8di)__b, 6,
                                                __u);
}

static __inline__ __mmask16 __DEFAULT_FN_ATTRS
_mm512_cmple_epi32_mask(__m512i __a, __m512i __b) {
  return (__mmask16)__builtin_ia32_cmpd512_mask((__v16si)__a, (__v16si)__b, 2,
                                                (__mmask16)-1);
}

static __inline__ __mmask16 __DEFAULT_FN_ATTRS
_mm512_mask_cmple_epi32_mask(__mmask16 __u, __m512i __a, __m512i __b) {
  return (__mmask16)__builtin_ia32_cmpd512_mask((__v16si)__a, (__v16si)__b, 2,
                                                __u);
}

static __inline__ __mmask16 __DEFAULT_FN_ATTRS
_mm512_cmple_epu32_mask(__m512i __a, __m512i __b) {
  return (__mmask16)__builtin_ia32_ucmpd512_mask((__v16si)__a, (__v16si)__b, 2,
                                                 (__mmask16)-1);
}

static __inline__ __mmask16 __DEFAULT_FN_ATTRS
_mm512_mask_cmple_epu32_mask(__mmask16 __u, __m512i __a, __m512i __b) {
  return (__mmask16)__builtin_ia32_ucmpd512_mask((__v16si)__a, (__v16si)__b, 2,
                                                 __u);
}

static __inline__ __mmask8 __DEFAULT_FN_ATTRS
_mm512_cmple_epi64_mask(__m512i __a, __m512i __b) {
  return (__mmask8)__builtin_ia32_cmpq512_mask((__v8di)__a, (__v8di)__b, 2,
                                               (__mmask8)-1);
}

static __inline__ __mmask8 __DEFAULT_FN_ATTRS
_mm512_mask_cmple_epi64_mask(__mmask8 __u, __m512i __a, __m512i __b) {
  return (__mmask8)__builtin_ia32_cmpq512_mask((__v8di)__a, (__v8di)__b, 2,
                                               __u);
}

static __inline__ __mmask8 __DEFAULT_FN_ATTRS
_mm512_cmple_epu64_mask(__m512i __a, __m512i __b) {
  return (__mmask8)__builtin_ia32_ucmpq512_mask((__v8di)__a, (__v8di)__b, 2,
                                                (__mmask8)-1);
}

static __inline__ __mmask8 __DEFAULT_FN_ATTRS
_mm512_mask_cmple_epu64_mask(__mmask8 __u, __m512i __a, __m512i __b) {
  return (__mmask8)__builtin_ia32_ucmpq512_mask((__v8di)__a, (__v8di)__b, 2,
                                                __u);
}

static __inline__ __mmask16 __DEFAULT_FN_ATTRS
_mm512_cmplt_epi32_mask(__m512i __a, __m512i __b) {
  return (__mmask16)__builtin_ia32_cmpd512_mask((__v16si)__a, (__v16si)__b, 1,
                                                (__mmask16)-1);
}

static __inline__ __mmask16 __DEFAULT_FN_ATTRS
_mm512_mask_cmplt_epi32_mask(__mmask16 __u, __m512i __a, __m512i __b) {
  return (__mmask16)__builtin_ia32_cmpd512_mask((__v16si)__a, (__v16si)__b, 1,
                                                __u);
}

static __inline__ __mmask16 __DEFAULT_FN_ATTRS
_mm512_cmplt_epu32_mask(__m512i __a, __m512i __b) {
  return (__mmask16)__builtin_ia32_ucmpd512_mask((__v16si)__a, (__v16si)__b, 1,
                                                 (__mmask16)-1);
}

static __inline__ __mmask16 __DEFAULT_FN_ATTRS
_mm512_mask_cmplt_epu32_mask(__mmask16 __u, __m512i __a, __m512i __b) {
  return (__mmask16)__builtin_ia32_ucmpd512_mask((__v16si)__a, (__v16si)__b, 1,
                                                 __u);
}

static __inline__ __mmask8 __DEFAULT_FN_ATTRS
_mm512_cmplt_epi64_mask(__m512i __a, __m512i __b) {
  return (__mmask8)__builtin_ia32_cmpq512_mask((__v8di)__a, (__v8di)__b, 1,
                                               (__mmask8)-1);
}

static __inline__ __mmask8 __DEFAULT_FN_ATTRS
_mm512_mask_cmplt_epi64_mask(__mmask8 __u, __m512i __a, __m512i __b) {
  return (__mmask8)__builtin_ia32_cmpq512_mask((__v8di)__a, (__v8di)__b, 1,
                                               __u);
}

static __inline__ __mmask8 __DEFAULT_FN_ATTRS
_mm512_cmplt_epu64_mask(__m512i __a, __m512i __b) {
  return (__mmask8)__builtin_ia32_ucmpq512_mask((__v8di)__a, (__v8di)__b, 1,
                                                (__mmask8)-1);
}

static __inline__ __mmask8 __DEFAULT_FN_ATTRS
_mm512_mask_cmplt_epu64_mask(__mmask8 __u, __m512i __a, __m512i __b) {
  return (__mmask8)__builtin_ia32_ucmpq512_mask((__v8di)__a, (__v8di)__b, 1,
                                                __u);
}

static __inline__ __mmask16 __DEFAULT_FN_ATTRS
_mm512_cmpneq_epi32_mask(__m512i __a, __m512i __b) {
  return (__mmask16)__builtin_ia32_cmpd512_mask((__v16si)__a, (__v16si)__b, 4,
                                                (__mmask16)-1);
}

static __inline__ __mmask16 __DEFAULT_FN_ATTRS
_mm512_mask_cmpneq_epi32_mask(__mmask16 __u, __m512i __a, __m512i __b) {
  return (__mmask16)__builtin_ia32_cmpd512_mask((__v16si)__a, (__v16si)__b, 4,
                                                __u);
}

static __inline__ __mmask16 __DEFAULT_FN_ATTRS
_mm512_cmpneq_epu32_mask(__m512i __a, __m512i __b) {
  return (__mmask16)__builtin_ia32_ucmpd512_mask((__v16si)__a, (__v16si)__b, 4,
                                                 (__mmask16)-1);
}

static __inline__ __mmask16 __DEFAULT_FN_ATTRS
_mm512_mask_cmpneq_epu32_mask(__mmask16 __u, __m512i __a, __m512i __b) {
  return (__mmask16)__builtin_ia32_ucmpd512_mask((__v16si)__a, (__v16si)__b, 4,
                                                 __u);
}

static __inline__ __mmask8 __DEFAULT_FN_ATTRS
_mm512_cmpneq_epi64_mask(__m512i __a, __m512i __b) {
  return (__mmask8)__builtin_ia32_cmpq512_mask((__v8di)__a, (__v8di)__b, 4,
                                               (__mmask8)-1);
}

static __inline__ __mmask8 __DEFAULT_FN_ATTRS
_mm512_mask_cmpneq_epi64_mask(__mmask8 __u, __m512i __a, __m512i __b) {
  return (__mmask8)__builtin_ia32_cmpq512_mask((__v8di)__a, (__v8di)__b, 4,
                                               __u);
}

static __inline__ __mmask8 __DEFAULT_FN_ATTRS
_mm512_cmpneq_epu64_mask(__m512i __a, __m512i __b) {
  return (__mmask8)__builtin_ia32_ucmpq512_mask((__v8di)__a, (__v8di)__b, 4,
                                                (__mmask8)-1);
}

static __inline__ __mmask8 __DEFAULT_FN_ATTRS
_mm512_mask_cmpneq_epu64_mask(__mmask8 __u, __m512i __a, __m512i __b) {
  return (__mmask8)__builtin_ia32_ucmpq512_mask((__v8di)__a, (__v8di)__b, 4,
                                                __u);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_cvtepi8_epi32 (__m128i __A)
{
  return (__m512i) __builtin_ia32_pmovsxbd512_mask ((__v16qi) __A,
                (__v16si)
                _mm512_setzero_si512 (),
                (__mmask16) -1);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_cvtepi8_epi32 (__m512i __W, __mmask16 __U, __m128i __A)
{
  return (__m512i) __builtin_ia32_pmovsxbd512_mask ((__v16qi) __A,
                (__v16si) __W,
                (__mmask16) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_cvtepi8_epi32 (__mmask16 __U, __m128i __A)
{
  return (__m512i) __builtin_ia32_pmovsxbd512_mask ((__v16qi) __A,
                (__v16si)
                _mm512_setzero_si512 (),
                (__mmask16) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_cvtepi8_epi64 (__m128i __A)
{
  return (__m512i) __builtin_ia32_pmovsxbq512_mask ((__v16qi) __A,
                (__v8di)
                _mm512_setzero_si512 (),
                (__mmask8) -1);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_cvtepi8_epi64 (__m512i __W, __mmask8 __U, __m128i __A)
{
  return (__m512i) __builtin_ia32_pmovsxbq512_mask ((__v16qi) __A,
                (__v8di) __W,
                (__mmask8) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_cvtepi8_epi64 (__mmask8 __U, __m128i __A)
{
  return (__m512i) __builtin_ia32_pmovsxbq512_mask ((__v16qi) __A,
                (__v8di)
                _mm512_setzero_si512 (),
                (__mmask8) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_cvtepi32_epi64 (__m256i __X)
{
  return (__m512i) __builtin_ia32_pmovsxdq512_mask ((__v8si) __X,
                (__v8di)
                _mm512_setzero_si512 (),
                (__mmask8) -1);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_cvtepi32_epi64 (__m512i __W, __mmask8 __U, __m256i __X)
{
  return (__m512i) __builtin_ia32_pmovsxdq512_mask ((__v8si) __X,
                (__v8di) __W,
                (__mmask8) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_cvtepi32_epi64 (__mmask8 __U, __m256i __X)
{
  return (__m512i) __builtin_ia32_pmovsxdq512_mask ((__v8si) __X,
                (__v8di)
                _mm512_setzero_si512 (),
                (__mmask8) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_cvtepi16_epi32 (__m256i __A)
{
  return (__m512i) __builtin_ia32_pmovsxwd512_mask ((__v16hi) __A,
                (__v16si)
                _mm512_setzero_si512 (),
                (__mmask16) -1);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_cvtepi16_epi32 (__m512i __W, __mmask16 __U, __m256i __A)
{
  return (__m512i) __builtin_ia32_pmovsxwd512_mask ((__v16hi) __A,
                (__v16si) __W,
                (__mmask16) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_cvtepi16_epi32 (__mmask16 __U, __m256i __A)
{
  return (__m512i) __builtin_ia32_pmovsxwd512_mask ((__v16hi) __A,
                (__v16si)
                _mm512_setzero_si512 (),
                (__mmask16) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_cvtepi16_epi64 (__m128i __A)
{
  return (__m512i) __builtin_ia32_pmovsxwq512_mask ((__v8hi) __A,
                (__v8di)
                _mm512_setzero_si512 (),
                (__mmask8) -1);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_cvtepi16_epi64 (__m512i __W, __mmask8 __U, __m128i __A)
{
  return (__m512i) __builtin_ia32_pmovsxwq512_mask ((__v8hi) __A,
                (__v8di) __W,
                (__mmask8) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_cvtepi16_epi64 (__mmask8 __U, __m128i __A)
{
  return (__m512i) __builtin_ia32_pmovsxwq512_mask ((__v8hi) __A,
                (__v8di)
                _mm512_setzero_si512 (),
                (__mmask8) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_cvtepu8_epi32 (__m128i __A)
{
  return (__m512i) __builtin_ia32_pmovzxbd512_mask ((__v16qi) __A,
                (__v16si)
                _mm512_setzero_si512 (),
                (__mmask16) -1);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_cvtepu8_epi32 (__m512i __W, __mmask16 __U, __m128i __A)
{
  return (__m512i) __builtin_ia32_pmovzxbd512_mask ((__v16qi) __A,
                (__v16si) __W,
                (__mmask16) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_cvtepu8_epi32 (__mmask16 __U, __m128i __A)
{
  return (__m512i) __builtin_ia32_pmovzxbd512_mask ((__v16qi) __A,
                (__v16si)
                _mm512_setzero_si512 (),
                (__mmask16) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_cvtepu8_epi64 (__m128i __A)
{
  return (__m512i) __builtin_ia32_pmovzxbq512_mask ((__v16qi) __A,
                (__v8di)
                _mm512_setzero_si512 (),
                (__mmask8) -1);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_cvtepu8_epi64 (__m512i __W, __mmask8 __U, __m128i __A)
{
  return (__m512i) __builtin_ia32_pmovzxbq512_mask ((__v16qi) __A,
                (__v8di) __W,
                (__mmask8) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_cvtepu8_epi64 (__mmask8 __U, __m128i __A)
{
  return (__m512i) __builtin_ia32_pmovzxbq512_mask ((__v16qi) __A,
                (__v8di)
                _mm512_setzero_si512 (),
                (__mmask8) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_cvtepu32_epi64 (__m256i __X)
{
  return (__m512i) __builtin_ia32_pmovzxdq512_mask ((__v8si) __X,
                (__v8di)
                _mm512_setzero_si512 (),
                (__mmask8) -1);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_cvtepu32_epi64 (__m512i __W, __mmask8 __U, __m256i __X)
{
  return (__m512i) __builtin_ia32_pmovzxdq512_mask ((__v8si) __X,
                (__v8di) __W,
                (__mmask8) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_cvtepu32_epi64 (__mmask8 __U, __m256i __X)
{
  return (__m512i) __builtin_ia32_pmovzxdq512_mask ((__v8si) __X,
                (__v8di)
                _mm512_setzero_si512 (),
                (__mmask8) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_cvtepu16_epi32 (__m256i __A)
{
  return (__m512i) __builtin_ia32_pmovzxwd512_mask ((__v16hi) __A,
                (__v16si)
                _mm512_setzero_si512 (),
                (__mmask16) -1);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_cvtepu16_epi32 (__m512i __W, __mmask16 __U, __m256i __A)
{
  return (__m512i) __builtin_ia32_pmovzxwd512_mask ((__v16hi) __A,
                (__v16si) __W,
                (__mmask16) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_cvtepu16_epi32 (__mmask16 __U, __m256i __A)
{
  return (__m512i) __builtin_ia32_pmovzxwd512_mask ((__v16hi) __A,
                (__v16si)
                _mm512_setzero_si512 (),
                (__mmask16) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_cvtepu16_epi64 (__m128i __A)
{
  return (__m512i) __builtin_ia32_pmovzxwq512_mask ((__v8hi) __A,
                (__v8di)
                _mm512_setzero_si512 (),
                (__mmask8) -1);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_cvtepu16_epi64 (__m512i __W, __mmask8 __U, __m128i __A)
{
  return (__m512i) __builtin_ia32_pmovzxwq512_mask ((__v8hi) __A,
                (__v8di) __W,
                (__mmask8) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_cvtepu16_epi64 (__mmask8 __U, __m128i __A)
{
  return (__m512i) __builtin_ia32_pmovzxwq512_mask ((__v8hi) __A,
                (__v8di)
                _mm512_setzero_si512 (),
                (__mmask8) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_rorv_epi32 (__m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_prorvd512_mask ((__v16si) __A,
              (__v16si) __B,
              (__v16si)
              _mm512_setzero_si512 (),
              (__mmask16) -1);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_rorv_epi32 (__m512i __W, __mmask16 __U, __m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_prorvd512_mask ((__v16si) __A,
              (__v16si) __B,
              (__v16si) __W,
              (__mmask16) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_rorv_epi32 (__mmask16 __U, __m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_prorvd512_mask ((__v16si) __A,
              (__v16si) __B,
              (__v16si)
              _mm512_setzero_si512 (),
              (__mmask16) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_rorv_epi64 (__m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_prorvq512_mask ((__v8di) __A,
              (__v8di) __B,
              (__v8di)
              _mm512_setzero_si512 (),
              (__mmask8) -1);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_rorv_epi64 (__m512i __W, __mmask8 __U, __m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_prorvq512_mask ((__v8di) __A,
              (__v8di) __B,
              (__v8di) __W,
              (__mmask8) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_rorv_epi64 (__mmask8 __U, __m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_prorvq512_mask ((__v8di) __A,
              (__v8di) __B,
              (__v8di)
              _mm512_setzero_si512 (),
              (__mmask8) __U);
}



#define _mm512_cmp_epi32_mask(a, b, p) __extension__ ({ \
  (__mmask16)__builtin_ia32_cmpd512_mask((__v16si)(__m512i)(a), \
                                         (__v16si)(__m512i)(b), (int)(p), \
                                         (__mmask16)-1); })

#define _mm512_cmp_epu32_mask(a, b, p) __extension__ ({ \
  (__mmask16)__builtin_ia32_ucmpd512_mask((__v16si)(__m512i)(a), \
                                          (__v16si)(__m512i)(b), (int)(p), \
                                          (__mmask16)-1); })

#define _mm512_cmp_epi64_mask(a, b, p) __extension__ ({ \
  (__mmask8)__builtin_ia32_cmpq512_mask((__v8di)(__m512i)(a), \
                                        (__v8di)(__m512i)(b), (int)(p), \
                                        (__mmask8)-1); })

#define _mm512_cmp_epu64_mask(a, b, p) __extension__ ({ \
  (__mmask8)__builtin_ia32_ucmpq512_mask((__v8di)(__m512i)(a), \
                                         (__v8di)(__m512i)(b), (int)(p), \
                                         (__mmask8)-1); })

#define _mm512_mask_cmp_epi32_mask(m, a, b, p) __extension__ ({ \
  (__mmask16)__builtin_ia32_cmpd512_mask((__v16si)(__m512i)(a), \
                                         (__v16si)(__m512i)(b), (int)(p), \
                                         (__mmask16)(m)); })

#define _mm512_mask_cmp_epu32_mask(m, a, b, p) __extension__ ({ \
  (__mmask16)__builtin_ia32_ucmpd512_mask((__v16si)(__m512i)(a), \
                                          (__v16si)(__m512i)(b), (int)(p), \
                                          (__mmask16)(m)); })

#define _mm512_mask_cmp_epi64_mask(m, a, b, p) __extension__ ({ \
  (__mmask8)__builtin_ia32_cmpq512_mask((__v8di)(__m512i)(a), \
                                        (__v8di)(__m512i)(b), (int)(p), \
                                        (__mmask8)(m)); })

#define _mm512_mask_cmp_epu64_mask(m, a, b, p) __extension__ ({ \
  (__mmask8)__builtin_ia32_ucmpq512_mask((__v8di)(__m512i)(a), \
                                         (__v8di)(__m512i)(b), (int)(p), \
                                         (__mmask8)(m)); })

#define _mm512_rol_epi32(a, b) __extension__ ({ \
  (__m512i)__builtin_ia32_prold512_mask((__v16si)(__m512i)(a), (int)(b), \
                                        (__v16si)_mm512_setzero_si512(), \
                                        (__mmask16)-1); })

#define _mm512_mask_rol_epi32(W, U, a, b) __extension__ ({ \
  (__m512i)__builtin_ia32_prold512_mask((__v16si)(__m512i)(a), (int)(b), \
                                        (__v16si)(__m512i)(W), \
                                        (__mmask16)(U)); })

#define _mm512_maskz_rol_epi32(U, a, b) __extension__ ({ \
  (__m512i)__builtin_ia32_prold512_mask((__v16si)(__m512i)(a), (int)(b), \
                                        (__v16si)_mm512_setzero_si512(), \
                                        (__mmask16)(U)); })

#define _mm512_rol_epi64(a, b) __extension__ ({ \
  (__m512i)__builtin_ia32_prolq512_mask((__v8di)(__m512i)(a), (int)(b), \
                                        (__v8di)_mm512_setzero_si512(), \
                                        (__mmask8)-1); })

#define _mm512_mask_rol_epi64(W, U, a, b) __extension__ ({ \
  (__m512i)__builtin_ia32_prolq512_mask((__v8di)(__m512i)(a), (int)(b), \
                                        (__v8di)(__m512i)(W), (__mmask8)(U)); })

#define _mm512_maskz_rol_epi64(U, a, b) __extension__ ({ \
  (__m512i)__builtin_ia32_prolq512_mask((__v8di)(__m512i)(a), (int)(b), \
                                        (__v8di)_mm512_setzero_si512(), \
                                        (__mmask8)(U)); })
static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_rolv_epi32 (__m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_prolvd512_mask ((__v16si) __A,
              (__v16si) __B,
              (__v16si)
              _mm512_setzero_si512 (),
              (__mmask16) -1);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_rolv_epi32 (__m512i __W, __mmask16 __U, __m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_prolvd512_mask ((__v16si) __A,
              (__v16si) __B,
              (__v16si) __W,
              (__mmask16) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_rolv_epi32 (__mmask16 __U, __m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_prolvd512_mask ((__v16si) __A,
              (__v16si) __B,
              (__v16si)
              _mm512_setzero_si512 (),
              (__mmask16) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_rolv_epi64 (__m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_prolvq512_mask ((__v8di) __A,
              (__v8di) __B,
              (__v8di)
              _mm512_setzero_si512 (),
              (__mmask8) -1);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_rolv_epi64 (__m512i __W, __mmask8 __U, __m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_prolvq512_mask ((__v8di) __A,
              (__v8di) __B,
              (__v8di) __W,
              (__mmask8) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_rolv_epi64 (__mmask8 __U, __m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_prolvq512_mask ((__v8di) __A,
              (__v8di) __B,
              (__v8di)
              _mm512_setzero_si512 (),
              (__mmask8) __U);
}

#define _mm512_ror_epi32(A, B) __extension__ ({ \
  (__m512i)__builtin_ia32_prord512_mask((__v16si)(__m512i)(A), (int)(B), \
                                        (__v16si)_mm512_setzero_si512(), \
                                        (__mmask16)-1); })

#define _mm512_mask_ror_epi32(W, U, A, B) __extension__ ({ \
  (__m512i)__builtin_ia32_prord512_mask((__v16si)(__m512i)(A), (int)(B), \
                                        (__v16si)(__m512i)(W), \
                                        (__mmask16)(U)); })

#define _mm512_maskz_ror_epi32(U, A, B) __extension__ ({ \
  (__m512i)__builtin_ia32_prord512_mask((__v16si)(__m512i)(A), (int)(B), \
                                        (__v16si)_mm512_setzero_si512(), \
                                        (__mmask16)(U)); })

#define _mm512_ror_epi64(A, B) __extension__ ({ \
  (__m512i)__builtin_ia32_prorq512_mask((__v8di)(__m512i)(A), (int)(B), \
                                        (__v8di)_mm512_setzero_si512(), \
                                        (__mmask8)-1); })

#define _mm512_mask_ror_epi64(W, U, A, B) __extension__ ({ \
  (__m512i)__builtin_ia32_prorq512_mask((__v8di)(__m512i)(A), (int)(B), \
                                        (__v8di)(__m512i)(W), (__mmask8)(U)); })

#define _mm512_maskz_ror_epi64(U, A, B) __extension__ ({ \
  (__m512i)__builtin_ia32_prorq512_mask((__v8di)(__m512i)(A), (int)(B), \
                                        (__v8di)_mm512_setzero_si512(), \
                                        (__mmask8)(U)); })

#define _mm512_slli_epi32(A, B) __extension__ ({ \
  (__m512i)__builtin_ia32_pslldi512_mask((__v16si)(__m512i)(A), (int)(B), \
                                         (__v16si)_mm512_setzero_si512(), \
                                         (__mmask16)-1); })

#define _mm512_mask_slli_epi32(W, U, A, B) __extension__ ({ \
  (__m512i)__builtin_ia32_pslldi512_mask((__v16si)(__m512i)(A), (int)(B), \
                                         (__v16si)(__m512i)(W), \
                                         (__mmask16)(U)); })

#define _mm512_maskz_slli_epi32(U, A, B) __extension__ ({ \
  (__m512i)__builtin_ia32_pslldi512_mask((__v16si)(__m512i)(A), (int)(B), \
                                         (__v16si)_mm512_setzero_si512(), \
                                         (__mmask16)(U)); })

#define _mm512_slli_epi64(A, B) __extension__ ({ \
  (__m512i)__builtin_ia32_psllqi512_mask((__v8di)(__m512i)(A), (int)(B), \
                                         (__v8di)_mm512_setzero_si512(), \
                                         (__mmask8)-1); })

#define _mm512_mask_slli_epi64(W, U, A, B) __extension__ ({ \
  (__m512i)__builtin_ia32_psllqi512_mask((__v8di)(__m512i)(A), (int)(B), \
                                         (__v8di)(__m512i)(W), \
                                         (__mmask8)(U)); })

#define _mm512_maskz_slli_epi64(U, A, B) __extension__ ({ \
  (__m512i)__builtin_ia32_psllqi512_mask((__v8di)(__m512i)(A), (int)(B), \
                                         (__v8di)_mm512_setzero_si512(), \
                                         (__mmask8)(U)); })



#define _mm512_srli_epi32(A, B) __extension__ ({ \
  (__m512i)__builtin_ia32_psrldi512_mask((__v16si)(__m512i)(A), (int)(B), \
                                         (__v16si)_mm512_setzero_si512(), \
                                         (__mmask16)-1); })

#define _mm512_mask_srli_epi32(W, U, A, B) __extension__ ({ \
  (__m512i)__builtin_ia32_psrldi512_mask((__v16si)(__m512i)(A), (int)(B), \
                                         (__v16si)(__m512i)(W), \
                                         (__mmask16)(U)); })

#define _mm512_maskz_srli_epi32(U, A, B) __extension__ ({ \
  (__m512i)__builtin_ia32_psrldi512_mask((__v16si)(__m512i)(A), (int)(B), \
                                         (__v16si)_mm512_setzero_si512(), \
                                         (__mmask16)(U)); })

#define _mm512_srli_epi64(A, B) __extension__ ({ \
  (__m512i)__builtin_ia32_psrlqi512_mask((__v8di)(__m512i)(A), (int)(B), \
                                         (__v8di)_mm512_setzero_si512(), \
                                         (__mmask8)-1); })

#define _mm512_mask_srli_epi64(W, U, A, B) __extension__ ({ \
  (__m512i)__builtin_ia32_psrlqi512_mask((__v8di)(__m512i)(A), (int)(B), \
                                         (__v8di)(__m512i)(W), \
                                         (__mmask8)(U)); })

#define _mm512_maskz_srli_epi64(U, A, B) __extension__ ({ \
  (__m512i)__builtin_ia32_psrlqi512_mask((__v8di)(__m512i)(A), (int)(B), \
                                         (__v8di)_mm512_setzero_si512(), \
                                         (__mmask8)(U)); })

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_load_epi32 (__m512i __W, __mmask16 __U, void const *__P)
{
  return (__m512i) __builtin_ia32_movdqa32load512_mask ((const __v16si *) __P,
              (__v16si) __W,
              (__mmask16) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_load_epi32 (__mmask16 __U, void const *__P)
{
  return (__m512i) __builtin_ia32_movdqa32load512_mask ((const __v16si *) __P,
              (__v16si)
              _mm512_setzero_si512 (),
              (__mmask16) __U);
}

static __inline__ void __DEFAULT_FN_ATTRS
_mm512_mask_store_epi32 (void *__P, __mmask16 __U, __m512i __A)
{
  __builtin_ia32_movdqa32store512_mask ((__v16si *) __P, (__v16si) __A,
          (__mmask16) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_mov_epi32 (__m512i __W, __mmask16 __U, __m512i __A)
{
  return (__m512i) __builtin_ia32_selectd_512 ((__mmask16) __U,
                 (__v16si) __A,
                 (__v16si) __W);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_mov_epi32 (__mmask16 __U, __m512i __A)
{
  return (__m512i) __builtin_ia32_selectd_512 ((__mmask16) __U,
                 (__v16si) __A,
                 (__v16si) _mm512_setzero_si512 ());
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_mov_epi64 (__m512i __W, __mmask8 __U, __m512i __A)
{
  return (__m512i) __builtin_ia32_selectq_512 ((__mmask8) __U,
                 (__v8di) __A,
                 (__v8di) __W);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_mov_epi64 (__mmask8 __U, __m512i __A)
{
  return (__m512i) __builtin_ia32_selectq_512 ((__mmask8) __U,
                 (__v8di) __A,
                 (__v8di) _mm512_setzero_si512 ());
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_load_epi64 (__m512i __W, __mmask8 __U, void const *__P)
{
  return (__m512i) __builtin_ia32_movdqa64load512_mask ((const __v8di *) __P,
              (__v8di) __W,
              (__mmask8) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_load_epi64 (__mmask8 __U, void const *__P)
{
  return (__m512i) __builtin_ia32_movdqa64load512_mask ((const __v8di *) __P,
              (__v8di)
              _mm512_setzero_si512 (),
              (__mmask8) __U);
}

static __inline__ void __DEFAULT_FN_ATTRS
_mm512_mask_store_epi64 (void *__P, __mmask8 __U, __m512i __A)
{
  __builtin_ia32_movdqa64store512_mask ((__v8di *) __P, (__v8di) __A,
          (__mmask8) __U);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_movedup_pd (__m512d __A)
{
  return (__m512d)__builtin_shufflevector((__v8df)__A, (__v8df)__A,
                                          0, 0, 2, 2, 4, 4, 6, 6);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_mask_movedup_pd (__m512d __W, __mmask8 __U, __m512d __A)
{
  return (__m512d)__builtin_ia32_selectpd_512((__mmask8)__U,
                                              (__v8df)_mm512_movedup_pd(__A),
                                              (__v8df)__W);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_maskz_movedup_pd (__mmask8 __U, __m512d __A)
{
  return (__m512d)__builtin_ia32_selectpd_512((__mmask8)__U,
                                              (__v8df)_mm512_movedup_pd(__A),
                                              (__v8df)_mm512_setzero_pd());
}

#define _mm512_fixupimm_round_pd(A, B, C, imm, R) __extension__ ({ \
  (__m512d)__builtin_ia32_fixupimmpd512_mask((__v8df)(__m512d)(A), \
                                             (__v8df)(__m512d)(B), \
                                             (__v8di)(__m512i)(C), (int)(imm), \
                                             (__mmask8)-1, (int)(R)); })

#define _mm512_mask_fixupimm_round_pd(A, U, B, C, imm, R) __extension__ ({ \
  (__m512d)__builtin_ia32_fixupimmpd512_mask((__v8df)(__m512d)(A), \
                                             (__v8df)(__m512d)(B), \
                                             (__v8di)(__m512i)(C), (int)(imm), \
                                             (__mmask8)(U), (int)(R)); })

#define _mm512_fixupimm_pd(A, B, C, imm) __extension__ ({ \
  (__m512d)__builtin_ia32_fixupimmpd512_mask((__v8df)(__m512d)(A), \
                                             (__v8df)(__m512d)(B), \
                                             (__v8di)(__m512i)(C), (int)(imm), \
                                             (__mmask8)-1, \
                                             _MM_FROUND_CUR_DIRECTION); })

#define _mm512_mask_fixupimm_pd(A, U, B, C, imm) __extension__ ({ \
  (__m512d)__builtin_ia32_fixupimmpd512_mask((__v8df)(__m512d)(A), \
                                             (__v8df)(__m512d)(B), \
                                             (__v8di)(__m512i)(C), (int)(imm), \
                                             (__mmask8)(U), \
                                             _MM_FROUND_CUR_DIRECTION); })

#define _mm512_maskz_fixupimm_round_pd(U, A, B, C, imm, R) __extension__ ({ \
  (__m512d)__builtin_ia32_fixupimmpd512_maskz((__v8df)(__m512d)(A), \
                                              (__v8df)(__m512d)(B), \
                                              (__v8di)(__m512i)(C), \
                                              (int)(imm), (__mmask8)(U), \
                                              (int)(R)); })

#define _mm512_maskz_fixupimm_pd(U, A, B, C, imm) __extension__ ({ \
  (__m512d)__builtin_ia32_fixupimmpd512_maskz((__v8df)(__m512d)(A), \
                                              (__v8df)(__m512d)(B), \
                                              (__v8di)(__m512i)(C), \
                                              (int)(imm), (__mmask8)(U), \
                                              _MM_FROUND_CUR_DIRECTION); })

#define _mm512_fixupimm_round_ps(A, B, C, imm, R) __extension__ ({ \
  (__m512)__builtin_ia32_fixupimmps512_mask((__v16sf)(__m512)(A), \
                                            (__v16sf)(__m512)(B), \
                                            (__v16si)(__m512i)(C), (int)(imm), \
                                            (__mmask16)-1, (int)(R)); })

#define _mm512_mask_fixupimm_round_ps(A, U, B, C, imm, R) __extension__ ({ \
  (__m512)__builtin_ia32_fixupimmps512_mask((__v16sf)(__m512)(A), \
                                            (__v16sf)(__m512)(B), \
                                            (__v16si)(__m512i)(C), (int)(imm), \
                                            (__mmask16)(U), (int)(R)); })

#define _mm512_fixupimm_ps(A, B, C, imm) __extension__ ({ \
  (__m512)__builtin_ia32_fixupimmps512_mask((__v16sf)(__m512)(A), \
                                            (__v16sf)(__m512)(B), \
                                            (__v16si)(__m512i)(C), (int)(imm), \
                                            (__mmask16)-1, \
                                            _MM_FROUND_CUR_DIRECTION); })

#define _mm512_mask_fixupimm_ps(A, U, B, C, imm) __extension__ ({ \
  (__m512)__builtin_ia32_fixupimmps512_mask((__v16sf)(__m512)(A), \
                                            (__v16sf)(__m512)(B), \
                                            (__v16si)(__m512i)(C), (int)(imm), \
                                            (__mmask16)(U), \
                                            _MM_FROUND_CUR_DIRECTION); })

#define _mm512_maskz_fixupimm_round_ps(U, A, B, C, imm, R) __extension__ ({ \
  (__m512)__builtin_ia32_fixupimmps512_maskz((__v16sf)(__m512)(A), \
                                             (__v16sf)(__m512)(B), \
                                             (__v16si)(__m512i)(C), \
                                             (int)(imm), (__mmask16)(U), \
                                             (int)(R)); })

#define _mm512_maskz_fixupimm_ps(U, A, B, C, imm) __extension__ ({ \
  (__m512)__builtin_ia32_fixupimmps512_maskz((__v16sf)(__m512)(A), \
                                             (__v16sf)(__m512)(B), \
                                             (__v16si)(__m512i)(C), \
                                             (int)(imm), (__mmask16)(U), \
                                             _MM_FROUND_CUR_DIRECTION); })

#define _mm_fixupimm_round_sd(A, B, C, imm, R) __extension__ ({ \
  (__m128d)__builtin_ia32_fixupimmsd_mask((__v2df)(__m128d)(A), \
                                          (__v2df)(__m128d)(B), \
                                          (__v2di)(__m128i)(C), (int)(imm), \
                                          (__mmask8)-1, (int)(R)); })

#define _mm_mask_fixupimm_round_sd(A, U, B, C, imm, R) __extension__ ({ \
  (__m128d)__builtin_ia32_fixupimmsd_mask((__v2df)(__m128d)(A), \
                                          (__v2df)(__m128d)(B), \
                                          (__v2di)(__m128i)(C), (int)(imm), \
                                          (__mmask8)(U), (int)(R)); })

#define _mm_fixupimm_sd(A, B, C, imm) __extension__ ({ \
  (__m128d)__builtin_ia32_fixupimmsd_mask((__v2df)(__m128d)(A), \
                                          (__v2df)(__m128d)(B), \
                                          (__v2di)(__m128i)(C), (int)(imm), \
                                          (__mmask8)-1, \
                                          _MM_FROUND_CUR_DIRECTION); })

#define _mm_mask_fixupimm_sd(A, U, B, C, imm) __extension__ ({ \
  (__m128d)__builtin_ia32_fixupimmsd_mask((__v2df)(__m128d)(A), \
                                          (__v2df)(__m128d)(B), \
                                          (__v2di)(__m128i)(C), (int)(imm), \
                                          (__mmask8)(U), \
                                          _MM_FROUND_CUR_DIRECTION); })

#define _mm_maskz_fixupimm_round_sd(U, A, B, C, imm, R) __extension__ ({ \
  (__m128d)__builtin_ia32_fixupimmsd_maskz((__v2df)(__m128d)(A), \
                                           (__v2df)(__m128d)(B), \
                                           (__v2di)(__m128i)(C), (int)(imm), \
                                           (__mmask8)(U), (int)(R)); })

#define _mm_maskz_fixupimm_sd(U, A, B, C, imm) __extension__ ({ \
  (__m128d)__builtin_ia32_fixupimmsd_maskz((__v2df)(__m128d)(A), \
                                           (__v2df)(__m128d)(B), \
                                           (__v2di)(__m128i)(C), (int)(imm), \
                                           (__mmask8)(U), \
                                           _MM_FROUND_CUR_DIRECTION); })

#define _mm_fixupimm_round_ss(A, B, C, imm, R) __extension__ ({ \
  (__m128)__builtin_ia32_fixupimmss_mask((__v4sf)(__m128)(A), \
                                         (__v4sf)(__m128)(B), \
                                         (__v4si)(__m128i)(C), (int)(imm), \
                                         (__mmask8)-1, (int)(R)); })

#define _mm_mask_fixupimm_round_ss(A, U, B, C, imm, R) __extension__ ({ \
  (__m128)__builtin_ia32_fixupimmss_mask((__v4sf)(__m128)(A), \
                                         (__v4sf)(__m128)(B), \
                                         (__v4si)(__m128i)(C), (int)(imm), \
                                         (__mmask8)(U), (int)(R)); })

#define _mm_fixupimm_ss(A, B, C, imm) __extension__ ({ \
  (__m128)__builtin_ia32_fixupimmss_mask((__v4sf)(__m128)(A), \
                                         (__v4sf)(__m128)(B), \
                                         (__v4si)(__m128i)(C), (int)(imm), \
                                         (__mmask8)-1, \
                                         _MM_FROUND_CUR_DIRECTION); })

#define _mm_mask_fixupimm_ss(A, U, B, C, imm) __extension__ ({ \
  (__m128)__builtin_ia32_fixupimmss_mask((__v4sf)(__m128)(A), \
                                         (__v4sf)(__m128)(B), \
                                         (__v4si)(__m128i)(C), (int)(imm), \
                                         (__mmask8)(U), \
                                         _MM_FROUND_CUR_DIRECTION); })

#define _mm_maskz_fixupimm_round_ss(U, A, B, C, imm, R) __extension__ ({ \
  (__m128)__builtin_ia32_fixupimmss_maskz((__v4sf)(__m128)(A), \
                                          (__v4sf)(__m128)(B), \
                                          (__v4si)(__m128i)(C), (int)(imm), \
                                          (__mmask8)(U), (int)(R)); })

#define _mm_maskz_fixupimm_ss(U, A, B, C, imm) __extension__ ({ \
  (__m128)__builtin_ia32_fixupimmss_maskz((__v4sf)(__m128)(A), \
                                          (__v4sf)(__m128)(B), \
                                          (__v4si)(__m128i)(C), (int)(imm), \
                                          (__mmask8)(U), \
                                          _MM_FROUND_CUR_DIRECTION); })

#define _mm_getexp_round_sd(A, B, R) __extension__ ({ \
  (__m128d)__builtin_ia32_getexpsd128_round_mask((__v2df)(__m128d)(A), \
                                                 (__v2df)(__m128d)(B), \
                                                 (__v2df)_mm_setzero_pd(), \
                                                 (__mmask8)-1, (int)(R)); })


static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_getexp_sd (__m128d __A, __m128d __B)
{
  return (__m128d) __builtin_ia32_getexpsd128_round_mask ((__v2df) __A,
                 (__v2df) __B, (__v2df) _mm_setzero_pd(), (__mmask8) -1, _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_mask_getexp_sd (__m128d __W, __mmask8 __U, __m128d __A, __m128d __B)
{
 return (__m128d) __builtin_ia32_getexpsd128_round_mask ( (__v2df) __A,
          (__v2df) __B,
          (__v2df) __W,
          (__mmask8) __U,
          _MM_FROUND_CUR_DIRECTION);
}

#define _mm_mask_getexp_round_sd(W, U, A, B, R) __extension__ ({\
  (__m128d)__builtin_ia32_getexpsd128_round_mask((__v2df)(__m128d)(A), \
                                                 (__v2df)(__m128d)(B), \
                                                 (__v2df)(__m128d)(W), \
                                                 (__mmask8)(U), (int)(R)); })

static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_maskz_getexp_sd (__mmask8 __U, __m128d __A, __m128d __B)
{
 return (__m128d) __builtin_ia32_getexpsd128_round_mask ( (__v2df) __A,
          (__v2df) __B,
          (__v2df) _mm_setzero_pd (),
          (__mmask8) __U,
          _MM_FROUND_CUR_DIRECTION);
}

#define _mm_maskz_getexp_round_sd(U, A, B, R) __extension__ ({\
  (__m128d)__builtin_ia32_getexpsd128_round_mask((__v2df)(__m128d)(A), \
                                                 (__v2df)(__m128d)(B), \
                                                 (__v2df)_mm_setzero_pd(), \
                                                 (__mmask8)(U), (int)(R)); })

#define _mm_getexp_round_ss(A, B, R) __extension__ ({ \
  (__m128)__builtin_ia32_getexpss128_round_mask((__v4sf)(__m128)(A), \
                                                (__v4sf)(__m128)(B), \
                                                (__v4sf)_mm_setzero_ps(), \
                                                (__mmask8)-1, (int)(R)); })

static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_getexp_ss (__m128 __A, __m128 __B)
{
  return (__m128) __builtin_ia32_getexpss128_round_mask ((__v4sf) __A,
                (__v4sf) __B, (__v4sf)  _mm_setzero_ps(), (__mmask8) -1, _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_mask_getexp_ss (__m128 __W, __mmask8 __U, __m128 __A, __m128 __B)
{
 return (__m128) __builtin_ia32_getexpss128_round_mask ((__v4sf) __A,
          (__v4sf) __B,
          (__v4sf) __W,
          (__mmask8) __U,
          _MM_FROUND_CUR_DIRECTION);
}

#define _mm_mask_getexp_round_ss(W, U, A, B, R) __extension__ ({\
  (__m128)__builtin_ia32_getexpss128_round_mask((__v4sf)(__m128)(A), \
                                                (__v4sf)(__m128)(B), \
                                                (__v4sf)(__m128)(W), \
                                                (__mmask8)(U), (int)(R)); })

static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_maskz_getexp_ss (__mmask8 __U, __m128 __A, __m128 __B)
{
 return (__m128) __builtin_ia32_getexpss128_round_mask ((__v4sf) __A,
          (__v4sf) __B,
          (__v4sf) _mm_setzero_pd (),
          (__mmask8) __U,
          _MM_FROUND_CUR_DIRECTION);
}

#define _mm_maskz_getexp_round_ss(U, A, B, R) __extension__ ({\
  (__m128)__builtin_ia32_getexpss128_round_mask((__v4sf)(__m128)(A), \
                                                (__v4sf)(__m128)(B), \
                                                (__v4sf)_mm_setzero_ps(), \
                                                (__mmask8)(U), (int)(R)); })

#define _mm_getmant_round_sd(A, B, C, D, R) __extension__ ({ \
  (__m128d)__builtin_ia32_getmantsd_round_mask((__v2df)(__m128d)(A), \
                                               (__v2df)(__m128d)(B), \
                                               (int)(((D)<<2) | (C)), \
                                               (__v2df)_mm_setzero_pd(), \
                                               (__mmask8)-1, (int)(R)); })

#define _mm_getmant_sd(A, B, C, D)  __extension__ ({ \
  (__m128d)__builtin_ia32_getmantsd_round_mask((__v2df)(__m128d)(A), \
                                               (__v2df)(__m128d)(B), \
                                               (int)(((D)<<2) | (C)), \
                                               (__v2df)_mm_setzero_pd(), \
                                               (__mmask8)-1, \
                                               _MM_FROUND_CUR_DIRECTION); })

#define _mm_mask_getmant_sd(W, U, A, B, C, D) __extension__ ({\
  (__m128d)__builtin_ia32_getmantsd_round_mask((__v2df)(__m128d)(A), \
                                               (__v2df)(__m128d)(B), \
                                               (int)(((D)<<2) | (C)), \
                                               (__v2df)(__m128d)(W), \
                                               (__mmask8)(U), \
                                               _MM_FROUND_CUR_DIRECTION); })

#define _mm_mask_getmant_round_sd(W, U, A, B, C, D, R)({\
  (__m128d)__builtin_ia32_getmantsd_round_mask((__v2df)(__m128d)(A), \
                                               (__v2df)(__m128d)(B), \
                                               (int)(((D)<<2) | (C)), \
                                               (__v2df)(__m128d)(W), \
                                               (__mmask8)(U), (int)(R)); })

#define _mm_maskz_getmant_sd(U, A, B, C, D) __extension__ ({\
  (__m128d)__builtin_ia32_getmantsd_round_mask((__v2df)(__m128d)(A), \
                                               (__v2df)(__m128d)(B), \
                                               (int)(((D)<<2) | (C)), \
                                               (__v2df)_mm_setzero_pd(), \
                                               (__mmask8)(U), \
                                               _MM_FROUND_CUR_DIRECTION); })

#define _mm_maskz_getmant_round_sd(U, A, B, C, D, R) __extension__ ({\
  (__m128d)__builtin_ia32_getmantsd_round_mask((__v2df)(__m128d)(A), \
                                               (__v2df)(__m128d)(B), \
                                               (int)(((D)<<2) | (C)), \
                                               (__v2df)_mm_setzero_pd(), \
                                               (__mmask8)(U), (int)(R)); })

#define _mm_getmant_round_ss(A, B, C, D, R) __extension__ ({ \
  (__m128)__builtin_ia32_getmantss_round_mask((__v4sf)(__m128)(A), \
                                              (__v4sf)(__m128)(B), \
                                              (int)(((D)<<2) | (C)), \
                                              (__v4sf)_mm_setzero_ps(), \
                                              (__mmask8)-1, (int)(R)); })

#define _mm_getmant_ss(A, B, C, D) __extension__ ({ \
  (__m128)__builtin_ia32_getmantss_round_mask((__v4sf)(__m128)(A), \
                                              (__v4sf)(__m128)(B), \
                                              (int)(((D)<<2) | (C)), \
                                              (__v4sf)_mm_setzero_ps(), \
                                              (__mmask8)-1, \
                                              _MM_FROUND_CUR_DIRECTION); })

#define _mm_mask_getmant_ss(W, U, A, B, C, D) __extension__ ({\
  (__m128)__builtin_ia32_getmantss_round_mask((__v4sf)(__m128)(A), \
                                              (__v4sf)(__m128)(B), \
                                              (int)(((D)<<2) | (C)), \
                                              (__v4sf)(__m128)(W), \
                                              (__mmask8)(U), \
                                              _MM_FROUND_CUR_DIRECTION); })

#define _mm_mask_getmant_round_ss(W, U, A, B, C, D, R)({\
  (__m128)__builtin_ia32_getmantss_round_mask((__v4sf)(__m128)(A), \
                                              (__v4sf)(__m128)(B), \
                                              (int)(((D)<<2) | (C)), \
                                              (__v4sf)(__m128)(W), \
                                              (__mmask8)(U), (int)(R)); })

#define _mm_maskz_getmant_ss(U, A, B, C, D) __extension__ ({\
  (__m128)__builtin_ia32_getmantss_round_mask((__v4sf)(__m128)(A), \
                                              (__v4sf)(__m128)(B), \
                                              (int)(((D)<<2) | (C)), \
                                              (__v4sf)_mm_setzero_pd(), \
                                              (__mmask8)(U), \
                                              _MM_FROUND_CUR_DIRECTION); })

#define _mm_maskz_getmant_round_ss(U, A, B, C, D, R) __extension__ ({\
  (__m128)__builtin_ia32_getmantss_round_mask((__v4sf)(__m128)(A), \
                                              (__v4sf)(__m128)(B), \
                                              (int)(((D)<<2) | (C)), \
                                              (__v4sf)_mm_setzero_ps(), \
                                              (__mmask8)(U), (int)(R)); })

static __inline__ __mmask16 __DEFAULT_FN_ATTRS
_mm512_kmov (__mmask16 __A)
{
  return  __A;
}

#define _mm_comi_round_sd(A, B, P, R) __extension__ ({\
  (int)__builtin_ia32_vcomisd((__v2df)(__m128d)(A), (__v2df)(__m128d)(B), \
                              (int)(P), (int)(R)); })

#define _mm_comi_round_ss(A, B, P, R) __extension__ ({\
  (int)__builtin_ia32_vcomiss((__v4sf)(__m128)(A), (__v4sf)(__m128)(B), \
                              (int)(P), (int)(R)); })

#define _mm_cvt_roundsd_si64(A, R) __extension__ ({ \
  (long long)__builtin_ia32_vcvtsd2si64((__v2df)(__m128d)(A), (int)(R)); })

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask2_permutex2var_epi32 (__m512i __A, __m512i __I,
         __mmask16 __U, __m512i __B)
{
  return (__m512i) __builtin_ia32_vpermi2vard512_mask ((__v16si) __A,
                   (__v16si) __I
                   /* idx */ ,
                   (__v16si) __B,
                   (__mmask16) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_sll_epi32 (__m512i __A, __m128i __B)
{
  return (__m512i) __builtin_ia32_pslld512_mask ((__v16si) __A,
             (__v4si) __B,
             (__v16si)
             _mm512_setzero_si512 (),
             (__mmask16) -1);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_sll_epi32 (__m512i __W, __mmask16 __U, __m512i __A, __m128i __B)
{
  return (__m512i) __builtin_ia32_pslld512_mask ((__v16si) __A,
             (__v4si) __B,
             (__v16si) __W,
             (__mmask16) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_sll_epi32 (__mmask16 __U, __m512i __A, __m128i __B)
{
  return (__m512i) __builtin_ia32_pslld512_mask ((__v16si) __A,
             (__v4si) __B,
             (__v16si)
             _mm512_setzero_si512 (),
             (__mmask16) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_sll_epi64 (__m512i __A, __m128i __B)
{
  return (__m512i) __builtin_ia32_psllq512_mask ((__v8di) __A,
             (__v2di) __B,
             (__v8di)
             _mm512_setzero_si512 (),
             (__mmask8) -1);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_sll_epi64 (__m512i __W, __mmask8 __U, __m512i __A, __m128i __B)
{
  return (__m512i) __builtin_ia32_psllq512_mask ((__v8di) __A,
             (__v2di) __B,
             (__v8di) __W,
             (__mmask8) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_sll_epi64 (__mmask8 __U, __m512i __A, __m128i __B)
{
  return (__m512i) __builtin_ia32_psllq512_mask ((__v8di) __A,
             (__v2di) __B,
             (__v8di)
             _mm512_setzero_si512 (),
             (__mmask8) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_sllv_epi32 (__m512i __X, __m512i __Y)
{
  return (__m512i) __builtin_ia32_psllv16si_mask ((__v16si) __X,
              (__v16si) __Y,
              (__v16si)
              _mm512_setzero_si512 (),
              (__mmask16) -1);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_sllv_epi32 (__m512i __W, __mmask16 __U, __m512i __X, __m512i __Y)
{
  return (__m512i) __builtin_ia32_psllv16si_mask ((__v16si) __X,
              (__v16si) __Y,
              (__v16si) __W,
              (__mmask16) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_sllv_epi32 (__mmask16 __U, __m512i __X, __m512i __Y)
{
  return (__m512i) __builtin_ia32_psllv16si_mask ((__v16si) __X,
              (__v16si) __Y,
              (__v16si)
              _mm512_setzero_si512 (),
              (__mmask16) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_sllv_epi64 (__m512i __X, __m512i __Y)
{
  return (__m512i) __builtin_ia32_psllv8di_mask ((__v8di) __X,
             (__v8di) __Y,
             (__v8di)
             _mm512_undefined_pd (),
             (__mmask8) -1);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_sllv_epi64 (__m512i __W, __mmask8 __U, __m512i __X, __m512i __Y)
{
  return (__m512i) __builtin_ia32_psllv8di_mask ((__v8di) __X,
             (__v8di) __Y,
             (__v8di) __W,
             (__mmask8) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_sllv_epi64 (__mmask8 __U, __m512i __X, __m512i __Y)
{
  return (__m512i) __builtin_ia32_psllv8di_mask ((__v8di) __X,
             (__v8di) __Y,
             (__v8di)
             _mm512_setzero_si512 (),
             (__mmask8) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_sra_epi32 (__m512i __A, __m128i __B)
{
  return (__m512i) __builtin_ia32_psrad512_mask ((__v16si) __A,
             (__v4si) __B,
             (__v16si)
             _mm512_setzero_si512 (),
             (__mmask16) -1);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_sra_epi32 (__m512i __W, __mmask16 __U, __m512i __A, __m128i __B)
{
  return (__m512i) __builtin_ia32_psrad512_mask ((__v16si) __A,
             (__v4si) __B,
             (__v16si) __W,
             (__mmask16) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_sra_epi32 (__mmask16 __U, __m512i __A, __m128i __B)
{
  return (__m512i) __builtin_ia32_psrad512_mask ((__v16si) __A,
             (__v4si) __B,
             (__v16si)
             _mm512_setzero_si512 (),
             (__mmask16) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_sra_epi64 (__m512i __A, __m128i __B)
{
  return (__m512i) __builtin_ia32_psraq512_mask ((__v8di) __A,
             (__v2di) __B,
             (__v8di)
             _mm512_setzero_si512 (),
             (__mmask8) -1);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_sra_epi64 (__m512i __W, __mmask8 __U, __m512i __A, __m128i __B)
{
  return (__m512i) __builtin_ia32_psraq512_mask ((__v8di) __A,
             (__v2di) __B,
             (__v8di) __W,
             (__mmask8) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_sra_epi64 (__mmask8 __U, __m512i __A, __m128i __B)
{
  return (__m512i) __builtin_ia32_psraq512_mask ((__v8di) __A,
             (__v2di) __B,
             (__v8di)
             _mm512_setzero_si512 (),
             (__mmask8) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_srav_epi32 (__m512i __X, __m512i __Y)
{
  return (__m512i) __builtin_ia32_psrav16si_mask ((__v16si) __X,
              (__v16si) __Y,
              (__v16si)
              _mm512_setzero_si512 (),
              (__mmask16) -1);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_srav_epi32 (__m512i __W, __mmask16 __U, __m512i __X, __m512i __Y)
{
  return (__m512i) __builtin_ia32_psrav16si_mask ((__v16si) __X,
              (__v16si) __Y,
              (__v16si) __W,
              (__mmask16) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_srav_epi32 (__mmask16 __U, __m512i __X, __m512i __Y)
{
  return (__m512i) __builtin_ia32_psrav16si_mask ((__v16si) __X,
              (__v16si) __Y,
              (__v16si)
              _mm512_setzero_si512 (),
              (__mmask16) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_srav_epi64 (__m512i __X, __m512i __Y)
{
  return (__m512i) __builtin_ia32_psrav8di_mask ((__v8di) __X,
             (__v8di) __Y,
             (__v8di)
             _mm512_setzero_si512 (),
             (__mmask8) -1);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_srav_epi64 (__m512i __W, __mmask8 __U, __m512i __X, __m512i __Y)
{
  return (__m512i) __builtin_ia32_psrav8di_mask ((__v8di) __X,
             (__v8di) __Y,
             (__v8di) __W,
             (__mmask8) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_srav_epi64 (__mmask8 __U, __m512i __X, __m512i __Y)
{
  return (__m512i) __builtin_ia32_psrav8di_mask ((__v8di) __X,
             (__v8di) __Y,
             (__v8di)
             _mm512_setzero_si512 (),
             (__mmask8) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_srl_epi32 (__m512i __A, __m128i __B)
{
  return (__m512i) __builtin_ia32_psrld512_mask ((__v16si) __A,
             (__v4si) __B,
             (__v16si)
             _mm512_setzero_si512 (),
             (__mmask16) -1);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_srl_epi32 (__m512i __W, __mmask16 __U, __m512i __A, __m128i __B)
{
  return (__m512i) __builtin_ia32_psrld512_mask ((__v16si) __A,
             (__v4si) __B,
             (__v16si) __W,
             (__mmask16) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_srl_epi32 (__mmask16 __U, __m512i __A, __m128i __B)
{
  return (__m512i) __builtin_ia32_psrld512_mask ((__v16si) __A,
             (__v4si) __B,
             (__v16si)
             _mm512_setzero_si512 (),
             (__mmask16) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_srl_epi64 (__m512i __A, __m128i __B)
{
  return (__m512i) __builtin_ia32_psrlq512_mask ((__v8di) __A,
             (__v2di) __B,
             (__v8di)
             _mm512_setzero_si512 (),
             (__mmask8) -1);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_srl_epi64 (__m512i __W, __mmask8 __U, __m512i __A, __m128i __B)
{
  return (__m512i) __builtin_ia32_psrlq512_mask ((__v8di) __A,
             (__v2di) __B,
             (__v8di) __W,
             (__mmask8) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_srl_epi64 (__mmask8 __U, __m512i __A, __m128i __B)
{
  return (__m512i) __builtin_ia32_psrlq512_mask ((__v8di) __A,
             (__v2di) __B,
             (__v8di)
             _mm512_setzero_si512 (),
             (__mmask8) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_srlv_epi32 (__m512i __X, __m512i __Y)
{
  return (__m512i) __builtin_ia32_psrlv16si_mask ((__v16si) __X,
              (__v16si) __Y,
              (__v16si)
              _mm512_setzero_si512 (),
              (__mmask16) -1);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_srlv_epi32 (__m512i __W, __mmask16 __U, __m512i __X, __m512i __Y)
{
  return (__m512i) __builtin_ia32_psrlv16si_mask ((__v16si) __X,
              (__v16si) __Y,
              (__v16si) __W,
              (__mmask16) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_srlv_epi32 (__mmask16 __U, __m512i __X, __m512i __Y)
{
  return (__m512i) __builtin_ia32_psrlv16si_mask ((__v16si) __X,
              (__v16si) __Y,
              (__v16si)
              _mm512_setzero_si512 (),
              (__mmask16) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_srlv_epi64 (__m512i __X, __m512i __Y)
{
  return (__m512i) __builtin_ia32_psrlv8di_mask ((__v8di) __X,
             (__v8di) __Y,
             (__v8di)
             _mm512_setzero_si512 (),
             (__mmask8) -1);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_srlv_epi64 (__m512i __W, __mmask8 __U, __m512i __X, __m512i __Y)
{
  return (__m512i) __builtin_ia32_psrlv8di_mask ((__v8di) __X,
             (__v8di) __Y,
             (__v8di) __W,
             (__mmask8) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_srlv_epi64 (__mmask8 __U, __m512i __X, __m512i __Y)
{
  return (__m512i) __builtin_ia32_psrlv8di_mask ((__v8di) __X,
             (__v8di) __Y,
             (__v8di)
             _mm512_setzero_si512 (),
             (__mmask8) __U);
}

#define _mm512_ternarylogic_epi32(A, B, C, imm) __extension__ ({ \
  (__m512i)__builtin_ia32_pternlogd512_mask((__v16si)(__m512i)(A), \
                                            (__v16si)(__m512i)(B), \
                                            (__v16si)(__m512i)(C), (int)(imm), \
                                            (__mmask16)-1); })

#define _mm512_mask_ternarylogic_epi32(A, U, B, C, imm) __extension__ ({ \
  (__m512i)__builtin_ia32_pternlogd512_mask((__v16si)(__m512i)(A), \
                                            (__v16si)(__m512i)(B), \
                                            (__v16si)(__m512i)(C), (int)(imm), \
                                            (__mmask16)(U)); })

#define _mm512_maskz_ternarylogic_epi32(U, A, B, C, imm) __extension__ ({ \
  (__m512i)__builtin_ia32_pternlogd512_maskz((__v16si)(__m512i)(A), \
                                             (__v16si)(__m512i)(B), \
                                             (__v16si)(__m512i)(C), \
                                             (int)(imm), (__mmask16)(U)); })

#define _mm512_ternarylogic_epi64(A, B, C, imm) __extension__ ({ \
  (__m512i)__builtin_ia32_pternlogq512_mask((__v8di)(__m512i)(A), \
                                            (__v8di)(__m512i)(B), \
                                            (__v8di)(__m512i)(C), (int)(imm), \
                                            (__mmask8)-1); })

#define _mm512_mask_ternarylogic_epi64(A, U, B, C, imm) __extension__ ({ \
  (__m512i)__builtin_ia32_pternlogq512_mask((__v8di)(__m512i)(A), \
                                            (__v8di)(__m512i)(B), \
                                            (__v8di)(__m512i)(C), (int)(imm), \
                                            (__mmask8)(U)); })

#define _mm512_maskz_ternarylogic_epi64(U, A, B, C, imm) __extension__ ({ \
  (__m512i)__builtin_ia32_pternlogq512_maskz((__v8di)(__m512i)(A), \
                                             (__v8di)(__m512i)(B), \
                                             (__v8di)(__m512i)(C), (int)(imm), \
                                             (__mmask8)(U)); })

#define _mm_cvt_roundsd_i64(A, R) __extension__ ({ \
  (long long)__builtin_ia32_vcvtsd2si64((__v2df)(__m128d)(A), (int)(R)); })

#define _mm_cvt_roundsd_si32(A, R) __extension__ ({ \
  (int)__builtin_ia32_vcvtsd2si32((__v2df)(__m128d)(A), (int)(R)); })

#define _mm_cvt_roundsd_i32(A, R) __extension__ ({ \
  (int)__builtin_ia32_vcvtsd2si32((__v2df)(__m128d)(A), (int)(R)); })

#define _mm_cvt_roundsd_u32(A, R) __extension__ ({ \
  (unsigned int)__builtin_ia32_vcvtsd2usi32((__v2df)(__m128d)(A), (int)(R)); })

static __inline__ unsigned __DEFAULT_FN_ATTRS
_mm_cvtsd_u32 (__m128d __A)
{
  return (unsigned) __builtin_ia32_vcvtsd2usi32 ((__v2df) __A,
             _MM_FROUND_CUR_DIRECTION);
}

#define _mm_cvt_roundsd_u64(A, R) __extension__ ({ \
  (unsigned long long)__builtin_ia32_vcvtsd2usi64((__v2df)(__m128d)(A), \
                                                  (int)(R)); })

static __inline__ unsigned long long __DEFAULT_FN_ATTRS
_mm_cvtsd_u64 (__m128d __A)
{
  return (unsigned long long) __builtin_ia32_vcvtsd2usi64 ((__v2df)
                 __A,
                 _MM_FROUND_CUR_DIRECTION);
}

#define _mm_cvt_roundss_si32(A, R) __extension__ ({ \
  (int)__builtin_ia32_vcvtss2si32((__v4sf)(__m128)(A), (int)(R)); })

#define _mm_cvt_roundss_i32(A, R) __extension__ ({ \
  (int)__builtin_ia32_vcvtss2si32((__v4sf)(__m128)(A), (int)(R)); })

#define _mm_cvt_roundss_si64(A, R) __extension__ ({ \
  (long long)__builtin_ia32_vcvtss2si64((__v4sf)(__m128)(A), (int)(R)); })

#define _mm_cvt_roundss_i64(A, R) __extension__ ({ \
  (long long)__builtin_ia32_vcvtss2si64((__v4sf)(__m128)(A), (int)(R)); })

#define _mm_cvt_roundss_u32(A, R) __extension__ ({ \
  (unsigned int)__builtin_ia32_vcvtss2usi32((__v4sf)(__m128)(A), (int)(R)); })

static __inline__ unsigned __DEFAULT_FN_ATTRS
_mm_cvtss_u32 (__m128 __A)
{
  return (unsigned) __builtin_ia32_vcvtss2usi32 ((__v4sf) __A,
             _MM_FROUND_CUR_DIRECTION);
}

#define _mm_cvt_roundss_u64(A, R) __extension__ ({ \
  (unsigned long long)__builtin_ia32_vcvtss2usi64((__v4sf)(__m128)(A), \
                                                  (int)(R)); })

static __inline__ unsigned long long __DEFAULT_FN_ATTRS
_mm_cvtss_u64 (__m128 __A)
{
  return (unsigned long long) __builtin_ia32_vcvtss2usi64 ((__v4sf)
                 __A,
                 _MM_FROUND_CUR_DIRECTION);
}

#define _mm_cvtt_roundsd_i32(A, R) __extension__ ({ \
  (int)__builtin_ia32_vcvttsd2si32((__v2df)(__m128d)(A), (int)(R)); })

#define _mm_cvtt_roundsd_si32(A, R) __extension__ ({ \
  (int)__builtin_ia32_vcvttsd2si32((__v2df)(__m128d)(A), (int)(R)); })

static __inline__ int __DEFAULT_FN_ATTRS
_mm_cvttsd_i32 (__m128d __A)
{
  return (int) __builtin_ia32_vcvttsd2si32 ((__v2df) __A,
              _MM_FROUND_CUR_DIRECTION);
}

#define _mm_cvtt_roundsd_si64(A, R) __extension__ ({ \
  (long long)__builtin_ia32_vcvttsd2si64((__v2df)(__m128d)(A), (int)(R)); })

#define _mm_cvtt_roundsd_i64(A, R) __extension__ ({ \
  (long long)__builtin_ia32_vcvttsd2si64((__v2df)(__m128d)(A), (int)(R)); })

static __inline__ long long __DEFAULT_FN_ATTRS
_mm_cvttsd_i64 (__m128d __A)
{
  return (long long) __builtin_ia32_vcvttsd2si64 ((__v2df) __A,
              _MM_FROUND_CUR_DIRECTION);
}

#define _mm_cvtt_roundsd_u32(A, R) __extension__ ({ \
  (unsigned int)__builtin_ia32_vcvttsd2usi32((__v2df)(__m128d)(A), (int)(R)); })

static __inline__ unsigned __DEFAULT_FN_ATTRS
_mm_cvttsd_u32 (__m128d __A)
{
  return (unsigned) __builtin_ia32_vcvttsd2usi32 ((__v2df) __A,
              _MM_FROUND_CUR_DIRECTION);
}

#define _mm_cvtt_roundsd_u64(A, R) __extension__ ({ \
  (unsigned long long)__builtin_ia32_vcvttsd2usi64((__v2df)(__m128d)(A), \
                                                   (int)(R)); })

static __inline__ unsigned long long __DEFAULT_FN_ATTRS
_mm_cvttsd_u64 (__m128d __A)
{
  return (unsigned long long) __builtin_ia32_vcvttsd2usi64 ((__v2df)
                  __A,
                  _MM_FROUND_CUR_DIRECTION);
}

#define _mm_cvtt_roundss_i32(A, R) __extension__ ({ \
  (int)__builtin_ia32_vcvttss2si32((__v4sf)(__m128)(A), (int)(R)); })

#define _mm_cvtt_roundss_si32(A, R) __extension__ ({ \
  (int)__builtin_ia32_vcvttss2si32((__v4sf)(__m128)(A), (int)(R)); })

static __inline__ int __DEFAULT_FN_ATTRS
_mm_cvttss_i32 (__m128 __A)
{
  return (int) __builtin_ia32_vcvttss2si32 ((__v4sf) __A,
              _MM_FROUND_CUR_DIRECTION);
}

#define _mm_cvtt_roundss_i64(A, R) __extension__ ({ \
  (long long)__builtin_ia32_vcvttss2si64((__v4sf)(__m128)(A), (int)(R)); })

#define _mm_cvtt_roundss_si64(A, R) __extension__ ({ \
  (long long)__builtin_ia32_vcvttss2si64((__v4sf)(__m128)(A), (int)(R)); })

static __inline__ long long __DEFAULT_FN_ATTRS
_mm_cvttss_i64 (__m128 __A)
{
  return (long long) __builtin_ia32_vcvttss2si64 ((__v4sf) __A,
              _MM_FROUND_CUR_DIRECTION);
}

#define _mm_cvtt_roundss_u32(A, R) __extension__ ({ \
  (unsigned int)__builtin_ia32_vcvttss2usi32((__v4sf)(__m128)(A), (int)(R)); })

static __inline__ unsigned __DEFAULT_FN_ATTRS
_mm_cvttss_u32 (__m128 __A)
{
  return (unsigned) __builtin_ia32_vcvttss2usi32 ((__v4sf) __A,
              _MM_FROUND_CUR_DIRECTION);
}

#define _mm_cvtt_roundss_u64(A, R) __extension__ ({ \
  (unsigned long long)__builtin_ia32_vcvttss2usi64((__v4sf)(__m128)(A), \
                                                   (int)(R)); })

static __inline__ unsigned long long __DEFAULT_FN_ATTRS
_mm_cvttss_u64 (__m128 __A)
{
  return (unsigned long long) __builtin_ia32_vcvttss2usi64 ((__v4sf)
                  __A,
                  _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_mask2_permutex2var_pd (__m512d __A, __m512i __I, __mmask8 __U,
            __m512d __B)
{
  return (__m512d) __builtin_ia32_vpermi2varpd512_mask ((__v8df) __A,
              (__v8di) __I
              /* idx */ ,
              (__v8df) __B,
              (__mmask8) __U);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_mask2_permutex2var_ps (__m512 __A, __m512i __I, __mmask16 __U,
            __m512 __B)
{
  return (__m512) __builtin_ia32_vpermi2varps512_mask ((__v16sf) __A,
                   (__v16si) __I
                   /* idx */ ,
                   (__v16sf) __B,
                   (__mmask16) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask2_permutex2var_epi64 (__m512i __A, __m512i __I,
         __mmask8 __U, __m512i __B)
{
  return (__m512i) __builtin_ia32_vpermi2varq512_mask ((__v8di) __A,
                   (__v8di) __I
                   /* idx */ ,
                   (__v8di) __B,
                   (__mmask8) __U);
}

#define _mm512_permute_pd(X, C) __extension__ ({ \
  (__m512d)__builtin_shufflevector((__v8df)(__m512d)(X), \
                                   (__v8df)_mm512_undefined_pd(), \
                                   0 + (((C) >> 0) & 0x1), \
                                   0 + (((C) >> 1) & 0x1), \
                                   2 + (((C) >> 2) & 0x1), \
                                   2 + (((C) >> 3) & 0x1), \
                                   4 + (((C) >> 4) & 0x1), \
                                   4 + (((C) >> 5) & 0x1), \
                                   6 + (((C) >> 6) & 0x1), \
                                   6 + (((C) >> 7) & 0x1)); })

#define _mm512_mask_permute_pd(W, U, X, C) __extension__ ({ \
  (__m512d)__builtin_ia32_selectpd_512((__mmask8)(U), \
                                       (__v8df)_mm512_permute_pd((X), (C)), \
                                       (__v8df)(__m512d)(W)); })

#define _mm512_maskz_permute_pd(U, X, C) __extension__ ({ \
  (__m512d)__builtin_ia32_selectpd_512((__mmask8)(U), \
                                       (__v8df)_mm512_permute_pd((X), (C)), \
                                       (__v8df)_mm512_setzero_pd()); })

#define _mm512_permute_ps(X, C) __extension__ ({ \
  (__m512)__builtin_shufflevector((__v16sf)(__m512)(X), \
                                  (__v16sf)_mm512_undefined_ps(), \
                                   0  + (((C) >> 0) & 0x3), \
                                   0  + (((C) >> 2) & 0x3), \
                                   0  + (((C) >> 4) & 0x3), \
                                   0  + (((C) >> 6) & 0x3), \
                                   4  + (((C) >> 0) & 0x3), \
                                   4  + (((C) >> 2) & 0x3), \
                                   4  + (((C) >> 4) & 0x3), \
                                   4  + (((C) >> 6) & 0x3), \
                                   8  + (((C) >> 0) & 0x3), \
                                   8  + (((C) >> 2) & 0x3), \
                                   8  + (((C) >> 4) & 0x3), \
                                   8  + (((C) >> 6) & 0x3), \
                                   12 + (((C) >> 0) & 0x3), \
                                   12 + (((C) >> 2) & 0x3), \
                                   12 + (((C) >> 4) & 0x3), \
                                   12 + (((C) >> 6) & 0x3)); })

#define _mm512_mask_permute_ps(W, U, X, C) __extension__ ({ \
  (__m512)__builtin_ia32_selectps_512((__mmask16)(U), \
                                      (__v16sf)_mm512_permute_ps((X), (C)), \
                                      (__v16sf)(__m512)(W)); })

#define _mm512_maskz_permute_ps(U, X, C) __extension__ ({ \
  (__m512)__builtin_ia32_selectps_512((__mmask16)(U), \
                                      (__v16sf)_mm512_permute_ps((X), (C)), \
                                      (__v16sf)_mm512_setzero_ps()); })

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_permutevar_pd (__m512d __A, __m512i __C)
{
  return (__m512d) __builtin_ia32_vpermilvarpd512_mask ((__v8df) __A,
              (__v8di) __C,
              (__v8df)
              _mm512_undefined_pd (),
              (__mmask8) -1);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_mask_permutevar_pd (__m512d __W, __mmask8 __U, __m512d __A, __m512i __C)
{
  return (__m512d) __builtin_ia32_vpermilvarpd512_mask ((__v8df) __A,
              (__v8di) __C,
              (__v8df) __W,
              (__mmask8) __U);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_maskz_permutevar_pd (__mmask8 __U, __m512d __A, __m512i __C)
{
  return (__m512d) __builtin_ia32_vpermilvarpd512_mask ((__v8df) __A,
              (__v8di) __C,
              (__v8df)
              _mm512_setzero_pd (),
              (__mmask8) __U);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_permutevar_ps (__m512 __A, __m512i __C)
{
  return (__m512) __builtin_ia32_vpermilvarps512_mask ((__v16sf) __A,
                   (__v16si) __C,
                   (__v16sf)
                   _mm512_undefined_ps (),
                   (__mmask16) -1);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_mask_permutevar_ps (__m512 __W, __mmask16 __U, __m512 __A, __m512i __C)
{
  return (__m512) __builtin_ia32_vpermilvarps512_mask ((__v16sf) __A,
                   (__v16si) __C,
                   (__v16sf) __W,
                   (__mmask16) __U);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_maskz_permutevar_ps (__mmask16 __U, __m512 __A, __m512i __C)
{
  return (__m512) __builtin_ia32_vpermilvarps512_mask ((__v16sf) __A,
                   (__v16si) __C,
                   (__v16sf)
                   _mm512_setzero_ps (),
                   (__mmask16) __U);
}

static __inline __m512d __DEFAULT_FN_ATTRS
_mm512_permutex2var_pd(__m512d __A, __m512i __I, __m512d __B)
{
  return (__m512d) __builtin_ia32_vpermt2varpd512_mask ((__v8di) __I
                    /* idx */ ,
                    (__v8df) __A,
                    (__v8df) __B,
                    (__mmask8) -1);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_mask_permutex2var_pd (__m512d __A, __mmask8 __U, __m512i __I, __m512d __B)
{
  return (__m512d) __builtin_ia32_vpermt2varpd512_mask ((__v8di) __I
                    /* idx */ ,
                    (__v8df) __A,
                    (__v8df) __B,
                    (__mmask8) __U);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_maskz_permutex2var_pd (__mmask8 __U, __m512d __A, __m512i __I,
            __m512d __B)
{
  return (__m512d) __builtin_ia32_vpermt2varpd512_maskz ((__v8di) __I
                                                         /* idx */ ,
                                                         (__v8df) __A,
                                                         (__v8df) __B,
                                                         (__mmask8) __U);
}

static __inline __m512 __DEFAULT_FN_ATTRS
_mm512_permutex2var_ps(__m512 __A, __m512i __I, __m512 __B)
{
  return (__m512) __builtin_ia32_vpermt2varps512_mask ((__v16si) __I
                                                         /* idx */ ,
                                                         (__v16sf) __A,
                                                         (__v16sf) __B,
                                                         (__mmask16) -1);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_mask_permutex2var_ps (__m512 __A, __mmask16 __U, __m512i __I, __m512 __B)
{
  return (__m512) __builtin_ia32_vpermt2varps512_mask ((__v16si) __I
                                                         /* idx */ ,
                                                         (__v16sf) __A,
                                                         (__v16sf) __B,
                                                         (__mmask16) __U);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_maskz_permutex2var_ps (__mmask16 __U, __m512 __A, __m512i __I,
            __m512 __B)
{
  return (__m512) __builtin_ia32_vpermt2varps512_maskz ((__v16si) __I
                                                        /* idx */ ,
                                                        (__v16sf) __A,
                                                        (__v16sf) __B,
                                                        (__mmask16) __U);
}

static __inline__ __mmask16 __DEFAULT_FN_ATTRS
_mm512_testn_epi32_mask (__m512i __A, __m512i __B)
{
  return (__mmask16) __builtin_ia32_ptestnmd512 ((__v16si) __A,
             (__v16si) __B,
             (__mmask16) -1);
}

static __inline__ __mmask16 __DEFAULT_FN_ATTRS
_mm512_mask_testn_epi32_mask (__mmask16 __U, __m512i __A, __m512i __B)
{
  return (__mmask16) __builtin_ia32_ptestnmd512 ((__v16si) __A,
             (__v16si) __B, __U);
}

static __inline__ __mmask8 __DEFAULT_FN_ATTRS
_mm512_testn_epi64_mask (__m512i __A, __m512i __B)
{
  return (__mmask8) __builtin_ia32_ptestnmq512 ((__v8di) __A,
            (__v8di) __B,
            (__mmask8) -1);
}

static __inline__ __mmask8 __DEFAULT_FN_ATTRS
_mm512_mask_testn_epi64_mask (__mmask8 __U, __m512i __A, __m512i __B)
{
  return (__mmask8) __builtin_ia32_ptestnmq512 ((__v8di) __A,
            (__v8di) __B, __U);
}

#define _mm512_cvtt_roundpd_epu32(A, R) __extension__ ({ \
  (__m256i)__builtin_ia32_cvttpd2udq512_mask((__v8df)(__m512d)(A), \
                                             (__v8si)_mm256_undefined_si256(), \
                                             (__mmask8)-1, (int)(R)); })

#define _mm512_mask_cvtt_roundpd_epu32(W, U, A, R) __extension__ ({ \
  (__m256i)__builtin_ia32_cvttpd2udq512_mask((__v8df)(__m512d)(A), \
                                             (__v8si)(__m256i)(W), \
                                             (__mmask8)(U), (int)(R)); })

#define _mm512_maskz_cvtt_roundpd_epu32(U, A, R) __extension__ ({ \
  (__m256i)__builtin_ia32_cvttpd2udq512_mask((__v8df)(__m512d)(A), \
                                             (__v8si)_mm256_setzero_si256(), \
                                             (__mmask8)(U), (int)(R)); })

static __inline__ __m256i __DEFAULT_FN_ATTRS
_mm512_cvttpd_epu32 (__m512d __A)
{
  return (__m256i) __builtin_ia32_cvttpd2udq512_mask ((__v8df) __A,
                  (__v8si)
                  _mm256_undefined_si256 (),
                  (__mmask8) -1,
                  _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS
_mm512_mask_cvttpd_epu32 (__m256i __W, __mmask8 __U, __m512d __A)
{
  return (__m256i) __builtin_ia32_cvttpd2udq512_mask ((__v8df) __A,
                  (__v8si) __W,
                  (__mmask8) __U,
                  _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS
_mm512_maskz_cvttpd_epu32 (__mmask8 __U, __m512d __A)
{
  return (__m256i) __builtin_ia32_cvttpd2udq512_mask ((__v8df) __A,
                  (__v8si)
                  _mm256_setzero_si256 (),
                  (__mmask8) __U,
                  _MM_FROUND_CUR_DIRECTION);
}

#define _mm_roundscale_round_sd(A, B, imm, R) __extension__ ({ \
  (__m128d)__builtin_ia32_rndscalesd_round_mask((__v2df)(__m128d)(A), \
                                                (__v2df)(__m128d)(B), \
                                                (__v2df)_mm_setzero_pd(), \
                                                (__mmask8)-1, (int)(imm), \
                                                (int)(R)); })

#define _mm_roundscale_sd(A, B, imm) __extension__ ({ \
  (__m128d)__builtin_ia32_rndscalesd_round_mask((__v2df)(__m128d)(A), \
                                                (__v2df)(__m128d)(B), \
                                                (__v2df)_mm_setzero_pd(), \
                                                (__mmask8)-1, (int)(imm), \
                                                _MM_FROUND_CUR_DIRECTION); })

#define _mm_mask_roundscale_sd(W, U, A, B, imm) __extension__ ({ \
  (__m128d)__builtin_ia32_rndscalesd_round_mask((__v2df)(__m128d)(A), \
                                                (__v2df)(__m128d)(B), \
                                                (__v2df)(__m128d)(W), \
                                                (__mmask8)(U), (int)(imm), \
                                                _MM_FROUND_CUR_DIRECTION); })

#define _mm_mask_roundscale_round_sd(W, U, A, B, I, R) __extension__ ({ \
  (__m128d)__builtin_ia32_rndscalesd_round_mask((__v2df)(__m128d)(A), \
                                                (__v2df)(__m128d)(B), \
                                                (__v2df)(__m128d)(W), \
                                                (__mmask8)(U), (int)(I), \
                                                (int)(R)); })

#define _mm_maskz_roundscale_sd(U, A, B, I) __extension__ ({ \
  (__m128d)__builtin_ia32_rndscalesd_round_mask((__v2df)(__m128d)(A), \
                                                (__v2df)(__m128d)(B), \
                                                (__v2df)_mm_setzero_pd(), \
                                                (__mmask8)(U), (int)(I), \
                                                _MM_FROUND_CUR_DIRECTION); })

#define _mm_maskz_roundscale_round_sd(U, A, B, I, R) __extension__ ({ \
  (__m128d)__builtin_ia32_rndscalesd_round_mask((__v2df)(__m128d)(A), \
                                                (__v2df)(__m128d)(B), \
                                                (__v2df)_mm_setzero_pd(), \
                                                (__mmask8)(U), (int)(I), \
                                                (int)(R)); })

#define _mm_roundscale_round_ss(A, B, imm, R) __extension__ ({ \
  (__m128)__builtin_ia32_rndscaless_round_mask((__v4sf)(__m128)(A), \
                                               (__v4sf)(__m128)(B), \
                                               (__v4sf)_mm_setzero_ps(), \
                                               (__mmask8)-1, (int)(imm), \
                                               (int)(R)); })

#define _mm_roundscale_ss(A, B, imm) __extension__ ({ \
  (__m128)__builtin_ia32_rndscaless_round_mask((__v4sf)(__m128)(A), \
                                               (__v4sf)(__m128)(B), \
                                               (__v4sf)_mm_setzero_ps(), \
                                               (__mmask8)-1, (int)(imm), \
                                               _MM_FROUND_CUR_DIRECTION); })

#define _mm_mask_roundscale_ss(W, U, A, B, I) __extension__ ({ \
  (__m128)__builtin_ia32_rndscaless_round_mask((__v4sf)(__m128)(A), \
                                               (__v4sf)(__m128)(B), \
                                               (__v4sf)(__m128)(W), \
                                               (__mmask8)(U), (int)(I), \
                                               _MM_FROUND_CUR_DIRECTION); })

#define _mm_mask_roundscale_round_ss(W, U, A, B, I, R) __extension__ ({ \
  (__m128)__builtin_ia32_rndscaless_round_mask((__v4sf)(__m128)(A), \
                                               (__v4sf)(__m128)(B), \
                                               (__v4sf)(__m128)(W), \
                                               (__mmask8)(U), (int)(I), \
                                               (int)(R)); })

#define _mm_maskz_roundscale_ss(U, A, B, I) __extension__ ({ \
  (__m128)__builtin_ia32_rndscaless_round_mask((__v4sf)(__m128)(A), \
                                               (__v4sf)(__m128)(B), \
                                               (__v4sf)_mm_setzero_ps(), \
                                               (__mmask8)(U), (int)(I), \
                                               _MM_FROUND_CUR_DIRECTION); })

#define _mm_maskz_roundscale_round_ss(U, A, B, I, R) __extension__ ({ \
  (__m128)__builtin_ia32_rndscaless_round_mask((__v4sf)(__m128)(A), \
                                               (__v4sf)(__m128)(B), \
                                               (__v4sf)_mm_setzero_ps(), \
                                               (__mmask8)(U), (int)(I), \
                                               (int)(R)); })

#define _mm512_scalef_round_pd(A, B, R) __extension__ ({ \
  (__m512d)__builtin_ia32_scalefpd512_mask((__v8df)(__m512d)(A), \
                                           (__v8df)(__m512d)(B), \
                                           (__v8df)_mm512_undefined_pd(), \
                                           (__mmask8)-1, (int)(R)); })

#define _mm512_mask_scalef_round_pd(W, U, A, B, R) __extension__ ({ \
  (__m512d)__builtin_ia32_scalefpd512_mask((__v8df)(__m512d)(A), \
                                           (__v8df)(__m512d)(B), \
                                           (__v8df)(__m512d)(W), \
                                           (__mmask8)(U), (int)(R)); })

#define _mm512_maskz_scalef_round_pd(U, A, B, R) __extension__ ({ \
  (__m512d)__builtin_ia32_scalefpd512_mask((__v8df)(__m512d)(A), \
                                           (__v8df)(__m512d)(B), \
                                           (__v8df)_mm512_setzero_pd(), \
                                           (__mmask8)(U), (int)(R)); })

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_scalef_pd (__m512d __A, __m512d __B)
{
  return (__m512d) __builtin_ia32_scalefpd512_mask ((__v8df) __A,
                (__v8df) __B,
                (__v8df)
                _mm512_undefined_pd (),
                (__mmask8) -1,
                _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_mask_scalef_pd (__m512d __W, __mmask8 __U, __m512d __A, __m512d __B)
{
  return (__m512d) __builtin_ia32_scalefpd512_mask ((__v8df) __A,
                (__v8df) __B,
                (__v8df) __W,
                (__mmask8) __U,
                _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_maskz_scalef_pd (__mmask8 __U, __m512d __A, __m512d __B)
{
  return (__m512d) __builtin_ia32_scalefpd512_mask ((__v8df) __A,
                (__v8df) __B,
                (__v8df)
                _mm512_setzero_pd (),
                (__mmask8) __U,
                _MM_FROUND_CUR_DIRECTION);
}

#define _mm512_scalef_round_ps(A, B, R) __extension__ ({ \
  (__m512)__builtin_ia32_scalefps512_mask((__v16sf)(__m512)(A), \
                                          (__v16sf)(__m512)(B), \
                                          (__v16sf)_mm512_undefined_ps(), \
                                          (__mmask16)-1, (int)(R)); })

#define _mm512_mask_scalef_round_ps(W, U, A, B, R) __extension__ ({ \
  (__m512)__builtin_ia32_scalefps512_mask((__v16sf)(__m512)(A), \
                                          (__v16sf)(__m512)(B), \
                                          (__v16sf)(__m512)(W), \
                                          (__mmask16)(U), (int)(R)); })

#define _mm512_maskz_scalef_round_ps(U, A, B, R) __extension__ ({ \
  (__m512)__builtin_ia32_scalefps512_mask((__v16sf)(__m512)(A), \
                                          (__v16sf)(__m512)(B), \
                                          (__v16sf)_mm512_setzero_ps(), \
                                          (__mmask16)(U), (int)(R)); })

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_scalef_ps (__m512 __A, __m512 __B)
{
  return (__m512) __builtin_ia32_scalefps512_mask ((__v16sf) __A,
               (__v16sf) __B,
               (__v16sf)
               _mm512_undefined_ps (),
               (__mmask16) -1,
               _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_mask_scalef_ps (__m512 __W, __mmask16 __U, __m512 __A, __m512 __B)
{
  return (__m512) __builtin_ia32_scalefps512_mask ((__v16sf) __A,
               (__v16sf) __B,
               (__v16sf) __W,
               (__mmask16) __U,
               _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_maskz_scalef_ps (__mmask16 __U, __m512 __A, __m512 __B)
{
  return (__m512) __builtin_ia32_scalefps512_mask ((__v16sf) __A,
               (__v16sf) __B,
               (__v16sf)
               _mm512_setzero_ps (),
               (__mmask16) __U,
               _MM_FROUND_CUR_DIRECTION);
}

#define _mm_scalef_round_sd(A, B, R) __extension__ ({ \
  (__m128d)__builtin_ia32_scalefsd_round_mask((__v2df)(__m128d)(A), \
                                              (__v2df)(__m128d)(B), \
                                              (__v2df)_mm_setzero_pd(), \
                                              (__mmask8)-1, (int)(R)); })

static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_scalef_sd (__m128d __A, __m128d __B)
{
  return (__m128d) __builtin_ia32_scalefsd_round_mask ((__v2df) __A,
              (__v2df)( __B), (__v2df) _mm_setzero_pd(),
              (__mmask8) -1,
              _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_mask_scalef_sd (__m128d __W, __mmask8 __U, __m128d __A, __m128d __B)
{
 return (__m128d) __builtin_ia32_scalefsd_round_mask ( (__v2df) __A,
                 (__v2df) __B,
                (__v2df) __W,
                (__mmask8) __U,
                _MM_FROUND_CUR_DIRECTION);
}

#define _mm_mask_scalef_round_sd(W, U, A, B, R) __extension__ ({ \
  (__m128d)__builtin_ia32_scalefsd_round_mask((__v2df)(__m128d)(A), \
                                              (__v2df)(__m128d)(B), \
                                              (__v2df)(__m128d)(W), \
                                              (__mmask8)(U), (int)(R)); })

static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_maskz_scalef_sd (__mmask8 __U, __m128d __A, __m128d __B)
{
 return (__m128d) __builtin_ia32_scalefsd_round_mask ( (__v2df) __A,
                 (__v2df) __B,
                (__v2df) _mm_setzero_pd (),
                (__mmask8) __U,
                _MM_FROUND_CUR_DIRECTION);
}

#define _mm_maskz_scalef_round_sd(U, A, B, R) __extension__ ({ \
  (__m128d)__builtin_ia32_scalefsd_round_mask((__v2df)(__m128d)(A), \
                                              (__v2df)(__m128d)(B), \
                                              (__v2df)_mm_setzero_pd(), \
                                              (__mmask8)(U), (int)(R)); })

#define _mm_scalef_round_ss(A, B, R) __extension__ ({ \
  (__m128)__builtin_ia32_scalefss_round_mask((__v4sf)(__m128)(A), \
                                             (__v4sf)(__m128)(B), \
                                             (__v4sf)_mm_setzero_ps(), \
                                             (__mmask8)-1, (int)(R)); })

static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_scalef_ss (__m128 __A, __m128 __B)
{
  return (__m128) __builtin_ia32_scalefss_round_mask ((__v4sf) __A,
             (__v4sf)( __B), (__v4sf) _mm_setzero_ps(),
             (__mmask8) -1,
             _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_mask_scalef_ss (__m128 __W, __mmask8 __U, __m128 __A, __m128 __B)
{
 return (__m128) __builtin_ia32_scalefss_round_mask ( (__v4sf) __A,
                (__v4sf) __B,
                (__v4sf) __W,
                (__mmask8) __U,
                _MM_FROUND_CUR_DIRECTION);
}

#define _mm_mask_scalef_round_ss(W, U, A, B, R) __extension__ ({ \
  (__m128)__builtin_ia32_scalefss_round_mask((__v4sf)(__m128)(A), \
                                             (__v4sf)(__m128)(B), \
                                             (__v4sf)(__m128)(W), \
                                             (__mmask8)(U), (int)(R)); })

static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_maskz_scalef_ss (__mmask8 __U, __m128 __A, __m128 __B)
{
 return (__m128) __builtin_ia32_scalefss_round_mask ( (__v4sf) __A,
                 (__v4sf) __B,
                (__v4sf) _mm_setzero_ps (),
                (__mmask8) __U,
                _MM_FROUND_CUR_DIRECTION);
}

#define _mm_maskz_scalef_round_ss(U, A, B, R) __extension__ ({ \
  (__m128)__builtin_ia32_scalefss_round_mask((__v4sf)(__m128)(A), \
                                             (__v4sf)(__m128)(B), \
                                             (__v4sf)_mm_setzero_ps(), \
                                             (__mmask8)(U), \
                                             _MM_FROUND_CUR_DIRECTION); })

#define _mm512_srai_epi32(A, B) __extension__ ({ \
  (__m512i)__builtin_ia32_psradi512_mask((__v16si)(__m512i)(A), (int)(B), \
                                         (__v16si)_mm512_setzero_si512(), \
                                         (__mmask16)-1); })

#define _mm512_mask_srai_epi32(W, U, A, B) __extension__ ({ \
  (__m512i)__builtin_ia32_psradi512_mask((__v16si)(__m512i)(A), (int)(B), \
                                         (__v16si)(__m512i)(W), \
                                         (__mmask16)(U)); })

#define _mm512_maskz_srai_epi32(U, A, B) __extension__ ({ \
  (__m512i)__builtin_ia32_psradi512_mask((__v16si)(__m512i)(A), (int)(B), \
                                         (__v16si)_mm512_setzero_si512(), \
                                         (__mmask16)(U)); })

#define _mm512_srai_epi64(A, B) __extension__ ({ \
  (__m512i)__builtin_ia32_psraqi512_mask((__v8di)(__m512i)(A), (int)(B), \
                                         (__v8di)_mm512_setzero_si512(), \
                                         (__mmask8)-1); })

#define _mm512_mask_srai_epi64(W, U, A, B) __extension__ ({ \
  (__m512i)__builtin_ia32_psraqi512_mask((__v8di)(__m512i)(A), (int)(B), \
                                         (__v8di)(__m512i)(W), \
                                         (__mmask8)(U)); })

#define _mm512_maskz_srai_epi64(U, A, B) __extension__ ({ \
  (__m512i)__builtin_ia32_psraqi512_mask((__v8di)(__m512i)(A), (int)(B), \
                                         (__v8di)_mm512_setzero_si512(), \
                                         (__mmask8)(U)); })

#define _mm512_shuffle_f32x4(A, B, imm) __extension__ ({ \
  (__m512)__builtin_ia32_shuf_f32x4_mask((__v16sf)(__m512)(A), \
                                         (__v16sf)(__m512)(B), (int)(imm), \
                                         (__v16sf)_mm512_undefined_ps(), \
                                         (__mmask16)-1); })

#define _mm512_mask_shuffle_f32x4(W, U, A, B, imm) __extension__ ({ \
  (__m512)__builtin_ia32_shuf_f32x4_mask((__v16sf)(__m512)(A), \
                                         (__v16sf)(__m512)(B), (int)(imm), \
                                         (__v16sf)(__m512)(W), \
                                         (__mmask16)(U)); })

#define _mm512_maskz_shuffle_f32x4(U, A, B, imm) __extension__ ({ \
  (__m512)__builtin_ia32_shuf_f32x4_mask((__v16sf)(__m512)(A), \
                                         (__v16sf)(__m512)(B), (int)(imm), \
                                         (__v16sf)_mm512_setzero_ps(), \
                                         (__mmask16)(U)); })

#define _mm512_shuffle_f64x2(A, B, imm) __extension__ ({ \
  (__m512d)__builtin_ia32_shuf_f64x2_mask((__v8df)(__m512d)(A), \
                                          (__v8df)(__m512d)(B), (int)(imm), \
                                          (__v8df)_mm512_undefined_pd(), \
                                          (__mmask8)-1); })

#define _mm512_mask_shuffle_f64x2(W, U, A, B, imm) __extension__ ({ \
  (__m512d)__builtin_ia32_shuf_f64x2_mask((__v8df)(__m512d)(A), \
                                          (__v8df)(__m512d)(B), (int)(imm), \
                                          (__v8df)(__m512d)(W), \
                                          (__mmask8)(U)); })

#define _mm512_maskz_shuffle_f64x2(U, A, B, imm) __extension__ ({ \
  (__m512d)__builtin_ia32_shuf_f64x2_mask((__v8df)(__m512d)(A), \
                                          (__v8df)(__m512d)(B), (int)(imm), \
                                          (__v8df)_mm512_setzero_pd(), \
                                          (__mmask8)(U)); })

#define _mm512_shuffle_i32x4(A, B, imm) __extension__ ({ \
  (__m512i)__builtin_ia32_shuf_i32x4_mask((__v16si)(__m512i)(A), \
                                          (__v16si)(__m512i)(B), (int)(imm), \
                                          (__v16si)_mm512_setzero_si512(), \
                                          (__mmask16)-1); })

#define _mm512_mask_shuffle_i32x4(W, U, A, B, imm) __extension__ ({ \
  (__m512i)__builtin_ia32_shuf_i32x4_mask((__v16si)(__m512i)(A), \
                                          (__v16si)(__m512i)(B), (int)(imm), \
                                          (__v16si)(__m512i)(W), \
                                          (__mmask16)(U)); })

#define _mm512_maskz_shuffle_i32x4(U, A, B, imm) __extension__ ({ \
  (__m512i)__builtin_ia32_shuf_i32x4_mask((__v16si)(__m512i)(A), \
                                          (__v16si)(__m512i)(B), (int)(imm), \
                                          (__v16si)_mm512_setzero_si512(), \
                                          (__mmask16)(U)); })

#define _mm512_shuffle_i64x2(A, B, imm) __extension__ ({ \
  (__m512i)__builtin_ia32_shuf_i64x2_mask((__v8di)(__m512i)(A), \
                                          (__v8di)(__m512i)(B), (int)(imm), \
                                          (__v8di)_mm512_setzero_si512(), \
                                          (__mmask8)-1); })

#define _mm512_mask_shuffle_i64x2(W, U, A, B, imm) __extension__ ({ \
  (__m512i)__builtin_ia32_shuf_i64x2_mask((__v8di)(__m512i)(A), \
                                          (__v8di)(__m512i)(B), (int)(imm), \
                                          (__v8di)(__m512i)(W), \
                                          (__mmask8)(U)); })

#define _mm512_maskz_shuffle_i64x2(U, A, B, imm) __extension__ ({ \
  (__m512i)__builtin_ia32_shuf_i64x2_mask((__v8di)(__m512i)(A), \
                                          (__v8di)(__m512i)(B), (int)(imm), \
                                          (__v8di)_mm512_setzero_si512(), \
                                          (__mmask8)(U)); })

#define _mm512_shuffle_pd(A, B, M) __extension__ ({ \
  (__m512d)__builtin_shufflevector((__v8df)(__m512d)(A), \
                                   (__v8df)(__m512d)(B), \
                                   0  + (((M) >> 0) & 0x1), \
                                   8  + (((M) >> 1) & 0x1), \
                                   2  + (((M) >> 2) & 0x1), \
                                   10 + (((M) >> 3) & 0x1), \
                                   4  + (((M) >> 4) & 0x1), \
                                   12 + (((M) >> 5) & 0x1), \
                                   6  + (((M) >> 6) & 0x1), \
                                   14 + (((M) >> 7) & 0x1)); })

#define _mm512_mask_shuffle_pd(W, U, A, B, M) __extension__ ({ \
  (__m512d)__builtin_ia32_selectpd_512((__mmask8)(U), \
                                       (__v8df)_mm512_shuffle_pd((A), (B), (M)), \
                                       (__v8df)(__m512d)(W)); })

#define _mm512_maskz_shuffle_pd(U, A, B, M) __extension__ ({ \
  (__m512d)__builtin_ia32_selectpd_512((__mmask8)(U), \
                                       (__v8df)_mm512_shuffle_pd((A), (B), (M)), \
                                       (__v8df)_mm512_setzero_pd()); })

#define _mm512_shuffle_ps(A, B, M) __extension__ ({ \
  (__m512d)__builtin_shufflevector((__v16sf)(__m512)(A), \
                                   (__v16sf)(__m512)(B), \
                                   0  + (((M) >> 0) & 0x3), \
                                   0  + (((M) >> 2) & 0x3), \
                                   16 + (((M) >> 4) & 0x3), \
                                   16 + (((M) >> 6) & 0x3), \
                                   4  + (((M) >> 0) & 0x3), \
                                   4  + (((M) >> 2) & 0x3), \
                                   20 + (((M) >> 4) & 0x3), \
                                   20 + (((M) >> 6) & 0x3), \
                                   8  + (((M) >> 0) & 0x3), \
                                   8  + (((M) >> 2) & 0x3), \
                                   24 + (((M) >> 4) & 0x3), \
                                   24 + (((M) >> 6) & 0x3), \
                                   12 + (((M) >> 0) & 0x3), \
                                   12 + (((M) >> 2) & 0x3), \
                                   28 + (((M) >> 4) & 0x3), \
                                   28 + (((M) >> 6) & 0x3)); })

#define _mm512_mask_shuffle_ps(W, U, A, B, M) __extension__ ({ \
  (__m512)__builtin_ia32_selectps_512((__mmask16)(U), \
                                      (__v16sf)_mm512_shuffle_ps((A), (B), (M)), \
                                      (__v16sf)(__m512)(W)); })

#define _mm512_maskz_shuffle_ps(U, A, B, M) __extension__ ({ \
  (__m512)__builtin_ia32_selectps_512((__mmask16)(U), \
                                      (__v16sf)_mm512_shuffle_ps((A), (B), (M)), \
                                      (__v16sf)_mm512_setzero_ps()); })

#define _mm_sqrt_round_sd(A, B, R) __extension__ ({ \
  (__m128d)__builtin_ia32_sqrtsd_round_mask((__v2df)(__m128d)(A), \
                                            (__v2df)(__m128d)(B), \
                                            (__v2df)_mm_setzero_pd(), \
                                            (__mmask8)-1, (int)(R)); })

static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_mask_sqrt_sd (__m128d __W, __mmask8 __U, __m128d __A, __m128d __B)
{
 return (__m128d) __builtin_ia32_sqrtsd_round_mask ( (__v2df) __A,
                 (__v2df) __B,
                (__v2df) __W,
                (__mmask8) __U,
                _MM_FROUND_CUR_DIRECTION);
}

#define _mm_mask_sqrt_round_sd(W, U, A, B, R) __extension__ ({ \
  (__m128d)__builtin_ia32_sqrtsd_round_mask((__v2df)(__m128d)(A), \
                                            (__v2df)(__m128d)(B), \
                                            (__v2df)(__m128d)(W), \
                                            (__mmask8)(U), (int)(R)); })

static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_maskz_sqrt_sd (__mmask8 __U, __m128d __A, __m128d __B)
{
 return (__m128d) __builtin_ia32_sqrtsd_round_mask ( (__v2df) __A,
                 (__v2df) __B,
                (__v2df) _mm_setzero_pd (),
                (__mmask8) __U,
                _MM_FROUND_CUR_DIRECTION);
}

#define _mm_maskz_sqrt_round_sd(U, A, B, R) __extension__ ({ \
  (__m128d)__builtin_ia32_sqrtsd_round_mask((__v2df)(__m128d)(A), \
                                            (__v2df)(__m128d)(B), \
                                            (__v2df)_mm_setzero_pd(), \
                                            (__mmask8)(U), (int)(R)); })

#define _mm_sqrt_round_ss(A, B, R) __extension__ ({ \
  (__m128)__builtin_ia32_sqrtss_round_mask((__v4sf)(__m128)(A), \
                                           (__v4sf)(__m128)(B), \
                                           (__v4sf)_mm_setzero_ps(), \
                                           (__mmask8)-1, (int)(R)); })

static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_mask_sqrt_ss (__m128 __W, __mmask8 __U, __m128 __A, __m128 __B)
{
 return (__m128) __builtin_ia32_sqrtss_round_mask ( (__v4sf) __A,
                 (__v4sf) __B,
                (__v4sf) __W,
                (__mmask8) __U,
                _MM_FROUND_CUR_DIRECTION);
}

#define _mm_mask_sqrt_round_ss(W, U, A, B, R) __extension__ ({ \
  (__m128)__builtin_ia32_sqrtss_round_mask((__v4sf)(__m128)(A), \
                                           (__v4sf)(__m128)(B), \
                                           (__v4sf)(__m128)(W), (__mmask8)(U), \
                                           (int)(R)); })

static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_maskz_sqrt_ss (__mmask8 __U, __m128 __A, __m128 __B)
{
 return (__m128) __builtin_ia32_sqrtss_round_mask ( (__v4sf) __A,
                 (__v4sf) __B,
                (__v4sf) _mm_setzero_ps (),
                (__mmask8) __U,
                _MM_FROUND_CUR_DIRECTION);
}

#define _mm_maskz_sqrt_round_ss(U, A, B, R) __extension__ ({ \
  (__m128)__builtin_ia32_sqrtss_round_mask((__v4sf)(__m128)(A), \
                                           (__v4sf)(__m128)(B), \
                                           (__v4sf)_mm_setzero_ps(), \
                                           (__mmask8)(U), (int)(R)); })

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_broadcast_f32x4 (__m128 __A)
{
  return (__m512) __builtin_ia32_broadcastf32x4_512 ((__v4sf) __A,
                 (__v16sf)
                 _mm512_undefined_ps (),
                 (__mmask16) -1);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_mask_broadcast_f32x4 (__m512 __O, __mmask16 __M, __m128 __A)
{
  return (__m512) __builtin_ia32_broadcastf32x4_512 ((__v4sf) __A,
                 (__v16sf) __O,
                 __M);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_maskz_broadcast_f32x4 (__mmask16 __M, __m128 __A)
{
  return (__m512) __builtin_ia32_broadcastf32x4_512 ((__v4sf) __A,
                 (__v16sf)
                 _mm512_setzero_ps (),
                 __M);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_broadcast_f64x4 (__m256d __A)
{
  return (__m512d) __builtin_ia32_broadcastf64x4_512 ((__v4df) __A,
                  (__v8df)
                  _mm512_undefined_pd (),
                  (__mmask8) -1);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_mask_broadcast_f64x4 (__m512d __O, __mmask8 __M, __m256d __A)
{
  return (__m512d) __builtin_ia32_broadcastf64x4_512 ((__v4df) __A,
                  (__v8df) __O,
                  __M);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_maskz_broadcast_f64x4 (__mmask8 __M, __m256d __A)
{
  return (__m512d) __builtin_ia32_broadcastf64x4_512 ((__v4df) __A,
                  (__v8df)
                  _mm512_setzero_pd (),
                  __M);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_broadcast_i32x4 (__m128i __A)
{
  return (__m512i) __builtin_ia32_broadcasti32x4_512 ((__v4si) __A,
                  (__v16si)
                  _mm512_undefined_epi32 (),
                  (__mmask16) -1);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_broadcast_i32x4 (__m512i __O, __mmask16 __M, __m128i __A)
{
  return (__m512i) __builtin_ia32_broadcasti32x4_512 ((__v4si) __A,
                  (__v16si) __O,
                  __M);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_broadcast_i32x4 (__mmask16 __M, __m128i __A)
{
  return (__m512i) __builtin_ia32_broadcasti32x4_512 ((__v4si) __A,
                  (__v16si)
                  _mm512_setzero_si512 (),
                  __M);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_broadcast_i64x4 (__m256i __A)
{
  return (__m512i) __builtin_ia32_broadcasti64x4_512 ((__v4di) __A,
                  (__v8di)
                  _mm512_undefined_epi32 (),
                  (__mmask8) -1);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_broadcast_i64x4 (__m512i __O, __mmask8 __M, __m256i __A)
{
  return (__m512i) __builtin_ia32_broadcasti64x4_512 ((__v4di) __A,
                  (__v8di) __O,
                  __M);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_broadcast_i64x4 (__mmask8 __M, __m256i __A)
{
  return (__m512i) __builtin_ia32_broadcasti64x4_512 ((__v4di) __A,
                  (__v8di)
                  _mm512_setzero_si512 (),
                  __M);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_mask_broadcastsd_pd (__m512d __O, __mmask8 __M, __m128d __A)
{
  return (__m512d)__builtin_ia32_selectpd_512(__M,
                                              (__v8df) _mm512_broadcastsd_pd(__A),
                                              (__v8df) __O);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_maskz_broadcastsd_pd (__mmask8 __M, __m128d __A)
{
  return (__m512d)__builtin_ia32_selectpd_512(__M,
                                              (__v8df) _mm512_broadcastsd_pd(__A),
                                              (__v8df) _mm512_setzero_pd());
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_mask_broadcastss_ps (__m512 __O, __mmask16 __M, __m128 __A)
{
  return (__m512)__builtin_ia32_selectps_512(__M,
                                             (__v16sf) _mm512_broadcastss_ps(__A),
                                             (__v16sf) __O);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_maskz_broadcastss_ps (__mmask16 __M, __m128 __A)
{
  return (__m512)__builtin_ia32_selectps_512(__M,
                                             (__v16sf) _mm512_broadcastss_ps(__A),
                                             (__v16sf) _mm512_setzero_ps());
}

static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm512_cvtsepi32_epi8 (__m512i __A)
{
  return (__m128i) __builtin_ia32_pmovsdb512_mask ((__v16si) __A,
               (__v16qi) _mm_undefined_si128 (),
               (__mmask16) -1);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm512_mask_cvtsepi32_epi8 (__m128i __O, __mmask16 __M, __m512i __A)
{
  return (__m128i) __builtin_ia32_pmovsdb512_mask ((__v16si) __A,
               (__v16qi) __O, __M);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm512_maskz_cvtsepi32_epi8 (__mmask16 __M, __m512i __A)
{
  return (__m128i) __builtin_ia32_pmovsdb512_mask ((__v16si) __A,
               (__v16qi) _mm_setzero_si128 (),
               __M);
}

static __inline__ void __DEFAULT_FN_ATTRS
_mm512_mask_cvtsepi32_storeu_epi8 (void * __P, __mmask16 __M, __m512i __A)
{
  __builtin_ia32_pmovsdb512mem_mask ((__v16qi *) __P, (__v16si) __A, __M);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS
_mm512_cvtsepi32_epi16 (__m512i __A)
{
  return (__m256i) __builtin_ia32_pmovsdw512_mask ((__v16si) __A,
               (__v16hi) _mm256_undefined_si256 (),
               (__mmask16) -1);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS
_mm512_mask_cvtsepi32_epi16 (__m256i __O, __mmask16 __M, __m512i __A)
{
  return (__m256i) __builtin_ia32_pmovsdw512_mask ((__v16si) __A,
               (__v16hi) __O, __M);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS
_mm512_maskz_cvtsepi32_epi16 (__mmask16 __M, __m512i __A)
{
  return (__m256i) __builtin_ia32_pmovsdw512_mask ((__v16si) __A,
               (__v16hi) _mm256_setzero_si256 (),
               __M);
}

static __inline__ void __DEFAULT_FN_ATTRS
_mm512_mask_cvtsepi32_storeu_epi16 (void *__P, __mmask16 __M, __m512i __A)
{
  __builtin_ia32_pmovsdw512mem_mask ((__v16hi*) __P, (__v16si) __A, __M);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm512_cvtsepi64_epi8 (__m512i __A)
{
  return (__m128i) __builtin_ia32_pmovsqb512_mask ((__v8di) __A,
               (__v16qi) _mm_undefined_si128 (),
               (__mmask8) -1);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm512_mask_cvtsepi64_epi8 (__m128i __O, __mmask8 __M, __m512i __A)
{
  return (__m128i) __builtin_ia32_pmovsqb512_mask ((__v8di) __A,
               (__v16qi) __O, __M);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm512_maskz_cvtsepi64_epi8 (__mmask8 __M, __m512i __A)
{
  return (__m128i) __builtin_ia32_pmovsqb512_mask ((__v8di) __A,
               (__v16qi) _mm_setzero_si128 (),
               __M);
}

static __inline__ void __DEFAULT_FN_ATTRS
_mm512_mask_cvtsepi64_storeu_epi8 (void * __P, __mmask8 __M, __m512i __A)
{
  __builtin_ia32_pmovsqb512mem_mask ((__v16qi *) __P, (__v8di) __A, __M);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS
_mm512_cvtsepi64_epi32 (__m512i __A)
{
  return (__m256i) __builtin_ia32_pmovsqd512_mask ((__v8di) __A,
               (__v8si) _mm256_undefined_si256 (),
               (__mmask8) -1);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS
_mm512_mask_cvtsepi64_epi32 (__m256i __O, __mmask8 __M, __m512i __A)
{
  return (__m256i) __builtin_ia32_pmovsqd512_mask ((__v8di) __A,
               (__v8si) __O, __M);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS
_mm512_maskz_cvtsepi64_epi32 (__mmask8 __M, __m512i __A)
{
  return (__m256i) __builtin_ia32_pmovsqd512_mask ((__v8di) __A,
               (__v8si) _mm256_setzero_si256 (),
               __M);
}

static __inline__ void __DEFAULT_FN_ATTRS
_mm512_mask_cvtsepi64_storeu_epi32 (void *__P, __mmask8 __M, __m512i __A)
{
  __builtin_ia32_pmovsqd512mem_mask ((__v8si *) __P, (__v8di) __A, __M);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm512_cvtsepi64_epi16 (__m512i __A)
{
  return (__m128i) __builtin_ia32_pmovsqw512_mask ((__v8di) __A,
               (__v8hi) _mm_undefined_si128 (),
               (__mmask8) -1);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm512_mask_cvtsepi64_epi16 (__m128i __O, __mmask8 __M, __m512i __A)
{
  return (__m128i) __builtin_ia32_pmovsqw512_mask ((__v8di) __A,
               (__v8hi) __O, __M);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm512_maskz_cvtsepi64_epi16 (__mmask8 __M, __m512i __A)
{
  return (__m128i) __builtin_ia32_pmovsqw512_mask ((__v8di) __A,
               (__v8hi) _mm_setzero_si128 (),
               __M);
}

static __inline__ void __DEFAULT_FN_ATTRS
_mm512_mask_cvtsepi64_storeu_epi16 (void * __P, __mmask8 __M, __m512i __A)
{
  __builtin_ia32_pmovsqw512mem_mask ((__v8hi *) __P, (__v8di) __A, __M);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm512_cvtusepi32_epi8 (__m512i __A)
{
  return (__m128i) __builtin_ia32_pmovusdb512_mask ((__v16si) __A,
                (__v16qi) _mm_undefined_si128 (),
                (__mmask16) -1);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm512_mask_cvtusepi32_epi8 (__m128i __O, __mmask16 __M, __m512i __A)
{
  return (__m128i) __builtin_ia32_pmovusdb512_mask ((__v16si) __A,
                (__v16qi) __O,
                __M);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm512_maskz_cvtusepi32_epi8 (__mmask16 __M, __m512i __A)
{
  return (__m128i) __builtin_ia32_pmovusdb512_mask ((__v16si) __A,
                (__v16qi) _mm_setzero_si128 (),
                __M);
}

static __inline__ void __DEFAULT_FN_ATTRS
_mm512_mask_cvtusepi32_storeu_epi8 (void * __P, __mmask16 __M, __m512i __A)
{
  __builtin_ia32_pmovusdb512mem_mask ((__v16qi *) __P, (__v16si) __A, __M);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS
_mm512_cvtusepi32_epi16 (__m512i __A)
{
  return (__m256i) __builtin_ia32_pmovusdw512_mask ((__v16si) __A,
                (__v16hi) _mm256_undefined_si256 (),
                (__mmask16) -1);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS
_mm512_mask_cvtusepi32_epi16 (__m256i __O, __mmask16 __M, __m512i __A)
{
  return (__m256i) __builtin_ia32_pmovusdw512_mask ((__v16si) __A,
                (__v16hi) __O,
                __M);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS
_mm512_maskz_cvtusepi32_epi16 (__mmask16 __M, __m512i __A)
{
  return (__m256i) __builtin_ia32_pmovusdw512_mask ((__v16si) __A,
                (__v16hi) _mm256_setzero_si256 (),
                __M);
}

static __inline__ void __DEFAULT_FN_ATTRS
_mm512_mask_cvtusepi32_storeu_epi16 (void *__P, __mmask16 __M, __m512i __A)
{
  __builtin_ia32_pmovusdw512mem_mask ((__v16hi*) __P, (__v16si) __A, __M);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm512_cvtusepi64_epi8 (__m512i __A)
{
  return (__m128i) __builtin_ia32_pmovusqb512_mask ((__v8di) __A,
                (__v16qi) _mm_undefined_si128 (),
                (__mmask8) -1);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm512_mask_cvtusepi64_epi8 (__m128i __O, __mmask8 __M, __m512i __A)
{
  return (__m128i) __builtin_ia32_pmovusqb512_mask ((__v8di) __A,
                (__v16qi) __O,
                __M);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm512_maskz_cvtusepi64_epi8 (__mmask8 __M, __m512i __A)
{
  return (__m128i) __builtin_ia32_pmovusqb512_mask ((__v8di) __A,
                (__v16qi) _mm_setzero_si128 (),
                __M);
}

static __inline__ void __DEFAULT_FN_ATTRS
_mm512_mask_cvtusepi64_storeu_epi8 (void * __P, __mmask8 __M, __m512i __A)
{
  __builtin_ia32_pmovusqb512mem_mask ((__v16qi *) __P, (__v8di) __A, __M);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS
_mm512_cvtusepi64_epi32 (__m512i __A)
{
  return (__m256i) __builtin_ia32_pmovusqd512_mask ((__v8di) __A,
                (__v8si) _mm256_undefined_si256 (),
                (__mmask8) -1);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS
_mm512_mask_cvtusepi64_epi32 (__m256i __O, __mmask8 __M, __m512i __A)
{
  return (__m256i) __builtin_ia32_pmovusqd512_mask ((__v8di) __A,
                (__v8si) __O, __M);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS
_mm512_maskz_cvtusepi64_epi32 (__mmask8 __M, __m512i __A)
{
  return (__m256i) __builtin_ia32_pmovusqd512_mask ((__v8di) __A,
                (__v8si) _mm256_setzero_si256 (),
                __M);
}

static __inline__ void __DEFAULT_FN_ATTRS
_mm512_mask_cvtusepi64_storeu_epi32 (void* __P, __mmask8 __M, __m512i __A)
{
  __builtin_ia32_pmovusqd512mem_mask ((__v8si*) __P, (__v8di) __A, __M);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm512_cvtusepi64_epi16 (__m512i __A)
{
  return (__m128i) __builtin_ia32_pmovusqw512_mask ((__v8di) __A,
                (__v8hi) _mm_undefined_si128 (),
                (__mmask8) -1);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm512_mask_cvtusepi64_epi16 (__m128i __O, __mmask8 __M, __m512i __A)
{
  return (__m128i) __builtin_ia32_pmovusqw512_mask ((__v8di) __A,
                (__v8hi) __O, __M);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm512_maskz_cvtusepi64_epi16 (__mmask8 __M, __m512i __A)
{
  return (__m128i) __builtin_ia32_pmovusqw512_mask ((__v8di) __A,
                (__v8hi) _mm_setzero_si128 (),
                __M);
}

static __inline__ void __DEFAULT_FN_ATTRS
_mm512_mask_cvtusepi64_storeu_epi16 (void *__P, __mmask8 __M, __m512i __A)
{
  __builtin_ia32_pmovusqw512mem_mask ((__v8hi*) __P, (__v8di) __A, __M);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm512_cvtepi32_epi8 (__m512i __A)
{
  return (__m128i) __builtin_ia32_pmovdb512_mask ((__v16si) __A,
              (__v16qi) _mm_undefined_si128 (),
              (__mmask16) -1);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm512_mask_cvtepi32_epi8 (__m128i __O, __mmask16 __M, __m512i __A)
{
  return (__m128i) __builtin_ia32_pmovdb512_mask ((__v16si) __A,
              (__v16qi) __O, __M);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm512_maskz_cvtepi32_epi8 (__mmask16 __M, __m512i __A)
{
  return (__m128i) __builtin_ia32_pmovdb512_mask ((__v16si) __A,
              (__v16qi) _mm_setzero_si128 (),
              __M);
}

static __inline__ void __DEFAULT_FN_ATTRS
_mm512_mask_cvtepi32_storeu_epi8 (void * __P, __mmask16 __M, __m512i __A)
{
  __builtin_ia32_pmovdb512mem_mask ((__v16qi *) __P, (__v16si) __A, __M);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS
_mm512_cvtepi32_epi16 (__m512i __A)
{
  return (__m256i) __builtin_ia32_pmovdw512_mask ((__v16si) __A,
              (__v16hi) _mm256_undefined_si256 (),
              (__mmask16) -1);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS
_mm512_mask_cvtepi32_epi16 (__m256i __O, __mmask16 __M, __m512i __A)
{
  return (__m256i) __builtin_ia32_pmovdw512_mask ((__v16si) __A,
              (__v16hi) __O, __M);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS
_mm512_maskz_cvtepi32_epi16 (__mmask16 __M, __m512i __A)
{
  return (__m256i) __builtin_ia32_pmovdw512_mask ((__v16si) __A,
              (__v16hi) _mm256_setzero_si256 (),
              __M);
}

static __inline__ void __DEFAULT_FN_ATTRS
_mm512_mask_cvtepi32_storeu_epi16 (void * __P, __mmask16 __M, __m512i __A)
{
  __builtin_ia32_pmovdw512mem_mask ((__v16hi *) __P, (__v16si) __A, __M);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm512_cvtepi64_epi8 (__m512i __A)
{
  return (__m128i) __builtin_ia32_pmovqb512_mask ((__v8di) __A,
              (__v16qi) _mm_undefined_si128 (),
              (__mmask8) -1);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm512_mask_cvtepi64_epi8 (__m128i __O, __mmask8 __M, __m512i __A)
{
  return (__m128i) __builtin_ia32_pmovqb512_mask ((__v8di) __A,
              (__v16qi) __O, __M);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm512_maskz_cvtepi64_epi8 (__mmask8 __M, __m512i __A)
{
  return (__m128i) __builtin_ia32_pmovqb512_mask ((__v8di) __A,
              (__v16qi) _mm_setzero_si128 (),
              __M);
}

static __inline__ void __DEFAULT_FN_ATTRS
_mm512_mask_cvtepi64_storeu_epi8 (void * __P, __mmask8 __M, __m512i __A)
{
  __builtin_ia32_pmovqb512mem_mask ((__v16qi *) __P, (__v8di) __A, __M);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS
_mm512_cvtepi64_epi32 (__m512i __A)
{
  return (__m256i) __builtin_ia32_pmovqd512_mask ((__v8di) __A,
              (__v8si) _mm256_undefined_si256 (),
              (__mmask8) -1);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS
_mm512_mask_cvtepi64_epi32 (__m256i __O, __mmask8 __M, __m512i __A)
{
  return (__m256i) __builtin_ia32_pmovqd512_mask ((__v8di) __A,
              (__v8si) __O, __M);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS
_mm512_maskz_cvtepi64_epi32 (__mmask8 __M, __m512i __A)
{
  return (__m256i) __builtin_ia32_pmovqd512_mask ((__v8di) __A,
              (__v8si) _mm256_setzero_si256 (),
              __M);
}

static __inline__ void __DEFAULT_FN_ATTRS
_mm512_mask_cvtepi64_storeu_epi32 (void* __P, __mmask8 __M, __m512i __A)
{
  __builtin_ia32_pmovqd512mem_mask ((__v8si *) __P, (__v8di) __A, __M);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm512_cvtepi64_epi16 (__m512i __A)
{
  return (__m128i) __builtin_ia32_pmovqw512_mask ((__v8di) __A,
              (__v8hi) _mm_undefined_si128 (),
              (__mmask8) -1);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm512_mask_cvtepi64_epi16 (__m128i __O, __mmask8 __M, __m512i __A)
{
  return (__m128i) __builtin_ia32_pmovqw512_mask ((__v8di) __A,
              (__v8hi) __O, __M);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm512_maskz_cvtepi64_epi16 (__mmask8 __M, __m512i __A)
{
  return (__m128i) __builtin_ia32_pmovqw512_mask ((__v8di) __A,
              (__v8hi) _mm_setzero_si128 (),
              __M);
}

static __inline__ void __DEFAULT_FN_ATTRS
_mm512_mask_cvtepi64_storeu_epi16 (void *__P, __mmask8 __M, __m512i __A)
{
  __builtin_ia32_pmovqw512mem_mask ((__v8hi *) __P, (__v8di) __A, __M);
}

#define _mm512_extracti32x4_epi32(A, imm) __extension__ ({ \
  (__m128i)__builtin_ia32_extracti32x4_mask((__v16si)(__m512i)(A), (int)(imm), \
                                            (__v4si)_mm_undefined_si128(), \
                                            (__mmask8)-1); })

#define _mm512_mask_extracti32x4_epi32(W, U, A, imm) __extension__ ({ \
  (__m128i)__builtin_ia32_extracti32x4_mask((__v16si)(__m512i)(A), (int)(imm), \
                                            (__v4si)(__m128i)(W), \
                                            (__mmask8)(U)); })

#define _mm512_maskz_extracti32x4_epi32(U, A, imm) __extension__ ({ \
  (__m128i)__builtin_ia32_extracti32x4_mask((__v16si)(__m512i)(A), (int)(imm), \
                                            (__v4si)_mm_setzero_si128(), \
                                            (__mmask8)(U)); })

#define _mm512_extracti64x4_epi64(A, imm) __extension__ ({ \
  (__m256i)__builtin_ia32_extracti64x4_mask((__v8di)(__m512i)(A), (int)(imm), \
                                            (__v4di)_mm256_undefined_si256(), \
                                            (__mmask8)-1); })

#define _mm512_mask_extracti64x4_epi64(W, U, A, imm) __extension__ ({ \
  (__m256i)__builtin_ia32_extracti64x4_mask((__v8di)(__m512i)(A), (int)(imm), \
                                            (__v4di)(__m256i)(W), \
                                            (__mmask8)(U)); })

#define _mm512_maskz_extracti64x4_epi64(U, A, imm) __extension__ ({ \
  (__m256i)__builtin_ia32_extracti64x4_mask((__v8di)(__m512i)(A), (int)(imm), \
                                            (__v4di)_mm256_setzero_si256(), \
                                            (__mmask8)(U)); })

#define _mm512_insertf64x4(A, B, imm) __extension__ ({ \
  (__m512d)__builtin_ia32_insertf64x4_mask((__v8df)(__m512d)(A), \
                                           (__v4df)(__m256d)(B), (int)(imm), \
                                           (__v8df)_mm512_undefined_pd(), \
                                           (__mmask8)-1); })

#define _mm512_mask_insertf64x4(W, U, A, B, imm) __extension__ ({ \
  (__m512d)__builtin_ia32_insertf64x4_mask((__v8df)(__m512d)(A), \
                                           (__v4df)(__m256d)(B), (int)(imm), \
                                           (__v8df)(__m512d)(W), \
                                           (__mmask8)(U)); })

#define _mm512_maskz_insertf64x4(U, A, B, imm) __extension__ ({ \
  (__m512d)__builtin_ia32_insertf64x4_mask((__v8df)(__m512d)(A), \
                                           (__v4df)(__m256d)(B), (int)(imm), \
                                           (__v8df)_mm512_setzero_pd(), \
                                           (__mmask8)(U)); })

#define _mm512_inserti64x4(A, B, imm) __extension__ ({ \
  (__m512i)__builtin_ia32_inserti64x4_mask((__v8di)(__m512i)(A), \
                                           (__v4di)(__m256i)(B), (int)(imm), \
                                           (__v8di)_mm512_setzero_si512(), \
                                           (__mmask8)-1); })

#define _mm512_mask_inserti64x4(W, U, A, B, imm) __extension__ ({ \
  (__m512i)__builtin_ia32_inserti64x4_mask((__v8di)(__m512i)(A), \
                                           (__v4di)(__m256i)(B), (int)(imm), \
                                           (__v8di)(__m512i)(W), \
                                           (__mmask8)(U)); })

#define _mm512_maskz_inserti64x4(U, A, B, imm) __extension__ ({ \
  (__m512i)__builtin_ia32_inserti64x4_mask((__v8di)(__m512i)(A), \
                                           (__v4di)(__m256i)(B), (int)(imm), \
                                           (__v8di)_mm512_setzero_si512(), \
                                           (__mmask8)(U)); })

#define _mm512_insertf32x4(A, B, imm) __extension__ ({ \
  (__m512)__builtin_ia32_insertf32x4_mask((__v16sf)(__m512)(A), \
                                          (__v4sf)(__m128)(B), (int)(imm), \
                                          (__v16sf)_mm512_undefined_ps(), \
                                          (__mmask16)-1); })

#define _mm512_mask_insertf32x4(W, U, A, B, imm) __extension__ ({ \
  (__m512)__builtin_ia32_insertf32x4_mask((__v16sf)(__m512)(A), \
                                          (__v4sf)(__m128)(B), (int)(imm), \
                                          (__v16sf)(__m512)(W), \
                                          (__mmask16)(U)); })

#define _mm512_maskz_insertf32x4(U, A, B, imm) __extension__ ({ \
  (__m512)__builtin_ia32_insertf32x4_mask((__v16sf)(__m512)(A), \
                                          (__v4sf)(__m128)(B), (int)(imm), \
                                          (__v16sf)_mm512_setzero_ps(), \
                                          (__mmask16)(U)); })

#define _mm512_inserti32x4(A, B, imm) __extension__ ({ \
  (__m512i)__builtin_ia32_inserti32x4_mask((__v16si)(__m512i)(A), \
                                           (__v4si)(__m128i)(B), (int)(imm), \
                                           (__v16si)_mm512_setzero_si512(), \
                                           (__mmask16)-1); })

#define _mm512_mask_inserti32x4(W, U, A, B, imm) __extension__ ({ \
  (__m512i)__builtin_ia32_inserti32x4_mask((__v16si)(__m512i)(A), \
                                           (__v4si)(__m128i)(B), (int)(imm), \
                                           (__v16si)(__m512i)(W), \
                                           (__mmask16)(U)); })

#define _mm512_maskz_inserti32x4(U, A, B, imm) __extension__ ({ \
  (__m512i)__builtin_ia32_inserti32x4_mask((__v16si)(__m512i)(A), \
                                           (__v4si)(__m128i)(B), (int)(imm), \
                                           (__v16si)_mm512_setzero_si512(), \
                                           (__mmask16)(U)); })

#define _mm512_getmant_round_pd(A, B, C, R) __extension__ ({ \
  (__m512d)__builtin_ia32_getmantpd512_mask((__v8df)(__m512d)(A), \
                                            (int)(((C)<<2) | (B)), \
                                            (__v8df)_mm512_undefined_pd(), \
                                            (__mmask8)-1, (int)(R)); })

#define _mm512_mask_getmant_round_pd(W, U, A, B, C, R) __extension__ ({ \
  (__m512d)__builtin_ia32_getmantpd512_mask((__v8df)(__m512d)(A), \
                                            (int)(((C)<<2) | (B)), \
                                            (__v8df)(__m512d)(W), \
                                            (__mmask8)(U), (int)(R)); })

#define _mm512_maskz_getmant_round_pd(U, A, B, C, R) __extension__ ({ \
  (__m512d)__builtin_ia32_getmantpd512_mask((__v8df)(__m512d)(A), \
                                            (int)(((C)<<2) | (B)), \
                                            (__v8df)_mm512_setzero_pd(), \
                                            (__mmask8)(U), (int)(R)); })

#define _mm512_getmant_pd(A, B, C) __extension__ ({ \
  (__m512d)__builtin_ia32_getmantpd512_mask((__v8df)(__m512d)(A), \
                                            (int)(((C)<<2) | (B)), \
                                            (__v8df)_mm512_setzero_pd(), \
                                            (__mmask8)-1, \
                                            _MM_FROUND_CUR_DIRECTION); })

#define _mm512_mask_getmant_pd(W, U, A, B, C) __extension__ ({ \
  (__m512d)__builtin_ia32_getmantpd512_mask((__v8df)(__m512d)(A), \
                                            (int)(((C)<<2) | (B)), \
                                            (__v8df)(__m512d)(W), \
                                            (__mmask8)(U), \
                                            _MM_FROUND_CUR_DIRECTION); })

#define _mm512_maskz_getmant_pd(U, A, B, C) __extension__ ({ \
  (__m512d)__builtin_ia32_getmantpd512_mask((__v8df)(__m512d)(A), \
                                            (int)(((C)<<2) | (B)), \
                                            (__v8df)_mm512_setzero_pd(), \
                                            (__mmask8)(U), \
                                            _MM_FROUND_CUR_DIRECTION); })

#define _mm512_getmant_round_ps(A, B, C, R) __extension__ ({ \
  (__m512)__builtin_ia32_getmantps512_mask((__v16sf)(__m512)(A), \
                                           (int)(((C)<<2) | (B)), \
                                           (__v16sf)_mm512_undefined_ps(), \
                                           (__mmask16)-1, (int)(R)); })

#define _mm512_mask_getmant_round_ps(W, U, A, B, C, R) __extension__ ({ \
  (__m512)__builtin_ia32_getmantps512_mask((__v16sf)(__m512)(A), \
                                           (int)(((C)<<2) | (B)), \
                                           (__v16sf)(__m512)(W), \
                                           (__mmask16)(U), (int)(R)); })

#define _mm512_maskz_getmant_round_ps(U, A, B, C, R) __extension__ ({ \
  (__m512)__builtin_ia32_getmantps512_mask((__v16sf)(__m512)(A), \
                                           (int)(((C)<<2) | (B)), \
                                           (__v16sf)_mm512_setzero_ps(), \
                                           (__mmask16)(U), (int)(R)); })

#define _mm512_getmant_ps(A, B, C) __extension__ ({ \
  (__m512)__builtin_ia32_getmantps512_mask((__v16sf)(__m512)(A), \
                                           (int)(((C)<<2)|(B)), \
                                           (__v16sf)_mm512_undefined_ps(), \
                                           (__mmask16)-1, \
                                           _MM_FROUND_CUR_DIRECTION); })

#define _mm512_mask_getmant_ps(W, U, A, B, C) __extension__ ({ \
  (__m512)__builtin_ia32_getmantps512_mask((__v16sf)(__m512)(A), \
                                           (int)(((C)<<2)|(B)), \
                                           (__v16sf)(__m512)(W), \
                                           (__mmask16)(U), \
                                           _MM_FROUND_CUR_DIRECTION); })

#define _mm512_maskz_getmant_ps(U, A, B, C) __extension__ ({ \
  (__m512)__builtin_ia32_getmantps512_mask((__v16sf)(__m512)(A), \
                                           (int)(((C)<<2)|(B)), \
                                           (__v16sf)_mm512_setzero_ps(), \
                                           (__mmask16)(U), \
                                           _MM_FROUND_CUR_DIRECTION); })

#define _mm512_getexp_round_pd(A, R) __extension__ ({ \
  (__m512d)__builtin_ia32_getexppd512_mask((__v8df)(__m512d)(A), \
                                           (__v8df)_mm512_undefined_pd(), \
                                           (__mmask8)-1, (int)(R)); })

#define _mm512_mask_getexp_round_pd(W, U, A, R) __extension__ ({ \
  (__m512d)__builtin_ia32_getexppd512_mask((__v8df)(__m512d)(A), \
                                           (__v8df)(__m512d)(W), \
                                           (__mmask8)(U), (int)(R)); })

#define _mm512_maskz_getexp_round_pd(U, A, R) __extension__ ({ \
  (__m512d)__builtin_ia32_getexppd512_mask((__v8df)(__m512d)(A), \
                                           (__v8df)_mm512_setzero_pd(), \
                                           (__mmask8)(U), (int)(R)); })

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_getexp_pd (__m512d __A)
{
  return (__m512d) __builtin_ia32_getexppd512_mask ((__v8df) __A,
                (__v8df) _mm512_undefined_pd (),
                (__mmask8) -1,
                _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_mask_getexp_pd (__m512d __W, __mmask8 __U, __m512d __A)
{
  return (__m512d) __builtin_ia32_getexppd512_mask ((__v8df) __A,
                (__v8df) __W,
                (__mmask8) __U,
                _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_maskz_getexp_pd (__mmask8 __U, __m512d __A)
{
  return (__m512d) __builtin_ia32_getexppd512_mask ((__v8df) __A,
                (__v8df) _mm512_setzero_pd (),
                (__mmask8) __U,
                _MM_FROUND_CUR_DIRECTION);
}

#define _mm512_getexp_round_ps(A, R) __extension__ ({ \
  (__m512)__builtin_ia32_getexpps512_mask((__v16sf)(__m512)(A), \
                                          (__v16sf)_mm512_undefined_ps(), \
                                          (__mmask16)-1, (int)(R)); })

#define _mm512_mask_getexp_round_ps(W, U, A, R) __extension__ ({ \
  (__m512)__builtin_ia32_getexpps512_mask((__v16sf)(__m512)(A), \
                                          (__v16sf)(__m512)(W), \
                                          (__mmask16)(U), (int)(R)); })

#define _mm512_maskz_getexp_round_ps(U, A, R) __extension__ ({ \
  (__m512)__builtin_ia32_getexpps512_mask((__v16sf)(__m512)(A), \
                                          (__v16sf)_mm512_setzero_ps(), \
                                          (__mmask16)(U), (int)(R)); })

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_getexp_ps (__m512 __A)
{
  return (__m512) __builtin_ia32_getexpps512_mask ((__v16sf) __A,
               (__v16sf) _mm512_undefined_ps (),
               (__mmask16) -1,
               _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_mask_getexp_ps (__m512 __W, __mmask16 __U, __m512 __A)
{
  return (__m512) __builtin_ia32_getexpps512_mask ((__v16sf) __A,
               (__v16sf) __W,
               (__mmask16) __U,
               _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_maskz_getexp_ps (__mmask16 __U, __m512 __A)
{
  return (__m512) __builtin_ia32_getexpps512_mask ((__v16sf) __A,
               (__v16sf) _mm512_setzero_ps (),
               (__mmask16) __U,
               _MM_FROUND_CUR_DIRECTION);
}

#define _mm512_i64gather_ps(index, addr, scale) __extension__ ({ \
  (__m256)__builtin_ia32_gatherdiv16sf((__v8sf)_mm256_undefined_ps(), \
                                       (float const *)(addr), \
                                       (__v8di)(__m512i)(index), (__mmask8)-1, \
                                       (int)(scale)); })

#define _mm512_mask_i64gather_ps( __v1_old, __mask, __index,\
                                  __addr, __scale) __extension__({\
__builtin_ia32_gatherdiv16sf ((__v8sf) __v1_old,\
                              __addr,(__v8di) __index, __mask, __scale);\
})

#define _mm512_i64gather_epi32(index, addr, scale) __extension__ ({\
  (__m256i)__builtin_ia32_gatherdiv16si((__v8si)_mm256_undefined_ps(), \
                                        (int const *)(addr), \
                                        (__v8di)(__m512i)(index), \
                                        (__mmask8)-1, (int)(scale)); })

#define _mm512_mask_i64gather_epi32(v1_old, mask, index, addr, scale) __extension__ ({\
  (__m256i)__builtin_ia32_gatherdiv16si((__v8si)(__m256i)(v1_old), \
                                        (int const *)(addr), \
                                        (__v8di)(__m512i)(index), \
                                        (__mmask8)(mask), (int)(scale)); })

#define _mm512_i64gather_pd(index, addr, scale) __extension__ ({\
  (__m512d)__builtin_ia32_gatherdiv8df((__v8df)_mm512_undefined_pd(), \
                                       (double const *)(addr), \
                                       (__v8di)(__m512i)(index), (__mmask8)-1, \
                                       (int)(scale)); })

#define _mm512_mask_i64gather_pd(v1_old, mask, index, addr, scale) __extension__ ({\
  (__m512d)__builtin_ia32_gatherdiv8df((__v8df)(__m512d)(v1_old), \
                                       (double const *)(addr), \
                                       (__v8di)(__m512i)(index), \
                                       (__mmask8)(mask), (int)(scale)); })

#define _mm512_i64gather_epi64(index, addr, scale) __extension__ ({\
  (__m512i)__builtin_ia32_gatherdiv8di((__v8di)_mm512_undefined_pd(), \
                                       (long long const *)(addr), \
                                       (__v8di)(__m512i)(index), (__mmask8)-1, \
                                       (int)(scale)); })

#define _mm512_mask_i64gather_epi64(v1_old, mask, index, addr, scale) __extension__ ({\
  (__m512i)__builtin_ia32_gatherdiv8di((__v8di)(__m512i)(v1_old), \
                                       (long long const *)(addr), \
                                       (__v8di)(__m512i)(index), \
                                       (__mmask8)(mask), (int)(scale)); })

#define _mm512_i32gather_ps(index, addr, scale) __extension__ ({\
  (__m512)__builtin_ia32_gathersiv16sf((__v16sf)_mm512_undefined_ps(), \
                                       (float const *)(addr), \
                                       (__v16sf)(__m512)(index), \
                                       (__mmask16)-1, (int)(scale)); })

#define _mm512_mask_i32gather_ps(v1_old, mask, index, addr, scale) __extension__ ({\
  (__m512)__builtin_ia32_gathersiv16sf((__v16sf)(__m512)(v1_old), \
                                       (float const *)(addr), \
                                       (__v16sf)(__m512)(index), \
                                       (__mmask16)(mask), (int)(scale)); })

#define _mm512_i32gather_epi32(index, addr, scale) __extension__ ({\
  (__m512i)__builtin_ia32_gathersiv16si((__v16si)_mm512_undefined_epi32(), \
                                        (int const *)(addr), \
                                        (__v16si)(__m512i)(index), \
                                        (__mmask16)-1, (int)(scale)); })

#define _mm512_mask_i32gather_epi32(v1_old, mask, index, addr, scale) __extension__ ({\
  (__m512i)__builtin_ia32_gathersiv16si((__v16si)(__m512i)(v1_old), \
                                        (int const *)(addr), \
                                        (__v16si)(__m512i)(index), \
                                        (__mmask16)(mask), (int)(scale)); })

#define _mm512_i32gather_pd(index, addr, scale) __extension__ ({\
  (__m512d)__builtin_ia32_gathersiv8df((__v8df)_mm512_undefined_pd(), \
                                       (double const *)(addr), \
                                       (__v8si)(__m256i)(index), (__mmask8)-1, \
                                       (int)(scale)); })

#define _mm512_mask_i32gather_pd(v1_old, mask, index, addr, scale) __extension__ ({\
  (__m512d)__builtin_ia32_gathersiv8df((__v8df)(__m512d)(v1_old), \
                                       (double const *)(addr), \
                                       (__v8si)(__m256i)(index), \
                                       (__mmask8)(mask), (int)(scale)); })

#define _mm512_i32gather_epi64(index, addr, scale) __extension__ ({\
  (__m512i)__builtin_ia32_gathersiv8di((__v8di)_mm512_undefined_epi32(), \
                                       (long long const *)(addr), \
                                       (__v8si)(__m256i)(index), (__mmask8)-1, \
                                       (int)(scale)); })

#define _mm512_mask_i32gather_epi64(v1_old, mask, index, addr, scale) __extension__ ({\
  (__m512i)__builtin_ia32_gathersiv8di((__v8di)(__m512i)(v1_old), \
                                       (long long const *)(addr), \
                                       (__v8si)(__m256i)(index), \
                                       (__mmask8)(mask), (int)(scale)); })

#define _mm512_i64scatter_ps(addr, index, v1, scale) __extension__ ({\
  __builtin_ia32_scatterdiv16sf((float *)(addr), (__mmask8)-1, \
                                (__v8di)(__m512i)(index), \
                                (__v8sf)(__m256)(v1), (int)(scale)); })

#define _mm512_mask_i64scatter_ps(addr, mask, index, v1, scale) __extension__ ({\
  __builtin_ia32_scatterdiv16sf((float *)(addr), (__mmask8)(mask), \
                                (__v8di)(__m512i)(index), \
                                (__v8sf)(__m256)(v1), (int)(scale)); })

#define _mm512_i64scatter_epi32(addr, index, v1, scale) __extension__ ({\
  __builtin_ia32_scatterdiv16si((int *)(addr), (__mmask8)-1, \
                                (__v8di)(__m512i)(index), \
                                (__v8si)(__m256i)(v1), (int)(scale)); })

#define _mm512_mask_i64scatter_epi32(addr, mask, index, v1, scale) __extension__ ({\
  __builtin_ia32_scatterdiv16si((int *)(addr), (__mmask8)(mask), \
                                (__v8di)(__m512i)(index), \
                                (__v8si)(__m256i)(v1), (int)(scale)); })

#define _mm512_i64scatter_pd(addr, index, v1, scale) __extension__ ({\
  __builtin_ia32_scatterdiv8df((double *)(addr), (__mmask8)-1, \
                               (__v8di)(__m512i)(index), \
                               (__v8df)(__m512d)(v1), (int)(scale)); })

#define _mm512_mask_i64scatter_pd(addr, mask, index, v1, scale) __extension__ ({\
  __builtin_ia32_scatterdiv8df((double *)(addr), (__mmask8)(mask), \
                               (__v8di)(__m512i)(index), \
                               (__v8df)(__m512d)(v1), (int)(scale)); })

#define _mm512_i64scatter_epi64(addr, index, v1, scale) __extension__ ({\
  __builtin_ia32_scatterdiv8di((long long *)(addr), (__mmask8)-1, \
                               (__v8di)(__m512i)(index), \
                               (__v8di)(__m512i)(v1), (int)(scale)); })

#define _mm512_mask_i64scatter_epi64(addr, mask, index, v1, scale) __extension__ ({\
  __builtin_ia32_scatterdiv8di((long long *)(addr), (__mmask8)(mask), \
                               (__v8di)(__m512i)(index), \
                               (__v8di)(__m512i)(v1), (int)(scale)); })

#define _mm512_i32scatter_ps(addr, index, v1, scale) __extension__ ({\
  __builtin_ia32_scattersiv16sf((float *)(addr), (__mmask16)-1, \
                                (__v16si)(__m512i)(index), \
                                (__v16sf)(__m512)(v1), (int)(scale)); })

#define _mm512_mask_i32scatter_ps(addr, mask, index, v1, scale) __extension__ ({\
  __builtin_ia32_scattersiv16sf((float *)(addr), (__mmask16)(mask), \
                                (__v16si)(__m512i)(index), \
                                (__v16sf)(__m512)(v1), (int)(scale)); })

#define _mm512_i32scatter_epi32(addr, index, v1, scale) __extension__ ({\
  __builtin_ia32_scattersiv16si((int *)(addr), (__mmask16)-1, \
                                (__v16si)(__m512i)(index), \
                                (__v16si)(__m512i)(v1), (int)(scale)); })

#define _mm512_mask_i32scatter_epi32(addr, mask, index, v1, scale) __extension__ ({\
  __builtin_ia32_scattersiv16si((int *)(addr), (__mmask16)(mask), \
                                (__v16si)(__m512i)(index), \
                                (__v16si)(__m512i)(v1), (int)(scale)); })

#define _mm512_i32scatter_pd(addr, index, v1, scale) __extension__ ({\
  __builtin_ia32_scattersiv8df((double *)(addr), (__mmask8)-1, \
                               (__v8si)(__m256i)(index), \
                               (__v8df)(__m512d)(v1), (int)(scale)); })

#define _mm512_mask_i32scatter_pd(addr, mask, index, v1, scale) __extension__ ({\
  __builtin_ia32_scattersiv8df((double *)(addr), (__mmask8)(mask), \
                               (__v8si)(__m256i)(index), \
                               (__v8df)(__m512d)(v1), (int)(scale)); })

#define _mm512_i32scatter_epi64(addr, index, v1, scale) __extension__ ({\
  __builtin_ia32_scattersiv8di((long long *)(addr), (__mmask8)-1, \
                               (__v8si)(__m256i)(index), \
                               (__v8di)(__m512i)(v1), (int)(scale)); })

#define _mm512_mask_i32scatter_epi64(addr, mask, index, v1, scale) __extension__ ({\
  __builtin_ia32_scattersiv8di((long long *)(addr), (__mmask8)(mask), \
                               (__v8si)(__m256i)(index), \
                               (__v8di)(__m512i)(v1), (int)(scale)); })

static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_mask_fmadd_ss (__m128 __W, __mmask8 __U, __m128 __A, __m128 __B)
{
 return (__m128) __builtin_ia32_vfmaddss3_mask ((__v4sf) __A,
          (__v4sf) __B,
          (__v4sf) __W,
          (__mmask8) __U,
          _MM_FROUND_CUR_DIRECTION);
}

#define _mm_mask_fmadd_round_ss(W, U, A, B, R) __extension__({\
  (__m128)__builtin_ia32_vfmaddss3_mask((__v4sf)(__m128)(A), \
                                        (__v4sf)(__m128)(B), \
                                        (__v4sf)(__m128)(W), (__mmask8)(U), \
                                        (int)(R)); })

static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_maskz_fmadd_ss (__mmask8 __U, __m128 __A, __m128 __B, __m128 __C)
{
 return (__m128) __builtin_ia32_vfmaddss3_maskz ((__v4sf) __A,
          (__v4sf) __B,
          (__v4sf) __C,
          (__mmask8) __U,
          _MM_FROUND_CUR_DIRECTION);
}

#define _mm_maskz_fmadd_round_ss(U, A, B, C, R) __extension__ ({\
  (__m128)__builtin_ia32_vfmaddss3_maskz((__v4sf)(__m128)(A), \
                                         (__v4sf)(__m128)(B), \
                                         (__v4sf)(__m128)(C), (__mmask8)(U), \
                                         _MM_FROUND_CUR_DIRECTION); })

static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_mask3_fmadd_ss (__m128 __W, __m128 __X, __m128 __Y, __mmask8 __U)
{
 return (__m128) __builtin_ia32_vfmaddss3_mask3 ((__v4sf) __W,
          (__v4sf) __X,
          (__v4sf) __Y,
          (__mmask8) __U,
          _MM_FROUND_CUR_DIRECTION);
}

#define _mm_mask3_fmadd_round_ss(W, X, Y, U, R) __extension__ ({\
  (__m128)__builtin_ia32_vfmaddss3_mask3((__v4sf)(__m128)(W), \
                                         (__v4sf)(__m128)(X), \
                                         (__v4sf)(__m128)(Y), (__mmask8)(U), \
                                         (int)(R)); })

static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_mask_fmsub_ss (__m128 __W, __mmask8 __U, __m128 __A, __m128 __B)
{
 return (__m128) __builtin_ia32_vfmaddss3_mask ((__v4sf) __A,
          -(__v4sf) __B,
          (__v4sf) __W,
          (__mmask8) __U,
          _MM_FROUND_CUR_DIRECTION);
}

#define _mm_mask_fmsub_round_ss(W, U, A, B, R) __extension__ ({\
  (__m128)__builtin_ia32_vfmaddss3_mask((__v4sf)(__m128)(A), \
                                        -(__v4sf)(__m128)(B), \
                                        (__v4sf)(__m128)(W), (__mmask8)(U), \
                                        (int)(R)); })

static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_maskz_fmsub_ss (__mmask8 __U, __m128 __A, __m128 __B, __m128 __C)
{
 return (__m128) __builtin_ia32_vfmaddss3_maskz ((__v4sf) __A,
          (__v4sf) __B,
          -(__v4sf) __C,
          (__mmask8) __U,
          _MM_FROUND_CUR_DIRECTION);
}

#define _mm_maskz_fmsub_round_ss(U, A, B, C, R) __extension__ ({\
  (__m128)__builtin_ia32_vfmaddss3_maskz((__v4sf)(__m128)(A), \
                                         (__v4sf)(__m128)(B), \
                                         -(__v4sf)(__m128)(C), (__mmask8)(U), \
                                         (int)(R)); })

static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_mask3_fmsub_ss (__m128 __W, __m128 __X, __m128 __Y, __mmask8 __U)
{
 return (__m128) __builtin_ia32_vfmaddss3_mask3 ((__v4sf) __W,
          (__v4sf) __X,
          -(__v4sf) __Y,
          (__mmask8) __U,
          _MM_FROUND_CUR_DIRECTION);
}

#define _mm_mask3_fmsub_round_ss(W, X, Y, U, R) __extension__ ({\
  (__m128)__builtin_ia32_vfmaddss3_mask3((__v4sf)(__m128)(W), \
                                         (__v4sf)(__m128)(X), \
                                         -(__v4sf)(__m128)(Y), (__mmask8)(U), \
                                         (int)(R)); })

static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_mask_fnmadd_ss (__m128 __W, __mmask8 __U, __m128 __A, __m128 __B)
{
 return (__m128) __builtin_ia32_vfmaddss3_mask (-(__v4sf) __A,
          (__v4sf) __B,
          (__v4sf) __W,
          (__mmask8) __U,
          _MM_FROUND_CUR_DIRECTION);
}

#define _mm_mask_fnmadd_round_ss(W, U, A, B, R) __extension__ ({\
  (__m128)__builtin_ia32_vfmaddss3_mask(-(__v4sf)(__m128)(A), \
                                        (__v4sf)(__m128)(B), \
                                        (__v4sf)(__m128)(W), (__mmask8)(U), \
                                        (int)(R)); })

static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_maskz_fnmadd_ss (__mmask8 __U, __m128 __A, __m128 __B, __m128 __C)
{
 return (__m128) __builtin_ia32_vfmaddss3_maskz (-(__v4sf) __A,
          (__v4sf) __B,
          (__v4sf) __C,
          (__mmask8) __U,
          _MM_FROUND_CUR_DIRECTION);
}

#define _mm_maskz_fnmadd_round_ss(U, A, B, C, R) __extension__ ({\
  (__m128)__builtin_ia32_vfmaddss3_maskz(-(__v4sf)(__m128)(A), \
                                         (__v4sf)(__m128)(B), \
                                         (__v4sf)(__m128)(C), (__mmask8)(U), \
                                         (int)(R)); })

static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_mask3_fnmadd_ss (__m128 __W, __m128 __X, __m128 __Y, __mmask8 __U)
{
 return (__m128) __builtin_ia32_vfmaddss3_mask3 (-(__v4sf) __W,
          (__v4sf) __X,
          (__v4sf) __Y,
          (__mmask8) __U,
          _MM_FROUND_CUR_DIRECTION);
}

#define _mm_mask3_fnmadd_round_ss(W, X, Y, U, R) __extension__({\
  (__m128)__builtin_ia32_vfmaddss3_mask3(-(__v4sf)(__m128)(W), \
                                         (__v4sf)(__m128)(X), \
                                         (__v4sf)(__m128)(Y), (__mmask8)(U), \
                                         (int)(R)); })

static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_mask_fnmsub_ss (__m128 __W, __mmask8 __U, __m128 __A, __m128 __B)
{
 return (__m128) __builtin_ia32_vfmaddss3_mask (-(__v4sf) __A,
          -(__v4sf) __B,
          (__v4sf) __W,
          (__mmask8) __U,
          _MM_FROUND_CUR_DIRECTION);
}

#define _mm_mask_fnmsub_round_ss(W, U, A, B, R) __extension__ ({\
  (__m128)__builtin_ia32_vfmaddss3_mask(-(__v4sf)(__m128)(A), \
                                        -(__v4sf)(__m128)(B), \
                                        (__v4sf)(__m128)(W), (__mmask8)(U), \
                                        (int)(R)); })

static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_maskz_fnmsub_ss (__mmask8 __U, __m128 __A, __m128 __B, __m128 __C)
{
 return (__m128) __builtin_ia32_vfmaddss3_maskz (-(__v4sf) __A,
          (__v4sf) __B,
          -(__v4sf) __C,
          (__mmask8) __U,
          _MM_FROUND_CUR_DIRECTION);
}

#define _mm_maskz_fnmsub_round_ss(U, A, B, C, R) __extension__ ({\
  (__m128)__builtin_ia32_vfmaddss3_maskz(-(__v4sf)(__m128)(A), \
                                         (__v4sf)(__m128)(B), \
                                         -(__v4sf)(__m128)(C), (__mmask8)(U), \
                                         _MM_FROUND_CUR_DIRECTION); })

static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_mask3_fnmsub_ss (__m128 __W, __m128 __X, __m128 __Y, __mmask8 __U)
{
 return (__m128) __builtin_ia32_vfmaddss3_mask3 (-(__v4sf) __W,
          (__v4sf) __X,
          -(__v4sf) __Y,
          (__mmask8) __U,
          _MM_FROUND_CUR_DIRECTION);
}

#define _mm_mask3_fnmsub_round_ss(W, X, Y, U, R) __extension__({\
  (__m128)__builtin_ia32_vfmaddss3_mask3(-(__v4sf)(__m128)(W), \
                                         (__v4sf)(__m128)(X), \
                                         -(__v4sf)(__m128)(Y), (__mmask8)(U), \
                                         (int)(R)); })

static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_mask_fmadd_sd (__m128d __W, __mmask8 __U, __m128d __A, __m128d __B)
{
 return (__m128d) __builtin_ia32_vfmaddsd3_mask ( (__v2df) __A,
          (__v2df) __B,
          (__v2df) __W,
          (__mmask8) __U,
          _MM_FROUND_CUR_DIRECTION);
}

#define _mm_mask_fmadd_round_sd(W, U, A, B, R) __extension__({\
  (__m128d)__builtin_ia32_vfmaddsd3_mask((__v2df)(__m128d)(A), \
                                         (__v2df)(__m128d)(B), \
                                         (__v2df)(__m128d)(W), (__mmask8)(U), \
                                         (int)(R)); })

static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_maskz_fmadd_sd (__mmask8 __U, __m128d __A, __m128d __B, __m128d __C)
{
 return (__m128d) __builtin_ia32_vfmaddsd3_maskz ( (__v2df) __A,
          (__v2df) __B,
          (__v2df) __C,
          (__mmask8) __U,
          _MM_FROUND_CUR_DIRECTION);
}

#define _mm_maskz_fmadd_round_sd(U, A, B, C, R) __extension__ ({\
  (__m128d)__builtin_ia32_vfmaddsd3_maskz((__v2df)(__m128d)(A), \
                                          (__v2df)(__m128d)(B), \
                                          (__v2df)(__m128d)(C), (__mmask8)(U), \
                                          _MM_FROUND_CUR_DIRECTION); })

static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_mask3_fmadd_sd (__m128d __W, __m128d __X, __m128d __Y, __mmask8 __U)
{
 return (__m128d) __builtin_ia32_vfmaddsd3_mask3 ((__v2df) __W,
          (__v2df) __X,
          (__v2df) __Y,
          (__mmask8) __U,
          _MM_FROUND_CUR_DIRECTION);
}

#define _mm_mask3_fmadd_round_sd(W, X, Y, U, R) __extension__ ({\
  (__m128d)__builtin_ia32_vfmaddsd3_mask3((__v2df)(__m128d)(W), \
                                          (__v2df)(__m128d)(X), \
                                          (__v2df)(__m128d)(Y), (__mmask8)(U), \
                                          (int)(R)); })

static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_mask_fmsub_sd (__m128d __W, __mmask8 __U, __m128d __A, __m128d __B)
{
 return (__m128d) __builtin_ia32_vfmaddsd3_mask ( (__v2df) __A,
          -(__v2df) __B,
          (__v2df) __W,
          (__mmask8) __U,
          _MM_FROUND_CUR_DIRECTION);
}

#define _mm_mask_fmsub_round_sd(W, U, A, B, R) __extension__ ({\
  (__m128d)__builtin_ia32_vfmaddsd3_mask((__v2df)(__m128d)(A), \
                                         -(__v2df)(__m128d)(B), \
                                         (__v2df)(__m128d)(W), (__mmask8)(U), \
                                         (int)(R)); })

static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_maskz_fmsub_sd (__mmask8 __U, __m128d __A, __m128d __B, __m128d __C)
{
 return (__m128d) __builtin_ia32_vfmaddsd3_maskz ( (__v2df) __A,
          (__v2df) __B,
          -(__v2df) __C,
          (__mmask8) __U,
          _MM_FROUND_CUR_DIRECTION);
}

#define _mm_maskz_fmsub_round_sd(U, A, B, C, R) __extension__ ({\
  (__m128d)__builtin_ia32_vfmaddsd3_maskz((__v2df)(__m128d)(A), \
                                          (__v2df)(__m128d)(B), \
                                          -(__v2df)(__m128d)(C), \
                                          (__mmask8)(U), (int)(R)); })

static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_mask3_fmsub_sd (__m128d __W, __m128d __X, __m128d __Y, __mmask8 __U)
{
 return (__m128d) __builtin_ia32_vfmaddsd3_mask3 ((__v2df) __W,
          (__v2df) __X,
          -(__v2df) __Y,
          (__mmask8) __U,
          _MM_FROUND_CUR_DIRECTION);
}

#define _mm_mask3_fmsub_round_sd(W, X, Y, U, R) __extension__ ({\
  (__m128d)__builtin_ia32_vfmaddsd3_mask3((__v2df)(__m128d)(W), \
                                          (__v2df)(__m128d)(X), \
                                          -(__v2df)(__m128d)(Y), \
                                          (__mmask8)(U), (int)(R)); })

static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_mask_fnmadd_sd (__m128d __W, __mmask8 __U, __m128d __A, __m128d __B)
{
 return (__m128d) __builtin_ia32_vfmaddsd3_mask ( -(__v2df) __A,
          (__v2df) __B,
          (__v2df) __W,
          (__mmask8) __U,
          _MM_FROUND_CUR_DIRECTION);
}

#define _mm_mask_fnmadd_round_sd(W, U, A, B, R) __extension__ ({\
  (__m128d)__builtin_ia32_vfmaddsd3_mask(-(__v2df)(__m128d)(A), \
                                         (__v2df)(__m128d)(B), \
                                         (__v2df)(__m128d)(W), (__mmask8)(U), \
                                         (int)(R)); })

static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_maskz_fnmadd_sd (__mmask8 __U, __m128d __A, __m128d __B, __m128d __C)
{
 return (__m128d) __builtin_ia32_vfmaddsd3_maskz ( -(__v2df) __A,
          (__v2df) __B,
          (__v2df) __C,
          (__mmask8) __U,
          _MM_FROUND_CUR_DIRECTION);
}

#define _mm_maskz_fnmadd_round_sd(U, A, B, C, R) __extension__ ({\
  (__m128d)__builtin_ia32_vfmaddsd3_maskz(-(__v2df)(__m128d)(A), \
                                          (__v2df)(__m128d)(B), \
                                          (__v2df)(__m128d)(C), (__mmask8)(U), \
                                          (int)(R)); })

static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_mask3_fnmadd_sd (__m128d __W, __m128d __X, __m128d __Y, __mmask8 __U)
{
 return (__m128d) __builtin_ia32_vfmaddsd3_mask3 (-(__v2df) __W,
          (__v2df) __X,
          (__v2df) __Y,
          (__mmask8) __U,
          _MM_FROUND_CUR_DIRECTION);
}

#define _mm_mask3_fnmadd_round_sd(W, X, Y, U, R) __extension__({\
  (__m128d)__builtin_ia32_vfmaddsd3_mask3(-(__v2df)(__m128d)(W), \
                                          (__v2df)(__m128d)(X), \
                                          (__v2df)(__m128d)(Y), (__mmask8)(U), \
                                          (int)(R)); })

static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_mask_fnmsub_sd (__m128d __W, __mmask8 __U, __m128d __A, __m128d __B)
{
 return (__m128d) __builtin_ia32_vfmaddsd3_mask ( -(__v2df) __A,
          -(__v2df) __B,
          (__v2df) __W,
          (__mmask8) __U,
          _MM_FROUND_CUR_DIRECTION);
}

#define _mm_mask_fnmsub_round_sd(W, U, A, B, R) __extension__ ({\
  (__m128d)__builtin_ia32_vfmaddsd3_mask(-(__v2df)(__m128d)(A), \
                                         -(__v2df)(__m128d)(B), \
                                         (__v2df)(__m128d)(W), (__mmask8)(U), \
                                         (int)(R)); })

static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_maskz_fnmsub_sd (__mmask8 __U, __m128d __A, __m128d __B, __m128d __C)
{
 return (__m128d) __builtin_ia32_vfmaddsd3_maskz ( -(__v2df) __A,
          (__v2df) __B,
          -(__v2df) __C,
          (__mmask8) __U,
          _MM_FROUND_CUR_DIRECTION);
}

#define _mm_maskz_fnmsub_round_sd(U, A, B, C, R) __extension__ ({\
  (__m128d)__builtin_ia32_vfmaddsd3_maskz(-(__v2df)(__m128d)(A), \
                                          (__v2df)(__m128d)(B), \
                                          -(__v2df)(__m128d)(C), \
                                          (__mmask8)(U), \
                                          _MM_FROUND_CUR_DIRECTION); })

static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_mask3_fnmsub_sd (__m128d __W, __m128d __X, __m128d __Y, __mmask8 __U)
{
 return (__m128d) __builtin_ia32_vfmaddsd3_mask3 (-(__v2df) (__W),
          (__v2df) __X,
          -(__v2df) (__Y),
          (__mmask8) __U,
          _MM_FROUND_CUR_DIRECTION);
}

#define _mm_mask3_fnmsub_round_sd(W, X, Y, U, R) __extension__({\
  (__m128d)__builtin_ia32_vfmaddsd3_mask3(-(__v2df)(__m128d)(W), \
                                          (__v2df)(__m128d)(X), \
                                          -(__v2df)(__m128d)(Y), \
                                          (__mmask8)(U), (int)(R)); })

#define _mm512_permutex_pd(X, C) __extension__ ({ \
  (__m512d)__builtin_shufflevector((__v8df)(__m512d)(X), \
                                   (__v8df)_mm512_undefined_pd(), \
                                   0 + (((C) >> 0) & 0x3), \
                                   0 + (((C) >> 2) & 0x3), \
                                   0 + (((C) >> 4) & 0x3), \
                                   0 + (((C) >> 6) & 0x3), \
                                   4 + (((C) >> 0) & 0x3), \
                                   4 + (((C) >> 2) & 0x3), \
                                   4 + (((C) >> 4) & 0x3), \
                                   4 + (((C) >> 6) & 0x3)); })

#define _mm512_mask_permutex_pd(W, U, X, C) __extension__ ({ \
  (__m512d)__builtin_ia32_selectpd_512((__mmask8)(U), \
                                       (__v8df)_mm512_permutex_pd((X), (C)), \
                                       (__v8df)(__m512d)(W)); })

#define _mm512_maskz_permutex_pd(U, X, C) __extension__ ({ \
  (__m512d)__builtin_ia32_selectpd_512((__mmask8)(U), \
                                       (__v8df)_mm512_permutex_pd((X), (C)), \
                                       (__v8df)_mm512_setzero_pd()); })

#define _mm512_permutex_epi64(X, C) __extension__ ({ \
  (__m512i)__builtin_shufflevector((__v8di)(__m512i)(X), \
                                   (__v8di)_mm512_undefined_epi32(), \
                                   0 + (((C) >> 0) & 0x3), \
                                   0 + (((C) >> 2) & 0x3), \
                                   0 + (((C) >> 4) & 0x3), \
                                   0 + (((C) >> 6) & 0x3), \
                                   4 + (((C) >> 0) & 0x3), \
                                   4 + (((C) >> 2) & 0x3), \
                                   4 + (((C) >> 4) & 0x3), \
                                   4 + (((C) >> 6) & 0x3)); })

#define _mm512_mask_permutex_epi64(W, U, X, C) __extension__ ({ \
  (__m512i)__builtin_ia32_selectq_512((__mmask8)(U), \
                                      (__v8di)_mm512_permutex_epi64((X), (C)), \
                                      (__v8di)(__m512i)(W)); })

#define _mm512_maskz_permutex_epi64(U, X, C) __extension__ ({ \
  (__m512i)__builtin_ia32_selectq_512((__mmask8)(U), \
                                      (__v8di)_mm512_permutex_epi64((X), (C)), \
                                      (__v8di)_mm512_setzero_si512()); })

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_permutexvar_pd (__m512i __X, __m512d __Y)
{
  return (__m512d) __builtin_ia32_permvardf512_mask ((__v8df) __Y,
                 (__v8di) __X,
                 (__v8df) _mm512_undefined_pd (),
                 (__mmask8) -1);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_mask_permutexvar_pd (__m512d __W, __mmask8 __U, __m512i __X, __m512d __Y)
{
  return (__m512d) __builtin_ia32_permvardf512_mask ((__v8df) __Y,
                 (__v8di) __X,
                 (__v8df) __W,
                 (__mmask8) __U);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_maskz_permutexvar_pd (__mmask8 __U, __m512i __X, __m512d __Y)
{
  return (__m512d) __builtin_ia32_permvardf512_mask ((__v8df) __Y,
                 (__v8di) __X,
                 (__v8df) _mm512_setzero_pd (),
                 (__mmask8) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_permutexvar_epi64 (__mmask8 __M, __m512i __X, __m512i __Y)
{
  return (__m512i) __builtin_ia32_permvardi512_mask ((__v8di) __Y,
                 (__v8di) __X,
                 (__v8di) _mm512_setzero_si512 (),
                 __M);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_permutexvar_epi64 (__m512i __X, __m512i __Y)
{
  return (__m512i) __builtin_ia32_permvardi512_mask ((__v8di) __Y,
                 (__v8di) __X,
                 (__v8di) _mm512_undefined_epi32 (),
                 (__mmask8) -1);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_permutexvar_epi64 (__m512i __W, __mmask8 __M, __m512i __X,
             __m512i __Y)
{
  return (__m512i) __builtin_ia32_permvardi512_mask ((__v8di) __Y,
                 (__v8di) __X,
                 (__v8di) __W,
                 __M);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_permutexvar_ps (__m512i __X, __m512 __Y)
{
  return (__m512) __builtin_ia32_permvarsf512_mask ((__v16sf) __Y,
                (__v16si) __X,
                (__v16sf) _mm512_undefined_ps (),
                (__mmask16) -1);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_mask_permutexvar_ps (__m512 __W, __mmask16 __U, __m512i __X, __m512 __Y)
{
  return (__m512) __builtin_ia32_permvarsf512_mask ((__v16sf) __Y,
                (__v16si) __X,
                (__v16sf) __W,
                (__mmask16) __U);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_maskz_permutexvar_ps (__mmask16 __U, __m512i __X, __m512 __Y)
{
  return (__m512) __builtin_ia32_permvarsf512_mask ((__v16sf) __Y,
                (__v16si) __X,
                (__v16sf) _mm512_setzero_ps (),
                (__mmask16) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_permutexvar_epi32 (__mmask16 __M, __m512i __X, __m512i __Y)
{
  return (__m512i) __builtin_ia32_permvarsi512_mask ((__v16si) __Y,
                 (__v16si) __X,
                 (__v16si) _mm512_setzero_si512 (),
                 __M);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_permutexvar_epi32 (__m512i __X, __m512i __Y)
{
  return (__m512i) __builtin_ia32_permvarsi512_mask ((__v16si) __Y,
                 (__v16si) __X,
                 (__v16si) _mm512_undefined_epi32 (),
                 (__mmask16) -1);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_permutexvar_epi32 (__m512i __W, __mmask16 __M, __m512i __X,
             __m512i __Y)
{
  return (__m512i) __builtin_ia32_permvarsi512_mask ((__v16si) __Y,
                 (__v16si) __X,
                 (__v16si) __W,
                 __M);
}

static __inline__ __mmask16 __DEFAULT_FN_ATTRS
_mm512_kand (__mmask16 __A, __mmask16 __B)
{
  return (__mmask16) __builtin_ia32_kandhi ((__mmask16) __A, (__mmask16) __B);
}

static __inline__ __mmask16 __DEFAULT_FN_ATTRS
_mm512_kandn (__mmask16 __A, __mmask16 __B)
{
  return (__mmask16) __builtin_ia32_kandnhi ((__mmask16) __A, (__mmask16) __B);
}

static __inline__ __mmask16 __DEFAULT_FN_ATTRS
_mm512_kor (__mmask16 __A, __mmask16 __B)
{
  return (__mmask16) __builtin_ia32_korhi ((__mmask16) __A, (__mmask16) __B);
}

static __inline__ int __DEFAULT_FN_ATTRS
_mm512_kortestc (__mmask16 __A, __mmask16 __B)
{
  return __builtin_ia32_kortestchi ((__mmask16) __A, (__mmask16) __B);
}

static __inline__ int __DEFAULT_FN_ATTRS
_mm512_kortestz (__mmask16 __A, __mmask16 __B)
{
  return __builtin_ia32_kortestzhi ((__mmask16) __A, (__mmask16) __B);
}

static __inline__ __mmask16 __DEFAULT_FN_ATTRS
_mm512_kunpackb (__mmask16 __A, __mmask16 __B)
{
  return (__mmask16) __builtin_ia32_kunpckhi ((__mmask16) __A, (__mmask16) __B);
}

static __inline__ __mmask16 __DEFAULT_FN_ATTRS
_mm512_kxnor (__mmask16 __A, __mmask16 __B)
{
  return (__mmask16) __builtin_ia32_kxnorhi ((__mmask16) __A, (__mmask16) __B);
}

static __inline__ __mmask16 __DEFAULT_FN_ATTRS
_mm512_kxor (__mmask16 __A, __mmask16 __B)
{
  return (__mmask16) __builtin_ia32_kxorhi ((__mmask16) __A, (__mmask16) __B);
}

static __inline__ void __DEFAULT_FN_ATTRS
_mm512_stream_si512 (__m512i * __P, __m512i __A)
{
  __builtin_nontemporal_store((__v8di)__A, (__v8di*)__P);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_stream_load_si512 (void *__P)
{
  return __builtin_ia32_movntdqa512 ((__v8di *)__P);
}

static __inline__ void __DEFAULT_FN_ATTRS
_mm512_stream_pd (double *__P, __m512d __A)
{
  __builtin_nontemporal_store((__v8df)__A, (__v8df*)__P);
}

static __inline__ void __DEFAULT_FN_ATTRS
_mm512_stream_ps (float *__P, __m512 __A)
{
  __builtin_nontemporal_store((__v16sf)__A, (__v16sf*)__P);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_mask_compress_pd (__m512d __W, __mmask8 __U, __m512d __A)
{
  return (__m512d) __builtin_ia32_compressdf512_mask ((__v8df) __A,
                  (__v8df) __W,
                  (__mmask8) __U);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_maskz_compress_pd (__mmask8 __U, __m512d __A)
{
  return (__m512d) __builtin_ia32_compressdf512_mask ((__v8df) __A,
                  (__v8df)
                  _mm512_setzero_pd (),
                  (__mmask8) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_compress_epi64 (__m512i __W, __mmask8 __U, __m512i __A)
{
  return (__m512i) __builtin_ia32_compressdi512_mask ((__v8di) __A,
                  (__v8di) __W,
                  (__mmask8) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_compress_epi64 (__mmask8 __U, __m512i __A)
{
  return (__m512i) __builtin_ia32_compressdi512_mask ((__v8di) __A,
                  (__v8di)
                  _mm512_setzero_si512 (),
                  (__mmask8) __U);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_mask_compress_ps (__m512 __W, __mmask16 __U, __m512 __A)
{
  return (__m512) __builtin_ia32_compresssf512_mask ((__v16sf) __A,
                 (__v16sf) __W,
                 (__mmask16) __U);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_maskz_compress_ps (__mmask16 __U, __m512 __A)
{
  return (__m512) __builtin_ia32_compresssf512_mask ((__v16sf) __A,
                 (__v16sf)
                 _mm512_setzero_ps (),
                 (__mmask16) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_compress_epi32 (__m512i __W, __mmask16 __U, __m512i __A)
{
  return (__m512i) __builtin_ia32_compresssi512_mask ((__v16si) __A,
                  (__v16si) __W,
                  (__mmask16) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_compress_epi32 (__mmask16 __U, __m512i __A)
{
  return (__m512i) __builtin_ia32_compresssi512_mask ((__v16si) __A,
                  (__v16si)
                  _mm512_setzero_si512 (),
                  (__mmask16) __U);
}

#define _mm_cmp_round_ss_mask(X, Y, P, R) __extension__ ({ \
  (__mmask8)__builtin_ia32_cmpss_mask((__v4sf)(__m128)(X), \
                                      (__v4sf)(__m128)(Y), (int)(P), \
                                      (__mmask8)-1, (int)(R)); })

#define _mm_mask_cmp_round_ss_mask(M, X, Y, P, R) __extension__ ({ \
  (__mmask8)__builtin_ia32_cmpss_mask((__v4sf)(__m128)(X), \
                                      (__v4sf)(__m128)(Y), (int)(P), \
                                      (__mmask8)(M), (int)(R)); })

#define _mm_cmp_ss_mask(X, Y, P) __extension__ ({ \
  (__mmask8)__builtin_ia32_cmpss_mask((__v4sf)(__m128)(X), \
                                      (__v4sf)(__m128)(Y), (int)(P), \
                                      (__mmask8)-1, \
                                      _MM_FROUND_CUR_DIRECTION); })

#define _mm_mask_cmp_ss_mask(M, X, Y, P) __extension__ ({ \
  (__mmask8)__builtin_ia32_cmpss_mask((__v4sf)(__m128)(X), \
                                      (__v4sf)(__m128)(Y), (int)(P), \
                                      (__mmask8)(M), \
                                      _MM_FROUND_CUR_DIRECTION); })

#define _mm_cmp_round_sd_mask(X, Y, P, R) __extension__ ({ \
  (__mmask8)__builtin_ia32_cmpsd_mask((__v2df)(__m128d)(X), \
                                      (__v2df)(__m128d)(Y), (int)(P), \
                                      (__mmask8)-1, (int)(R)); })

#define _mm_mask_cmp_round_sd_mask(M, X, Y, P, R) __extension__ ({ \
  (__mmask8)__builtin_ia32_cmpsd_mask((__v2df)(__m128d)(X), \
                                      (__v2df)(__m128d)(Y), (int)(P), \
                                      (__mmask8)(M), (int)(R)); })

#define _mm_cmp_sd_mask(X, Y, P) __extension__ ({ \
  (__mmask8)__builtin_ia32_cmpsd_mask((__v2df)(__m128d)(X), \
                                      (__v2df)(__m128d)(Y), (int)(P), \
                                      (__mmask8)-1, \
                                      _MM_FROUND_CUR_DIRECTION); })

#define _mm_mask_cmp_sd_mask(M, X, Y, P) __extension__ ({ \
  (__mmask8)__builtin_ia32_cmpsd_mask((__v2df)(__m128d)(X), \
                                      (__v2df)(__m128d)(Y), (int)(P), \
                                      (__mmask8)(M), \
                                      _MM_FROUND_CUR_DIRECTION); })

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_movehdup_ps (__m512 __A)
{
  return (__m512)__builtin_shufflevector((__v16sf)__A, (__v16sf)__A,
                         1, 1, 3, 3, 5, 5, 7, 7, 9, 9, 11, 11, 13, 13, 15, 15);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_mask_movehdup_ps (__m512 __W, __mmask16 __U, __m512 __A)
{
  return (__m512)__builtin_ia32_selectps_512((__mmask16)__U,
                                             (__v16sf)_mm512_movehdup_ps(__A),
                                             (__v16sf)__W);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_maskz_movehdup_ps (__mmask16 __U, __m512 __A)
{
  return (__m512)__builtin_ia32_selectps_512((__mmask16)__U,
                                             (__v16sf)_mm512_movehdup_ps(__A),
                                             (__v16sf)_mm512_setzero_ps());
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_moveldup_ps (__m512 __A)
{
  return (__m512)__builtin_shufflevector((__v16sf)__A, (__v16sf)__A,
                         0, 0, 2, 2, 4, 4, 6, 6, 8, 8, 10, 10, 12, 12, 14, 14);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_mask_moveldup_ps (__m512 __W, __mmask16 __U, __m512 __A)
{
  return (__m512)__builtin_ia32_selectps_512((__mmask16)__U,
                                             (__v16sf)_mm512_moveldup_ps(__A),
                                             (__v16sf)__W);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_maskz_moveldup_ps (__mmask16 __U, __m512 __A)
{
  return (__m512)__builtin_ia32_selectps_512((__mmask16)__U,
                                             (__v16sf)_mm512_moveldup_ps(__A),
                                             (__v16sf)_mm512_setzero_ps());
}

#define _mm512_shuffle_epi32(A, I) __extension__ ({ \
  (__m512i)__builtin_shufflevector((__v16si)(__m512i)(A), \
                                   (__v16si)_mm512_undefined_epi32(), \
                                   0  + (((I) >> 0) & 0x3), \
                                   0  + (((I) >> 2) & 0x3), \
                                   0  + (((I) >> 4) & 0x3), \
                                   0  + (((I) >> 6) & 0x3), \
                                   4  + (((I) >> 0) & 0x3), \
                                   4  + (((I) >> 2) & 0x3), \
                                   4  + (((I) >> 4) & 0x3), \
                                   4  + (((I) >> 6) & 0x3), \
                                   8  + (((I) >> 0) & 0x3), \
                                   8  + (((I) >> 2) & 0x3), \
                                   8  + (((I) >> 4) & 0x3), \
                                   8  + (((I) >> 6) & 0x3), \
                                   12 + (((I) >> 0) & 0x3), \
                                   12 + (((I) >> 2) & 0x3), \
                                   12 + (((I) >> 4) & 0x3), \
                                   12 + (((I) >> 6) & 0x3)); })

#define _mm512_mask_shuffle_epi32(W, U, A, I) __extension__ ({ \
  (__m512i)__builtin_ia32_selectd_512((__mmask16)(U), \
                                      (__v16si)_mm512_shuffle_epi32((A), (I)), \
                                      (__v16si)(__m512i)(W)); })

#define _mm512_maskz_shuffle_epi32(U, A, I) __extension__ ({ \
  (__m512i)__builtin_ia32_selectd_512((__mmask16)(U), \
                                      (__v16si)_mm512_shuffle_epi32((A), (I)), \
                                      (__v16si)_mm512_setzero_si512()); })

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_mask_expand_pd (__m512d __W, __mmask8 __U, __m512d __A)
{
  return (__m512d) __builtin_ia32_expanddf512_mask ((__v8df) __A,
                (__v8df) __W,
                (__mmask8) __U);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_maskz_expand_pd (__mmask8 __U, __m512d __A)
{
  return (__m512d) __builtin_ia32_expanddf512_mask ((__v8df) __A,
                (__v8df) _mm512_setzero_pd (),
                (__mmask8) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_expand_epi64 (__m512i __W, __mmask8 __U, __m512i __A)
{
  return (__m512i) __builtin_ia32_expanddi512_mask ((__v8di) __A,
                (__v8di) __W,
                (__mmask8) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_expand_epi64 ( __mmask8 __U, __m512i __A)
{
  return (__m512i) __builtin_ia32_expanddi512_mask ((__v8di) __A,
                (__v8di) _mm512_setzero_pd (),
                (__mmask8) __U);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_mask_expandloadu_pd(__m512d __W, __mmask8 __U, void const *__P)
{
  return (__m512d) __builtin_ia32_expandloaddf512_mask ((const __v8df *)__P,
              (__v8df) __W,
              (__mmask8) __U);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_maskz_expandloadu_pd(__mmask8 __U, void const *__P)
{
  return (__m512d) __builtin_ia32_expandloaddf512_mask ((const __v8df *)__P,
              (__v8df) _mm512_setzero_pd(),
              (__mmask8) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_expandloadu_epi64(__m512i __W, __mmask8 __U, void const *__P)
{
  return (__m512i) __builtin_ia32_expandloaddi512_mask ((const __v8di *)__P,
              (__v8di) __W,
              (__mmask8) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_expandloadu_epi64(__mmask8 __U, void const *__P)
{
  return (__m512i) __builtin_ia32_expandloaddi512_mask ((const __v8di *)__P,
              (__v8di) _mm512_setzero_pd(),
              (__mmask8) __U);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_mask_expandloadu_ps(__m512 __W, __mmask16 __U, void const *__P)
{
  return (__m512) __builtin_ia32_expandloadsf512_mask ((const __v16sf *)__P,
                   (__v16sf) __W,
                   (__mmask16) __U);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_maskz_expandloadu_ps(__mmask16 __U, void const *__P)
{
  return (__m512) __builtin_ia32_expandloadsf512_mask ((const __v16sf *)__P,
                   (__v16sf) _mm512_setzero_ps(),
                   (__mmask16) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_expandloadu_epi32(__m512i __W, __mmask16 __U, void const *__P)
{
  return (__m512i) __builtin_ia32_expandloadsi512_mask ((const __v16si *)__P,
              (__v16si) __W,
              (__mmask16) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_expandloadu_epi32(__mmask16 __U, void const *__P)
{
  return (__m512i) __builtin_ia32_expandloadsi512_mask ((const __v16si *)__P,
              (__v16si) _mm512_setzero_ps(),
              (__mmask16) __U);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_mask_expand_ps (__m512 __W, __mmask16 __U, __m512 __A)
{
  return (__m512) __builtin_ia32_expandsf512_mask ((__v16sf) __A,
               (__v16sf) __W,
               (__mmask16) __U);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_maskz_expand_ps (__mmask16 __U, __m512 __A)
{
  return (__m512) __builtin_ia32_expandsf512_mask ((__v16sf) __A,
               (__v16sf) _mm512_setzero_ps(),
               (__mmask16) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_expand_epi32 (__m512i __W, __mmask16 __U, __m512i __A)
{
  return (__m512i) __builtin_ia32_expandsi512_mask ((__v16si) __A,
                (__v16si) __W,
                (__mmask16) __U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_maskz_expand_epi32 (__mmask16 __U, __m512i __A)
{
  return (__m512i) __builtin_ia32_expandsi512_mask ((__v16si) __A,
                (__v16si) _mm512_setzero_ps(),
                (__mmask16) __U);
}

#define _mm512_cvt_roundps_pd(A, R) __extension__ ({ \
  (__m512d)__builtin_ia32_cvtps2pd512_mask((__v8sf)(__m256)(A), \
                                           (__v8df)_mm512_undefined_pd(), \
                                           (__mmask8)-1, (int)(R)); })

#define _mm512_mask_cvt_roundps_pd(W, U, A, R) __extension__ ({ \
  (__m512d)__builtin_ia32_cvtps2pd512_mask((__v8sf)(__m256)(A), \
                                           (__v8df)(__m512d)(W), \
                                           (__mmask8)(U), (int)(R)); })

#define _mm512_maskz_cvt_roundps_pd(U, A, R) __extension__ ({ \
  (__m512d)__builtin_ia32_cvtps2pd512_mask((__v8sf)(__m256)(A), \
                                           (__v8df)_mm512_setzero_pd(), \
                                           (__mmask8)(U), (int)(R)); })

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_cvtps_pd (__m256 __A)
{
  return (__m512d) __builtin_ia32_cvtps2pd512_mask ((__v8sf) __A,
                (__v8df)
                _mm512_undefined_pd (),
                (__mmask8) -1,
                _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_mask_cvtps_pd (__m512d __W, __mmask8 __U, __m256 __A)
{
  return (__m512d) __builtin_ia32_cvtps2pd512_mask ((__v8sf) __A,
                (__v8df) __W,
                (__mmask8) __U,
                _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_maskz_cvtps_pd (__mmask8 __U, __m256 __A)
{
  return (__m512d) __builtin_ia32_cvtps2pd512_mask ((__v8sf) __A,
                (__v8df)
                _mm512_setzero_pd (),
                (__mmask8) __U,
                _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_mask_mov_pd (__m512d __W, __mmask8 __U, __m512d __A)
{
  return (__m512d) __builtin_ia32_selectpd_512 ((__mmask8) __U,
              (__v8df) __A,
              (__v8df) __W);
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_maskz_mov_pd (__mmask8 __U, __m512d __A)
{
  return (__m512d) __builtin_ia32_selectpd_512 ((__mmask8) __U,
              (__v8df) __A,
              (__v8df) _mm512_setzero_pd ());
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_mask_mov_ps (__m512 __W, __mmask16 __U, __m512 __A)
{
  return (__m512) __builtin_ia32_selectps_512 ((__mmask16) __U,
             (__v16sf) __A,
             (__v16sf) __W);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_maskz_mov_ps (__mmask16 __U, __m512 __A)
{
  return (__m512) __builtin_ia32_selectps_512 ((__mmask16) __U,
             (__v16sf) __A,
             (__v16sf) _mm512_setzero_ps ());
}

static __inline__ void __DEFAULT_FN_ATTRS
_mm512_mask_compressstoreu_pd (void *__P, __mmask8 __U, __m512d __A)
{
  __builtin_ia32_compressstoredf512_mask ((__v8df *) __P, (__v8df) __A,
            (__mmask8) __U);
}

static __inline__ void __DEFAULT_FN_ATTRS
_mm512_mask_compressstoreu_epi64 (void *__P, __mmask8 __U, __m512i __A)
{
  __builtin_ia32_compressstoredi512_mask ((__v8di *) __P, (__v8di) __A,
            (__mmask8) __U);
}

static __inline__ void __DEFAULT_FN_ATTRS
_mm512_mask_compressstoreu_ps (void *__P, __mmask16 __U, __m512 __A)
{
  __builtin_ia32_compressstoresf512_mask ((__v16sf *) __P, (__v16sf) __A,
            (__mmask16) __U);
}

static __inline__ void __DEFAULT_FN_ATTRS
_mm512_mask_compressstoreu_epi32 (void *__P, __mmask16 __U, __m512i __A)
{
  __builtin_ia32_compressstoresi512_mask ((__v16si *) __P, (__v16si) __A,
            (__mmask16) __U);
}

#define _mm_cvt_roundsd_ss(A, B, R) __extension__ ({ \
  (__m128)__builtin_ia32_cvtsd2ss_round_mask((__v4sf)(__m128)(A), \
                                             (__v2df)(__m128d)(B), \
                                             (__v4sf)_mm_undefined_ps(), \
                                             (__mmask8)-1, (int)(R)); })

#define _mm_mask_cvt_roundsd_ss(W, U, A, B, R) __extension__ ({ \
  (__m128)__builtin_ia32_cvtsd2ss_round_mask((__v4sf)(__m128)(A), \
                                             (__v2df)(__m128d)(B), \
                                             (__v4sf)(__m128)(W), \
                                             (__mmask8)(U), (int)(R)); })

#define _mm_maskz_cvt_roundsd_ss(U, A, B, R) __extension__ ({ \
  (__m128)__builtin_ia32_cvtsd2ss_round_mask((__v4sf)(__m128)(A), \
                                             (__v2df)(__m128d)(B), \
                                             (__v4sf)_mm_setzero_ps(), \
                                             (__mmask8)(U), (int)(R)); })

static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_mask_cvtsd_ss (__m128 __W, __mmask8 __U, __m128 __A, __m128d __B)
{
  return __builtin_ia32_cvtsd2ss_round_mask ((__v4sf)(__A),
                                             (__v2df)(__B),
                                             (__v4sf)(__W), 
                                             (__mmask8)(__U), _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_maskz_cvtsd_ss (__mmask8 __U, __m128 __A, __m128d __B)
{
  return __builtin_ia32_cvtsd2ss_round_mask ((__v4sf)(__A),
                                             (__v2df)(__B),
                                             (__v4sf)_mm_setzero_ps(), 
                                             (__mmask8)(__U), _MM_FROUND_CUR_DIRECTION);
}

#define _mm_cvtss_i32 _mm_cvtss_si32
#define _mm_cvtss_i64 _mm_cvtss_si64
#define _mm_cvtsd_i32 _mm_cvtsd_si32
#define _mm_cvtsd_i64 _mm_cvtsd_si64
#define _mm_cvti32_sd _mm_cvtsi32_sd
#define _mm_cvti64_sd _mm_cvtsi64_sd
#define _mm_cvti32_ss _mm_cvtsi32_ss
#define _mm_cvti64_ss _mm_cvtsi64_ss

#define _mm_cvt_roundi64_sd(A, B, R) __extension__ ({ \
  (__m128d)__builtin_ia32_cvtsi2sd64((__v2df)(__m128d)(A), (long long)(B), \
                                     (int)(R)); })

#define _mm_cvt_roundsi64_sd(A, B, R) __extension__ ({ \
  (__m128d)__builtin_ia32_cvtsi2sd64((__v2df)(__m128d)(A), (long long)(B), \
                                     (int)(R)); })

#define _mm_cvt_roundsi32_ss(A, B, R) __extension__ ({ \
  (__m128)__builtin_ia32_cvtsi2ss32((__v4sf)(__m128)(A), (int)(B), (int)(R)); })

#define _mm_cvt_roundi32_ss(A, B, R) __extension__ ({ \
  (__m128)__builtin_ia32_cvtsi2ss32((__v4sf)(__m128)(A), (int)(B), (int)(R)); })

#define _mm_cvt_roundsi64_ss(A, B, R) __extension__ ({ \
  (__m128)__builtin_ia32_cvtsi2ss64((__v4sf)(__m128)(A), (long long)(B), \
                                    (int)(R)); })

#define _mm_cvt_roundi64_ss(A, B, R) __extension__ ({ \
  (__m128)__builtin_ia32_cvtsi2ss64((__v4sf)(__m128)(A), (long long)(B), \
                                    (int)(R)); })

#define _mm_cvt_roundss_sd(A, B, R) __extension__ ({ \
  (__m128d)__builtin_ia32_cvtss2sd_round_mask((__v2df)(__m128d)(A), \
                                              (__v4sf)(__m128)(B), \
                                              (__v2df)_mm_undefined_pd(), \
                                              (__mmask8)-1, (int)(R)); })

#define _mm_mask_cvt_roundss_sd(W, U, A, B, R) __extension__ ({ \
  (__m128d)__builtin_ia32_cvtss2sd_round_mask((__v2df)(__m128d)(A), \
                                              (__v4sf)(__m128)(B), \
                                              (__v2df)(__m128d)(W), \
                                              (__mmask8)(U), (int)(R)); })

#define _mm_maskz_cvt_roundss_sd(U, A, B, R) __extension__ ({ \
  (__m128d)__builtin_ia32_cvtss2sd_round_mask((__v2df)(__m128d)(A), \
                                              (__v4sf)(__m128)(B), \
                                              (__v2df)_mm_setzero_pd(), \
                                              (__mmask8)(U), (int)(R)); })

static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_mask_cvtss_sd (__m128d __W, __mmask8 __U, __m128d __A, __m128 __B)
{
  return __builtin_ia32_cvtss2sd_round_mask((__v2df)(__A),
                                              (__v4sf)(__B),
                                              (__v2df)(__W),
                                              (__mmask8)(__U), _MM_FROUND_CUR_DIRECTION); 
}

static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_maskz_cvtss_sd (__mmask8 __U, __m128d __A, __m128 __B)
{
  return __builtin_ia32_cvtss2sd_round_mask((__v2df)(__A),
                                              (__v4sf)(__B),
                                              (__v2df)_mm_setzero_pd(), 
                                              (__mmask8)(__U), _MM_FROUND_CUR_DIRECTION); 
}

static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_cvtu32_sd (__m128d __A, unsigned __B)
{
  return (__m128d) __builtin_ia32_cvtusi2sd32 ((__v2df) __A, __B);
}

#define _mm_cvt_roundu64_sd(A, B, R) __extension__ ({ \
  (__m128d)__builtin_ia32_cvtusi2sd64((__v2df)(__m128d)(A), \
                                      (unsigned long long)(B), (int)(R)); })

static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_cvtu64_sd (__m128d __A, unsigned long long __B)
{
  return (__m128d) __builtin_ia32_cvtusi2sd64 ((__v2df) __A, __B,
                 _MM_FROUND_CUR_DIRECTION);
}

#define _mm_cvt_roundu32_ss(A, B, R) __extension__ ({ \
  (__m128)__builtin_ia32_cvtusi2ss32((__v4sf)(__m128)(A), (unsigned int)(B), \
                                     (int)(R)); })

static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_cvtu32_ss (__m128 __A, unsigned __B)
{
  return (__m128) __builtin_ia32_cvtusi2ss32 ((__v4sf) __A, __B,
                _MM_FROUND_CUR_DIRECTION);
}

#define _mm_cvt_roundu64_ss(A, B, R) __extension__ ({ \
  (__m128)__builtin_ia32_cvtusi2ss64((__v4sf)(__m128)(A), \
                                     (unsigned long long)(B), (int)(R)); })

static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_cvtu64_ss (__m128 __A, unsigned long long __B)
{
  return (__m128) __builtin_ia32_cvtusi2ss64 ((__v4sf) __A, __B,
                _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_set1_epi32 (__m512i __O, __mmask16 __M, int __A)
{
  return (__m512i) __builtin_ia32_pbroadcastd512_gpr_mask (__A, (__v16si) __O,
                 __M);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_mask_set1_epi64 (__m512i __O, __mmask8 __M, long long __A)
{
  return (__m512i) __builtin_ia32_pbroadcastq512_gpr_mask (__A, (__v8di) __O,
                 __M);
}

static __inline __m512i __DEFAULT_FN_ATTRS
_mm512_set_epi32 (int __A, int __B, int __C, int __D,
     int __E, int __F, int __G, int __H,
     int __I, int __J, int __K, int __L,
     int __M, int __N, int __O, int __P)
{
  return __extension__ (__m512i)(__v16si)
  { __P, __O, __N, __M, __L, __K, __J, __I,
    __H, __G, __F, __E, __D, __C, __B, __A };
}

#define _mm512_setr_epi32(e0,e1,e2,e3,e4,e5,e6,e7,           \
       e8,e9,e10,e11,e12,e13,e14,e15)          \
  _mm512_set_epi32((e15),(e14),(e13),(e12),(e11),(e10),(e9),(e8),(e7),(e6), \
                   (e5),(e4),(e3),(e2),(e1),(e0))

static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_set_epi64 (long long __A, long long __B, long long __C,
     long long __D, long long __E, long long __F,
     long long __G, long long __H)
{
  return __extension__ (__m512i) (__v8di)
  { __H, __G, __F, __E, __D, __C, __B, __A };
}

#define _mm512_setr_epi64(e0,e1,e2,e3,e4,e5,e6,e7)           \
  _mm512_set_epi64((e7),(e6),(e5),(e4),(e3),(e2),(e1),(e0))

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_set_pd (double __A, double __B, double __C, double __D,
        double __E, double __F, double __G, double __H)
{
  return __extension__ (__m512d)
  { __H, __G, __F, __E, __D, __C, __B, __A };
}

#define _mm512_setr_pd(e0,e1,e2,e3,e4,e5,e6,e7)              \
  _mm512_set_pd((e7),(e6),(e5),(e4),(e3),(e2),(e1),(e0))

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_set_ps (float __A, float __B, float __C, float __D,
        float __E, float __F, float __G, float __H,
        float __I, float __J, float __K, float __L,
        float __M, float __N, float __O, float __P)
{
  return __extension__ (__m512)
  { __P, __O, __N, __M, __L, __K, __J, __I,
    __H, __G, __F, __E, __D, __C, __B, __A };
}

#define _mm512_setr_ps(e0,e1,e2,e3,e4,e5,e6,e7,e8,e9,e10,e11,e12,e13,e14,e15) \
  _mm512_set_ps((e15),(e14),(e13),(e12),(e11),(e10),(e9),(e8),(e7),(e6),(e5), \
                (e4),(e3),(e2),(e1),(e0))

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_abs_ps(__m512 A)
{
  return (__m512)_mm512_and_epi32(_mm512_set1_epi32(0x7FFFFFFF),(__m512i)A) ;
}

static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_mask_abs_ps(__m512 W, __mmask16 K, __m512 A)
{
  return (__m512)_mm512_mask_and_epi32((__m512i)W, K, _mm512_set1_epi32(0x7FFFFFFF),(__m512i)A) ;
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_abs_pd(__m512d A)
{
  return (__m512d)_mm512_and_epi64(_mm512_set1_epi64(0x7FFFFFFFFFFFFFFF),(__v8di)A) ;
}

static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_mask_abs_pd(__m512d W, __mmask8 K, __m512d A)
{
  return (__m512d)_mm512_mask_and_epi64((__v8di)W, K, _mm512_set1_epi64(0x7FFFFFFFFFFFFFFF),(__v8di)A);
}

#undef __DEFAULT_FN_ATTRS

#endif // __AVX512FINTRIN_H
