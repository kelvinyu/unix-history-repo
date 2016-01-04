//===-- source/Host/netbsd/Host.cpp ------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// C Includes
#include <stdio.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/sysctl.h>
#include <sys/proc.h>

#include <limits.h>

#include <sys/ptrace.h>
#include <sys/exec.h>
#include <elf.h>
#include <kvm.h>

// C++ Includes
// Other libraries and framework includes
// Project includes
#include "lldb/Core/Error.h"
#include "lldb/Host/Endian.h"
#include "lldb/Host/Host.h"
#include "lldb/Host/HostInfo.h"
#include "lldb/Core/Module.h"
#include "lldb/Core/DataExtractor.h"
#include "lldb/Core/StreamFile.h"
#include "lldb/Core/StreamString.h"
#include "lldb/Core/Log.h"
#include "lldb/Target/Process.h"
#include "lldb/Target/Platform.h"

#include "lldb/Core/DataBufferHeap.h"
#include "lldb/Core/DataExtractor.h"
#include "lldb/Utility/CleanUp.h"
#include "lldb/Utility/NameMatches.h"

#include "llvm/Support/Host.h"

extern "C" {
    extern char **environ;
}

using namespace lldb;
using namespace lldb_private;

size_t
Host::GetEnvironment (StringList &env)
{
    char *v;
    char **var = environ;
    for (; var != NULL && *var != NULL; ++var)
    {
        v = ::strchr(*var, (int)'-');
        if (v == NULL)
            continue;
        env.AppendString(v);
    }
    return env.GetSize();
}

static bool
GetNetBSDProcessArgs (const ProcessInstanceInfoMatch *match_info_ptr,
                      ProcessInstanceInfo &process_info)
{
    if (!process_info.ProcessIDIsValid())
        return false;

    int pid = process_info.GetProcessID();

    int mib[4] = { CTL_KERN, KERN_PROC_ARGS, pid, KERN_PROC_ARGV };

    char arg_data[8192];
    size_t arg_data_size = sizeof(arg_data);
    if (::sysctl (mib, 4, arg_data, &arg_data_size , NULL, 0) != 0)
        return false;

    DataExtractor data (arg_data, arg_data_size, endian::InlHostByteOrder(), sizeof(void *));
    lldb::offset_t offset = 0;
    const char *cstr;

    cstr = data.GetCStr (&offset);
    if (!cstr)
        return false;

    process_info.GetExecutableFile().SetFile(cstr, false);

    if (!(match_info_ptr == NULL ||
        NameMatches (process_info.GetExecutableFile().GetFilename().GetCString(),
                     match_info_ptr->GetNameMatchType(),
                     match_info_ptr->GetProcessInfo().GetName())))
        return false;

    Args &proc_args = process_info.GetArguments();
    while (1)
    {
        const uint8_t *p = data.PeekData(offset, 1);
        while ((p != NULL) && (*p == '\0') && offset < arg_data_size)
        {
            ++offset;
            p = data.PeekData(offset, 1);
        }
        if (p == NULL || offset >= arg_data_size)
            break;

        cstr = data.GetCStr(&offset);
        if (!cstr)
            break;

        proc_args.AppendArgument(cstr);
    }

    return true;
}

static bool
GetNetBSDProcessCPUType (ProcessInstanceInfo &process_info)
{
    if (process_info.ProcessIDIsValid())
    {
        process_info.GetArchitecture() = HostInfo::GetArchitecture(HostInfo::eArchKindDefault);
        return true;
    }
    process_info.GetArchitecture().Clear();
    return false;
}

static bool
GetNetBSDProcessUserAndGroup(ProcessInstanceInfo &process_info)
{
    ::kvm_t *kdp;
    char errbuf[_POSIX2_LINE_MAX]; /* XXX: error string unused */

    struct ::kinfo_proc2 *proc_kinfo;
    const int pid = process_info.GetProcessID();
    int nproc;

    if (!process_info.ProcessIDIsValid())
        goto error;

    if ((kdp = ::kvm_openfiles(NULL, NULL, NULL, KVM_NO_FILES, errbuf)) == NULL)
        goto error;

    if ((proc_kinfo = ::kvm_getproc2(kdp, KERN_PROC_PID, pid,
                                     sizeof(struct ::kinfo_proc2),
                                     &nproc)) == NULL) {
        ::kvm_close(kdp);
        goto error;
    }

    if (nproc < 1) {
        ::kvm_close(kdp); /* XXX: we don't check for error here */
        goto error;
    }

    process_info.SetParentProcessID (proc_kinfo->p_ppid);
    process_info.SetUserID (proc_kinfo->p_ruid);
    process_info.SetGroupID (proc_kinfo->p_rgid);
    process_info.SetEffectiveUserID (proc_kinfo->p_uid);
    process_info.SetEffectiveGroupID (proc_kinfo->p_gid);

    ::kvm_close(kdp); /* XXX: we don't check for error here */

    return true;

error:
     process_info.SetParentProcessID (LLDB_INVALID_PROCESS_ID);
     process_info.SetUserID (UINT32_MAX);
     process_info.SetGroupID (UINT32_MAX);
     process_info.SetEffectiveUserID (UINT32_MAX);
     process_info.SetEffectiveGroupID (UINT32_MAX);
     return false;
}

