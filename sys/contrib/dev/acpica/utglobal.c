/******************************************************************************
 *
 * Module Name: utglobal - Global variables for the ACPI subsystem
 *              $Revision: 127 $
 *
 *****************************************************************************/

/******************************************************************************
 *
 * 1. Copyright Notice
 *
 * Some or all of this work - Copyright (c) 1999, 2000, 2001, Intel Corp.
 * All rights reserved.
 *
 * 2. License
 *
 * 2.1. This is your license from Intel Corp. under its intellectual property
 * rights.  You may have additional license terms from the party that provided
 * you this software, covering your right to use that party's intellectual
 * property rights.
 *
 * 2.2. Intel grants, free of charge, to any person ("Licensee") obtaining a
 * copy of the source code appearing in this file ("Covered Code") an
 * irrevocable, perpetual, worldwide license under Intel's copyrights in the
 * base code distributed originally by Intel ("Original Intel Code") to copy,
 * make derivatives, distribute, use and display any portion of the Covered
 * Code in any form, with the right to sublicense such rights; and
 *
 * 2.3. Intel grants Licensee a non-exclusive and non-transferable patent
 * license (with the right to sublicense), under only those claims of Intel
 * patents that are infringed by the Original Intel Code, to make, use, sell,
 * offer to sell, and import the Covered Code and derivative works thereof
 * solely to the minimum extent necessary to exercise the above copyright
 * license, and in no event shall the patent license extend to any additions
 * to or modifications of the Original Intel Code.  No other license or right
 * is granted directly or by implication, estoppel or otherwise;
 *
 * The above copyright and patent license is granted only if the following
 * conditions are met:
 *
 * 3. Conditions
 *
 * 3.1. Redistribution of Source with Rights to Further Distribute Source.
 * Redistribution of source code of any substantial portion of the Covered
 * Code or modification with rights to further distribute source must include
 * the above Copyright Notice, the above License, this list of Conditions,
 * and the following Disclaimer and Export Compliance provision.  In addition,
 * Licensee must cause all Covered Code to which Licensee contributes to
 * contain a file documenting the changes Licensee made to create that Covered
 * Code and the date of any change.  Licensee must include in that file the
 * documentation of any changes made by any predecessor Licensee.  Licensee
 * must include a prominent statement that the modification is derived,
 * directly or indirectly, from Original Intel Code.
 *
 * 3.2. Redistribution of Source with no Rights to Further Distribute Source.
 * Redistribution of source code of any substantial portion of the Covered
 * Code or modification without rights to further distribute source must
 * include the following Disclaimer and Export Compliance provision in the
 * documentation and/or other materials provided with distribution.  In
 * addition, Licensee may not authorize further sublicense of source of any
 * portion of the Covered Code, and must include terms to the effect that the
 * license from Licensee to its licensee is limited to the intellectual
 * property embodied in the software Licensee provides to its licensee, and
 * not to intellectual property embodied in modifications its licensee may
 * make.
 *
 * 3.3. Redistribution of Executable. Redistribution in executable form of any
 * substantial portion of the Covered Code or modification must reproduce the
 * above Copyright Notice, and the following Disclaimer and Export Compliance
 * provision in the documentation and/or other materials provided with the
 * distribution.
 *
 * 3.4. Intel retains all right, title, and interest in and to the Original
 * Intel Code.
 *
 * 3.5. Neither the name Intel nor any other trademark owned or controlled by
 * Intel shall be used in advertising or otherwise to promote the sale, use or
 * other dealings in products derived from or relating to the Covered Code
 * without prior written authorization from Intel.
 *
 * 4. Disclaimer and Export Compliance
 *
 * 4.1. INTEL MAKES NO WARRANTY OF ANY KIND REGARDING ANY SOFTWARE PROVIDED
 * HERE.  ANY SOFTWARE ORIGINATING FROM INTEL OR DERIVED FROM INTEL SOFTWARE
 * IS PROVIDED "AS IS," AND INTEL WILL NOT PROVIDE ANY SUPPORT,  ASSISTANCE,
 * INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL WILL NOT PROVIDE ANY
 * UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY DISCLAIMS ANY
 * IMPLIED WARRANTIES OF MERCHANTABILITY, NONINFRINGEMENT AND FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 * 4.2. IN NO EVENT SHALL INTEL HAVE ANY LIABILITY TO LICENSEE, ITS LICENSEES
 * OR ANY OTHER THIRD PARTY, FOR ANY LOST PROFITS, LOST DATA, LOSS OF USE OR
 * COSTS OF PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES, OR FOR ANY INDIRECT,
 * SPECIAL OR CONSEQUENTIAL DAMAGES ARISING OUT OF THIS AGREEMENT, UNDER ANY
 * CAUSE OF ACTION OR THEORY OF LIABILITY, AND IRRESPECTIVE OF WHETHER INTEL
 * HAS ADVANCE NOTICE OF THE POSSIBILITY OF SUCH DAMAGES.  THESE LIMITATIONS
 * SHALL APPLY NOTWITHSTANDING THE FAILURE OF THE ESSENTIAL PURPOSE OF ANY
 * LIMITED REMEDY.
 *
 * 4.3. Licensee shall not export, either directly or indirectly, any of this
 * software or system incorporating such software without first obtaining any
 * required license or other approval from the U. S. Department of Commerce or
 * any other agency or department of the United States Government.  In the
 * event Licensee exports any such software from the United States or
 * re-exports any such software from a foreign destination, Licensee shall
 * ensure that the distribution and export/re-export of the software is in
 * compliance with all laws, regulations, orders, or other restrictions of the
 * U.S. Export Administration Regulations. Licensee agrees that neither it nor
 * any of its subsidiaries will export/re-export any technical data, process,
 * software, or service, directly or indirectly, to any country for which the
 * United States government or any agency thereof requires an export license,
 * other governmental approval, or letter of assurance, without first obtaining
 * such license, approval or letter.
 *
 *****************************************************************************/

