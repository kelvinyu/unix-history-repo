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
static char sccsid[] = "@(#)lstAppend.c	5.2 (Berkeley) %G%";
#endif /* not lint */

/*-
 * LstAppend.c --
 *	Add a new node with a new datum after an existing node
 */

#include	"lstInt.h"

/*-
 *-----------------------------------------------------------------------
 * Lst_Append --
 *	Create a new node and add it to the given list after the given node.
 *
 * Results:
 *	SUCCESS if all went well.
 *
 * Side Effects:
 *	A new ListNode is created and linked in to the List. The lastPtr
 *	field of the List will be altered if ln is the last node in the
 *	list. lastPtr and firstPtr will alter if the list was empty and
 *	ln was NILLNODE.
 *
 *-----------------------------------------------------------------------
 */
ReturnStatus
Lst_Append (l, ln, d)
    Lst	  	l;	/* affected list */
    LstNode	ln;	/* node after which to append the datum */
    ClientData	d;	/* said datum */
{
    register List 	list;
    register ListNode	lNode;
    register ListNode	nLNode;
    
    if (LstValid (l) && (ln == NILLNODE && LstIsEmpty (l))) {
	goto ok;
    }
    
    if (!LstValid (l) || LstIsEmpty (l)  || ! LstNodeValid (ln, l)) {
	return (FAILURE);
    }
    ok:
    
    list = (List)l;
    lNode = (ListNode)ln;

    PAlloc (nLNode, ListNode);
    nLNode->datum = d;
    nLNode->useCount = nLNode->flags = 0;
    
    if (lNode == NilListNode) {
	if (list->isCirc) {
	    nLNode->nextPtr = nLNode->prevPtr = nLNode;
	} else {
	    nLNode->nextPtr = nLNode->prevPtr = NilListNode;
	}
	list->firstPtr = list->lastPtr = nLNode;
    } else {
	nLNode->prevPtr = lNode;
	nLNode->nextPtr = lNode->nextPtr;
	
	lNode->nextPtr = nLNode;
	if (nLNode->nextPtr != NilListNode) {
	    nLNode->nextPtr->prevPtr = nLNode;
	}
	
	if (lNode == list->lastPtr) {
	    list->lastPtr = nLNode;
	}
    }
    
    return (SUCCESS);
}

