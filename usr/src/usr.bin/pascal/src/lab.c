/* Copyright (c) 1979 Regents of the University of California */

static	char sccsid[] = "@(#)lab.c 1.1 %G%";

#include "whoami.h"
#include "0.h"
#include "tree.h"
#include "opcode.h"
#include "objfmt.h"
#ifdef PC
#   include	"pc.h"
#   include	"pcops.h"
#endif PC

/*
 * Label enters the definitions
 * of the label declaration part
 * into the namelist.
 */
label(r, l)
	int *r, l;
{
#ifndef PI0
	register *ll;
	register struct nl *p, *lp;

	lp = NIL;
#else
	send(REVLAB, r);
#endif
	if ( ! progseen ) {
	    level1();
	}
	line = l;
#ifndef PI1
#ifdef PC
    if (opt('s')) {
	if (parts & (CPRT|TPRT|VPRT)){
		standard();
		error("Label declarations must precede const, type and var declarations");
	}
	if (parts & LPRT) {
		standard();
		error("All labels must be declared in one label part");
	}
    }
#endif PC
#ifdef OBJ
	if (parts & (CPRT|TPRT|VPRT))
		error("Label declarations must precede const, type and var declarations");
	if (parts & LPRT)
		error("All labels must be declared in one label part");
#endif OBJ
	parts |= LPRT;
#endif
#ifndef PI0
	for (ll = r; ll != NIL; ll = ll[2]) {
		l = getlab();
		p = enter(defnl(ll[1], LABEL, 0, l));
		/*
		 * Get the label for the eventual target
		 */
		p->value[1] = getlab();
		p->chain = lp;
		p->nl_flags |= (NFORWD|NMOD);
		p->value[NL_GOLEV] = NOTYET;
		p->entloc = l;
		lp = p;
#		ifdef OBJ
		    /*
		     * This operator is between
		     * the bodies of two procedures
		     * and provides a target for
		     * gotos for this label via TRA.
		     */
		    putlab(l);
		    put2(O_GOTO | cbn<<8+INDX, p->value[1]);
#		endif OBJ
#		ifdef PC
		    /*
		     *	labels have to be .globl otherwise /lib/c2 may
		     *	throw them away if they aren't used in the function
		     *	which defines them.
		     */
		    if (cbn == 1) {
				/*
				 *	stab the label for separate compilation.
				 *	make label number = label name.
				 */
			    p -> value[1] = atol( p -> symbol );
			    stabglab( p -> value[1] );
			    putprintf( "	.globl	" , 1 );
			    putprintf( PREFIXFORMAT , 0 , PLABELPREFIX
					, p -> value[1] );
		    } else {
			    putprintf( "	.globl	" , 1 );
			    putprintf( PREFIXFORMAT , 0 , GLABELPREFIX
					, p -> value[1] );
		    }
#		endif PC
	}
	gotos[cbn] = lp;
#	ifdef PTREE
	    {
		pPointer	Labels = LabelDCopy( r );

		pDEF( PorFHeader[ nesting ] ).PorFLabels = Labels;
	    }
#	endif PTREE
#endif
}

#ifndef PI0
/*
 * Gotoop is called when
 * we get a statement "goto label"
 * and generates the needed tra.
 */
gotoop(s)
	char *s;
{
	register struct nl *p;

	gocnt++;
	p = lookup(s);
	if (p == NIL)
		return (NIL);
#	ifdef OBJ
	    put2(O_TRA4, p->entloc);
#	endif OBJ
#	ifdef PC
	    if ( cbn != bn ) {
		    /*
		     *	call goto to unwind the stack to the destination level
		     */
		putleaf( P2ICON , 0 , 0 , ADDTYPE( P2FTN | P2INT , P2PTR )
			, "_GOTO" );
		putLV( DISPLAYNAME , 0 , bn * sizeof( struct dispsave )
			, P2PTR | P2INT );
		putop( P2CALL , P2INT );
		putdot( filename , line );
	    }
	    if ( bn <= 1 ) {
		printjbr( PLABELPREFIX , p -> value[1] );
	    } else {
		printjbr( GLABELPREFIX , p -> value[1] );
	    }
#	endif PC
	if (bn == cbn)
		if (p->nl_flags & NFORWD) {
			if (p->value[NL_GOLEV] == NOTYET) {
				p->value[NL_GOLEV] = level;
				p->value[NL_GOLINE] = line;
			}
		} else
			if (p->value[NL_GOLEV] == DEAD) {
				recovered();
				error("Goto %s is into a structured statement", p->symbol);
			}
}

/*
 * Labeled is called when a label
 * definition is encountered, and
 * marks that it has been found and
 * patches the associated GOTO generated
 * by gotoop.
 */
labeled(s)
	char *s;
{
	register struct nl *p;

	p = lookup(s);
	if (p == NIL)
		return (NIL);
	if (bn != cbn) {
		error("Label %s not defined in correct block", s);
		return;
	}
	if ((p->nl_flags & NFORWD) == 0) {
		error("Label %s redefined", s);
		return;
	}
	p->nl_flags &= ~NFORWD;
#	ifdef OBJ
	    patch4(p->entloc);
#	endif OBJ
#	ifdef PC
	    if ( bn <= 1 ) {
		putprintf( PREFIXFORMAT , 1 , PLABELPREFIX , p -> value[1] );
	    } else {
		putprintf( PREFIXFORMAT , 1 , GLABELPREFIX , p -> value[1] );
	    }
	    putprintf( ":" , 0 );
#	endif PC
	if (p->value[NL_GOLEV] != NOTYET)
		if (p->value[NL_GOLEV] < level) {
			recovered();
			error("Goto %s from line %d is into a structured statement", s, p->value[NL_GOLINE]);
		}
	p->value[NL_GOLEV] = level;
}
#endif
