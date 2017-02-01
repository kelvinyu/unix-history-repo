/*
 * Copyright (c) 2015 Netflix, Inc. All Rights Reserved.
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
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <stand.h>
#include <string.h>
#include <efi.h>
#include <efilib.h>
#include <uuid.h>
#include <stdbool.h>
#include "bootstrap.h"
#ifdef BOOT_FORTH
#include "ficl.h"
#endif

/*
 * Simple wrappers to the underlying UEFI functions.
 * See http://wiki.phoenix.com/wiki/index.php/EFI_RUNTIME_SERVICES
 * for details.
 */
EFI_STATUS
efi_get_next_variable_name(UINTN *variable_name_size, CHAR16 *variable_name,
    EFI_GUID *vendor_guid)
{
	return (RS->GetNextVariableName(variable_name_size, variable_name,
	    vendor_guid));
}

EFI_STATUS
efi_get_variable(CHAR16 *variable_name, EFI_GUID *vendor_guid,
    UINT32 *attributes, UINTN *data_size, void *data)
{
	return (RS->GetVariable(variable_name, vendor_guid, attributes,
	    data_size, data));
}

EFI_STATUS
efi_set_variable(CHAR16 *variable_name, EFI_GUID *vendor_guid,
    UINT32 attributes, UINTN data_size, void *data)
{
	return (RS->SetVariable(variable_name, vendor_guid, attributes,
	    data_size, data));
}

void
efi_init_environment(void)
{
	char var[128];

	snprintf(var, sizeof(var), "%d.%02d", ST->Hdr.Revision >> 16,
	    ST->Hdr.Revision & 0xffff);
	env_setenv("efi-version", EV_VOLATILE, var, env_noset, env_nounset);
}

COMMAND_SET(efishow, "efi-show", "print some or all EFI variables", command_efi_show);

static int
efi_print_var(CHAR16 *varnamearg, EFI_GUID *matchguid, int lflag)
{
	UINTN		datasz, i;
	EFI_STATUS	status;
	UINT32		attr;
	CHAR16		*data;
	char		*str;
	uint32_t	uuid_status;
	int		is_ascii;

	datasz = 0;
	status = RS->GetVariable(varnamearg, matchguid, &attr,
	    &datasz, NULL);
	if (status != EFI_BUFFER_TOO_SMALL) {
		printf("Can't get the variable: error %#lx\n", status);
		return (CMD_ERROR);
	}
	data = malloc(datasz);
	status = RS->GetVariable(varnamearg, matchguid, &attr,
	    &datasz, data);
	if (status != EFI_SUCCESS) {
		printf("Can't get the variable: error %#lx\n", status);
		return (CMD_ERROR);
	}
	uuid_to_string((uuid_t *)matchguid, &str, &uuid_status);
	if (lflag) {
		printf("%s 0x%x %S", str, attr, varnamearg);
	} else {
		printf("%s 0x%x %S=", str, attr, varnamearg);
		is_ascii = 1;
		free(str);
		str = (char *)data;
		for (i = 0; i < datasz - 1; i++) {
			/* Quick hack to see if this ascii-ish string printable range plus tab, cr and lf */
			if ((str[i] < 32 || str[i] > 126) && str[i] != 9 && str[i] != 10 && str[i] != 13) {
				is_ascii = 0;
				break;
			}
		}
		if (str[datasz - 1] != '\0')
			is_ascii = 0;
		if (is_ascii)
			printf("%s", str);
		else {
			for (i = 0; i < datasz / 2; i++) {
				if (isalnum(data[i]) || isspace(data[i]))
					printf("%c", data[i]);
				else
					printf("\\x%02x", data[i]);
			}
		}
	}
	free(data);
	if (pager_output("\n"))
		return (CMD_WARN);
	return (CMD_OK);
}

