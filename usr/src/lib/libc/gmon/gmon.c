static	char *sccsid = "@(#)gmon.c	4.5 (Berkeley) %G%";

#ifdef DEBUG
#include <stdio.h>
#endif DEBUG

#include "gmon.h"

    /*
     *	froms is actually a bunch of unsigned shorts indexing tos
     */
static unsigned short	*froms;
static struct tostruct	*tos = 0;
static unsigned short	tolimit = 0;
static char		*s_lowpc = 0;
static char		*s_highpc = 0;
static unsigned long	s_textsize = 0;
static char		*minsbrk = 0;

static int	ssiz;
static int	*sbuf;

#define	MSG "No space for monitor buffer(s)\n"

monstartup(lowpc, highpc)
    char	*lowpc;
    char	*highpc;
{
    int			monsize;
    char		*buffer;
    char		*sbrk();
    unsigned long	limit;

    s_lowpc = lowpc;
    s_highpc = highpc;
    s_textsize = highpc - lowpc;
    monsize = s_textsize + sizeof(struct phdr);
    buffer = sbrk( monsize );
    if ( buffer == (char *) -1 ) {
	write( 2 , MSG , sizeof(MSG) );
	return;
    }
    froms = (unsigned short *) sbrk( s_textsize );
    if ( froms == (unsigned short *) -1 ) {
	write( 2 , MSG , sizeof(MSG) );
	froms = 0;
	return;
    }
    tos = (struct tostruct *) sbrk(s_textsize);
    if ( tos == (struct tostruct *) -1 ) {
	write( 2 , MSG , sizeof(MSG) );
	froms = 0;
	tos = 0;
	return;
    }
    tos[0].link = 0;
    limit = s_textsize / sizeof(struct tostruct);
	/*
	 *	tolimit is what mcount checks to see if
	 *	all the data structures are ready!!!
	 *	make sure it won't overflow.
	 */
    tolimit = limit > 65534 ? 65534 : limit;
    monitor( lowpc , highpc , buffer , monsize , tolimit );
}

_mcleanup()
{
    int			fd;
    int			fromindex;
    char		*frompc;
    int			toindex;
    struct rawarc	rawarc;

    fd = creat( "gmon.out" , 0666 );
    if ( fd < 0 ) {
	perror( "mcount: gmon.out" );
	return;
    }
#   ifdef DEBUG
	fprintf( stderr , "[mcleanup] sbuf 0x%x ssiz %d\n" , sbuf , ssiz );
#   endif DEBUG
    write( fd , sbuf , ssiz );
    for ( fromindex = 0 ; fromindex < s_textsize>>1 ; fromindex++ ) {
	if ( froms[fromindex] == 0 ) {
	    continue;
	}
	frompc = s_lowpc + (fromindex<<1);
	for (toindex=froms[fromindex]; toindex!=0; toindex=tos[toindex].link) {
#	    ifdef DEBUG
		fprintf( stderr ,
			"[mcleanup] frompc 0x%x selfpc 0x%x count %d\n" ,
			frompc , tos[toindex].selfpc , tos[toindex].count );
#	    endif DEBUG
	    rawarc.raw_frompc = (unsigned long) frompc;
	    rawarc.raw_selfpc = (unsigned long) tos[toindex].selfpc;
	    rawarc.raw_count = tos[toindex].count;
	    write( fd , &rawarc , sizeof rawarc );
	}
    }
    close( fd );
}

    /*
     *	This routine is massaged so that it may be jsb'ed to
     */