#define __UTGLOBAL_C__
#define DEFINE_ACPI_GLOBALS

#include "acpi.h"
#include "acevents.h"
#include "acnamesp.h"
#include "acinterp.h"
#include "amlcode.h"


#define _COMPONENT          ACPI_UTILITIES
        MODULE_NAME         ("utglobal")



/******************************************************************************
 *
 * FUNCTION:    AcpiFormatException
 *
 * PARAMETERS:  Status       - The ACPI_STATUS code to be formatted
 *
 * RETURN:      A string containing the exception  text
 *
 * DESCRIPTION: This function translates an ACPI exception into an ASCII string.
 *
 ******************************************************************************/

const char *
AcpiFormatException (
    ACPI_STATUS             Status)
{
    char                    *Exception = "UNKNOWN_STATUS_CODE";
    ACPI_STATUS             SubStatus;


    SubStatus = (Status & ~AE_CODE_MASK);


    switch (Status & AE_CODE_MASK)
    {
    case AE_CODE_ENVIRONMENTAL:

        if (SubStatus <= AE_CODE_ENV_MAX)
        {
            Exception = AcpiGbl_ExceptionNames_Env [SubStatus];
        }
        break;

    case AE_CODE_PROGRAMMER:

        if (SubStatus <= AE_CODE_PGM_MAX)
        {
            Exception = AcpiGbl_ExceptionNames_Pgm [SubStatus -1];
        }
        break;

    case AE_CODE_ACPI_TABLES:

        if (SubStatus <= AE_CODE_TBL_MAX)
        {
            Exception = AcpiGbl_ExceptionNames_Tbl [SubStatus -1];
        }
        break;

    case AE_CODE_AML:

        if (SubStatus <= AE_CODE_AML_MAX)
        {
            Exception = AcpiGbl_ExceptionNames_Aml [SubStatus -1];
        }
        break;

    case AE_CODE_CONTROL:

        if (SubStatus <= AE_CODE_CTRL_MAX)
        {
            Exception = AcpiGbl_ExceptionNames_Ctrl [SubStatus -1];
        }
        break;

    default:
        break;
    }


    return ((const char *) Exception);
}