static int
command_efi_show(int argc, char *argv[])
{
	/*
	 * efi-show [-a]
	 *	print all the env
	 * efi-show -u UUID
	 *	print all the env vars tagged with UUID
	 * efi-show -v var
	 *	search all the env vars and print the ones matching var
	 * eif-show -u UUID -v var
	 * eif-show UUID var
	 *	print all the env vars that match UUID and var
	 */
	/* NB: We assume EFI_GUID is the same as uuid_t */
	int		aflag = 0, gflag = 0, lflag = 0, vflag = 0;
	int		ch, rv;
	unsigned	i;
	EFI_STATUS	status;
	EFI_GUID	varguid = { 0,0,0,{0,0,0,0,0,0,0,0} };
	EFI_GUID	matchguid = { 0,0,0,{0,0,0,0,0,0,0,0} };
	uint32_t	uuid_status;
	CHAR16		*varname;
	CHAR16		*newnm;
	CHAR16		varnamearg[128];
	UINTN		varalloc;
	UINTN		varsz;

	while ((ch = getopt(argc, argv, "ag:lv:")) != -1) {
		switch (ch) {
		case 'a':
			aflag = 1;
			break;
		case 'g':
			gflag = 1;
			uuid_from_string(optarg, (uuid_t *)&matchguid,
			    &uuid_status);
			if (uuid_status != uuid_s_ok) {
				printf("uid %s could not be parsed\n", optarg);
				return (CMD_ERROR);
			}
			break;
		case 'l':
			lflag = 1;
			break;
		case 'v':
			vflag = 1;
			if (strlen(optarg) >= nitems(varnamearg)) {
				printf("Variable %s is longer than %zd characters\n",
				    optarg, nitems(varnamearg));
				return (CMD_ERROR);
			}
			for (i = 0; i < strlen(optarg); i++)
				varnamearg[i] = optarg[i];
			varnamearg[i] = 0;
			break;
		default:
			printf("Invalid argument %c\n", ch);
			return (CMD_ERROR);
		}
	}

	if (aflag && (gflag || vflag)) {
		printf("-a isn't compatible with -v or -u\n");
		return (CMD_ERROR);
	}

	if (aflag && optind < argc) {
		printf("-a doesn't take any args\n");
		return (CMD_ERROR);
	}

	if (optind == argc)
		aflag = 1;

	argc -= optind;
	argv += optind;

	pager_open();
	if (vflag && gflag) {
		rv = efi_print_var(varnamearg, &matchguid, lflag);
		pager_close();
		return (rv);
	}

	if (argc == 2) {
		optarg = argv[0];
		if (strlen(optarg) >= nitems(varnamearg)) {
			printf("Variable %s is longer than %zd characters\n",
			    optarg, nitems(varnamearg));
			pager_close();
			return (CMD_ERROR);
		}
		for (i = 0; i < strlen(optarg); i++)
			varnamearg[i] = optarg[i];
		varnamearg[i] = 0;
		optarg = argv[1];
		uuid_from_string(optarg, (uuid_t *)&matchguid,
		    &uuid_status);
		if (uuid_status != uuid_s_ok) {
			printf("uid %s could not be parsed\n", optarg);
			pager_close();
			return (CMD_ERROR);
		}
		rv = efi_print_var(varnamearg, &matchguid, lflag);
		pager_close();
		return (rv);
	}

	if (argc > 0) {
		printf("Too many args %d\n", argc);
		pager_close();
		return (CMD_ERROR);
	}

	/*
	 * Initiate the search -- note the standard takes pain
	 * to specify the initial call must be a poiner to a NULL
	 * character.
	 */
	varalloc = 1024;
	varname = malloc(varalloc);
	if (varname == NULL) {
		printf("Can't allocate memory to get variables\n");
		pager_close();
		return (CMD_ERROR);
	}
	varname[0] = 0;
	while (1) {
		varsz = varalloc;
		status = RS->GetNextVariableName(&varsz, varname, &varguid);
		if (status == EFI_BUFFER_TOO_SMALL) {
			varalloc = varsz;
			newnm = realloc(varname, varalloc);
			if (newnm == NULL) {
				printf("Can't allocate memory to get variables\n");
				free(varname);
				pager_close();
				return (CMD_ERROR);
			}
			varname = newnm;
			continue; /* Try again with bigger buffer */
		}
		if (status != EFI_SUCCESS)
			break;
		if (aflag) {
			if (efi_print_var(varname, &varguid, lflag) != CMD_OK)
				break;
			continue;
		}
		if (vflag) {
			if (wcscmp(varnamearg, varname) == 0) {
				if (efi_print_var(varname, &varguid, lflag) != CMD_OK)
					break;
				continue;
			}
		}
		if (gflag) {
			if (memcmp(&varguid, &matchguid, sizeof(varguid)) == 0) {
				if (efi_print_var(varname, &varguid, lflag) != CMD_OK)
					break;
				continue;
			}
		}
	}
	free(varname);
	pager_close();

	return (CMD_OK);
}

