# include "conf.h"
# include <time.h>
# ifndef V6
# include <sys/types.h>
# include <sys/timeb.h>
# endif

static char SccsId[] = "@(#)arpadate.c	3.6.1.1	%G%";

/*
**  ARPADATE -- Create date in ARPANET format
**
**	Parameters:
**		ud -- unix style date string.  if NULL, one is created.
**
**	Returns:
**		pointer to an ARPANET date field
**
**	Side Effects:
**		none
**
**	WARNING:
**		date is stored in a local buffer -- subsequent
**		calls will overwrite.
**
**	Bugs:
**		Timezone is computed from local time, rather than
**		from whereever (and whenever) the message was sent.
**		To do better is very hard.
**
**		Some sites are now inserting the timezone into the
**		local date.  This routine should figure out what
**		the format is and work appropriately.
*/

# define NULL		0

struct cvttab
{
	char	*old;
	char	*new;
};

struct cvttab	DowTab[] =
{
	"Sun",		"Sunday",
	"Mon",		"Monday",
	"Tue",		"Tuesday",
	"Wed",		"Wednesday",
	"Thu",		"Thursday",
	"Fri",		"Friday",
	"Sat",		"Saturday",
	NULL,		NULL
};

struct cvttab	MonthTab[] =
{
	"Jan",		"January",
	"Feb",		"February",
	"Mar",		"March",
	"Apr",		"April",
	"May",		"May",
	"Jun",		"June",
	"Jul",		"July",
	"Aug",		"August",
	"Sep",		"September",
	"Oct",		"October",
	"Nov",		"November",
	"Dec",		"December",
	NULL,		NULL
};

char *
arpadate(ud)
	register char *ud;
{
	register char *p;
	register char *q;
	static char b[40];
	extern char *ctime();
	register int i;
	struct cvttab *c;
	extern struct tm *localtime();
# ifdef V6
	long t;
	extern char *StdTimezone, *DstTimezone;
	extern long time();
# else
	struct timeb t;
	extern struct timeb *ftime();
	extern char *timezone();
# endif

	/*
	**  Get current time.
	**	This will be used if a null argument is passed and
	**	to resolve the timezone.
	*/

# ifdef V6
	(void) time(&t);
	if (ud == NULL)
		ud = ctime(&t);
# else
	ftime(&t);
	if (ud == NULL)
		ud = ctime(&t.time);
# endif

	/*
	**  Crack the UNIX date line in a singularly unoriginal way.
	*/

	q = b;

	p = &ud[8];		/* 16 */
	if (*p == ' ')
		p++;
	else
		*q++ = *p++;
	*q++ = *p++;
	*q++ = ' ';

	p = NULL;		/* Sep */
	for (c = MonthTab; c->old != NULL; c++)
	{
		if (strncmp(&ud[4], c->old, 3) == 0)
		{
			p = c->new;
			break;
		}
	}
	if (p != NULL)
	{
		while (*p != '\0')
			*q++ = *p++;
	}
	else
	{
		p = &ud[4];
		*q++ = *p++;
		*q++ = *p++;
		*q++ = *p++;
	}
	*q++ = ' ';

	p = &ud[20];		/* 1979 */
	for (i = 4; i > 0; i--)
		*q++ = *p++;
	*q++ = ' ';

	p = &ud[11];		/* 01:03:52 */
	for (i = 5; i > 0; i--)
		*q++ = *p++;

				/* -PST or -PDT */
# ifdef V6
	if (localtime(&t)->tm_isdst)
		p = DstTimezone;
	else
		p = StdTimezone;
# else
	p = timezone(t.timezone, localtime(&t.time)->tm_isdst);
# endif V6
	if (p[3] != '\0')
	{
		/* hours from GMT */
		p += 3;
		*q++ = *p++;
		if (p[1] == ':')
			*q++ = '0';
		else
			*q++ = *p++;
		*q++ = *p++;
		p++;		/* skip ``:'' */
		*q++ = *p++;
		*q++ = *p++;
	}
	else
	{
		*q++ = '-';
		*q++ = *p++;
		*q++ = *p++;
		*q++ = *p++;
	}

	p = NULL;		/* Mon */
	for (c = DowTab; c->old != NULL; c++)
	{
		if (strncmp(&ud[0], c->old, 3) == 0)
		{
			p = c->new;
			break;
		}
	}
	if (p != NULL)
	{
		*q++ = ' ';
		*q++ = '(';
		while (*p != '\0')
			*q++ = *p++;
		*q++ = ')';
	}

	*q = '\0';

	return (b);
}
