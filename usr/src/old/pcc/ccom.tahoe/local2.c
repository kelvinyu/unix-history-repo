#ifndef lint
static char sccsid[] = "@(#)local2.c	1.4 (Berkeley) %G%";
#endif

# include "pass2.h"
# include <ctype.h>
# ifdef FORT
int ftlab1, ftlab2;
# endif
/* a lot of the machine dependent parts of the second pass */

# define BITMASK(n) ((1L<<n)-1)

# ifndef ONEPASS
where(c){
	fprintf( stderr, "%s, line %d: ", filename, lineno );
	}
# endif

lineid( l, fn ) char *fn; {
	/* identify line l and file fn */
	printf( "#	line %d, file %s\n", l, fn );
	}

int ent_mask;

eobl2(){
	register OFFSZ spoff;	/* offset from stack pointer */
#ifndef FORT
	extern int ftlab1, ftlab2;
#endif

	spoff = maxoff;
	spoff /= SZCHAR;
	SETOFF(spoff,4);
#ifdef FORT
#ifndef FLEXNAMES
	printf( "	.set	.F%d,%d\n", ftnno, spoff );
#else
	/* SHOULD BE L%d ... ftnno but must change pc/f77 */
	printf( "	.set	LF%d,%d\n", ftnno, spoff );
#endif
	printf( "	.set	LWM%d,0x%x\n", ftnno, ent_mask&0x1ffc|0x1000);
#else
	printf( "	.set	L%d,0x%x\n", ftnno, ent_mask&0x1ffc);
	printf( "L%d:\n", ftlab1);
	if( maxoff > AUTOINIT )
		printf( "	subl3	$%d,fp,sp\n", spoff);
	printf( "	jbr 	L%d\n", ftlab2);
#endif
	ent_mask = 0;
	maxargs = -1;
	}

struct hoptab { int opmask; char * opstring; } ioptab[] = {

	PLUS,	"add",
	MINUS,	"sub",
	MUL,	"mul",
	DIV,	"div",
	MOD,	"div",
	OR,	"or",
	ER,	"xor",
	AND,	"and",
	-1,	""    };

hopcode( f, o ){
	/* output the appropriate string from the above table */

	register struct hoptab *q;

	if(asgop(o))
		o = NOASG o;
	for( q = ioptab;  q->opmask>=0; ++q ){
		if( q->opmask == o ){
			if(f == 'E')
				printf( "e%s", q->opstring);
			else
				printf( "%s%c", q->opstring, tolower(f));
			return;
			}
		}
	cerror( "no hoptab for %s", opst[o] );
	}

char *
rnames[] = {  /* keyed to register number tokens */

	"r0", "r1",
	"r2", "r3", "r4", "r5",
	"r6", "r7", "r8", "r9", "r10", "r11",
	"r12", "fp", "sp", "pc",
	};

/* output register name and update entry mask */
char *
rname(r)
	register int r;
{

	ent_mask |= 1<<r;
	return(rnames[r]);
}

int rstatus[] = {
	SAREG|STAREG, SAREG|STAREG,
	SAREG|STAREG, SAREG|STAREG, SAREG|STAREG, SAREG|STAREG,
	SAREG, SAREG, SAREG, SAREG, SAREG, SAREG,
	SAREG, SAREG, SAREG, SAREG,
	};

tlen(p) NODE *p;
{
	switch(p->in.type) {
		case CHAR:
		case UCHAR:
			return(1);

		case SHORT:
		case USHORT:
			return(2);

		case DOUBLE:
			return(8);

		default:
			return(4);
		}
}

prtype(n) NODE *n;
{
	switch (n->in.type)
		{

		case DOUBLE:
			printf("d");
			return;

		case FLOAT:
			printf("f");
			return;

		case INT:
		case UNSIGNED:
			printf("l");
			return;

		case SHORT:
		case USHORT:
			printf("w");
			return;

		case CHAR:
		case UCHAR:
			printf("b");
			return;

		default:
			if ( !ISPTR( n->in.type ) ) cerror("zzzcode- bad type");
			else {
				printf("l");
				return;
				}
		}
}