/******************************************************************************
 *
 * Static global variable initialization.
 *
 ******************************************************************************/

/*
 * We want the debug switches statically initialized so they
 * are already set when the debugger is entered.
 */

/* Debug switch - level and trace mask */

#ifdef ACPI_DEBUG
UINT32                      AcpiDbgLevel = DEBUG_DEFAULT;
#else
UINT32                      AcpiDbgLevel = NORMAL_DEFAULT;
#endif

/* Debug switch - layer (component) mask */

UINT32                      AcpiDbgLayer = ACPI_COMPONENT_DEFAULT;
UINT32                      AcpiGbl_NestingLevel = 0;


/* Debugger globals */

BOOLEAN                     AcpiGbl_DbTerminateThreads = FALSE;
BOOLEAN                     AcpiGbl_MethodExecuting = FALSE;

/* System flags */

UINT32                      AcpiGbl_SystemFlags = 0;
UINT32                      AcpiGbl_StartupFlags = 0;

/* System starts unitialized! */
BOOLEAN                     AcpiGbl_Shutdown = TRUE;


UINT8                       AcpiGbl_DecodeTo8bit [8] = {1,2,4,8,16,32,64,128};


/******************************************************************************
 *
 * Namespace globals
 *
 ******************************************************************************/


/*
 * Names built-in to the interpreter
 *
 * Initial values are currently supported only for types String and Number.
 * To avoid type punning, both are specified as strings in this table.
 *
 * NOTES:
 * 1) _SB_ is defined to be a device to allow _SB_/_INI to be run
 *    during the initialization sequence.
 */

PREDEFINED_NAMES            AcpiGbl_PreDefinedNames[] =
{
    {"_GPE",    INTERNAL_TYPE_DEF_ANY},
    {"_PR_",    INTERNAL_TYPE_DEF_ANY},
    {"_SB_",    ACPI_TYPE_DEVICE},
    {"_SI_",    INTERNAL_TYPE_DEF_ANY},
    {"_TZ_",    INTERNAL_TYPE_DEF_ANY},
    {"_REV",    ACPI_TYPE_INTEGER, "2"},
    {"_OS_",    ACPI_TYPE_STRING, ACPI_OS_NAME},
    {"_GL_",    ACPI_TYPE_MUTEX, "0"},
    {NULL,      ACPI_TYPE_ANY}           /* Table terminator */
};


/*
 * Properties of the ACPI Object Types, both internal and external.
 *
 * Elements of AcpiNsProperties are bit significant
 * and the table is indexed by values of ACPI_OBJECT_TYPE
 */

