/*******************************************************************************
 *
 * Module Name: utmath - Integer math support routines
 *
 ******************************************************************************/

/*
 * Copyright (C) 2000 - 2014, Intel Corp.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions, and the following disclaimer,
 *    without modification.
 * 2. Redistributions in binary form must reproduce at minimum a disclaimer
 *    substantially similar to the "NO WARRANTY" disclaimer below
 *    ("Disclaimer") and any redistribution must be conditioned upon
 *    including a substantially similar Disclaimer requirement for further
 *    binary redistribution.
 * 3. Neither the names of the above-listed copyright holders nor the names
 *    of any contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 *
 * NO WARRANTY
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDERS OR CONTRIBUTORS BE LIABLE FOR SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES.
 */


#define __UTMATH_C__

#include "acpi.h"
#include "accommon.h"


#define _COMPONENT          ACPI_UTILITIES
        ACPI_MODULE_NAME    ("utmath")

/*
 * Optional support for 64-bit double-precision integer divide. This code
 * is configurable and is implemented in order to support 32-bit kernel
 * environments where a 64-bit double-precision math library is not available.
 *
 * Support for a more normal 64-bit divide/modulo (with check for a divide-
 * by-zero) appears after this optional section of code.
 */
#ifndef ACPI_USE_NATIVE_DIVIDE

/* Structures used only for 64-bit divide */

typedef struct uint64_struct
{
    UINT32                          Lo;
    UINT32                          Hi;

} UINT64_STRUCT;

typedef union uint64_overlay
{
    UINT64                          Full;
    UINT64_STRUCT                   Part;

} UINT64_OVERLAY;


/*******************************************************************************
 *
 * FUNCTION:    AcpiUtShortDivide
 *
 * PARAMETERS:  Dividend            - 64-bit dividend
 *              Divisor             - 32-bit divisor
 *              OutQuotient         - Pointer to where the quotient is returned
 *              OutRemainder        - Pointer to where the remainder is returned
 *
 * RETURN:      Status (Checks for divide-by-zero)
 *
 * DESCRIPTION: Perform a short (maximum 64 bits divided by 32 bits)
 *              divide and modulo. The result is a 64-bit quotient and a
 *              32-bit remainder.
 *
 ******************************************************************************/

ACPI_STATUS
AcpiUtShortDivide (
    UINT64                  Dividend,
    UINT32                  Divisor,
    UINT64                  *OutQuotient,
    UINT32                  *OutRemainder)
{
    UINT64_OVERLAY          DividendOvl;
    UINT64_OVERLAY          Quotient;
    UINT32                  Remainder32;


    ACPI_FUNCTION_TRACE (UtShortDivide);


    /* Always check for a zero divisor */

    if (Divisor == 0)
    {
        ACPI_ERROR ((AE_INFO, "Divide by zero"));
        return_ACPI_STATUS (AE_AML_DIVIDE_BY_ZERO);
    }

    DividendOvl.Full = Dividend;

    /*
     * The quotient is 64 bits, the remainder is always 32 bits,
     * and is generated by the second divide.
     */
    ACPI_DIV_64_BY_32 (0, DividendOvl.Part.Hi, Divisor,
                       Quotient.Part.Hi, Remainder32);
    ACPI_DIV_64_BY_32 (Remainder32, DividendOvl.Part.Lo, Divisor,
                       Quotient.Part.Lo, Remainder32);

    /* Return only what was requested */

    if (OutQuotient)
    {
        *OutQuotient = Quotient.Full;
    }
    if (OutRemainder)
    {
        *OutRemainder = Remainder32;
    }

    return_ACPI_STATUS (AE_OK);
}


/*******************************************************************************
 *
 * FUNCTION:    AcpiUtDivide
 *
 * PARAMETERS:  InDividend          - Dividend
 *              InDivisor           - Divisor
 *              OutQuotient         - Pointer to where the quotient is returned
 *              OutRemainder        - Pointer to where the remainder is returned
 *
 * RETURN:      Status (Checks for divide-by-zero)
 *
 * DESCRIPTION: Perform a divide and modulo.
 *
 ******************************************************************************/