zzzcode( p, c ) register NODE *p; {
	register int m;
	int val;
	switch( c ){

	case 'N':  /* logical ops, turned into 0-1 */
		/* use register given by register 1 */
		cbgen( 0, m=getlab(), 'I' );
		deflab( p->bn.label );
		printf( "	clrl	%s\n", rname(getlr( p, '1' )->tn.rval) );
		deflab( m );
		return;

	case 'P':
		cbgen( p->in.op, p->bn.label, c );
		return;

	case 'A':	/* assignment and load (integer only) */
		{
		register NODE *l, *r;

		if (xdebug) eprint(p, 0, &val, &val);
		r = getlr(p, 'R');
		if (optype(p->in.op) == LTYPE || p->in.op == UNARY MUL) {
			l = resc;
			l->in.type = INT;
		} else
			l = getlr(p, 'L');
		if(r->in.type==FLOAT || r->in.type==DOUBLE
		 || l->in.type==FLOAT || l->in.type==DOUBLE)
			cerror("float in ZA");
		if (r->in.op == ICON)
			if(r->in.name[0] == '\0') {
				if (r->tn.lval == 0) {
					printf("clr");
					prtype(l);
					printf("	");
					adrput(l);
					return;
				}
				if (r->tn.lval < 0 && r->tn.lval >= -63) {
					printf("mneg");
					prtype(l);
					r->tn.lval = -r->tn.lval;
					goto ops;
				}
#ifdef MOVAFASTER
			} else {
				printf("movab");
				printf("	");
				acon(r);
				printf(",");
				adrput(l);
				return;
#endif MOVAFASTER
			}

		if (l->in.op == REG) {
			if( tlen(l) < tlen(r) ) {
				!ISUNSIGNED(l->in.type)?
					printf("cvt"):
					printf("movz");
				prtype(l);
				printf("l");
				goto ops;
			} else
				l->in.type = INT;
		}
		if (tlen(l) == tlen(r)) {
			printf("mov");
			prtype(l);
			goto ops;
		} else if (tlen(l) > tlen(r) && ISUNSIGNED(r->in.type))
			printf("movz");
		else
			printf("cvt");
		prtype(r);
		prtype(l);
	ops:
		printf("	");
		adrput(r);
		printf(",");
		adrput(l);
		return;
		}

	case 'B':	/* get oreg value in temp register for shift */
		{
		register NODE *r;
		if (xdebug) eprint(p, 0, &val, &val);
		r = p->in.right;
		if( tlen(r) == sizeof(int) && r->in.type != FLOAT )
			printf("movl");
		else {
			printf(ISUNSIGNED(r->in.type) ? "movz" : "cvt");
			prtype(r);
			printf("l");
			}
		return;
		}

	case 'C':	/* num bytes pushed on arg stack */
		{
		extern int gc_numbytes;
		extern int xdebug;

		if (xdebug) printf("->%d<-",gc_numbytes);

		printf("call%c	$%d",
		 (p->in.left->in.op==ICON && gc_numbytes<60)?'f':'s',
		 gc_numbytes+4);
		/* dont change to double (here's the only place to catch it) */
		if(p->in.type == FLOAT)
			rtyflg = 1;
		return;
		}

	case 'D':	/* INCR and DECR */
		zzzcode(p->in.left, 'A');
		printf("\n	");

	case 'E':	/* INCR and DECR, FOREFF */
 		if (p->in.right->tn.lval == 1)
			{
			printf("%s", (p->in.op == INCR ? "inc" : "dec") );
			prtype(p->in.left);
			printf("	");
			adrput(p->in.left);
			return;
			}
		printf("%s", (p->in.op == INCR ? "add" : "sub") );
		prtype(p->in.left);
		printf("2	");
		adrput(p->in.right);
		printf(",");
		adrput(p->in.left);
		return;

	case 'F':	/* masked constant for fields */
		printf(ACONFMT, (p->in.right->tn.lval&((1<<fldsz)-1))<<fldshf);
		return;

	case 'H':	/* opcode for shift */
		if(p->in.op == LS || p->in.op == ASG LS)
			printf("shll");
		else if(ISUNSIGNED(p->in.left->in.type))
			printf("shrl");
		else
			printf("shar");
		return;

	case 'L':	/* type of left operand */
	case 'R':	/* type of right operand */
		{
		register NODE *n;
		extern int xdebug;

		n = getlr ( p, c);
		if (xdebug) printf("->%d<-", n->in.type);

		prtype(n);
		return;
		}

	case 'M':  /* initiate ediv for mod and unsigned div */
		{
		register char *r;
		m = getlr(p, '1')->tn.rval;
		r = rname(m);
		printf("\tclrl\t%s\n\tmovl\t", r);
		adrput(p->in.left);
		printf(",%s\n", rname(m+1));
		if(!ISUNSIGNED(p->in.type)) { 	/* should be MOD */
			m = getlab();
			printf("\tjgeq\tL%d\n\tmnegl\t$1,%s\n", m, r);
			deflab(m);
		}
		}
		return;

	case 'U':
		/* Truncate int for type conversions:
		    LONG|ULONG -> CHAR|UCHAR|SHORT|USHORT
		    SHORT|USHORT -> CHAR|UCHAR
		   increment offset to correct byte */
		{
		register NODE *p1;
		int dif;

		p1 = p->in.left;
		switch( p1->in.op ){
		case NAME:
		case OREG:
			dif = tlen(p1)-tlen(p);
			p1->tn.lval += dif;
			adrput(p1);
			p1->tn.lval -= dif;
			return;
		default:
			cerror( "Illegal ZU type conversion" );
			return;
			}
		}

	case 'T':	/* rounded structure length for arguments */
		{
		int size;

		size = p->stn.stsize;
		SETOFF( size, 4);
		printf("movab	-%d(sp),sp", size);
		return;
		}

	case 'S':  /* structure assignment */
		stasg(p);
		break;

	default:
		cerror( "illegal zzzcode" );
		}
	}

