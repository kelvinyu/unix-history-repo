.\" Copyright (c) 1983, 1993
.\"	The Regents of the University of California.  All rights reserved.
.\"
.\" %sccs.include.redist.roff%
.\"
.\"	@(#)2.5.t	8.3 (Berkeley) %G%
.\"
.Sh 2 "Process debugging
.Sh 3 "Traditional debugging
.LP
Debuggers traditionally use the
.Fn ptrace
interface:
.DS
.Fd ptrace 4 "process trace
ptrace(request, pid, addr, data);
int request, pid, *addr, data;
.DE
This interface provides a means by which a parent process may
control the execution of a child process,
and examine and change its core image.
Its primary use is for the implementation of breakpoint debugging.
There are four arguments whose interpretation
depends on a request argument.
Generally, pid is the process ID of the traced process,
which must be a child (no more distant descendant)
of the tracing process.
A process being traced behaves normally until it
encounters some signal whether internally generated like
``illegal instruction'' or externally generated
like ``interrupt.''
Then the traced process enters a stopped state
and its parent is notified via
.Fn wait .
When the child is in the stopped state,
its core image can be examined and modified using
.Fn ptrace .
If desired, another ptrace request can then cause
the child either to terminate or to continue,
possibly ignoring the signal.
.PP
A more general interface is also provided in 4.4BSD;
the \fImount_procfs\fP filesystem attaches an instance of
the process name space to the global filesystem name space.
The conventional mount point is \fI/proc\fP.
The root of the process filesystem contains an entry for each active process.
These processes are visible as a directory whose name is the process' pid.
In addition, the special entry curproc references the current process.
Each directory contains several files, including a \fIctl\fP file.
The debugger finds (or creates) the process that it wants to
debug and then issues an attach command via the \fIctl\fP file.
Further interaction can then be done with the process through
the other files provided by the \fI/proc\fP filesystem.
This interface has the benefit of being able to debug
arbitrary processes on the system rather than just processes
started by the debugger.
.Sh 3 "Kernel tracing
.LP
Another facility for debugging programs is provided by the
.Fn ktrace
interface:
.DS
.Fd ktrace 4 "process tracing
ktrace(tracefile, ops, trpoints, pid);
char *tracefile; int ops, trpoints, pid;
.DE
.Fn Ktrace
does kernel trace logging for the specified processes.
The kernel operations that are traced include system calls,
pathname translations, signal processing, and I/O.
This facility can be particularly useful for debugging
programs for which you do not have the source.
.\"
.\" We currently do not document the vtrace call
.\" .Fd vtrace 2
