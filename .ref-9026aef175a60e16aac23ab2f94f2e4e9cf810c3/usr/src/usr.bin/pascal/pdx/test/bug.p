(*
 * Copyright (c) 1980, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)bug.p	8.1 (Berkeley) %G%
 *)

program test (output);
var a : array [1..10] of integer;
procedure foo;
begin
 a[4] := 100;
 a[5] := 100;
 a[6] := 100;
end;
function bar (x:integer):integer;
  begin
    bar := 10*x;
  end;
begin
 a[1] := 100;
 foo;
 a[2] := bar(2);
 a[3] := 100;
end.