#define	MOVB(dst, src, off) { \
	printf("\tmovb\t"); upput(src, off); putchar(','); \
	upput(dst, off); putchar('\n'); \
}
#define	MOVW(dst, src, off) { \
	printf("\tmovw\t"); upput(src, off); putchar(','); \
	upput(dst, off); putchar('\n'); \
}
#define	MOVL(dst, src, off) { \
	printf("\tmovl\t"); upput(src, off); putchar(','); \
	upput(dst, off); putchar('\n'); \
}
/*
 * Generate code for a structure assignment.
 */
stasg(p)
	register NODE *p;
{
	register NODE *l, *r;
	register int size;

	switch (p->in.op) {
	case STASG:			/* regular assignment */
		l = p->in.left;
		r = p->in.right;
		break;
	case STARG:			/* place arg on the stack */
		l = getlr(p, '3');
		r = p->in.left;
		break;
	default:
		cerror("STASG bad");
		/*NOTREACHED*/
	}
	/*
	 * Pun source for use in code generation.
	 */
	switch (r->in.op) {
	case ICON:
		r->in.op = NAME;
		break;
	case REG:
		r->in.op = OREG;
		break;
	default:
		cerror( "STASG-r" );
		/*NOTREACHED*/
	}
	size = p->stn.stsize;
	if (size <= 0 || size > 65535)
		cerror("structure size out of range");
	/*
	 * Generate optimized code based on structure size
	 * and alignment properties....
	 */
	switch (size) {

	case 1:
		printf("\tmovb\t");
	optimized:
		adrput(r);
		printf(",");
		adrput(l);
		printf("\n");
werror("optimized structure assignment (size %d alignment %d)", size, p->stn.stalign);
		break;

	case 2:
		if (p->stn.stalign != 2) {
			MOVB(l, r, SZCHAR);
			printf("\tmovb\t");
		} else
			printf("\tmovw\t");
		goto optimized;

	case 4:
		if (p->stn.stalign != 4) {
			if (p->stn.stalign != 2) {
				MOVB(l, r, 3*SZCHAR);
				MOVB(l, r, 2*SZCHAR);
				MOVB(l, r, 1*SZCHAR);
				printf("\tmovb\t");
			} else {
				MOVW(l, r, SZSHORT);
				printf("\tmovw\t");
			}
		} else
			printf("\tmovl\t");
		goto optimized;

	case 6:
		if (p->stn.stalign != 2)
			goto movblk;
		MOVW(l, r, 2*SZSHORT);
		MOVW(l, r, 1*SZSHORT);
		printf("\tmovw\t");
		goto optimized;

	case 8:
		if (p->stn.stalign == 4) {
			MOVL(l, r, SZLONG);
			printf("\tmovl\t");
			goto optimized;
		}
		/* fall thru...*/

	default:
	movblk:
		/*
		 * Can we ever get a register conflict with R1 here?
		 */
		printf("\tmovab\t");
		adrput(l);
		printf(",r1\n\tmovab\t");
		adrput(r);
		printf(",r0\n\tmovl\t$%d,r2\n\tmovblk\n", size);
		rname(R2);
		break;
	}
	/*
	 * Reverse above pun for reclaim.
	 */
	if (r->in.op == NAME)
		r->in.op = ICON;
	else if (r->in.op == OREG)
		r->in.op = REG;
}

