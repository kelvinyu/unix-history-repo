/*
 * Copyright (c) 1995
 *	Mark Murray.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by Mark Murray
 *	and Eric Young
 * 4. Neither the name of the author nor the names of any co-contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY MARK MURRAY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id$
 */

#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <machine/types.h>

#include "des_locl.h"

/* This counter keeps track of the pseudo-random sequence */
static union {
	u_int32_t ul0, ul1;
	u_char uc[8];
} counter;

/* The current encryption schedule */
static des_key_schedule current_key;

/*
 *   des_set_random_generator_seed: starts a new pseudorandom sequence
 *                                  dependant on the supplied key
 */
void
des_set_random_generator_seed(new_key)
	des_cblock *new_key;
{
	des_key_sched(new_key, current_key);

	/* reset the counter */
	counter.ul0 = counter.ul1 = 0;
}

/*
 *   des_set_sequence_number: set the counter to a known value
 */
void
des_set_sequence_number(new_counter)
	des_cblock new_counter;
{
	bcopy((void *)new_counter, (char *)counter.uc, sizeof(counter));
}

/*
 *   des_generate_random_block: returns the next 64 bit random number
 */

void des_generate_random_block(random_block)
	des_cblock *random_block;
{
	/* Encrypt the counter to get pseudo-random numbers */
    	des_ecb_encrypt(&counter.uc, random_block, current_key, DES_ENCRYPT);

	/* increment the 64-bit counter */
	counter.ul0++;
	if (!counter.ul0) counter.ul1++;
}

/*
 *   des_new_random_key: Invents a new, random strong key with odd parity.
 */
int 
des_new_random_key(new_key)
	des_cblock *new_key;
{
	do {
		des_generate_random_block(new_key);
		des_set_odd_parity(new_key);
	} while (des_is_weak_key(new_key));
	return(0);
}

/*
 *   des_init_random_number_generator: intialises the random number generator
 *                                     to a truly nasty sequence using system
 *                                     supplied volatile variables.
 */
void
des_init_random_number_generator(key)
	des_cblock *key;
{
	/* 64-bit structures */
	struct {
		u_int32_t pid;
		u_int32_t hostid;
	} sysblock;
	struct timeval timeblock;
	struct {
		u_int32_t tv_sec;
		u_int32_t tv_usec;
	} time64bit;
	des_cblock new_key;
	int mib[2];
	size_t len;

	/* Get host ID using official BSD 4.4 method */
	mib[0] = CTL_KERN;
	mib[1] = KERN_HOSTID;
	len = sizeof(sysblock.hostid);
	sysctl(mib, 2, &sysblock.hostid, &len, NULL, 0);

	/* Get Process ID */
	sysblock.pid = getpid();

	/* Generate a new key, and use it to seed the random generator */
	des_set_random_generator_seed(key);
	des_set_sequence_number((unsigned char *)&sysblock);
	des_new_random_key(&new_key);
	des_set_random_generator_seed(&new_key);

	/* Try to confuse the sequence counter */
	gettimeofday(&timeblock, NULL);
	time64bit.tv_sec = (u_int32_t)timeblock.tv_sec;
	time64bit.tv_usec = (u_int32_t)timeblock.tv_usec;
	des_set_sequence_number((unsigned char *)&time64bit);

	/* Do the work */
	des_new_random_key(&new_key);
	des_set_random_generator_seed(&new_key);
}
