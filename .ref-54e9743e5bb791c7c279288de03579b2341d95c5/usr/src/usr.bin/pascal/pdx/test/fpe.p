(*
 * Copyright (c) 1980, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)fpe.p	8.1 (Berkeley) %G%
 *)

{
  test of floating point exception handling
}

program fpe(input, output);
var x, y : real;
begin
	x := 1;
	y := 0;
	writeln(x/y);
end.