COMMAND_SET(efiset, "efi-set", "set EFI variables", command_efi_set);

static int
command_efi_set(int argc, char *argv[])
{
	char *uuid, *var, *val;
	CHAR16 wvar[128];
	EFI_GUID guid;
	uint32_t status;
	EFI_STATUS err;

	if (argc != 4) {
		printf("efi-set uuid var new-value\n");
		return (CMD_ERROR);
	}
	uuid = argv[1];
	var = argv[2];
	val = argv[3];
	uuid_from_string(uuid, (uuid_t *)&guid, &status);
	if (status != uuid_s_ok) {
		printf("Invalid uuid %s %d\n", uuid, status);
		return (CMD_ERROR);
	}
	cpy8to16(var, wvar, sizeof(wvar));
	err = RS->SetVariable(wvar, &guid,
	    EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS,
	    strlen(val) + 1, val);
	if (EFI_ERROR(err)) {
		printf("Failed to set variable: error %lu\n", EFI_ERROR_CODE(err));
		return (CMD_ERROR);
	}
	return (CMD_OK);
}

COMMAND_SET(efiunset, "efi-unset", "delete / unset EFI variables", command_efi_unset);

static int
command_efi_unset(int argc, char *argv[])
{
	char *uuid, *var;
	CHAR16 wvar[128];
	EFI_GUID guid;
	uint32_t status;
	EFI_STATUS err;

	if (argc != 3) {
		printf("efi-unset uuid var\n");
		return (CMD_ERROR);
	}
	uuid = argv[1];
	var = argv[2];
	uuid_from_string(uuid, (uuid_t *)&guid, &status);
	if (status != uuid_s_ok) {
		printf("Invalid uuid %s\n", uuid);
		return (CMD_ERROR);
	}
	cpy8to16(var, wvar, sizeof(wvar));
	err = RS->SetVariable(wvar, &guid, 0, 0, NULL);
	if (EFI_ERROR(err)) {
		printf("Failed to unset variable: error %lu\n", EFI_ERROR_CODE(err));
		return (CMD_ERROR);
	}
	return (CMD_OK);
}

#ifdef BOOT_FORTH
/*
 * FreeBSD's loader interaction words and extras
 *
 *	efi-setenv  ( value n name n guid n attr -- 0 | -1)
 * 	efi-getenv  ( guid n addr n -- addr' n' | -1 )
 * 	efi-unsetenv ( name n guid n'' -- )
 */

/*
 * efi-setenv
 * 	efi-setenv  ( value n name n guid n attr -- 0 | -1)
 *
 * Set environment variables using the SetVariable EFI runtime service.
 *
 * Value and guid are passed through in binary form (so guid needs to be
 * converted to binary form from its string form). Name is converted from
 * ASCII to CHAR16. Since ficl doesn't have support for internationalization,
 * there's no native CHAR16 interface provided.
 *
 * attr is an int in the bitmask of the following attributes for this variable.
 *
 *	1	Non volatile
 *	2	Boot service access
 *	4	Run time access
 * (corresponding to the same bits in the UEFI spec).
 */
