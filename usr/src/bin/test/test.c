/*
 * Copyright (c) 1988 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Kenneth Almquist.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char copyright[] =
"@(#) Copyright (c) 1988 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif				/* not lint */

#ifndef lint
static char sccsid[] = "@(#)test.c	1.4 (Berkeley) %G%";
#endif				/* not lint */

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include "operators.h"
#include "extern.h"

#define STACKSIZE 12
#define NESTINCR 16

/* data types */
#define STRING 0
#define INTEGER 1
#define BOOLEAN 2

#define INITARGS(argv)	if (argv[0] == NULL) {fputs("Argc is zero\n", stderr); exit(2);} else

#define IS_BANG(s) (s[0] == '!' && s[1] == '\0')
#define equal(s1, s2)   (strcmp(s1, s2) == 0)

/*
 * This structure hold a value.  The type keyword specifies the type of
 * the value, and the union u holds the value.  The value of a boolean
 * is stored in u.num (1 = TRUE, 0 = FALSE).
 */

struct value {
	int     type;
	union {
		char   *string;
		long    num;
	}       u;
};

struct operator {
	short   op;		/* which operator */
	short   pri;		/* priority of operator */
};

struct filestat {
	char   *name;		/* name of file */
	int     rcode;		/* return code from stat */
	struct stat stat;	/* status info on file */
};

static int expr_is_false __P((struct value *));
static void expr_operator __P((int, struct value *, struct filestat *));
static int lookup_op __P((char *, char *const *));
static long atol __P((const char *));
static int posix_binary_op __P((char **));
static int posix_unary_op __P((char **));
static int int_tcheck __P((char *));

