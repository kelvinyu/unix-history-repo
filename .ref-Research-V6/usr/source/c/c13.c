int opdope[] {
	000000,	/* EOF */
	000000,	/* ; */
	000000,	/* { */
	000000,	/* } */
	036000,	/* [ */
	002000,	/* ] */
	036000,	/* ( */
	002000,	/* ) */
	014201,	/* : */
	007001,	/* , */
	000000,	/* field selection */
	000000,	/* 11 */
	000000,	/* 12 */
	000000,	/* 13 */
	000000,	/* 14 */
	000000,	/* 15 */
	000000,	/* 16 */
	000000,	/* 17 */
	000000,	/* 18 */
	000000,	/* 19 */
	000400,	/* name */
	000400,	/* short constant */
	000400,	/* string */
	000400,	/* float */
	000400,	/* double */
	000000,	/* 25 */
	000000,	/* 26 */
	000400,	/* autoi, *r++ */
	000400,	/* autod, *--r */
	000000,	/* 29 */
	034203,	/* ++pre */
	034203,	/* --pre */
	034203,	/* ++post */
	034203,	/* --post */
	034220,	/* !un */
	034202,	/* &un */
	034220,	/* *un */
	034200,	/* -un */
	034220,	/* ~un */
	036001,	/* . (structure reference) */
	030101,	/* + */
	030001,	/* - */
	032101,	/* * */
	032001,	/* / */
	032001,	/* % */
	026061,	/* >> */
	026061,	/* << */
	020161,	/* & */
	016161,	/* | */
	016161,	/* ^ */
	036001,	/* -> */
	001000,	/* int -> double */
	001000,	/* double -> int */
	000001,	/* && */
	000001,	/* || */
	030001, /* &~ */
	001000,	/* double -> long */
	001000,	/* long -> double */
	001000,	/* integer -> long */
	001000,	/* long -> integer */
	022005,	/* == */
	022005,	/* != */
	024005,	/* <= */
	024005,	/* < */
	024005,	/* >= */
	024005,	/* > */
	024005,	/* <p */
	024005,	/* <=p */
	024005,	/* >p */
	024005,	/* >=p */
	012213,	/* =+ */
	012213,	/* =- */
	012213,	/* =* */
	012213,	/* =/ */
	012213,	/* =% */
	012253,	/* =>> */
	012253,	/* =<< */
	012253,	/* =& */
	012253,	/* =| */
	012253,	/* =^ */
	012213,	/* = */
	030001, /* & for tests */
	032101,	/*  * (long) */
	032001,	/*  / (long) */
	032001,	/* % (long) */
	012253,	/* =& ~ */
	012213,	/* =* (long) */
	012213,	/* / (long) */
	012213,	/* % (long) */
	000000,	/* 89 */
	014201,	/* ? */
	026061,	/* long << */
	012253,	/* long =<< */
	000000,	/* 93 */
	000000,	/* 94 */
	000000,	/* 95 */
	000000,	/* 96 */
	000000,	/* 97 */
	000000,	/* 98 */
	000000,	/* 99 */
	036001,	/* call */
	036000,	/* mcall */
	000000,	/* goto */
	000000,	/* jump cond */
	000000,	/* branch cond */
	000400,	/* set nregs */
	000000, /* 106 */
	000000,	/* 107 */
	000000,	/* 108 */
	000000,	/* 109 */
	000000	/* force r0 */
};

char	*opntab[] {
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	":",
	",",
	"field select",
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	"name",
	"short constant",
	"string",
	"float",
	"double",
	0,
	0,
	"*r++",
	"*--r",
	0,
	"++pre",
	"--pre",
	"++post",
	"--post",
	"!un",
	"&",
	"*",
	"-",
	"~",
	".",
	"+",
	"-",
	"*",
	"/",
	"%",
	">>",
	"<<",
	"&",
	"|",
	"^",
	"->",
	"int->double",
	"double->int",
	"&&",
	"||",
	"&~",
	"double->long",
	"long->double",
	"integer->long",
	"long->integer",
	"==",
	"!=",
	"<=",
	"<",
	">=",
	">",
	"<p",
	"<=p",
	">p",
	">=p",
	"=+",
	"=-",
	"=*",
	"=/",
	"=%",
	"=>>",
	"=<<",
	"=&",
	"=|",
	"=^",
	"=",
	"& for tests",
	"*",
	"/",
	"%",
	"=& ~",
	"=*",
	"=/",
	"=%",
	0,
	"?",
	"<<",
	"=<<",
	0,
	0,
	0,
	0,
	0,
	"call",
	"call",
	"call",
	0,
	"goto",
	"jump cond",
	"branch cond",
	"set nregs",
	"load value",
	0,
	0,
	0,
	"force register"
};
