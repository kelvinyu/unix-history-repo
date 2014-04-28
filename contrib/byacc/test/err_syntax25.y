%{
int yylex(void);
static void yyerror(const char *);
%}

%union {
	int ival;
	double dval;
}

%union {
	int ival2;
	double dval2;
}

%start expr
%type <tag2> expr

%token NUMBER

%%

expr  :  '(' recur ')'
      ;

recur :  NUMBER
	{ $$ = 1; }
      ;

%%

#include <stdio.h>

int
main(void)
{
    printf("yyparse() = %d\n", yyparse());
    return 0;
}

int
yylex(void)
{
    return -1;
}

static void
yyerror(const char* s)
{
    printf("%s\n", s);
}
