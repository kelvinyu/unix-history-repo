/* Core file generic interface routines for BFD.
   Copyright 1990, 1991, 1992, 1993, 1994, 2000, 2001, 2002, 2003, 2005
   Free Software Foundation, Inc.
   Written by Cygnus Support.

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
Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.  */

/*
SECTION
	Core files

SUBSECTION
	Core file functions

DESCRIPTION
	These are functions pertaining to core files.
*/

#include "bfd.h"
#include "sysdep.h"
#include "libbfd.h"

/*
FUNCTION
	bfd_core_file_failing_command

SYNOPSIS
	const char *bfd_core_file_failing_command (bfd *abfd);

DESCRIPTION
	Return a read-only string explaining which program was running
	when it failed and produced the core file @var{abfd}.

*/

const char *
bfd_core_file_failing_command (bfd *abfd)
{
  if (abfd->format != bfd_core)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return NULL;
    }
  return BFD_SEND (abfd, _core_file_failing_command, (abfd));
}

/*
FUNCTION
	bfd_core_file_failing_signal

SYNOPSIS
	int bfd_core_file_failing_signal (bfd *abfd);

DESCRIPTION
	Returns the signal number which caused the core dump which
	generated the file the BFD @var{abfd} is attached to.
*/

int
bfd_core_file_failing_signal (bfd *abfd)
{
  if (abfd->format != bfd_core)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return 0;
    }
  return BFD_SEND (abfd, _core_file_failing_signal, (abfd));
}

/*
FUNCTION
	core_file_matches_executable_p

SYNOPSIS
	bfd_boolean core_file_matches_executable_p
	  (bfd *core_bfd, bfd *exec_bfd);

DESCRIPTION
	Return <<TRUE>> if the core file attached to @var{core_bfd}
	was generated by a run of the executable file attached to
	@var{exec_bfd}, <<FALSE>> otherwise.
*/

bfd_boolean
core_file_matches_executable_p (bfd *core_bfd, bfd *exec_bfd)
{
  if (core_bfd->format != bfd_core || exec_bfd->format != bfd_object)
    {
      bfd_set_error (bfd_error_wrong_format);
      return FALSE;
    }

  return BFD_SEND (core_bfd, _core_file_matches_executable_p,
		   (core_bfd, exec_bfd));
}

/*
FUNCTION
        generic_core_file_matches_executable_p

SYNOPSIS
        bfd_boolean generic_core_file_matches_executable_p
          (bfd *core_bfd, bfd *exec_bfd);

DESCRIPTION
        Return TRUE if the core file attached to @var{core_bfd}
        was generated by a run of the executable file attached
        to @var{exec_bfd}.  The match is based on executable
        basenames only.

        Note: When not able to determine the core file failing
        command or the executable name, we still return TRUE even
        though we're not sure that core file and executable match.
        This is to avoid generating a false warning in situations
        where we really don't know whether they match or not.
*/

bfd_boolean
generic_core_file_matches_executable_p (bfd *core_bfd, bfd *exec_bfd)
{
  char *exec;
  char *core;
  char *last_slash;

  if (exec_bfd == NULL || core_bfd == NULL)
    return TRUE;

  /* The cast below is to avoid a compiler warning due to the assignment
     of the const char * returned by bfd_core_file_failing_command to a
     non-const char *.  In this case, the assignement does not lead to
     breaking the const, as we're only reading the string.  */
     
  core = (char *) bfd_core_file_failing_command (core_bfd);
  if (core == NULL)
    return TRUE;

  exec = bfd_get_filename (exec_bfd);
  if (exec == NULL)
    return TRUE;

  last_slash = strrchr (core, '/');
  if (last_slash != NULL)
    core = last_slash + 1;

  last_slash = strrchr (exec, '/');
  if (last_slash != NULL)
    exec = last_slash + 1;
  
  return strcmp (exec, core) == 0;
}