asm("#define _mcount mcount");
mcount()
{
    register char		*selfpc;	/* r11 */
    register unsigned short	*frompcindex;	/* r10 */
    register struct tostruct	*top;		/* r9 */
    static int			profiling = 0;

    asm( "	forgot to run ex script on gcrt0.s" );
    asm( "#define r11 r5" );
    asm( "#define r10 r4" );
    asm( "#define r9 r3" );
#ifdef lint
    selfpc = (char *) 0;
    frompcindex = 0;
#else not lint
	/*
	 *	find the return address for mcount,
	 *	and the return address for mcount's caller.
	 */
    asm("	movl (sp), r11");	/* selfpc = ... (jsb frame) */
    asm("	movl 16(fp), r10");	/* frompcindex =     (calls frame) */
#endif not lint
	/*
	 *	check that we are profiling
	 *	and that we aren't recursively invoked.
	 */
    if ( tolimit == 0 ) {
	goto out;
    }
    if ( profiling ) {
	goto out;
    }
    profiling = 1;
	/*
	 *	check that frompcindex is a reasonable pc value.
	 *	for example:	signal catchers get called from the stack,
	 *			not from text space.  too bad.
	 */
    frompcindex = (unsigned short *) ( (long) frompcindex - (long) s_lowpc );
    if ( (unsigned long) frompcindex > s_textsize ) {
	goto done;
    }
    frompcindex = &froms[ ( (long) frompcindex ) >> 1 ];
    if ( *frompcindex == 0 ) {
	*frompcindex = ++tos[0].link;
	if ( *frompcindex >= tolimit ) {
	    goto overflow;
	}
	top = &tos[ *frompcindex ];
	top->selfpc = selfpc;
	top->count = 0;
	top->link = 0;
    } else {
	top = &tos[ *frompcindex ];
    }
    for ( ; /* goto done */ ; top = &tos[ top -> link ] ) {
	if ( top -> selfpc == selfpc ) {
	    top -> count++;
	    goto done;
	}
	if ( top -> link == 0 ) {
	    top -> link = ++tos[0].link;
	    if ( top -> link >= tolimit )
		goto overflow;
	    top = &tos[ top -> link ];
	    top -> selfpc = selfpc;
	    top -> count = 1;
	    top -> link = 0;
	    goto done;
	}
    }
done:
    profiling = 0;
    /* and fall through */
out:
    asm( "	rsb" );
    asm( "#undef r11" );
    asm( "#undef r10" );
    asm( "#undef r9" );
    asm( "#undef _mcount");

overflow:
    tolimit = 0;
#   define	TOLIMIT	"mcount: tos overflow\n"
    write( 2 , TOLIMIT , sizeof( TOLIMIT ) );
    goto out;
}

/*VARARGS1*/
monitor( lowpc , highpc , buf , bufsiz , nfunc )
    char	*lowpc;
    char	*highpc;
    int		*buf, bufsiz;
    int		nfunc;	/* not used, available for compatability only */
{
    register o;

    if ( lowpc == 0 ) {
	profil( (char *) 0 , 0 , 0 , 0 );
	_mcleanup();
	return;
    }
    sbuf = buf;
    ssiz = bufsiz;
    ( (struct phdr *) buf ) -> lpc = lowpc;
    ( (struct phdr *) buf ) -> hpc = highpc;
    ( (struct phdr *) buf ) -> ncnt = ssiz;
    o = sizeof(struct phdr);
    buf = (int *) ( ( (int) buf ) + o );
    bufsiz -= o;
    if ( bufsiz <= 0 )
	return;
    o = ( ( (char *) highpc - (char *) lowpc) );
    if( bufsiz < o )
	o = ( (float) bufsiz / o ) * 65536;
    else
	o = 65536;
    profil( buf , bufsiz , lowpc , o );
}

/*
 * This is a stub for the "brk" system call, which we want to
 * catch so that it will not deallocate our data space.
 * (of which the program is not aware)
 */
asm("#define _curbrk curbrk");
extern char *curbrk;

brk(addr)
	char *addr;
{

	if (addr < minsbrk)
		addr = minsbrk;
	asm("	chmk	$17");
	asm("	jcc	1f");
	asm("	jmp	cerror");
asm("1:");
	curbrk = addr;
	return (0);
}
