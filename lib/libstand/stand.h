/*
 * Copyright (c) 1998 Michael Smith.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	$Id: stand.h,v 1.1.1.1 1998/08/20 08:19:55 msmith Exp $
 * From	$NetBSD: stand.h,v 1.22 1997/06/26 19:17:40 drochner Exp $	
 */

/*-
 * Copyright (c) 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)stand.h	8.1 (Berkeley) 6/11/93
 */

#include <sys/types.h>
#include <sys/cdefs.h>
#include <sys/stat.h>

#ifndef NULL
#define	NULL	0
#endif

/* Avoid unwanted userlandish components */
#define KERNEL
#include <sys/errno.h>
#undef KERNEL

/* special stand error codes */
#define	EADAPT	(ELAST+1)	/* bad adaptor */
#define	ECTLR	(ELAST+2)	/* bad controller */
#define	EUNIT	(ELAST+3)	/* bad unit */
#define ESLICE	(ELAST+4)	/* bad slice */
#define	EPART	(ELAST+5)	/* bad partition */
#define	ERDLAB	(ELAST+6)	/* can't read disk label */
#define	EUNLAB	(ELAST+7)	/* unlabeled disk */
#define	EOFFSET	(ELAST+8)	/* relative seek not supported */
#define	ESALAST	(ELAST+8)	/* */

struct open_file;

/*
 * This structure is used to define file system operations in a file system
 * independent way.
 *
 * XXX note that filesystem providers should export a pointer to their fs_ops
 *     struct, so that consumers can reference this and thus include the
 *     filesystems that they require.
 */
struct fs_ops {
    char	*fs_name;
    int		(*fo_open)(char *path, struct open_file *f);
    int		(*fo_close)(struct open_file *f);
    int		(*fo_read)(struct open_file *f, void *buf,
			   size_t size, size_t *resid);
    int		(*fo_write)(struct open_file *f, void *buf,
			    size_t size, size_t *resid);
    off_t	(*fo_seek)(struct open_file *f, off_t offset, int where);
    int		(*fo_stat)(struct open_file *f, struct stat *sb);
};

/*
 * libstand-supplied filesystems
 */
extern struct fs_ops ufs_fsops;
extern struct fs_ops tftp_fsops;
extern struct fs_ops nfs_fsops;
extern struct fs_ops cd9660_fsops;
extern struct fs_ops zipfs_fsops;
#ifdef notyet
extern struct fs_ops dosfs_fsops;
#endif

/* where values for lseek(2) */
#define	SEEK_SET	0	/* set file offset to offset */
#define	SEEK_CUR	1	/* set file offset to current plus offset */
#define	SEEK_END	2	/* set file offset to EOF plus offset */

/* 
 * Device switch
 */
struct devsw {
    char	dv_name[8];
    int		dv_type;		/* opaque type constant, arch-dependant */
    int		(*dv_init)(void);	/* early probe call */
    int		(*dv_strategy)(void *devdata, int rw, daddr_t blk, size_t size, void *buf, size_t *rsize);
    int		(*dv_open)(struct open_file *f, ...);
    int		(*dv_close)(struct open_file *f);
    int		(*dv_ioctl)(struct open_file *f, u_long cmd, void *data);
};

extern int errno;

struct open_file {
    int			f_flags;	/* see F_* below */
    struct devsw	*f_dev;		/* pointer to device operations */
    void		*f_devdata;	/* device specific data */
    struct fs_ops	*f_ops;		/* pointer to file system operations */
    void		*f_fsdata;	/* file system specific data */
    off_t		f_offset;	/* current file offset (F_RAW) */
};

#define	SOPEN_MAX	8
extern struct open_file files[];

/* f_flags values */
#define	F_READ		0x0001	/* file opened for reading */
#define	F_WRITE		0x0002	/* file opened for writing */
#define	F_RAW		0x0004	/* raw device open - no file system */
#define F_NODEV		0x0008	/* network open - no device */

#define isupper(c)	((c) >= 'A' && (c) <= 'Z')
#define islower(c)	((c) >= 'a' && (c) <= 'z')
#define isspace(c)	((c) == ' ' || (c) == '\t')
#define isdigit(c)	((c) >= '0' && (c) <= '9')
#define isxdigit(c)	(isdigit(c) || ((c) >= 'a' && (c) <= 'f') || ((c) >= 'A' && (c) <= 'F'))
#define isascii(c)	((c) >= 0 || (c <= 0x7f))
#define isalpha(c)	(isupper(c) || (islower(c)))
#define toupper(c)	((c) - 'a' + 'A')
#define tolower(c)	((c) - 'A' + 'a')

extern void	setheap(void *, void *);
extern void	*malloc(size_t);
extern void	free(void *);
extern char	*sbrk(int junk);

/* disklabel support (undocumented, may be junk) */
struct		disklabel;
extern char	*getdisklabel(const char *, struct disklabel *);
extern int	dkcksum(struct disklabel *);

extern int	printf(const char *fmt, ...);
extern void	vprintf(const char *fmt, _BSD_VA_LIST_);
extern int	sprintf(char *buf, const char *cfmt, ...);