uint32_t
Host::FindProcesses (const ProcessInstanceInfoMatch &match_info, ProcessInstanceInfoList &process_infos)
{
    const ::pid_t our_pid = ::getpid();
    const ::uid_t our_uid = ::getuid();

    const bool all_users = match_info.GetMatchAllUsers() ||
        // Special case, if lldb is being run as root we can attach to anything
        (our_uid == 0);

    kvm_t *kdp;
    char errbuf[_POSIX2_LINE_MAX]; /* XXX: error string unused */
    if ((kdp = ::kvm_openfiles(NULL, NULL, NULL, KVM_NO_FILES, errbuf)) == NULL)
        return 0;

    struct ::kinfo_proc2 *proc_kinfo;
    int nproc;
    if ((proc_kinfo = ::kvm_getproc2(kdp, KERN_PROC_ALL, 0,
                                     sizeof(struct ::kinfo_proc2),
                                     &nproc)) == NULL) {
        ::kvm_close(kdp);
        return 0;
    }

    for (int i = 0; i < nproc; i++) {
        if (proc_kinfo[i].p_pid < 1)
            continue; /* not valid */
        /* Make sure the user is acceptable */
        if (!all_users && proc_kinfo[i].p_ruid != our_uid)
             continue;

        if (proc_kinfo[i].p_pid  == our_pid || // Skip this process
            proc_kinfo[i].p_pid  == 0       || // Skip kernel (kernel pid is 0)
            proc_kinfo[i].p_stat == LSZOMB  || // Zombies are bad
            proc_kinfo[i].p_flag & P_TRACED || // Being debugged?
            proc_kinfo[i].p_flag & P_WEXIT)    // Working on exiting
             continue;


        // Every thread is a process in NetBSD, but all the threads of a single
        // process have the same pid. Do not store the process info in the
        // result list if a process with given identifier is already registered
        // there.
        if (proc_kinfo[i].p_nlwps > 1) {
            bool already_registered = false;
            for (size_t pi = 0; pi < process_infos.GetSize(); pi++) {
                if (process_infos.GetProcessIDAtIndex(pi) ==
                    proc_kinfo[i].p_pid) {
                    already_registered = true;
                    break;
                }
            }

            if (already_registered)
                continue;
        }
        ProcessInstanceInfo process_info;
        process_info.SetProcessID (proc_kinfo[i].p_pid);
        process_info.SetParentProcessID (proc_kinfo[i].p_ppid);
        process_info.SetUserID (proc_kinfo[i].p_ruid);
        process_info.SetGroupID (proc_kinfo[i].p_rgid);
        process_info.SetEffectiveUserID (proc_kinfo[i].p_uid);
        process_info.SetEffectiveGroupID (proc_kinfo[i].p_gid);
        // Make sure our info matches before we go fetch the name and cpu type
        if (match_info.Matches (process_info) &&
            GetNetBSDProcessArgs (&match_info, process_info))
        {
            GetNetBSDProcessCPUType (process_info);
            if (match_info.Matches (process_info))
                process_infos.Append (process_info);
        }
    }

    kvm_close(kdp); /* XXX: we don't check for error here */

    return process_infos.GetSize();
}

bool
Host::GetProcessInfo (lldb::pid_t pid, ProcessInstanceInfo &process_info)
{
    process_info.SetProcessID(pid);

    if (GetNetBSDProcessArgs(NULL, process_info))
    {
        GetNetBSDProcessCPUType(process_info);
        GetNetBSDProcessUserAndGroup(process_info);
        return true;
    }

    process_info.Clear();
    return false;
}

lldb::DataBufferSP
Host::GetAuxvData(lldb_private::Process *process)
{
	return lldb::DataBufferSP();
}

Error
Host::ShellExpandArguments (ProcessLaunchInfo &launch_info)
{
    return Error("unimplemented");
}