/*
 * Output the address of the second item in the
 * pair pointed to by p.
 */
upput(p, size)
	register NODE *p;
{
	CONSZ save;

	if (p->in.op == FLD)
		p = p->in.left;
	switch (p->in.op) {

	case NAME:
	case OREG:
		save = p->tn.lval;
		p->tn.lval += size/SZCHAR;
		adrput(p);
		p->tn.lval = save;
		break;

	case REG:
		if (size == SZLONG) {
			printf("%s", rname(p->tn.rval+1));
			break;
		}
		/* fall thru... */

	default:
		cerror("illegal upper address op %s size %d",
		    opst[p->tn.op], size);
		/*NOTREACHED*/
	}
}

rmove( rt, rs, t ) TWORD t;{
	printf( "	movl	%s,%s\n", rname(rs), rname(rt) );
	if(t==DOUBLE)
		printf( "	movl	%s,%s\n", rname(rs+1), rname(rt+1) );
	}

struct respref
respref[] = {
	INTAREG|INTBREG,	INTAREG|INTBREG,
	INAREG|INBREG,	INAREG|INBREG|SOREG|STARREG|STARNM|SNAME|SCON,
	INTEMP,	INTEMP,
	FORARG,	FORARG,
	INTEMP,	INTAREG|INAREG|INTBREG|INBREG|SOREG|STARREG|STARNM,
	0,	0 };

setregs(){ /* set up temporary registers */
	fregs = 6;	/* tbl- 6 free regs on Tahoe (0-5) */
	}

#ifndef szty
szty(t) TWORD t;{ /* size, in registers, needed to hold thing of type t */
	return(t==DOUBLE ? 2 : 1 );
	}
#endif

rewfld( p ) NODE *p; {
	return(1);
	}

callreg(p) NODE *p; {
	return( R0 );
	}

base( p ) register NODE *p; {
	register int o = p->in.op;

	if( (o==ICON && p->in.name[0] != '\0')) return( 100 ); /* ie no base reg */
	if( o==REG ) return( p->tn.rval );
    if( (o==PLUS || o==MINUS) && p->in.left->in.op == REG && p->in.right->in.op==ICON)
		return( p->in.left->tn.rval );
    if( o==OREG && !R2TEST(p->tn.rval) && (p->in.type==INT || p->in.type==UNSIGNED || ISPTR(p->in.type)) )
		return( p->tn.rval + 0200*1 );
	return( -1 );
	}

offset( p, tyl ) register NODE *p; int tyl; {

	if(tyl > 8) return( -1 );
	if( tyl==1 && p->in.op==REG && (p->in.type==INT || p->in.type==UNSIGNED) ) return( p->tn.rval );
	if( (p->in.op==LS && p->in.left->in.op==REG && (p->in.left->in.type==INT || p->in.left->in.type==UNSIGNED) &&
	      (p->in.right->in.op==ICON && p->in.right->in.name[0]=='\0')
	      && (1<<p->in.right->tn.lval)==tyl))
		return( p->in.left->tn.rval );
	return( -1 );
	}

makeor2( p, q, b, o) register NODE *p, *q; register int b, o; {
	register NODE *t;
	register int i;
	NODE *f;

	p->in.op = OREG;
	f = p->in.left; 	/* have to free this subtree later */

	/* init base */
	switch (q->in.op) {
		case ICON:
		case REG:
		case OREG:
			t = q;
			break;

		case MINUS:
			q->in.right->tn.lval = -q->in.right->tn.lval;
		case PLUS:
			t = q->in.right;
			break;

		case UNARY MUL:
			t = q->in.left->in.left;
			break;

		default:
			cerror("illegal makeor2");
	}

	p->tn.lval = t->tn.lval;
#ifndef FLEXNAMES
	for(i=0; i<NCHNAM; ++i)
		p->in.name[i] = t->in.name[i];
#else
	p->in.name = t->in.name;
#endif

	/* init offset */
	p->tn.rval = R2PACK( (b & 0177), o, (b>>7) );

	tfree(f);
	return;
	}

canaddr( p ) NODE *p; {
	register int o = p->in.op;

	if( o==NAME || o==REG || o==ICON || o==OREG || (o==UNARY MUL && shumul(p->in.left)) ) return(1);
	return(0);
	}