static void
ficlEfiSetenv(FICL_VM *pVM)
{
	char	*value = NULL, *guid = NULL;
	CHAR16	*name = NULL;
	int	i;
	char	*namep, *valuep, *guidp;
	int	names, values, guids, attr;
	EFI_STATUS status;
	uuid_t	u;
	uint32_t ustatus;
	bool	error = true;

#if FICL_ROBUST > 1
	vmCheckStack(pVM, 6, 0);
#endif
	attr = stackPopINT(pVM->pStack);
	guids = stackPopINT(pVM->pStack);
	guidp = (char*)stackPopPtr(pVM->pStack);
	names = stackPopINT(pVM->pStack);
	namep = (char*)stackPopPtr(pVM->pStack);
	values = stackPopINT(pVM->pStack);
	valuep = (char*)stackPopPtr(pVM->pStack);

	guid = (char*)ficlMalloc(guids);
	if (guid == NULL)
		goto out;
	memcpy(guid, guidp, guids);
	uuid_from_string(guid, &u, &ustatus);
	if (ustatus != uuid_s_ok) {
		stackPushINT(pVM->pStack, -1);
		goto out;
	}

	name = ficlMalloc((names + 1) * sizeof(CHAR16));
	if (name == NULL)
		goto out;
	for (i = 0; i < names; i++)
		name[i] = namep[i];
	name[names] = 0;

	value = ficlMalloc(values + 1);
	if (value == NULL)
		goto out;
	memcpy(value, valuep, values);

	status = efi_set_variable(name, (EFI_GUID *)&u, attr, values, value);
	if (status == EFI_SUCCESS)
		stackPushINT(pVM->pStack, 0);
	else
		stackPushINT(pVM->pStack, -1);
	error = false;
out:
	ficlFree(name);
	ficlFree(value);
	ficlFree(guid);

	if (error == true)
		vmThrowErr(pVM, "Error: out of memory");
}

static void
ficlEfiGetenv(FICL_VM *pVM)
{
	char	*name, *value;
	char	*namep;
	int	names;

#if FICL_ROBUST > 1
	vmCheckStack(pVM, 2, 2);
#endif
	names = stackPopINT(pVM->pStack);
	namep = (char*) stackPopPtr(pVM->pStack);

	name = (char*) ficlMalloc(names+1);
	if (name == NULL)
		vmThrowErr(pVM, "Error: out of memory");
	strncpy(name, namep, names);
	name[names] = '\0';

	value = getenv(name);
	ficlFree(name);

	if(value != NULL) {
		stackPushPtr(pVM->pStack, value);
		stackPushINT(pVM->pStack, strlen(value));
	} else
		stackPushINT(pVM->pStack, -1);
}

static void
ficlEfiUnsetenv(FICL_VM *pVM)
{
	char	*name;
	char	*namep;
	int	names;

#if FICL_ROBUST > 1
	vmCheckStack(pVM, 2, 0);
#endif
	names = stackPopINT(pVM->pStack);
	namep = (char*) stackPopPtr(pVM->pStack);

	name = (char*) ficlMalloc(names+1);
	if (name == NULL)
		vmThrowErr(pVM, "Error: out of memory");
	strncpy(name, namep, names);
	name[names] = '\0';

	unsetenv(name);
	ficlFree(name);
}

/**************************************************************************
** Add FreeBSD UEFI platform extensions into the system dictionary
**************************************************************************/
void ficlEfiCompilePlatform(FICL_SYSTEM *pSys)
{
	FICL_DICT *dp = pSys->dp;
	assert (dp);

	dictAppendWord(dp, "efi-setenv",    ficlEfiSetenv,	FW_DEFAULT);
	dictAppendWord(dp, "efi-getenv",    ficlEfiGetenv,	FW_DEFAULT);
	dictAppendWord(dp, "efi-unsetenv",  ficlEfiUnsetenv,    FW_DEFAULT);
}

FICL_COMPILE_SET(ficlEfiCompilePlatform);

#endif	/* BOOT_FORTH */
