/* Copyright (c) 1979 Regents of the University of California */

/* static char sccsid[] = "@(#)0.h 1.14 %G%"; */

#define DEBUG
#define CONSETS
#define	CHAR
#define	STATIC
#define hp21mx 0

#include	<stdio.h>
#include	<sys/types.h>

typedef enum {FALSE, TRUE} bool;

/*
 * Option flags
 *
 * The following options are recognized in the text of the program
 * and also on the command line:
 *
 *	b	block buffer the file output
 *
 *	i	make a listing of the procedures and functions in
 *		the following include files
 *
 *	l	make a listing of the program
 *
 *	n	place each include file on a new page with a header
 *
 *	p	disable post mortem and statement limit counting
 *
 *	t	disable run-time tests
 *
 *	u	card image mode; only first 72 chars of input count
 *
 *	w	suppress special diagnostic warnings
 *
 *	z	generate counters for an execution profile
 */
#ifdef DEBUG
bool	fulltrace, errtrace, testtrace, yyunique;
#endif DEBUG

/*
 * Each option has a stack of 17 option values, with opts giving
 * the current, top value, and optstk the value beneath it.
 * One refers to option `l' as, e.g., opt('l') in the text for clarity.
 */
char	opts[ 'z' - 'A' + 1];
short	optstk[ 'z' - 'A' + 1];

#define opt(c) opts[c-'A']

/*
 * Monflg is set when we are generating
 * a pxp profile.  this is set by the -z command line option.
 */
bool	monflg;

    /*
     *	profflag is set when we are generating a prof profile.
     *	this is set by the -p command line option.
     */
bool	profflag;


/*
 * NOTES ON THE DYNAMIC NATURE OF THE DATA STRUCTURES
 *
 * Pi uses expandable tables for
 * its namelist (symbol table), string table
 * hash table, and parse tree space.  The following
 * definitions specify the size of the increments
 * for these items in fundamental units so that
 * each uses approximately 1024 bytes.
 */

#define	STRINC	1024		/* string space increment */
#define	TRINC	512		/* tree space increment */
#define	HASHINC	509		/* hash table size in words, each increment */
#define	NLINC	56		/* namelist increment size in nl structs */

/*
 * The initial sizes of the structures.
 * These should be large enough to compile
 * an "average" sized program so as to minimize
 * storage requests.
 * On a small system or and 11/34 or 11/40
 * these numbers can be trimmed to make the
 * compiler smaller.
 */
#define	ITREE	2000
#define	INL	200
#define	IHASH	509

/*
 * The following limits on hash and tree tables currently
 * allow approximately 1200 symbols and 20k words of tree
 * space.  The fundamental limit of 64k total data space
 * should be exceeded well before these are full.
 */
/*
 * TABLE_MULTIPLIER is for uniformly increasing the sizes of the tables
 */
#ifdef VAX
#define TABLE_MULTIPLIER	8
#else
#define TABLE_MULTIPLIER	1
#endif VAX
#define	MAXHASH	(4 * TABLE_MULTIPLIER)
#define	MAXNL	(12 * TABLE_MULTIPLIER)
#define	MAXTREE	(30 * TABLE_MULTIPLIER)
/*
 * MAXDEPTH is the depth of the parse stack.
 * STACK_MULTIPLIER is for increasing its size.
 */
#ifdef VAX
#define	STACK_MULTIPLIER	8
#else
#define	STACK_MULTIPLIER	1
#endif VAX
#define	MAXDEPTH ( 150 * STACK_MULTIPLIER )

/*
 * ERROR RELATED DEFINITIONS
 */

/*
 * Exit statuses to pexit
 *
 * AOK
 * ERRS		Compilation errors inhibit obj productin
 * NOSTART	Errors before we ever got started
 * DIED		We ran out of memory or some such
 */
#define	AOK	0
#define	ERRS	1
#define	NOSTART	2
#define	DIED	3

bool	Recovery;

#define	eholdnl()	Eholdnl = 1
#define	nocascade()	Enocascade = 1

bool	Eholdnl, Enocascade;


/*
 * The flag eflg is set whenever we have a hard error.
 * The character in errpfx will precede the next error message.
 * When cgenflg is set code generation is suppressed.
 * This happens whenver we have an error (i.e. if eflg is set)
 * and when we are walking the tree to determine types only.
 */
bool	eflg;
char	errpfx;