UINT8                       AcpiGbl_NsProperties[] =
{
    NSP_NORMAL,                 /* 00 Any              */
    NSP_NORMAL,                 /* 01 Number           */
    NSP_NORMAL,                 /* 02 String           */
    NSP_NORMAL,                 /* 03 Buffer           */
    NSP_LOCAL,                  /* 04 Package          */
    NSP_NORMAL,                 /* 05 FieldUnit        */
    NSP_NEWSCOPE | NSP_LOCAL,   /* 06 Device           */
    NSP_LOCAL,                  /* 07 AcpiEvent        */
    NSP_NEWSCOPE | NSP_LOCAL,   /* 08 Method           */
    NSP_LOCAL,                  /* 09 Mutex            */
    NSP_LOCAL,                  /* 10 Region           */
    NSP_NEWSCOPE | NSP_LOCAL,   /* 11 Power            */
    NSP_NEWSCOPE | NSP_LOCAL,   /* 12 Processor        */
    NSP_NEWSCOPE | NSP_LOCAL,   /* 13 Thermal          */
    NSP_NORMAL,                 /* 14 BufferField      */
    NSP_NORMAL,                 /* 15 DdbHandle        */
    NSP_NORMAL,                 /* 16 Debug Object     */
    NSP_NORMAL,                 /* 17 DefField         */
    NSP_NORMAL,                 /* 18 BankField        */
    NSP_NORMAL,                 /* 19 IndexField       */
    NSP_NORMAL,                 /* 20 Reference        */
    NSP_NORMAL,                 /* 21 Alias            */
    NSP_NORMAL,                 /* 22 Notify           */
    NSP_NORMAL,                 /* 23 Address Handler  */
    NSP_NEWSCOPE | NSP_LOCAL,   /* 24 Resource Desc    */
    NSP_NEWSCOPE | NSP_LOCAL,   /* 25 Resource Field   */
    NSP_NORMAL,                 /* 26 DefFieldDefn     */
    NSP_NORMAL,                 /* 27 BankFieldDefn    */
    NSP_NORMAL,                 /* 28 IndexFieldDefn   */
    NSP_NORMAL,                 /* 29 If               */
    NSP_NORMAL,                 /* 30 Else             */
    NSP_NORMAL,                 /* 31 While            */
    NSP_NEWSCOPE,               /* 32 Scope            */
    NSP_LOCAL,                  /* 33 DefAny           */
    NSP_NORMAL,                 /* 34 Extra            */
    NSP_NORMAL                  /* 35 Invalid          */
};


/* Hex to ASCII conversion table */

NATIVE_CHAR                 AcpiGbl_HexToAscii[] =
                                {'0','1','2','3','4','5','6','7',
                                '8','9','A','B','C','D','E','F'};

UINT8
AcpiUtHexToAsciiChar (
    ACPI_INTEGER            Integer,
    UINT32                  Position)
{

    return (AcpiGbl_HexToAscii[(Integer >> Position) & 0xF]);
}

/******************************************************************************
 *
 * Table globals
 *
 * NOTE: This table includes ONLY the ACPI tables that the subsystem consumes.
 * it is NOT an exhaustive list of all possible ACPI tables.  All ACPI tables
 * that are not used by the subsystem are simply ignored.
 *
 ******************************************************************************/


ACPI_TABLE_DESC             AcpiGbl_AcpiTables[NUM_ACPI_TABLES];


ACPI_TABLE_SUPPORT          AcpiGbl_AcpiTableData[NUM_ACPI_TABLES] =
{
    /***********    Name,    Signature,  Signature size,    How many allowed?,   Supported?  Global typed pointer */

    /* RSDP 0 */ {RSDP_NAME, RSDP_SIG, sizeof (RSDP_SIG)-1, ACPI_TABLE_SINGLE,   AE_OK,      NULL},
    /* DSDT 1 */ {DSDT_SIG,  DSDT_SIG, sizeof (DSDT_SIG)-1, ACPI_TABLE_SINGLE,   AE_OK,      (void **) &AcpiGbl_DSDT},
    /* FADT 2 */ {FADT_SIG,  FADT_SIG, sizeof (FADT_SIG)-1, ACPI_TABLE_SINGLE,   AE_OK,      (void **) &AcpiGbl_FADT},
    /* FACS 3 */ {FACS_SIG,  FACS_SIG, sizeof (FACS_SIG)-1, ACPI_TABLE_SINGLE,   AE_OK,      (void **) &AcpiGbl_FACS},
    /* PSDT 4 */ {PSDT_SIG,  PSDT_SIG, sizeof (PSDT_SIG)-1, ACPI_TABLE_MULTIPLE, AE_OK,      NULL},
    /* SSDT 5 */ {SSDT_SIG,  SSDT_SIG, sizeof (SSDT_SIG)-1, ACPI_TABLE_MULTIPLE, AE_OK,      NULL},
    /* XSDT 6 */ {XSDT_SIG,  XSDT_SIG, sizeof (RSDT_SIG)-1, ACPI_TABLE_SINGLE,   AE_OK,      NULL},
};


#ifdef ACPI_DEBUG