#ifndef shltype
shltype( o, p ) register NODE *p; {
	return( o== REG || o == NAME || o == ICON || o == OREG || ( o==UNARY MUL && shumul(p->in.left)) );
	}
#endif

flshape( p ) NODE *p; {
	register int o = p->in.op;

	if( o==NAME || o==REG || o==ICON || o==OREG || (o==UNARY MUL && shumul(p->in.left)) ) return(1);
	return(0);
	}

shtemp( p ) register NODE *p; {
	if( p->in.op == STARG ) p = p->in.left;
	return( p->in.op==NAME || p->in.op ==ICON || p->in.op == OREG || (p->in.op==UNARY MUL && shumul(p->in.left)) );
	}

shumul( p ) register NODE *p; {
	register int o;
	extern int xdebug;

	if (xdebug) {
		 printf("\nshumul:op=%d,lop=%d,rop=%d", p->in.op, p->in.left->in.op, p->in.right->in.op);
		printf(" prname=%s,plty=%d, prlval=%D\n", p->in.right->in.name, p->in.left->in.type, p->in.right->tn.lval);
		}

	o = p->in.op;
	if(( o == NAME || (o == OREG && !R2TEST(p->tn.rval)) || o == ICON )
	 && p->in.type != PTR+DOUBLE)
		return( STARNM );

	return( 0 );
	}

special( p, shape ) register NODE *p; {
	if( shape==SIREG && p->in.op == OREG && R2TEST(p->tn.rval) ) return(1);
	else return(0);
}

adrcon( val ) CONSZ val; {
	printf(ACONFMT, val);
	}

conput( p ) register NODE *p; {
	switch( p->in.op ){

	case ICON:
		acon( p );
		return;

	case REG:
		printf( "%s", rname(p->tn.rval) );
		return;

	default:
		cerror( "illegal conput" );
		}
	}

insput( p ) NODE *p; {
	cerror( "insput" );
	}

adrput( p ) register NODE *p; {
	register int r;
	/* output an address, with offsets, from p */

	if( p->in.op == FLD ){
		p = p->in.left;
		}
	switch( p->in.op ){

	case NAME:
		acon( p );
		return;

	case ICON:
		/* addressable value of the constant */
		printf( "$" );
		acon( p );
		return;

	case REG:
		printf( "%s", rname(p->tn.rval) );
		if(p->in.type == DOUBLE)	/* for entry mask */
			(void) rname(p->tn.rval+1);
		return;

	case OREG:
		r = p->tn.rval;
		if( R2TEST(r) ){ /* double indexing */
			register int flags;

			flags = R2UPK3(r);
			if( flags & 1 ) printf("*");
			if( p->tn.lval != 0 || p->in.name[0] != '\0' ) acon(p);
			if( R2UPK1(r) != 100) printf( "(%s)", rname(R2UPK1(r)) );
			printf( "[%s]", rname(R2UPK2(r)) );
			return;
			}
		if( r == FP && p->tn.lval > 0 ){  /* in the argument region */
			if( p->in.name[0] != '\0' ) werror( "bad arg temp" );
			printf( CONFMT, p->tn.lval );
			printf( "(fp)" );
			return;
			}
		if( p->tn.lval != 0 || p->in.name[0] != '\0') acon( p );
		printf( "(%s)", rname(p->tn.rval) );
		return;

	case UNARY MUL:
		/* STARNM or STARREG found */
		if( tshape(p, STARNM) ) {
			printf( "*" );
			adrput( p->in.left);
			}
		return;

	default:
		cerror( "illegal address" );
		return;

		}

	}

acon( p ) register NODE *p; { /* print out a constant */

	if( p->in.name[0] == '\0' ){
		printf( CONFMT, p->tn.lval);
		}
	else if( p->tn.lval == 0 ) {
#ifndef FLEXNAMES
		printf( "%.8s", p->in.name );
#else
		printf( "%s", p->in.name );
#endif
		}
	else {
#ifndef FLEXNAMES
		printf( "%.8s+", p->in.name );
#else
		printf( "%s+", p->in.name );
#endif
		printf( CONFMT, p->tn.lval );
		}
	}

genscall( p, cookie ) register NODE *p; {
	/* structure valued call */
	return( gencall( p, cookie ) );
	}