#define	setpfx(x)	errpfx = x

#define	standard()	setpfx('s')
#define	warning()	setpfx('w')
#define	recovered()	setpfx('e')

int	cgenflg;


/*
 * The flag syneflg is used to suppress the diagnostics of the form
 *	E 10 a, defined in someprocedure, is neither used nor set
 * when there were syntax errors in "someprocedure".
 * In this case, it is likely that these warinings would be spurious.
 */
bool	syneflg;

/*
 * The compiler keeps its error messages in a file.
 * The variable efil is the unit number on which
 * this file is open for reading of error message text.
 * Similarly, the file ofil is the unit of the file
 * "obj" where we write the interpreter code.
 */
short	efil;
short	ofil;
short	obuf[518];

bool	Enoline;
#define	elineoff()	Enoline = TRUE
#define	elineon()	Enoline = FALSE


/*
 * SYMBOL TABLE STRUCTURE DEFINITIONS
 *
 * The symbol table is henceforth referred to as the "namelist".
 * It consists of a number of structures of the form "nl" below.
 * These are contained in a number of segments of the symbol
 * table which are dynamically allocated as needed.
 * The major namelist manipulation routines are contained in the
 * file "nl.c".
 *
 * The major components of a namelist entry are the "symbol", giving
 * a pointer into the string table for the string associated with this
 * entry and the "class" which tells which of the (currently 19)
 * possible types of structure this is.
 *
 * Many of the classes use the "type" field for a pointer to the type
 * which the entry has.
 *
 * Other pieces of information in more than one class include the block
 * in which the symbol is defined, flags indicating whether the symbol
 * has been used and whether it has been assigned to, etc.
 *
 * A more complete discussion of the features of the namelist is impossible
 * here as it would be too voluminous.  Refer to the "PI 1.0 Implementation
 * Notes" for more details.
 */

/*
 * The basic namelist structure.
 * There are also two other variants, defining the real
 * field as longs or integers given below.
 *
 * The array disptab defines the hash header for the symbol table.
 * Symbols are hashed based on the low 6 bits of their pointer into
 * the string table; see the routines in the file "lookup.c" and also "fdec.c"
 * especially "funcend".
 */
extern struct	nl *Fp;
extern int	pnumcnt;

#ifdef PTREE
#   include	"pTree.h"
#endif PTREE
struct	nl {
	char	*symbol;
	char	class, nl_flags;
#ifdef PC
	char	extra_flags;	/* for where things are */
#endif PC
	struct	nl *type;
	struct	nl *chain, *nl_next;
	int	value[5];
#	ifdef PTREE
	    pPointer	inTree;
#	endif PTREE
} *nlp, *disptab[077+1];

extern struct nl nl[INL];

struct {
	char	*symbol;
	char	class, nl_flags;
#ifdef PC
	char	extra_flags;
#endif
	struct	nl *type;
	struct	nl *chain, *nl_next;
	double	real;
};

struct {
	char	*symbol;
	char	class, nl_block;
#ifdef PC
	char	extra_flags;
#endif
	struct	nl *type;
	struct	nl *chain, *nl_next;
	long	range[2];
};

struct {
	char	*symbol;
	char	class, nl_flags;
#ifdef PC
	char	extra_flags;
#endif
	struct	nl *type;
	struct	nl *chain, *nl_next;
	int	*ptr[4];
#ifdef PI
	int	entloc;
#endif PI
};

/*
 * NL FLAGS BITS
 *
 * Definitions of the usage of the bits in
 * the nl_flags byte. Note that the low 5 bits of the
 * byte are the "nl_block" and that some classes make use
 * of this byte as a "width".
 *
 * The only non-obvious bit definition here is "NFILES"
 * which records whether a structure contains any files.
 * Such structures are not allowed to be dynamically allocated.
 */

#define	BLOCKNO( flag )	( flag & 037 )
#define NLFLAGS( flag ) ( flag &~ 037 )

#define	NUSED	0100
#define	NMOD	0040
#define	NFORWD	0200
#define	NFILES	0200
#ifdef PC
#define NEXTERN 0001	/* flag used to mark external funcs and procs */
#define	NLOCAL	0002	/* variable is a local */
#define	NPARAM	0004	/* variable is a parameter */
#define	NGLOBAL	0010	/* variable is a global */
#define	NREGVAR	0020	/* or'ed in if variable is in a register */
#endif PC

