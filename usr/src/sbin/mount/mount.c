static char *sccsid = "@(#)mount.c	4.1 (Berkeley) %G%";
#include <stdio.h>
#include <fstab.h>

/*
 *	Mount file systems.
 *
 *	mount -a	Mount all file systems, as determined from the
 *			file /etc/fstab.
 *	If the name entry in /etc/fstab is "/", don't mount.
 *	If the read only entry in /etc/fstab is "ro", mount read only
 *	The special file names in /etc/fstab are the block devices;
 *		this is what we want to mount.
 *	Tries to mount all of the files in /etc/fstab.
 *	
 *	mount special name	Mount special on name
 *	mount special name -r	Mount special on name, read/write
 */

int	mountall;
#define	NMOUNT	16
#define	NAMSIZ	32

struct mtab {
	char	file[NAMSIZ];
	char	spec[NAMSIZ];
} mtab[NMOUNT];

main(argc, argv)
char **argv;
{
	register int ro;
	register struct mtab *mp;
	register char *np;
	int mf;

	mountall = 0;
	mf = open("/etc/mtab", 0);
	read(mf, (char *)mtab, NMOUNT*2*NAMSIZ);
	if (argc==1) {
		for (mp = mtab; mp < &mtab[NMOUNT]; mp++)
			if (mp->file[0])
				printf("%s on %s\n", mp->spec, mp->file);
		exit(0);
	}

	if (argc == 2){
		if (strcmp(argv[1], "-a") == 0)
			mountall++;
		else {
			fprintf(stderr,"arg count\n");
			exit(1);
		}
	}

	if (!mountall){
		ro = 0;
		if(argc > 3)
			ro++;
		if (mountfs(argv[1], argv[2], ro))
			exit(1);
	} else {
		FILE	*fs_file;
		struct	fstab	fs;
		if ( (fs_file = fopen(FSTAB, "r")) == NULL){
			perror(FSTAB);
			exit(1);
		}
		while (!feof(fs_file)){
			fscanf(fs_file, FSTABFMT, FSTABARG(&fs));
			if (strcmp(fs.fs_file, "/") == 0)
				continue;
			fprintf(stderr, "Mounting %s on %s %s",
				fs.fs_file, fs.fs_spec,
				FSRO(&fs) ? "(Read Only)\n" : "\n");
			mountfs(fs.fs_spec, fs.fs_file, FSRO(&fs));
		}
		fclose(fs_file);
	}
	exit(0);
}

mountfs(spec, name, ro)
	char	*spec, *name;
	int	ro;
{
	register	char	*np;
	register	struct	mtab	*mp;
	int	mf;

	if(mount(spec, name, ro) < 0) {
		perror("mount");
		return(1);
	}
	np = spec;
	while(*np++)
		;
	np--;
	while(*--np == '/')
		*np = '\0';
	while(np > spec && *--np != '/')
		;
	if(*np == '/')
		np++;
	spec = np;
	for (mp = mtab; mp < &mtab[NMOUNT]; mp++) {
		if (mp->file[0] == 0) {
			for (np = mp->spec; np < &mp->spec[NAMSIZ-1];)
				if ((*np++ = *spec++) == 0)
					spec--;
			for (np = mp->file; np < &mp->file[NAMSIZ-1];)
				if ((*np++ = *name++) == 0)
					name--;
			mp = &mtab[NMOUNT];
			while ((--mp)->file[0] == 0);
			mf = creat("/etc/mtab", 0644);
			write(mf, (char *)mtab, (mp-mtab+1)*2*NAMSIZ);
		}
	}
	return(0);
}