/******************************************************************************
 *
 * Strings and procedures used for debug only
 *
 ******************************************************************************/

NATIVE_CHAR                 *MsgAcpiErrorBreak = "*** Break on ACPI_ERROR ***\n";


/*****************************************************************************
 *
 * FUNCTION:    AcpiUtGetMutexName
 *
 * PARAMETERS:  None.
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Translate a mutex ID into a name string (Debug only)
 *
 ****************************************************************************/

NATIVE_CHAR *
AcpiUtGetMutexName (
    UINT32                  MutexId)
{

    if (MutexId > MAX_MTX)
    {
        return ("Invalid Mutex ID");
    }

    return (AcpiGbl_MutexNames[MutexId]);
}


/*
 * Elements of AcpiGbl_NsTypeNames below must match
 * one-to-one with values of ACPI_OBJECT_TYPE
 *
 * The type ACPI_TYPE_ANY (Untyped) is used as a "don't care" when searching; when
 * stored in a table it really means that we have thus far seen no evidence to
 * indicatewhat type is actually going to be stored for this entry.
 */

static NATIVE_CHAR          AcpiGbl_BadType[] = "UNDEFINED";
#define TYPE_NAME_LENGTH    9                       /* Maximum length of each string */

static NATIVE_CHAR          *AcpiGbl_NsTypeNames[] =    /* printable names of ACPI types */
{
    /* 00 */ "Untyped",
    /* 01 */ "Integer",
    /* 02 */ "String",
    /* 03 */ "Buffer",
    /* 04 */ "Package",
    /* 05 */ "FieldUnit",
    /* 06 */ "Device",
    /* 07 */ "Event",
    /* 08 */ "Method",
    /* 09 */ "Mutex",
    /* 10 */ "Region",
    /* 11 */ "Power",
    /* 12 */ "Processor",
    /* 13 */ "Thermal",
    /* 14 */ "BuffField",
    /* 15 */ "DdbHandle",
    /* 16 */ "DebugObj",
    /* 17 */ "RegnField",
    /* 18 */ "BankField",
    /* 19 */ "IndxField",
    /* 20 */ "Reference",
    /* 21 */ "Alias",
    /* 22 */ "Notify",
    /* 23 */ "AddrHndlr",
    /* 24 */ "RsrcDesc",
    /* 25 */ "RsrcField",
    /* 26 */ "RgnFldDfn",
    /* 27 */ "BnkFldDfn",
    /* 28 */ "IdxFldDfn",
    /* 29 */ "If",
    /* 30 */ "Else",
    /* 31 */ "While",
    /* 32 */ "Scope",
    /* 33 */ "DefAny",
    /* 34 */ "Extra",
    /* 35 */ "Invalid"
};


/*****************************************************************************
 *
 * FUNCTION:    AcpiUtGetTypeName
 *
 * PARAMETERS:  None.
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Translate a Type ID into a name string (Debug only)
 *
 ****************************************************************************/

NATIVE_CHAR *
AcpiUtGetTypeName (
    UINT32                  Type)
{

    if (Type > INTERNAL_TYPE_INVALID)
    {
        return (AcpiGbl_BadType);
    }

    return (AcpiGbl_NsTypeNames[Type]);
}


/* Region type decoding */

NATIVE_CHAR *AcpiGbl_RegionTypes[NUM_REGION_TYPES] =
{
    "SystemMemory",
    "SystemIO",
    "PCIConfig",
    "EmbeddedControl",
    "SMBus",
    "CMOS",
    "PCIBarTarget",
};


/*****************************************************************************
 *
 * FUNCTION:    AcpiUtGetRegionName
 *
 * PARAMETERS:  None.
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Translate a Space ID into a name string (Debug only)
 *
 ****************************************************************************/

NATIVE_CHAR *
AcpiUtGetRegionName (
    UINT8                   SpaceId)
{

    if (SpaceId >= USER_REGION_BEGIN)
    {
        return ("UserDefinedRegion");
    }

    else if (SpaceId >= NUM_REGION_TYPES)
    {
        return ("InvalidSpaceID");
    }

    return (AcpiGbl_RegionTypes[SpaceId]);
}


