/*
 * Copyright (c) 1988, 1989, 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Adam de Boor.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
static char sccsid[] = "@(#)lstNext.c	5.2 (Berkeley) %G%";
#endif /* not lint */

/*-
 * LstNext.c --
 *	Return the next node for a list.
 *	The sequential functions access the list in a slightly different way.
 *	CurPtr points to their idea of the current node in the list and they
 *	access the list based on it. Because the list is circular, Lst_Next
 *	and Lst_Prev will go around the list forever. Lst_IsAtEnd must be
 *	used to determine when to stop.
 */

#include	"lstInt.h"

/*-
 *-----------------------------------------------------------------------
 * Lst_Next --
 *	Return the next node for the given list.
 *
 * Results:
 *	The next node or NILLNODE if the list has yet to be opened. Also
 *	if the list is non-circular and the end has been reached, NILLNODE
 *	is returned.
 *
 * Side Effects:
 *	the curPtr field is updated.
 *
 *-----------------------------------------------------------------------
 */
LstNode
Lst_Next (l)
    Lst	    	  l;
{
    register ListNode	tln;
    register List 	list = (List)l;
    
    if ((LstValid (l) == FALSE) ||
	(list->isOpen == FALSE)) {
	    return (NILLNODE);
    }
    
    list->prevPtr = list->curPtr;
    
    if (list->curPtr == NilListNode) {
	if (list->atEnd == Unknown) {
	    /*
	     * If we're just starting out, atEnd will be Unknown.
	     * Then we want to start this thing off in the right
	     * direction -- at the start with atEnd being Middle.
	     */
	    list->curPtr = tln = list->firstPtr;
	    list->atEnd = Middle;
	} else {
	    tln = NilListNode;
	    list->atEnd = Tail;
	}
    } else {
	tln = list->curPtr->nextPtr;
	list->curPtr = tln;

	if (tln == list->firstPtr || tln == NilListNode) {
	    /*
	     * If back at the front, then we've hit the end...
	     */
	    list->atEnd = Tail;
	} else {
	    /*
	     * Reset to Middle if gone past first.
	     */
	    list->atEnd = Middle;
	}
    }
    
    return ((LstNode)tln);
}