int
main(argc, argv)
	int     argc;
	char  **argv;
{
	char  **ap;
	char   *opname;
	char    c;
	char   *p;
	int     nest;		/* parentheses nesting */
	int     op;
	int     pri;
	int     skipping;
	int     binary;
	struct operator opstack[STACKSIZE];
	struct operator *opsp;
	struct value valstack[STACKSIZE + 1];
	struct value *valsp;
	struct filestat fs;
	int     ret_val;

	INITARGS(argv);
	if (**argv == '[') {
		if (!equal(argv[argc - 1], "]"))
			error("missing ]");
		argv[argc - 1] = NULL;
	}
	ap = argv + 1;
	fs.name = NULL;

	/* Test(1) implements an inherently ambiguous grammer.  In order to
	 * assure some degree of consistency, we special case the POSIX
	 * requirements to assure correct evaluation for POSIX following
	 * scripts.  The following special cases comply with POSIX
	 * P1003.2/D11.2 Section 4.62.4. */
	switch (argc - 1) {
	case 0:		/* % test */
		return 1;
		break;
	case 1:		/* % test arg */
		return (*argv[1] == '\0') ? 1 : 0;
		break;
	case 2:		/* % test op arg */
		opname = argv[1];
		if (IS_BANG(opname))
			return (*argv[2] == '\0') ? 1 : 0;
		else {
			ret_val = posix_unary_op(&argv[1]);
			if (ret_val >= 0)
				return ret_val;
		}
		break;
	case 3:		/* % test arg1 op arg2 */
		if (IS_BANG(argv[1])) {
			ret_val = posix_unary_op(&argv[1]);
			if (ret_val >= 0)
				return !ret_val;
		} else {
			ret_val = posix_binary_op(&argv[1]);
			if (ret_val >= 0)
				return ret_val;
		}
		break;
	case 4:		/* % test ! arg1 op arg2 */
		if (IS_BANG(argv[1])) {
			ret_val = posix_binary_op(&argv[2]);
			if (ret_val >= 0)
				return !ret_val;
		}
		break;
	default:
		break;
	}

	/* We use operator precedence parsing, evaluating the expression as
	 * we parse it.  Parentheses are handled by bumping up the priority
	 * of operators using the variable "nest."  We use the variable
	 * "skipping" to turn off evaluation temporarily for the short
	 * circuit boolean operators.  (It is important do the short circuit
	 * evaluation because under NFS a stat operation can take infinitely
	 * long.) */

	opsp = opstack + STACKSIZE;
	valsp = valstack;
	nest = 0;
	skipping = 0;
	if (*ap == NULL) {
		valstack[0].type = BOOLEAN;
		valstack[0].u.num = 0;
		goto done;
	}
	for (;;) {
		opname = *ap++;
		if (opname == NULL)
			goto syntax;
		if (opname[0] == '(' && opname[1] == '\0') {
			nest += NESTINCR;
			continue;
		} else if (*ap && (op = lookup_op(opname, unary_op)) >= 0) {
			if (opsp == &opstack[0])
				goto overflow;
			--opsp;
			opsp->op = op;
			opsp->pri = op_priority[op] + nest;
			continue;

		} else {
			valsp->type = STRING;
			valsp->u.string = opname;
			valsp++;
		}
		for (;;) {
			opname = *ap++;
			if (opname == NULL) {
				if (nest != 0)
					goto syntax;
				pri = 0;
				break;
			}
			if (opname[0] != ')' || opname[1] != '\0') {
				if ((op = lookup_op(opname, binary_op)) < 0)
					goto syntax;
				op += FIRST_BINARY_OP;
				pri = op_priority[op] + nest;
				break;
			}
			if ((nest -= NESTINCR) < 0)
				goto syntax;
		}
		while (opsp < &opstack[STACKSIZE] && opsp->pri >= pri) {
			binary = opsp->op;
			for (;;) {
				valsp--;
				c = op_argflag[opsp->op];
				if (c == OP_INT) {
					if (valsp->type == STRING &&
					    int_tcheck(valsp->u.string))
						valsp->u.num =
						    atol(valsp->u.string);
					valsp->type = INTEGER;
				} else if (c >= OP_STRING) {	
					            /* OP_STRING or OP_FILE */
					if (valsp->type == INTEGER) {
						p = stalloc(32);
#ifdef SHELL
						fmtstr(p, 32, "%d", 
						    valsp->u.num);
#else
						sprintf(p, "%d", valsp->u.num);
#endif
						valsp->u.string = p;
					} else if (valsp->type == BOOLEAN) {
						if (valsp->u.num)
							valsp->u.string = 
						            "true";
						else
							valsp->u.string = "";
					}
					valsp->type = STRING;
					if (c == OP_FILE
					    && (fs.name == NULL 
						|| !equal(fs.name, 
                                                    valsp->u.string))) {
						fs.name = valsp->u.string;
						fs.rcode = 
						    stat(valsp->u.string, 
                                                    &fs.stat);
					}
				}
				if (binary < FIRST_BINARY_OP)
					break;
				binary = 0;
			}
			if (!skipping)
				expr_operator(opsp->op, valsp, &fs);
			else if (opsp->op == AND1 || opsp->op == OR1)
				skipping--;
			valsp++;/* push value */
			opsp++;	/* pop operator */
		}
		if (opname == NULL)
			break;
		if (opsp == &opstack[0])
			goto overflow;
		if (op == AND1 || op == AND2) {
			op = AND1;
			if (skipping || expr_is_false(valsp - 1))
				skipping++;
		}
		if (op == OR1 || op == OR2) {
			op = OR1;
			if (skipping || !expr_is_false(valsp - 1))
				skipping++;
		}
		opsp--;
		opsp->op = op;
		opsp->pri = pri;
	}
done:
	return expr_is_false(&valstack[0]);

syntax:   error("syntax error");
overflow: error("Expression too complex");

}


static int
expr_is_false(val)
	struct value *val;
{
	if (val->type == STRING) {
		if (val->u.string[0] == '\0')
			return 1;
	} else {		/* INTEGER or BOOLEAN */
		if (val->u.num == 0)
			return 1;
	}
	return 0;
}


/*
 * Execute an operator.  Op is the operator.  Sp is the stack pointer;
 * sp[0] refers to the first operand, sp[1] refers to the second operand
 * (if any), and the result is placed in sp[0].  The operands are converted
 * to the type expected by the operator before expr_operator is called.
 * Fs is a pointer to a structure which holds the value of the last call
 * to stat, to avoid repeated stat calls on the same file.
 */

static void
expr_operator(op, sp, fs)
	int     op;
	struct value *sp;
	struct filestat *fs;
{
	int     i;