genfcall( p, cookie ) register NODE *p; {
	register NODE *p1;
	register int m;
	static char *funcops[6] = {
		"sin", "cos", "sqrt", "exp", "log", "atan"
	};

	/* generate function opcodes */
	if(p->in.op==UNARY FORTCALL && p->in.type==FLOAT &&
	 (p1 = p->in.left)->in.op==ICON &&
	 p1->tn.lval==0 && p1->in.type==INCREF(FTN|FLOAT)) {
#ifdef FLEXNAMES
		p1->in.name++;
#else
		strcpy(p1->in.name, p1->in.name[1]);
#endif
		for(m=0; m<6; m++)
			if(!strcmp(p1->in.name, funcops[m]))
				break;
		if(m >= 6)
			uerror("no opcode for fortarn function %s", p1->in.name);
	} else
		uerror("illegal type of fortarn function");
	p1 = p->in.right;
	p->in.op = FORTCALL;
	if(!canaddr(p1))
		order( p1, INAREG|INBREG|SOREG|STARREG|STARNM );
	m = match( p, INTAREG|INTBREG );
	return(m != MDONE);
}

/* tbl */
int gc_numbytes;
/* tbl */

gencall( p, cookie ) register NODE *p; {
	/* generate the call given by p */
	register NODE *p1, *ptemp;
	register int temp, temp1;
	register int m;

	if( p->in.right ) temp = argsize( p->in.right );
	else temp = 0;

	if( p->in.op == STCALL || p->in.op == UNARY STCALL ){
		/* set aside room for structure return */

		if( p->stn.stsize > temp ) temp1 = p->stn.stsize;
		else temp1 = temp;
		}

	if( temp > maxargs ) maxargs = temp;
	SETOFF(temp1,4);

	if( p->in.right ){ /* make temp node, put offset in, and generate args */
		ptemp = talloc();
		ptemp->in.op = OREG;
		ptemp->tn.lval = -1;
		ptemp->tn.rval = SP;
#ifndef FLEXNAMES
		ptemp->in.name[0] = '\0';
#else
		ptemp->in.name = "";
#endif
		ptemp->in.rall = NOPREF;
		ptemp->in.su = 0;
		genargs( p->in.right, ptemp );
		ptemp->in.op = FREE;
		}

	p1 = p->in.left;
	if( p1->in.op != ICON ){
		if( p1->in.op != REG ){
			if( p1->in.op != OREG || R2TEST(p1->tn.rval) ){
				if( p1->in.op != NAME ){
					order( p1, INAREG );
					}
				}
			}
		}

/* tbl
	setup gc_numbytes so reference to ZC works */

	gc_numbytes = temp&(0x3ff);

	p->in.op = UNARY CALL;
	m = match( p, INTAREG|INTBREG );

	return(m != MDONE);
	}

/* tbl */
char *
ccbranches[] = {
	"eql",
	"neq",
	"leq",
	"lss",
	"geq",
	"gtr",
	"lequ",
	"lssu",
	"gequ",
	"gtru",
	};
/* tbl */

cbgen( o, lab, mode ) { /*   printf conditional and unconditional branches */

		if(o != 0 && (o < EQ || o > UGT ))
			cerror( "bad conditional branch: %s", opst[o] );
		printf( "	j%s	L%d\n",
		 o == 0 ? "br" : ccbranches[o-EQ], lab );
	}

nextcook( p, cookie ) NODE *p; {
	/* we have failed to match p with cookie; try another */
	if( cookie == FORREW ) return( 0 );  /* hopeless! */
	if( !(cookie&(INTAREG|INTBREG)) ) return( INTAREG|INTBREG );
	if( !(cookie&INTEMP) && asgop(p->in.op) ) return( INTEMP|INAREG|INTAREG|INTBREG|INBREG );
	return( FORREW );
	}

lastchance( p, cook ) NODE *p; {
	/* forget it! */
	return(0);
	}

optim2( p ) register NODE *p; {
# ifdef ONEPASS
	/* do local tree transformations and optimizations */
# define RV(p) p->in.right->tn.lval
# define nncon(p)	((p)->in.op == ICON && (p)->in.name[0] == 0)
	register int o = p->in.op;
	register int i;

	/* change unsigned mods and divs to logicals (mul is done in mip & c2) */
	if(optype(o) == BITYPE && ISUNSIGNED(p->in.left->in.type)
	 && nncon(p->in.right) && (i=ispow2(RV(p)))>=0){
		switch(o) {
		case DIV:
		case ASG DIV:
			p->in.op = RS;
			RV(p) = i;
			break;
		case MOD:
		case ASG MOD:
			p->in.op = AND;
			RV(p)--;
			break;
		default:
			return;
		}
		if(asgop(o))
			p->in.op = ASG p->in.op;
	}
# endif
}

