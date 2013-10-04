/*-
 * Copyright (c) 2013 Arthur Mesh <arthurmesh@gmail.com>
 * Copyright (c) 2013 Mark R V Murray
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer
 *    in this position and unchanged.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/param.h>
__FBSDID("$FreeBSD$");

#include <sys/kernel.h>
#include <sys/libkern.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/queue.h>
#include <sys/random.h>
#include <sys/selinfo.h>
#include <sys/sx.h>
#include <sys/sysctl.h>
#include <sys/systm.h>
#include <sys/unistd.h>

#include <dev/random/randomdev.h>
#include <dev/random/randomdev_soft.h>
#include <dev/random/random_adaptors.h>

#include "live_entropy_sources.h"

LIST_HEAD(les_head, live_entropy_sources);
static struct les_head sources = LIST_HEAD_INITIALIZER(sources);
static struct sx les_lock; /* need a sleepable lock */

#define LES_THRESHOLD 10

MALLOC_DEFINE(M_LIVE_ENTROPY_SRCS, "live_entropy_sources",
    "Live Entropy Sources");

void
live_entropy_source_register(struct random_hardware_source *rsource)
{
	struct live_entropy_sources *les;

	KASSERT(rsource != NULL, ("invalid input to %s", __func__));

	les = malloc(sizeof(struct live_entropy_sources), M_LIVE_ENTROPY_SRCS,
	    M_WAITOK);
	les->rsource = rsource;

	sx_xlock(&les_lock);
	LIST_INSERT_HEAD(&sources, les, entries);
	sx_xunlock(&les_lock);
}

void
live_entropy_source_deregister(struct random_hardware_source *rsource)
{
	struct live_entropy_sources *les;

	KASSERT(rsource != NULL, ("invalid input to %s", __func__));

	sx_xlock(&les_lock);
	LIST_FOREACH(les, &sources, entries) {
		if (les->rsource == rsource) {
			LIST_REMOVE(les, entries);
			free(les, M_LIVE_ENTROPY_SRCS);
			break;
		}
	}
	sx_xunlock(&les_lock);
}

static int
live_entropy_source_handler(SYSCTL_HANDLER_ARGS)
{
	struct live_entropy_sources *les;
	int error, count;

	count = error = 0;

	sx_slock(&les_lock);

	if (LIST_EMPTY(&sources))
		error = SYSCTL_OUT(req, "", 0);
	else {
		LIST_FOREACH(les, &sources, entries) {

			error = SYSCTL_OUT(req, ",", count++ ? 1 : 0);
			if (error)
				break;

			error = SYSCTL_OUT(req, les->rsource->ident, strlen(les->rsource->ident));
			if (error)
				break;
		}
	}

	sx_sunlock(&les_lock);

	return (error);
}

static void
live_entropy_sources_init(void *unused)
{

	SYSCTL_PROC(_kern_random, OID_AUTO, live_entropy_sources,
	    CTLTYPE_STRING | CTLFLAG_RD | CTLFLAG_MPSAFE,
	    NULL, 0, live_entropy_source_handler, "",
	    "List of Active Live Entropy Sources");

	sx_init(&les_lock, "live_entropy_sources");
}

/*
 * Run through all "live" sources reading entropy for the given
 * number of rounds, which should be a multiple of the number
 * of entropy accumulation pools in use; 2 for Yarrow and 32
 * for Fortuna.
 */
void
live_entropy_sources_feed(int rounds)
{
	struct live_entropy_sources *les;
	uint8_t buf[HARVESTSIZE];
	int i, n;

	sx_slock(&les_lock);

	/*
	 * Walk over all of live entropy sources, and feed their output
	 * to the system-wide RNG.
	 */
	LIST_FOREACH(les, &sources, entries) {

		for (i = 0; i < rounds; i++) {
			/*
			 * This should be quick, since it's a live entropy
			 * source.
			 */
			n = les->rsource->read(buf, sizeof(buf));
			/* FIXME: Whine loudly if this didn't work. */

			/*
			 * FIXME: Cannot harvest this stuff into the queue;
			 * the poor thing will choke to death!
			 */
			random_harvest(buf, n, 0, les->rsource->source);
		}

	}

	sx_sunlock(&les_lock);
}

static void
live_entropy_sources_deinit(void *unused)
{

	sx_destroy(&les_lock);
}

SYSINIT(random_adaptors, SI_SUB_DRIVERS, SI_ORDER_FIRST,
    live_entropy_sources_init, NULL);
SYSUNINIT(random_adaptors, SI_SUB_DRIVERS, SI_ORDER_FIRST,
    live_entropy_sources_deinit, NULL);