/*
 * used to mark value[ NL_FORV ] for loop variables
 */
#define	FORVAR		1

/*
 * Definition of the commonly used "value" fields.
 * The most important one is NL_OFFS which gives
 * the offset of a variable in its stack mark.
 */
#define NL_OFFS	0

#define	NL_CNTR	1
#define NL_NLSTRT 2
#define	NL_LINENO 3
#define	NL_FVAR	3
#define	NL_FCHAIN 4

#define NL_GOLEV 2
#define NL_GOLINE 3
#define NL_FORV 1

#define	NL_FLDSZ 1
#define	NL_VARNT 2
#define	NL_VTOREC 2
#define	NL_TAG	3

#define	NL_ELABEL	4

/*
 * For BADUSE nl structures, NL_KINDS is a bit vector
 * indicating the kinds of illegal usages complained about
 * so far.  For kind of bad use "kind", "1 << kind" is set.
 * The low bit is reserved as ISUNDEF to indicate whether
 * this identifier is totally undefined.
 */
#define	NL_KINDS	0

#define	ISUNDEF		1

    /*
     *	variables come in three flavors: globals, parameters, locals;
     *	they can also hide in registers, but that's a different flag
     */
#define PARAMVAR	1
#define LOCALVAR	2
#define	GLOBALVAR	3

/*
 * NAMELIST CLASSES
 *
 * The following are the namelist classes.
 * Different classes make use of the value fields
 * of the namelist in different ways.
 *
 * The namelist should be redesigned by providing
 * a number of structure definitions with one corresponding
 * to each namelist class, ala a variant record in Pascal.
 */
#define	BADUSE	0
#define	CONST	1
#define	TYPE	2
#define	VAR	3
#define	ARRAY	4
#define	PTRFILE	5
#define	RECORD	6
#define	FIELD	7
#define	PROC	8
#define	FUNC	9
#define	FVAR	10
#define	REF	11
#define	PTR	12
#define	FILET	13
#define	SET	14
#define	RANGE	15
#define	LABEL	16
#define	WITHPTR 17
#define	SCAL	18
#define	STR	19
#define	PROG	20
#define	IMPROPER 21
#define	VARNT	22
#define	FPROC	23
#define	FFUNC	24

/*
 * Clnames points to an array of names for the
 * namelist classes.
 */
char	**clnames;

/*
 * PRE-DEFINED NAMELIST OFFSETS
 *
 * The following are the namelist offsets for the
 * primitive types. The ones which are negative
 * don't actually exist, but are generated and tested
 * internally. These definitions are sensitive to the
 * initializations in nl.c.
 */
#define	TFIRST -7
#define	TFILE  -7
#define	TREC   -6
#define	TARY   -5
#define	TSCAL  -4
#define	TPTR   -3
#define	TSET   -2
#define	TSTR   -1
#define	NIL	0
#define	TBOOL	1
#define	TCHAR	2
#define	TINT	3
#define	TDOUBLE	4
#define	TNIL	5
#define	T1INT	6
#define	T2INT	7
#define	T4INT	8
#define	T1CHAR	9
#define	T1BOOL	10
#define	T8REAL	11
#define TLAST	11

/*
 * SEMANTIC DEFINITIONS
 */

/*
 * NOCON and SAWCON are flags in the tree telling whether
 * a constant set is part of an expression.
 *	these are no longer used,
 *	since we now do constant sets at compile time.
 */
#define NOCON	0
#define SAWCON	1

/*
 * The variable cbn gives the current block number,
 * the variable bn is set as a side effect of a call to
 * lookup, and is the block number of the variable which
 * was found.
 */
short	bn, cbn;

/*
 * The variable line is the current semantic
 * line and is set in stat.c from the numbers
 * embedded in statement type tree nodes.
 */
short	line;

/*
 * The size of the display
 * which defines the maximum nesting
 * of procedures and functions allowed.
 * Because of the flags in the current namelist
 * this must be no greater than 32.
 */
#define	DSPLYSZ 20

    /*
     *	the display is made up of saved AP's and FP's.
     *	FP's are used to find locals, and AP's are used to find parameters.
     *	FP and AP are untyped pointers, but are used throughout as (char *).
     *	the display is used by adding AP_OFFSET or FP_OFFSET to the 
     *	address of the approriate display entry.
     */