struct functbl {
	int fop;
	char *func;
} opfunc[] = {
	DIV,		"udiv", 
	ASG DIV,	"udiv", 
	0
};

hardops(p)  register NODE *p; {
	/* change hard to do operators into function calls.  */
	register NODE *q;
	register struct functbl *f;
	register int o;
	register TWORD t, t1, t2;

	o = p->in.op;

	for( f=opfunc; f->fop; f++ ) {
		if( o==f->fop ) goto convert;
	}
	return;

	convert:
	t = p->in.type;
	t1 = p->in.left->in.type;
	t2 = p->in.right->in.type;
	if ( t1 != UNSIGNED && (t2 != UNSIGNED)) return;

	/* need to rewrite tree for ASG OP */
	/* must change ASG OP to a simple OP */
	if( asgop( o ) ) {
		q = talloc();
		q->in.op = NOASG ( o );
		q->in.rall = NOPREF;
		q->in.type = p->in.type;
		q->in.left = tcopy(p->in.left);
		q->in.right = p->in.right;
		p->in.op = ASSIGN;
		p->in.right = q;
		zappost(q->in.left); /* remove post-INCR(DECR) from new node */
		fixpre(q->in.left);	/* change pre-INCR(DECR) to +/-	*/
		p = q;

	}
	/* turn logicals to compare 0 */
	else if( logop( o ) ) {
		ncopy(q = talloc(), p);
		p->in.left = q;
		p->in.right = q = talloc();
		q->in.op = ICON;
		q->in.type = INT;
#ifndef FLEXNAMES
		q->in.name[0] = '\0';
#else
		q->in.name = "";
#endif
		q->tn.lval = 0;
		q->tn.rval = 0;
		p = p->in.left;
	}

	/* build comma op for args to function */
	t1 = p->in.left->in.type;
	t2 = 0;
	if ( optype(p->in.op) == BITYPE) {
		q = talloc();
		q->in.op = CM;
		q->in.rall = NOPREF;
		q->in.type = INT;
		q->in.left = p->in.left;
		q->in.right = p->in.right;
		t2 = p->in.right->in.type;
	} else
		q = p->in.left;

	p->in.op = CALL;
	p->in.right = q;

	/* put function name in left node of call */
	p->in.left = q = talloc();
	q->in.op = ICON;
	q->in.rall = NOPREF;
	q->in.type = INCREF( FTN + p->in.type );
#ifndef FLEXNAMES
		strcpy( q->in.name, f->func );
#else
		q->in.name = f->func;
#endif
	q->tn.lval = 0;
	q->tn.rval = 0;

	}

zappost(p) NODE *p; {
	/* look for ++ and -- operators and remove them */

	register int o, ty;
	register NODE *q;
	o = p->in.op;
	ty = optype( o );

	switch( o ){

	case INCR:
	case DECR:
			q = p->in.left;
			p->in.right->in.op = FREE;  /* zap constant */
			ncopy( p, q );
			q->in.op = FREE;
			return;

		}

	if( ty == BITYPE ) zappost( p->in.right );
	if( ty != LTYPE ) zappost( p->in.left );
}

fixpre(p) NODE *p; {

	register int o, ty;
	o = p->in.op;
	ty = optype( o );

	switch( o ){

	case ASG PLUS:
			p->in.op = PLUS;
			break;
	case ASG MINUS:
			p->in.op = MINUS;
			break;
		}

	if( ty == BITYPE ) fixpre( p->in.right );
	if( ty != LTYPE ) fixpre( p->in.left );
}

NODE * addroreg(l) NODE *l;
				/* OREG was built in clocal()
				 * for an auto or formal parameter
				 * now its address is being taken
				 * local code must unwind it
				 * back to PLUS/MINUS REG ICON
				 * according to local conventions
				 */
{
	cerror("address of OREG taken");
}

# ifndef ONEPASS
main( argc, argv ) char *argv[]; {
	return( mainp2( argc, argv ) );
	}
# endif

myreader(p) register NODE *p; {
	walkf( p, hardops );	/* convert ops to function calls */
	canon( p );		/* expands r-vals for fileds */
	walkf( p, optim2 );
	}
