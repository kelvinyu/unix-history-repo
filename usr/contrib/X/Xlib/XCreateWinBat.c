#include <X/mit-copyright.h>

/* $Header: XCreateWinBat.c,v 10.4 86/02/01 15:31:31 tony Rel $ */
/* Copyright    Massachusetts Institute of Technology    1985	*/

#include "XlibInternal.h"

int XCreateWindowBatch(defs, ndefs)
	BatchFrame defs[];
	int ndefs;
{
	register Display *dpy;
	register int i;
	register BatchFrame *frame = defs;
	int nresult = 0;

	/*
	 * Issue requests.
	 */
	for (i=0; i < ndefs; i++) {
	    register XReq *req;
	    if (frame->type == IsOpaque) {
		GetReq(X_CreateWindow, frame->parent);
		req->param.s[0] = frame->height;
		req->param.s[1] = frame->width;
		req->param.s[2] = frame->x;
		req->param.s[3] = frame->y;
		req->param.l[2] = frame->border;
		req->param.l[3] = frame->background;
		req->func = (frame++)->bdrwidth;
	    }
	    else {
		GetReq(X_CreateTransparency, frame->parent);
		req->param.s[0] = frame->height;
		req->param.s[1] = frame->width;
		req->param.s[2] = frame->x;
		req->param.s[3] = (frame++)->y;
	    }
	}
	
	/*
	 * Reset request number to its old value, so that
	 * error packets are processed correctly.
	 */
	dpy->request -= ndefs;

	/*
	 * Retrieve replies.
	 */
	frame = defs;
	for (i=0;i<ndefs;i++) {
	    XRep rep;
	    /*
	     * Increment request number so error packets
	     * are processed correctly.
	     */
	    dpy->request++;
	    if (!_XReply(dpy, &rep)) (frame++)->self = NULL;
	    else {
		(frame++)->self = rep.param.l[0];
		nresult++;
	    }
	}
	return (nresult);
}