/* Data used in keeping track of fields */

NATIVE_CHAR *AcpiGbl_FENames[NUM_FIELD_NAMES] =
{
    "skip",
    "?access?"
};              /* FE = Field Element */


NATIVE_CHAR *AcpiGbl_MatchOps[NUM_MATCH_OPS] =
{
    "Error",
    "MTR",
    "MEQ",
    "MLE",
    "MLT",
    "MGE",
    "MGT"
};


/* Access type decoding */

NATIVE_CHAR *AcpiGbl_AccessTypes[NUM_ACCESS_TYPES] =
{
    "AnyAcc",
    "ByteAcc",
    "WordAcc",
    "DWordAcc",
    "BlockAcc",
    "SMBSendRecvAcc",
    "SMBQuickAcc"
};


/* Update rule decoding */

NATIVE_CHAR *AcpiGbl_UpdateRules[NUM_UPDATE_RULES] =
{
    "Preserve",
    "WriteAsOnes",
    "WriteAsZeros"
};

#endif


/*****************************************************************************
 *
 * FUNCTION:    AcpiUtValidObjectType
 *
 * PARAMETERS:  None.
 *
 * RETURN:      TRUE if valid object type
 *
 * DESCRIPTION: Validate an object type
 *
 ****************************************************************************/

BOOLEAN
AcpiUtValidObjectType (
    UINT32                  Type)
{

    if (Type > ACPI_TYPE_MAX)
    {
        if ((Type < INTERNAL_TYPE_BEGIN) ||
            (Type > INTERNAL_TYPE_MAX))
        {
            return (FALSE);
        }
    }

    return (TRUE);
}


/****************************************************************************
 *
 * FUNCTION:    AcpiUtAllocateOwnerId
 *
 * PARAMETERS:  IdType          - Type of ID (method or table)
 *
 * DESCRIPTION: Allocate a table or method owner id
 *
 ***************************************************************************/

ACPI_OWNER_ID
AcpiUtAllocateOwnerId (
    UINT32                  IdType)
{
    ACPI_OWNER_ID           OwnerId = 0xFFFF;


    FUNCTION_TRACE ("UtAllocateOwnerId");


    AcpiUtAcquireMutex (ACPI_MTX_CACHES);

    switch (IdType)
    {
    case OWNER_TYPE_TABLE:

        OwnerId = AcpiGbl_NextTableOwnerId;
        AcpiGbl_NextTableOwnerId++;

        if (AcpiGbl_NextTableOwnerId == FIRST_METHOD_ID)
        {
            AcpiGbl_NextTableOwnerId = FIRST_TABLE_ID;
        }
        break;


    case OWNER_TYPE_METHOD:

        OwnerId = AcpiGbl_NextMethodOwnerId;
        AcpiGbl_NextMethodOwnerId++;

        if (AcpiGbl_NextMethodOwnerId == FIRST_TABLE_ID)
        {
            AcpiGbl_NextMethodOwnerId = FIRST_METHOD_ID;
        }
        break;
    }


    AcpiUtReleaseMutex (ACPI_MTX_CACHES);

    return_VALUE (OwnerId);
}


/****************************************************************************
 *
 * FUNCTION:    AcpiUtInitGlobals
 *
 * PARAMETERS:  none
 *
 * DESCRIPTION: Init library globals.  All globals that require specific
 *              initialization should be initialized here!
 *
 ***************************************************************************/