struct dispsave {
    char	*savedAP;
    char	*savedFP;
} display[ DSPLYSZ ];

#define	AP_OFFSET	( 0 )
#define FP_OFFSET	( sizeof(char *) )

    /*
     *	formal routine structure:
     */
struct formalrtn {
	long		(*fentryaddr)();	/* formal entry point */
	long		fbn;			/* block number of function */
	struct dispsave	fdisp[ DSPLYSZ ];	/* saved at first passing */
} frtn;

#define	FENTRYOFFSET	0
#define FBNOFFSET	( FENTRYOFFSET + sizeof frtn.fentryaddr )
#define	FDISPOFFSET	( FBNOFFSET + sizeof frtn.fbn )

/*
 * The following structure is used
 * to keep track of the amount of variable
 * storage required by each block.
 * "Max" is the high water mark, "off"
 * the current need. Temporaries for "for"
 * loops and "with" statements are allocated
 * in the local variable area and these
 * numbers are thereby changed if necessary.
 */
struct om {
	long	om_max;
	long	reg_max;
	struct tmps {
		long	om_off;
		long	reg_off;
	} curtmps;
} sizes[DSPLYSZ];
#define NOREG 0
#define REGOK 1

    /*
     *	the following structure records whether a level declares
     *	any variables which are (or contain) files.
     *	this so that the runtime routines for file cleanup can be invoked.
     */
bool	dfiles[ DSPLYSZ ];

/*
 * Structure recording information about a constant
 * declaration.  It is actually the return value from
 * the routine "gconst", but since C doesn't support
 * record valued functions, this is more convenient.
 */
struct {
	struct nl	*ctype;
	short		cival;
	double		crval;
	int		*cpval;
} con;

/*
 * The set structure records the lower bound
 * and upper bound with the lower bound normalized
 * to zero when working with a set. It is set by
 * the routine setran in var.c.
 */
struct {
	short	lwrb, uprbp;
} set;

    /*
     *	structures of this kind are filled in by precset and used by postcset
     *	to indicate things about constant sets.
     */
struct csetstr {
    struct nl	*csettype;
    long	paircnt;
    long	singcnt;
    bool	comptime;
};
/*
 * The following flags are passed on calls to lvalue
 * to indicate how the reference is to affect the usage
 * information for the variable being referenced.
 * MOD is used to set the NMOD flag in the namelist
 * entry for the variable, ASGN permits diagnostics
 * to be formed when a for variable is assigned to in
 * the range of the loop.
 */
#define	NOFLAGS	0
#define	MOD	01
#define	ASGN	02
#define	NOUSE	04

    /*
     *	the following flags are passed to lvalue and rvalue
     *	to tell them whether an lvalue or rvalue is required.
     *	the semantics checking is done according to the function called,
     *	but for pc, lvalue may put out an rvalue by indirecting afterwards,
     *	and rvalue may stop short of putting out the indirection.
     */
#define	LREQ	01
#define	RREQ	02

double	MAXINT;
double	MININT;

/*
 * Variables for generation of profile information.
 * Monflg is set when we want to generate a profile.
 * Gocnt record the total number of goto's and
 * cnts records the current counter for generating
 * COUNT operators.
 */
short	gocnt;
short	cnts;

/*
 * Most routines call "incompat" rather than asking "!compat"
 * for historical reasons.
 */
#define incompat 	!compat

/*
 * Parts records which declaration parts have been seen.
 * The grammar allows the "label" "const" "type" "var" and routine
 * parts to be repeated and to be in any order, so that
 * they can be detected semantically to give better
 * error diagnostics.
 */
int	parts[ DSPLYSZ ];

#define	LPRT	1
#define	CPRT	2
#define	TPRT	4
#define	VPRT	8
#define	RPRT	16

/*
 * Flags for the "you used / instead of div" diagnostic
 */
bool	divchk;
bool	divflg;

bool	errcnt[DSPLYSZ];

/*
 * Forechain links those types which are
 *	^ sometype
 * so that they can be evaluated later, permitting
 * circular, recursive list structures to be defined.
 */
struct	nl *forechain;

/*
 * Withlist links all the records which are currently
 * opened scopes because of with statements.
 */
struct	nl *withlist;

struct	nl *intset;
struct	nl *input, *output;
struct	nl *program;

/* progseen flag used by PC to determine if
 * a routine segment is being compiled (and
 * therefore no program statement seen)
 */
