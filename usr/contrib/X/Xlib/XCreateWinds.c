#include <X/mit-copyright.h>

/* $Header: XCreateWinds.c,v 10.4 86/02/01 15:31:46 tony Rel $ */
/* Copyright    Massachusetts Institute of Technology    1985	*/

#include "XlibInternal.h"
int XCreateWindows (parent, defs, ndefs)
	Window parent;
	OpaqueFrame defs[];
	int ndefs;
{
	register Display *dpy;
	register int i;
	register OpaqueFrame *frame = defs;
	int nresult = 0;

	for (i=0;i<ndefs;i++) {
	    register XReq *req;
	    GetReq(X_CreateWindow, parent);
	    req->param.s[0] = frame->height;
	    req->param.s[1] = frame->width;
	    req->param.s[2] = frame->x;
	    req->param.s[3] = frame->y;
	    req->param.l[2] = frame->border;
	    req->param.l[3] = frame->background;
	    req->func = (frame++)->bdrwidth;
	    }
	
	/* Reset request number to its old value, so that
	    error packets are processed correctly.  */
	dpy->request -= ndefs;

	frame = defs;
	for (i=0;i<ndefs;i++) {
	    XRep rep;
	    /* Increment request number so error packets
	       are processed correctly. */
	    dpy->request++;
	    if (!_XReply(dpy, &rep))
	    	(frame++)->self = NULL;
	    else {
		(frame++)->self = rep.param.l[0];
		nresult++;
		}
	    }
	return (nresult);
}