void
AcpiUtInitGlobals (
    void)
{
    UINT32                  i;


    FUNCTION_TRACE ("UtInitGlobals");



    /* Memory allocation and cache lists */

    MEMSET (AcpiGbl_MemoryLists, 0, 
            sizeof (ACPI_MEMORY_LIST) * ACPI_NUM_MEM_LISTS);

    AcpiGbl_MemoryLists[ACPI_MEM_LIST_STATE].LinkOffset      = (UINT16) (NATIVE_UINT) &(((ACPI_GENERIC_STATE *) NULL)->Common.Next);
    AcpiGbl_MemoryLists[ACPI_MEM_LIST_PSNODE].LinkOffset     = (UINT16) (NATIVE_UINT) &(((ACPI_PARSE_OBJECT *) NULL)->Next);
    AcpiGbl_MemoryLists[ACPI_MEM_LIST_PSNODE_EXT].LinkOffset = (UINT16) (NATIVE_UINT) &(((ACPI_PARSE2_OBJECT *) NULL)->Next);
    AcpiGbl_MemoryLists[ACPI_MEM_LIST_OPERAND].LinkOffset    = (UINT16) (NATIVE_UINT) &(((ACPI_OPERAND_OBJECT *) NULL)->Cache.Next);
    AcpiGbl_MemoryLists[ACPI_MEM_LIST_WALK].LinkOffset       = (UINT16) (NATIVE_UINT) &(((ACPI_WALK_STATE *) NULL)->Next);

    AcpiGbl_MemoryLists[ACPI_MEM_LIST_NSNODE].ObjectSize     = sizeof (ACPI_NAMESPACE_NODE);
    AcpiGbl_MemoryLists[ACPI_MEM_LIST_STATE].ObjectSize      = sizeof (ACPI_GENERIC_STATE);
    AcpiGbl_MemoryLists[ACPI_MEM_LIST_PSNODE].ObjectSize     = sizeof (ACPI_PARSE_OBJECT);
    AcpiGbl_MemoryLists[ACPI_MEM_LIST_PSNODE_EXT].ObjectSize = sizeof (ACPI_PARSE2_OBJECT);
    AcpiGbl_MemoryLists[ACPI_MEM_LIST_OPERAND].ObjectSize    = sizeof (ACPI_OPERAND_OBJECT);
    AcpiGbl_MemoryLists[ACPI_MEM_LIST_WALK].ObjectSize       = sizeof (ACPI_WALK_STATE);

    AcpiGbl_MemoryLists[ACPI_MEM_LIST_STATE].MaxCacheDepth      = MAX_STATE_CACHE_DEPTH;
    AcpiGbl_MemoryLists[ACPI_MEM_LIST_PSNODE].MaxCacheDepth     = MAX_PARSE_CACHE_DEPTH;
    AcpiGbl_MemoryLists[ACPI_MEM_LIST_PSNODE_EXT].MaxCacheDepth = MAX_EXTPARSE_CACHE_DEPTH;
    AcpiGbl_MemoryLists[ACPI_MEM_LIST_OPERAND].MaxCacheDepth    = MAX_OBJECT_CACHE_DEPTH;
    AcpiGbl_MemoryLists[ACPI_MEM_LIST_WALK].MaxCacheDepth       = MAX_WALK_CACHE_DEPTH;


    ACPI_MEM_TRACKING (AcpiGbl_MemoryLists[ACPI_MEM_LIST_GLOBAL].ListName          = "Global Memory Allocation");
    ACPI_MEM_TRACKING (AcpiGbl_MemoryLists[ACPI_MEM_LIST_NSNODE].ListName          = "Namespace Nodes");
    ACPI_MEM_TRACKING (AcpiGbl_MemoryLists[ACPI_MEM_LIST_STATE].ListName           = "State Object Cache");
    ACPI_MEM_TRACKING (AcpiGbl_MemoryLists[ACPI_MEM_LIST_PSNODE].ListName          = "Parse Node Cache");
    ACPI_MEM_TRACKING (AcpiGbl_MemoryLists[ACPI_MEM_LIST_PSNODE_EXT].ListName      = "Extended Parse Node Cache");
    ACPI_MEM_TRACKING (AcpiGbl_MemoryLists[ACPI_MEM_LIST_OPERAND].ListName         = "Operand Object Cache");
    ACPI_MEM_TRACKING (AcpiGbl_MemoryLists[ACPI_MEM_LIST_WALK].ListName            = "Tree Walk Node Cache");

    /* ACPI table structure */

    for (i = 0; i < NUM_ACPI_TABLES; i++)
    {
        AcpiGbl_AcpiTables[i].Prev          = &AcpiGbl_AcpiTables[i];
        AcpiGbl_AcpiTables[i].Next          = &AcpiGbl_AcpiTables[i];
        AcpiGbl_AcpiTables[i].Pointer       = NULL;
        AcpiGbl_AcpiTables[i].Length        = 0;
        AcpiGbl_AcpiTables[i].Allocation    = ACPI_MEM_NOT_ALLOCATED;
        AcpiGbl_AcpiTables[i].Count         = 0;
    }


    /* Address Space handler array */

    for (i = 0; i < ACPI_NUM_ADDRESS_SPACES; i++)
    {
        AcpiGbl_AddressSpaces[i].Handler    = NULL;
        AcpiGbl_AddressSpaces[i].Context    = NULL;
    }

    /* Mutex locked flags */

    for (i = 0; i < NUM_MTX; i++)
    {
        AcpiGbl_AcpiMutexInfo[i].Mutex      = NULL;
        AcpiGbl_AcpiMutexInfo[i].OwnerId    = ACPI_MUTEX_NOT_ACQUIRED;
        AcpiGbl_AcpiMutexInfo[i].UseCount   = 0;
    }

    /* Global notify handlers */

    AcpiGbl_SysNotify.Handler           = NULL;
    AcpiGbl_DrvNotify.Handler           = NULL;

    /* Global "typed" ACPI table pointers */

    AcpiGbl_RSDP                        = NULL;
    AcpiGbl_XSDT                        = NULL;
    AcpiGbl_FACS                        = NULL;
    AcpiGbl_FADT                        = NULL;
    AcpiGbl_DSDT                        = NULL;


    /* Global Lock support */

    AcpiGbl_GlobalLockAcquired          = FALSE;
    AcpiGbl_GlobalLockThreadCount       = 0;

    /* Miscellaneous variables */

    AcpiGbl_SystemFlags                 = 0;
    AcpiGbl_StartupFlags                = 0;
    AcpiGbl_RsdpOriginalLocation        = 0;
    AcpiGbl_CmSingleStep                = FALSE;
    AcpiGbl_DbTerminateThreads          = FALSE;
    AcpiGbl_Shutdown                    = FALSE;
    AcpiGbl_NsLookupCount               = 0;
    AcpiGbl_PsFindCount                 = 0;
    AcpiGbl_AcpiHardwarePresent         = TRUE;
    AcpiGbl_NextTableOwnerId            = FIRST_TABLE_ID;
    AcpiGbl_NextMethodOwnerId           = FIRST_METHOD_ID;
    AcpiGbl_DebuggerConfiguration       = DEBUGGER_THREADING;


    /* Hardware oriented */

    AcpiGbl_Gpe0EnableRegisterSave      = NULL;
    AcpiGbl_Gpe1EnableRegisterSave      = NULL;
    AcpiGbl_OriginalMode                = SYS_MODE_UNKNOWN;   /*  original ACPI/legacy mode   */
    AcpiGbl_GpeRegisters                = NULL;
    AcpiGbl_GpeInfo                     = NULL;

    /* Namespace */

    AcpiGbl_RootNode                    = NULL;

    AcpiGbl_RootNodeStruct.Name         = ACPI_ROOT_NAME;
    AcpiGbl_RootNodeStruct.DataType     = ACPI_DESC_TYPE_NAMED;
    AcpiGbl_RootNodeStruct.Type         = ACPI_TYPE_ANY;
    AcpiGbl_RootNodeStruct.Child        = NULL;
    AcpiGbl_RootNodeStruct.Peer         = NULL;
    AcpiGbl_RootNodeStruct.Object       = NULL;
    AcpiGbl_RootNodeStruct.Flags        = ANOBJ_END_OF_PEER_LIST;


    return_VOID;
}