bool	progseen;


/*
 * STRUCTURED STATEMENT GOTO CHECKING
 *
 * The variable level keeps track of the current
 * "structured statement level" when processing the statement
 * body of blocks.  This is used in the detection of goto's into
 * structured statements in a block.
 *
 * Each label's namelist entry contains two pieces of information
 * related to this check. The first `NL_GOLEV' either contains
 * the level at which the label was declared, `NOTYET' if the label
 * has not yet been declared, or `DEAD' if the label is dead, i.e.
 * if we have exited the level in which the label was defined.
 *
 * When we discover a "goto" statement, if the label has not
 * been defined yet, then we record the current level and the current line
 * for a later error check.  If the label has been already become "DEAD"
 * then a reference to it is an error.  Now the compiler maintains,
 * for each block, a linked list of the labels headed by "gotos[bn]".
 * When we exit a structured level, we perform the routine
 * ungoto in stat.c. It notices labels whose definition levels have been
 * exited and makes them be dead. For labels which have not yet been
 * defined, ungoto will maintain NL_GOLEV as the minimum structured level
 * since the first usage of the label. It is not hard to see that the label
 * must eventually be declared at this level or an outer level to this
 * one or a goto into a structured statement will exist.
 */
short	level;
struct	nl *gotos[DSPLYSZ];

#define	NOTYET	10000
#define	DEAD	10000

/*
 * Noreach is true when the next statement will
 * be unreachable unless something happens along
 * (like exiting a looping construct) to save
 * the day.
 */
bool	noreach;

/*
 * UNDEFINED VARIABLE REFERENCE STRUCTURES
 */
struct	udinfo {
	int	ud_line;
	struct	udinfo *ud_next;
	char	nullch;
};

/*
 * CODE GENERATION DEFINITIONS
 */

/*
 * NSTAND is or'ed onto the abstract machine opcode
 * for non-standard built-in procedures and functions.
 */
#define	NSTAND	0400

#define	codeon()	cgenflg++
#define	codeoff()	--cgenflg
#define	CGENNING	( cgenflg >= 0 )

/*
 * Codeline is the last lino output in the code generator.
 * It used to be used to suppress LINO operators but no
 * more since we now count statements.
 * Lc is the intepreter code location counter.
 *
short	codeline;
 */
char	*lc;


/*
 * Routines which need types
 * other than "integer" to be
 * assumed by the compiler.
 */
double		atof();
long		lwidth();
long		leven();
long		aryconst();
long		a8tol();
long		roundup();
struct nl 	*tmpalloc();
struct nl 	*lookup();
double		atof();
int		*tree();
int		*hash();
char		*alloc();
int		*calloc();
char		*savestr();
char		*parnam();
bool		fcompat();
struct nl	*lookup1();
struct nl	*hdefnl();
struct nl	*defnl();
struct nl	*enter();
struct nl	*nlcopy();
struct nl	*tyrecl();
struct nl	*tyary();
struct nl	*fields();
struct nl	*variants();
struct nl	*deffld();
struct nl	*defvnt();
struct nl	*tyrec1();
struct nl	*reclook();
struct nl	*asgnop1();
struct nl	*gtype();
struct nl	*call();
struct nl	*lvalue();
struct nl	*rvalue();
struct nl	*cset();

/*
 * type cast NIL to keep lint happy (which is not so bad)
 */
#define		NLNIL	( (struct nl *) NIL )

/*
 * Funny structures to use
 * pointers in wild and wooly ways
 */
struct {
	char	pchar;
};
struct {
	short	pint;
	short	pint2;
};
struct {
	long	plong;
};
struct {
	double	pdouble;
};

#define	OCT	1
#define	HEX	2

/*
 * MAIN PROGRAM VARIABLES, MISCELLANY
 */

/*
 * Variables forming a data base referencing
 * the command line arguments with the "i" option, e.g.
 * in "pi -i scanner.i compiler.p".
 */
char	**pflist;
short	pflstc;
short	pfcnt;

char	*filename;		/* current source file name */
long	tvec;
extern char	*snark;		/* SNARK */
extern char	*classes[ ];	/* maps namelist classes to string names */

#define	derror error

#ifdef	PC

    /*
     *	the current function number, for [ lines
     */
    int	ftnno;

    /*
     *	the pc output stream
     */
    FILE *pcstream;

#endif PC