extern void	twiddle(void);

extern void	ngets(char *, int);
#define gets(x)	ngets((x), 0)
extern int	fgetstr(char *buf, int size, int fd);

extern char	*strerror(int);

extern int	open(const char *, int);
#define	O_RDONLY	0x0
#define O_WRONLY	0x1			/* writing not (yet?) supported */
#define O_RDWR		0x2
extern int	close(int);
extern void	closeall(void);
extern ssize_t	read(int, void *, size_t);
extern ssize_t	write(int, void *, size_t);
extern off_t	lseek(int, off_t, int);
extern int	stat(const char *, struct stat *);

extern void	srandom(u_long seed);
extern u_long	random(void);
    
/* imports from stdlib, locally modified */
extern long	strtol(const char *, char **, int);
extern char	*optarg;			/* getopt(3) external variables */
extern int	optind, opterr, optopt;
extern int	getopt(int, char * const [], const char *);

/* pager.c */
extern void	pager_open(void);
extern void	pager_close(void);
extern int	pager_output(const char *lines);
extern int	pager_file(char *fname);

/* environment.c */
#define EV_DYNAMIC	(1<<0)		/* value was dynamically allocated, free if changed/unset */
#define EV_VOLATILE	(1<<1)		/* value is volatile, make a copy of it */
#define EV_NOHOOK	(1<<2)		/* don't call hook when setting */

struct env_var;
typedef char	*(ev_format_t)(struct env_var *ev);
typedef int	(ev_sethook_t)(struct env_var *ev, int flags, void *value);
typedef int	(ev_unsethook_t)(struct env_var *ev);

struct env_var
{
    char		*ev_name;
    int			ev_flags;
    void		*ev_value;
    ev_sethook_t	*ev_sethook;
    ev_unsethook_t	*ev_unsethook;
    struct env_var	*ev_next, *ev_prev;
};
extern struct env_var	*environ;

extern struct env_var	*env_getenv(const char *name);
extern int		env_setenv(const char *name, int flags, void *value, 
				   ev_sethook_t sethook, ev_unsethook_t unsethook);
extern char		*getenv(const char *name);
extern int		setenv(const char *name, char *value, int overwrite);
extern int		putenv(const char *string);
extern int		unsetenv(const char *name);

extern ev_sethook_t	env_noset;		/* refuse set operation */
extern ev_unsethook_t	env_nounset;		/* refuse unset operation */

/* BCD conversions (undocumented) */
extern u_char const	bcd2bin_data[];
extern u_char const	bin2bcd_data[];
extern char const	hex2ascii_data[];

#define	bcd2bin(bcd)	(bcd2bin_data[bcd])
#define	bin2bcd(bin)	(bin2bcd_data[bin])
#define	hex2ascii(hex)	(hex2ascii_data[hex])

/* min/max (undocumented) */
static __inline int imax(int a, int b) { return (a > b ? a : b); }
static __inline int imin(int a, int b) { return (a < b ? a : b); }
static __inline long lmax(long a, long b) { return (a > b ? a : b); }
static __inline long lmin(long a, long b) { return (a < b ? a : b); }
static __inline u_int max(u_int a, u_int b) { return (a > b ? a : b); }
static __inline u_int min(u_int a, u_int b) { return (a < b ? a : b); }
static __inline quad_t qmax(quad_t a, quad_t b) { return (a > b ? a : b); }
static __inline quad_t qmin(quad_t a, quad_t b) { return (a < b ? a : b); }
static __inline u_long ulmax(u_long a, u_long b) { return (a > b ? a : b); }
static __inline u_long ulmin(u_long a, u_long b) { return (a < b ? a : b); }

/* swaps (undocumented, useful?) */
#ifdef __i386__
extern u_int32_t	bswap32(u_int32_t x);
extern u_int64_t	bswap64(u_int32_t x);
#endif

/* null functions for device/filesystem switches (undocumented) */
extern int	nodev(void);
extern int	noioctl(struct open_file *, u_long, void *);
extern void	nullsys(void);

extern int	null_open(char *path, struct open_file *f);
extern int	null_close(struct open_file *f);
extern ssize_t	null_read(struct open_file *f, void *buf, size_t size, size_t *resid);
extern ssize_t	null_write(struct open_file *f, void *buf, size_t size, size_t *resid);
extern off_t	null_seek(struct open_file *f, off_t offset, int where);
extern int	null_stat(struct open_file *f, struct stat *sb);

/* stuff should be in bootstrap (undocumented) */
extern int	getfile(char *prompt, int mode);

/* 
 * Machine dependent functions and data, must be provided or stubbed by 
 * the consumer 
 */
extern int		getchar(void);
extern int		ischar(void);
extern void		putchar(int);
extern int		devopen(struct open_file *, const char *, char **);
extern int		devclose(struct open_file *f);
extern void		panic(const char *, ...) __dead2;
extern struct fs_ops	*file_system[];
extern struct devsw	*devsw[];