	switch (op) {
	case NOT:
		sp->u.num = expr_is_false(sp);
		sp->type = BOOLEAN;
		break;
	case ISEXIST:
		if (fs == NULL || fs->rcode == -1)
			goto false;
		else
			goto true;
	case ISREAD:
		i = 04;
		goto permission;
	case ISWRITE:
		i = 02;
		goto permission;
	case ISEXEC:
		i = 01;
permission:
		if (fs->stat.st_uid == geteuid())
			i <<= 6;
		else if (fs->stat.st_gid == getegid())
			i <<= 3;
		goto filebit;	/* true if (stat.st_mode & i) != 0 */
	case ISFILE:
		i = S_IFREG;
		goto filetype;
	case ISDIR:
		i = S_IFDIR;
		goto filetype;
	case ISCHAR:
		i = S_IFCHR;
		goto filetype;
	case ISBLOCK:
		i = S_IFBLK;
		goto filetype;
	case ISFIFO:
#ifdef S_IFIFO
		i = S_IFIFO;
		goto filetype;
#else
		goto false;
#endif
filetype:
		if ((fs->stat.st_mode & S_IFMT) == i && fs->rcode >= 0) {
	true:
			sp->u.num = 1;
		} else {
	false:
			sp->u.num = 0;
		}
		sp->type = BOOLEAN;
		break;
	case ISSETUID:
		i = S_ISUID;
		goto filebit;
	case ISSETGID:
		i = S_ISGID;
		goto filebit;
	case ISSTICKY:
		i = S_ISVTX;
filebit:
		if (fs->stat.st_mode & i && fs->rcode >= 0)
			goto true;
		goto false;
	case ISSIZE:
		sp->u.num = fs->rcode >= 0 ? fs->stat.st_size : 0L;
		sp->type = INTEGER;
		break;
	case ISTTY:
		sp->u.num = isatty(sp->u.num);
		sp->type = BOOLEAN;
		break;
	case NULSTR:
		if (sp->u.string[0] == '\0')
			goto true;
		goto false;
	case STRLEN:
		sp->u.num = strlen(sp->u.string);
		sp->type = INTEGER;
		break;
	case OR1:
	case AND1:
		/* These operators are mostly handled by the parser.  If we
		 * get here it means that both operands were evaluated, so
		 * the value is the value of the second operand. */
		*sp = *(sp + 1);
		break;
	case STREQ:
	case STRNE:
		i = 0;
		if (equal(sp->u.string, (sp + 1)->u.string))
			i++;
		if (op == STRNE)
			i = 1 - i;
		sp->u.num = i;
		sp->type = BOOLEAN;
		break;
	case EQ:
		if (sp->u.num == (sp + 1)->u.num)
			goto true;
		goto false;
	case NE:
		if (sp->u.num != (sp + 1)->u.num)
			goto true;
		goto false;
	case GT:
		if (sp->u.num > (sp + 1)->u.num)
			goto true;
		goto false;
	case LT:
		if (sp->u.num < (sp + 1)->u.num)
			goto true;
		goto false;
	case LE:
		if (sp->u.num <= (sp + 1)->u.num)
			goto true;
		goto false;
	case GE:
		if (sp->u.num >= (sp + 1)->u.num)
			goto true;
		goto false;

	}
}


static int
lookup_op(name, table)
	char   *name;
	char   *const * table;
{
	register char *const * tp;
	register char const *p;
	char    c = name[1];

	for (tp = table; (p = *tp) != NULL; tp++) {
		if (p[1] == c && equal(p, name))
			return tp - table;
	}
	return -1;
}

static int
posix_unary_op(argv)
	char  **argv;
{
	int     op, c;
	char   *opname;
	struct filestat fs;
	struct value valp;

	opname = *argv;
	if ((op = lookup_op(opname, unary_op)) < 0)
		return -1;
	c = op_argflag[op];
	opname = argv[1];
	valp.u.string = opname;
	if (c == OP_FILE) {
		fs.name = opname;
		fs.rcode = stat(opname, &fs.stat);
	} else if (c != OP_STRING)
		return -1;

	expr_operator(op, &valp, &fs);
	return (valp.u.num == 0);
}


static int
posix_binary_op(argv)
	char  **argv;
{
	int     op, c;
	char   *opname;
	struct value v[2];

	opname = argv[1];
	if ((op = lookup_op(opname, binary_op)) < 0)
		return -1;
	op += FIRST_BINARY_OP;
	c = op_argflag[op];

	if (c == OP_INT && int_tcheck(argv[0]) && int_tcheck(argv[2])) {
		v[0].u.num = atol(argv[0]);
		v[1].u.num = atol(argv[2]);
	} else {
		v[0].u.string = argv[0];
		v[1].u.string = argv[2];
	}
	expr_operator(op, v, NULL);
	return (v[0].u.num == 0);
}

/*
 * Integer type checking.
 */
static int
int_tcheck(v)
	char   *v;
{
	char   *p;
	char    outbuf[512];

	for (p = v; *p != '\0'; p++)
		if (*p < '0' || *p > '9') {
			snprintf(outbuf, sizeof(outbuf),
			  "Illegal operand \"%s\" -- expected integer.", v);
			error(outbuf);
		}
	return 1;
}
