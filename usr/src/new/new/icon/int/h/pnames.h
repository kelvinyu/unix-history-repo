struct pstrnm {
	char *pstrep;
	struct b_proc *pblock;
	};

extern struct b_proc
   Babs,           Bany,           Bbal,           Bcenter,
   Bclose,         Bcopy,          Bcset,          Bdisplay,
   Bexit,          Bfind,          Bget,           Bimage,
   Binteger,       Bleft,          Blist,
   Bmany,          Bmap,           Bmatch,         Bmove,
   Bnumeric,       Bopen,          Bpop,           Bpos,
   Bpull,          Bpush,          Bput,           Bread,
   Breads,         Breal,          Brepl,          Breverse,
   Bright,
   Bseq,
   Bsort,          Bstop,          Bstring,
   Bsystem,        Btab,           Btable,         Btrim,
   Btype,          Bupto,          Bwrite,         Bwrites;

extern struct b_proc
   Basgn,	Bbang,		Bcat,		Bcompl,
   Bdiff,	Bdiv,		Beqv,		Binter,
   Blconcat,	Blexeq,		Blexge,		Blexgt,
   Blexle,	Blexlt,		Blexne,		Bminus,
   Bmod,	Bmult,		Bneg,		Bneqv,
   Bnonnull,	Bnull,		Bnumber,	Bnumeq,
   Bnumge,	Bnumgt,		Bnumle,		Bnumlt,
   Bnumne,	Bplus,		Bpower,		Brandom,
   Brasgn,	Brefresh,	Brswap,		Bsect,
   Bsize,	Bsubsc,		Bswap,		Btabmat,
   Btoby,	Bunioncs,	Bvalue;

struct pstrnm pntab[] = {
	"abs",		&Babs,
	"any",		&Bany,
	"bal",		&Bbal,
	"center",	&Bcenter,
	"close",	&Bclose,
	"copy",		&Bcopy,
	"cset",		&Bcset,
	"display",	&Bdisplay,
	"exit",		&Bexit,
	"find",		&Bfind,
	"get",		&Bget,
	"image",	&Bimage,
	"integer",	&Binteger,
	"left",		&Bleft,
	"list",		&Blist,
	"many",		&Bmany,
	"map",		&Bmap,
	"match",	&Bmatch,
	"move",		&Bmove,
	"numeric",	&Bnumeric,
	"open",		&Bopen,
	"pop",		&Bpop,
	"pos",		&Bpos,
	"pull",		&Bpull,
	"push",		&Bpush,
	"put",		&Bput,
	"read",		&Bread,
	"reads",	&Breads,
	"real",		&Breal,
	"repl",		&Brepl,
	"reverse",	&Breverse,
	"right",	&Bright,
	"seq",		&Bseq,
	"sort",		&Bsort,
	"stop",		&Bstop,
	"string",	&Bstring,
	"system",	&Bsystem,
	"tab",		&Btab,
	"table",	&Btable,
	"trim",		&Btrim,
	"type",		&Btype,
	"upto",		&Bupto,
	"write",	&Bwrite,
	"writes",	&Bwrites,
	":=",		&Basgn,
	"!",		&Bbang,
	"||",		&Bcat,
	"~",		&Bcompl,
	"--",		&Bdiff,
	"/",		&Bdiv,
	"===",		&Beqv,
	"**",		&Binter,
	"|||",		&Blconcat,
	"==",		&Blexeq,
	">>=",		&Blexge,
	">>",		&Blexgt,
	"<<=",		&Blexle,
	"<<",		&Blexlt,
	"~==",		&Blexne,
	"-",		&Bminus,
	"%",		&Bmod,
	"*",		&Bmult,
	"-",		&Bneg,
	"~===",		&Bneqv,
	"\\",		&Bnonnull,
	"/",		&Bnull,
	"+",		&Bnumber,
	"=",		&Bnumeq,
	">=",		&Bnumge,
	">",		&Bnumgt,
	"<=",		&Bnumle,
	"<",		&Bnumlt,
	"~=",		&Bnumne,
	"+",		&Bplus,
	"^",		&Bpower,
	"?",		&Brandom,
	"<-",		&Brasgn,
	"^",		&Brefresh,
	"<->",		&Brswap,
	":",		&Bsect,
	"*",		&Bsize,
	"[]",		&Bsubsc,
	":=:",		&Bswap,
	"=",		&Btabmat,
	"...",		&Btoby,
	"++",		&Bunioncs,
	".",		&Bvalue,
	0,		0};

