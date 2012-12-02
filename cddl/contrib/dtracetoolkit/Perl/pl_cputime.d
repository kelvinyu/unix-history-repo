#!/usr/sbin/dtrace -Zs
/*
 * pl_cputime.d - measure Perl on-CPU times for subroutines.
 *                Written for the Perl DTrace provider.
 *
 * $Id: pl_cputime.d 41 2007-09-17 02:20:10Z brendan $
 *
 * This traces Perl activity from all programs running on the system with
 * Perl provider support.
 *
 * USAGE: pl_cputime.d 		# hit Ctrl-C to end
 *
 * FIELDS:
 *		FILE		Filename of the Perl program
 *		TYPE		Type of call (sub/total)
 *		NAME		Name of call (subroutine name)
 *		TOTAL		Total on-CPU time for calls (us)
 *
 * Filename and subroutine names are printed if available.
 *
 * COPYRIGHT: Copyright (c) 2007 Brendan Gregg.
 *
 * CDDL HEADER START
 *
 *  The contents of this file are subject to the terms of the
 *  Common Development and Distribution License, Version 1.0 only
 *  (the "License").  You may not use this file except in compliance
 *  with the License.
 *
 *  You can obtain a copy of the license at Docs/cddl1.txt
 *  or http://www.opensolaris.org/os/licensing.
 *  See the License for the specific language governing permissions
 *  and limitations under the License.
 *
 * CDDL HEADER END
 *
 * 09-Sep-2007	Brendan Gregg	Created this.
 */

#pragma D option quiet

dtrace:::BEGIN
{
	printf("Tracing... Hit Ctrl-C to end.\n");
}

perl*:::sub-entry
{
	self->depth++;
	self->exclude[self->depth] = 0;
	self->sub[self->depth] = vtimestamp;
}

perl*:::sub-return
/self->sub[self->depth]/
{
	this->oncpu_incl = vtimestamp - self->sub[self->depth];
	this->oncpu_excl = this->oncpu_incl - self->exclude[self->depth];
	self->sub[self->depth] = 0;
	self->exclude[self->depth] = 0;
	this->file = basename(copyinstr(arg1));
	this->name = copyinstr(arg0);

	@num[this->file, "sub", this->name] = count();
	@num["-", "total", "-"] = count();
	@types_incl[this->file, "sub", this->name] = sum(this->oncpu_incl);
	@types_excl[this->file, "sub", this->name] = sum(this->oncpu_excl);
	@types_excl["-", "total", "-"] = sum(this->oncpu_excl);

	self->depth--;
	self->exclude[self->depth] += this->oncpu_incl;
}

dtrace:::END
{
	printf("\nCount,\n");
	printf("   %-20s %-10s %-32s %8s\n", "FILE", "TYPE", "NAME", "COUNT");
	printa("   %-20s %-10s %-32s %@8d\n", @num);

	normalize(@types_excl, 1000);
	printf("\nExclusive subroutine on-CPU times (us),\n");
	printf("   %-20s %-10s %-32s %8s\n", "FILE", "TYPE", "NAME", "TOTAL");
	printa("   %-20s %-10s %-32s %@8d\n", @types_excl);

	normalize(@types_incl, 1000);
	printf("\nInclusive subroutine on-CPU times (us),\n");
	printf("   %-20s %-10s %-32s %8s\n", "FILE", "TYPE", "NAME", "TOTAL");
	printa("   %-20s %-10s %-32s %@8d\n", @types_incl);
}
