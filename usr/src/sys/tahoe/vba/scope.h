/*	scope.h	1.2	86/01/05	*/

/*  Some I/O addresses used to generate pulses for scopes */
#define	OUT1	0xffffb034
#define	OUT2	0xffffb018
#define	OUT3	0xffffb020
#define	OUT4	0xffffb004
#define	OUT5	0xffffb024
#define	OUT6	0xffffb00c
#define	OUT7	0xffffb02c

#define	IOaddr(off)	(caddr_t)(&vmem[(off) & 0x0fffff])

extern char vmem[];
int	iospace_mapped;
#define	scope_out(x)	if(iospace_mapped) movob(IOaddr(OUT/**/x),0)
#define	scope_in(x)	if(iospace_mapped) dummy =  *IOaddr(IN/**/x)
