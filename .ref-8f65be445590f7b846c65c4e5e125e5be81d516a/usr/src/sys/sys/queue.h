/* 
 * Copyright (c) 1985, Avadis Tevanian, Jr.
 * Copyright (c) 1987 Carnegie-Mellon University
 * Copyright (c) 1991 Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * The Mach Operating System project at Carnegie-Mellon University.
 *
 * The CMU software License Agreement specifies the terms and conditions
 * for use and redistribution.
 *
 *	@(#)queue.h	7.2 (Berkeley) %G%
 */

/*
 *	Type definitions for generic queues.
 */

#ifndef	_QUEUE_H_
#define	_QUEUE_H_

struct queue_entry {
	struct queue_entry	*next;		/* next element */
	struct queue_entry	*prev;		/* previous element */
};

typedef struct queue_entry	*queue_t;
typedef	struct queue_entry	queue_head_t;
typedef	struct queue_entry	queue_chain_t;
typedef	struct queue_entry	*queue_entry_t;

#define round_queue(size)	(((size)+7) & (~7))

#define enqueue(queue,elt)	enqueue_tail(queue, elt)
#define	dequeue(queue)		dequeue_head(queue)

#define	enqueue_head(queue,elt)	insque(elt,queue)
#define	enqueue_tail(queue,elt)	insque(elt,(queue)->prev)
#define	remqueue(queue,elt)	remque(elt)

#define	queue_init(q)		((q)->next = (q)->prev = q)
#define	queue_first(q)		((q)->next)
#define	queue_next(qc)		((qc)->next)
#define	queue_end(q, qe)	((q) == (qe))
#define	queue_empty(q)		queue_end((q), queue_first(q))

#define queue_enter(head, elt, type, field) {			\
	if (queue_empty((head))) {				\
		(head)->next = (queue_entry_t) elt;		\
		(head)->prev = (queue_entry_t) elt;		\
		(elt)->field.next = head;			\
		(elt)->field.prev = head;			\
	} else {						\
		register queue_entry_t prev = (head)->prev;	\
		(elt)->field.prev = prev;			\
		(elt)->field.next = head;			\
		(head)->prev = (queue_entry_t)(elt);		\
		((type)prev)->field.next = (queue_entry_t)(elt);\
	}							\
}

#define	queue_field(head, thing, type, field)			\
		(((head) == (thing)) ? (head) : &((type)(thing))->field)

#define	queue_remove(head, elt, type, field) {			\
	register queue_entry_t next = (elt)->field.next;	\
	register queue_entry_t prev = (elt)->field.prev;	\
	queue_field((head), next, type, field)->prev = prev;	\
	queue_field((head), prev, type, field)->next = next;	\
}

#define	queue_assign(to, from, type, field) {			\
	((type)((from)->prev))->field.next = (to);		\
	((type)((from)->next))->field.prev = (to);		\
	*to = *from;						\
}

#define	queue_remove_first(h, e, t, f) {			\
	e = (t) queue_first((h));				\
	queue_remove((h), (e), t, f);				\
}

#endif	/* !_QUEUE_H_ */
