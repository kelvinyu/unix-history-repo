/*
 * Copyright (c) 1980, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char copyright[] =
"@(#) Copyright (c) 1980, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)snscore.c	8.1 (Berkeley) %G%";
#endif /* not lint */

#include <sys/types.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include "pathnames.h"

char *recfile = _PATH_RAWSCORES;
#define MAXPLAYERS 256

struct	player	{
	short	uids;
	short	scores;
	char	*name;
} players[MAXPLAYERS], temp;

int
main()
{
	short	uid, score;
	FILE	*fd;
	int	noplayers;
	int	i, j, notsorted;
	short	whoallbest, allbest;
	char	*q;
	struct	passwd	*p;

	fd = fopen(recfile, "r");
	if (fd == NULL) {
		perror(recfile);
		exit(1);
	}
	printf("Snake players scores to date\n");
	fread(&whoallbest, sizeof(short), 1, fd);
	fread(&allbest, sizeof(short), 1, fd);
	noplayers = 0;
	for (uid = 2; ;uid++) {
		if(fread(&score, sizeof(short), 1, fd) == 0)
			break;
		if (score > 0) {
			if (noplayers > MAXPLAYERS) {
				printf("too many players\n");
				exit(2);
			}
			players[noplayers].uids = uid;
			players[noplayers].scores = score;
			p = getpwuid(uid);
			if (p == NULL)
				continue;
			q = p -> pw_name;
			players[noplayers].name = malloc(strlen(q) + 1);
			strcpy(players[noplayers].name, q);
			noplayers++;
		}
	}

	/* bubble sort scores */
	for (notsorted = 1; notsorted; ) {
		notsorted = 0;
		for (i = 0; i < noplayers - 1; i++)
			if (players[i].scores < players[i + 1].scores) {
				temp = players[i];
				players[i] = players[i + 1];
				players[i + 1] = temp;
				notsorted++;
			}
	}

	j = 1;
	for (i = 0; i < noplayers; i++) {
		printf("%d:\t$%d\t%s\n", j, players[i].scores, players[i].name);
		if (players[i].scores > players[i + 1].scores)
			j = i + 2;
	}
	exit(0);
}