ACPI_STATUS
AcpiUtDivide (
    UINT64                  InDividend,
    UINT64                  InDivisor,
    UINT64                  *OutQuotient,
    UINT64                  *OutRemainder)
{
    UINT64_OVERLAY          Dividend;
    UINT64_OVERLAY          Divisor;
    UINT64_OVERLAY          Quotient;
    UINT64_OVERLAY          Remainder;
    UINT64_OVERLAY          NormalizedDividend;
    UINT64_OVERLAY          NormalizedDivisor;
    UINT32                  Partial1;
    UINT64_OVERLAY          Partial2;
    UINT64_OVERLAY          Partial3;


    ACPI_FUNCTION_TRACE (UtDivide);


    /* Always check for a zero divisor */

    if (InDivisor == 0)
    {
        ACPI_ERROR ((AE_INFO, "Divide by zero"));
        return_ACPI_STATUS (AE_AML_DIVIDE_BY_ZERO);
    }

    Divisor.Full  = InDivisor;
    Dividend.Full = InDividend;
    if (Divisor.Part.Hi == 0)
    {
        /*
         * 1) Simplest case is where the divisor is 32 bits, we can
         * just do two divides
         */
        Remainder.Part.Hi = 0;

        /*
         * The quotient is 64 bits, the remainder is always 32 bits,
         * and is generated by the second divide.
         */
        ACPI_DIV_64_BY_32 (0, Dividend.Part.Hi, Divisor.Part.Lo,
                           Quotient.Part.Hi, Partial1);
        ACPI_DIV_64_BY_32 (Partial1, Dividend.Part.Lo, Divisor.Part.Lo,
                           Quotient.Part.Lo, Remainder.Part.Lo);
    }

    else
    {
        /*
         * 2) The general case where the divisor is a full 64 bits
         * is more difficult
         */
        Quotient.Part.Hi   = 0;
        NormalizedDividend = Dividend;
        NormalizedDivisor  = Divisor;

        /* Normalize the operands (shift until the divisor is < 32 bits) */

        do
        {
            ACPI_SHIFT_RIGHT_64 (NormalizedDivisor.Part.Hi,
                                 NormalizedDivisor.Part.Lo);
            ACPI_SHIFT_RIGHT_64 (NormalizedDividend.Part.Hi,
                                 NormalizedDividend.Part.Lo);

        } while (NormalizedDivisor.Part.Hi != 0);

        /* Partial divide */

        ACPI_DIV_64_BY_32 (NormalizedDividend.Part.Hi,
                           NormalizedDividend.Part.Lo,
                           NormalizedDivisor.Part.Lo,
                           Quotient.Part.Lo, Partial1);

        /*
         * The quotient is always 32 bits, and simply requires adjustment.
         * The 64-bit remainder must be generated.
         */
        Partial1      = Quotient.Part.Lo * Divisor.Part.Hi;
        Partial2.Full = (UINT64) Quotient.Part.Lo * Divisor.Part.Lo;
        Partial3.Full = (UINT64) Partial2.Part.Hi + Partial1;

        Remainder.Part.Hi = Partial3.Part.Lo;
        Remainder.Part.Lo = Partial2.Part.Lo;

        if (Partial3.Part.Hi == 0)
        {
            if (Partial3.Part.Lo >= Dividend.Part.Hi)
            {
                if (Partial3.Part.Lo == Dividend.Part.Hi)
                {
                    if (Partial2.Part.Lo > Dividend.Part.Lo)
                    {
                        Quotient.Part.Lo--;
                        Remainder.Full -= Divisor.Full;
                    }
                }
                else
                {
                    Quotient.Part.Lo--;
                    Remainder.Full -= Divisor.Full;
                }
            }

            Remainder.Full    = Remainder.Full - Dividend.Full;
            Remainder.Part.Hi = (UINT32) -((INT32) Remainder.Part.Hi);
            Remainder.Part.Lo = (UINT32) -((INT32) Remainder.Part.Lo);

            if (Remainder.Part.Lo)
            {
                Remainder.Part.Hi--;
            }
        }
    }

    /* Return only what was requested */

    if (OutQuotient)
    {
        *OutQuotient = Quotient.Full;
    }
    if (OutRemainder)
    {
        *OutRemainder = Remainder.Full;
    }

    return_ACPI_STATUS (AE_OK);
}

#else

/*******************************************************************************
 *
 * FUNCTION:    AcpiUtShortDivide, AcpiUtDivide
 *
 * PARAMETERS:  See function headers above
 *
 * DESCRIPTION: Native versions of the UtDivide functions. Use these if either
 *              1) The target is a 64-bit platform and therefore 64-bit
 *                 integer math is supported directly by the machine.
 *              2) The target is a 32-bit or 16-bit platform, and the
 *                 double-precision integer math library is available to
 *                 perform the divide.
 *
 ******************************************************************************/

ACPI_STATUS
AcpiUtShortDivide (
    UINT64                  InDividend,
    UINT32                  Divisor,
    UINT64                  *OutQuotient,
    UINT32                  *OutRemainder)
{

    ACPI_FUNCTION_TRACE (UtShortDivide);


    /* Always check for a zero divisor */

    if (Divisor == 0)
    {
        ACPI_ERROR ((AE_INFO, "Divide by zero"));
        return_ACPI_STATUS (AE_AML_DIVIDE_BY_ZERO);
    }

    /* Return only what was requested */

    if (OutQuotient)
    {
        *OutQuotient = InDividend / Divisor;
    }
    if (OutRemainder)
    {
        *OutRemainder = (UINT32) (InDividend % Divisor);
    }

    return_ACPI_STATUS (AE_OK);
}

ACPI_STATUS
AcpiUtDivide (
    UINT64                  InDividend,
    UINT64                  InDivisor,
    UINT64                  *OutQuotient,
    UINT64                  *OutRemainder)
{
    ACPI_FUNCTION_TRACE (UtDivide);


    /* Always check for a zero divisor */

    if (InDivisor == 0)
    {
        ACPI_ERROR ((AE_INFO, "Divide by zero"));
        return_ACPI_STATUS (AE_AML_DIVIDE_BY_ZERO);
    }


    /* Return only what was requested */

    if (OutQuotient)
    {
        *OutQuotient = InDividend / InDivisor;
    }
    if (OutRemainder)
    {
        *OutRemainder = InDividend % InDivisor;
    }

    return_ACPI_STATUS (AE_OK);
}

#endif
