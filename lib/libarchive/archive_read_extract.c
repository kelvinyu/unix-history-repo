/*-
 * Copyright (c) 2003-2004 Tim Kientzle
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer
 *    in this position and unchanged.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/stat.h>
#include <sys/acl.h>
#include <sys/time.h>
#include <sys/types.h>

#ifdef DMALLOC
#include <dmalloc.h>
#endif
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <limits.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tar.h>
#include <unistd.h>

#include "archive.h"
#include "archive_string.h"
#include "archive_entry.h"
#include "archive_private.h"

static void	archive_extract_cleanup(struct archive *);
static int	archive_read_extract_block_device(struct archive *,
		    struct archive_entry *, int);
static int	archive_read_extract_char_device(struct archive *,
		    struct archive_entry *, int);
static int	archive_read_extract_device(struct archive *,
		    struct archive_entry *, int flags, mode_t mode);
static int	archive_read_extract_dir(struct archive *,
		    struct archive_entry *, int);
static int	archive_read_extract_dir_create(struct archive *,
		    const char *name, int mode, int flags);
static int	archive_read_extract_fifo(struct archive *,
		    struct archive_entry *, int);
static int	archive_read_extract_hard_link(struct archive *,
		    struct archive_entry *, int);
static int	archive_read_extract_regular(struct archive *,
		    struct archive_entry *, int);
static int	archive_read_extract_regular_open(struct archive *,
		    const char *name, int mode, int flags);
static int	archive_read_extract_symbolic_link(struct archive *,
		    struct archive_entry *, int);
static int	mkdirpath(struct archive *, const char *);
static int	mkdirpath_recursive(char *path);
static int	mksubdir(char *path);
static int	set_acls(struct archive *, struct archive_entry *);
static int	set_extended_perm(struct archive *, struct archive_entry *,
		    int flags);
static int	set_fflags(struct archive *, struct archive_entry *);
static int	set_ownership(struct archive *, struct archive_entry *, int);
static int	set_perm(struct archive *, struct archive_entry *, int mode, int flags);
static int	set_time(struct archive *, struct archive_entry *, int);

/*
 * Extract this entry to disk.
 *
 * TODO: Validate hardlinks.  Is there any way to validate hardlinks
 * without keeping a complete list of filenames from the entire archive?? Ugh.
 *
 */
int
archive_read_extract(struct archive *a, struct archive_entry *entry, int flags)
{
	mode_t writable_mode;
	struct archive_extract_dir_entry *le;
	int ret;
	int restore_pwd;

	restore_pwd = -1;
	if (S_ISDIR(archive_entry_stat(entry)->st_mode)) {
		/*
		 * TODO: Does this really work under all conditions?
		 *
		 * E.g., root restores a dir owned by someone else?
		 */
		writable_mode = archive_entry_stat(entry)->st_mode | 0700;

		/*
		 * If this dir isn't writable, restore it with write
		 * permissions and add it to the fixup list for later
		 * handling.
		 */
		if (archive_entry_stat(entry)->st_mode != writable_mode) {
			le = malloc(sizeof(struct archive_extract_dir_entry));
			le->next = a->archive_extract_dir_list;
			a->archive_extract_dir_list = le;
			le->mode = archive_entry_stat(entry)->st_mode;
			le->name =
			    malloc(strlen(archive_entry_pathname(entry)) + 1);
			strcpy(le->name, archive_entry_pathname(entry));
			a->cleanup_archive_extract = archive_extract_cleanup;
			/* Make sure I can write to this directory. */
			archive_entry_set_mode(entry, writable_mode);
		}
	}

	if (archive_entry_hardlink(entry) != NULL)
		return (archive_read_extract_hard_link(a, entry, flags));

	/*
	 * TODO: If pathname is longer than PATH_MAX, record starting
	 * directory and move to a suitable intermediate dir, which
	 * might require creating them!
	 */
	if (strlen(archive_entry_pathname(entry)) > PATH_MAX) {
		restore_pwd = open(".", O_RDONLY);
		/* XXX chdir() to a suitable intermediate dir XXX */
		/* XXX Update pathname in 'entry' XXX */
	}

	switch (archive_entry_stat(entry)->st_mode & S_IFMT) {
	default:
		/* Fall through, as required by POSIX. */
	case S_IFREG:
		ret =  archive_read_extract_regular(a, entry, flags);
		break;
	case S_IFLNK:	/* Symlink */
		ret =  archive_read_extract_symbolic_link(a, entry, flags);
		break;
	case S_IFCHR:
		ret =  archive_read_extract_char_device(a, entry, flags);
		break;
	case S_IFBLK:
		ret =  archive_read_extract_block_device(a, entry, flags);
		break;
	case S_IFDIR:
		ret =  archive_read_extract_dir(a, entry, flags);
		break;
	case S_IFIFO:
		ret =  archive_read_extract_fifo(a, entry, flags);
		break;
	}

	/* If we changed directory above, restore it here. */
	if (restore_pwd >= 0)
		fchdir(restore_pwd);

	return (ret);
}

/*
 * Cleanup function for archive_extract.  Free name/mode list and
 * restore permissions.
 *
 * TODO: Restore times here as well.
 *
 * Registering this function (rather than calling it explicitly by
 * name from archive_read_finish) reduces link pollution, since
 * applications that don't use this API won't get this file linked in.
 */
static
void archive_extract_cleanup(struct archive *a)
{
	struct archive_extract_dir_entry *lp;

	/*
	 * TODO: Does dir list need to be sorted so permissions are restored
	 * depth-first?
	 */
	while (a->archive_extract_dir_list) {
		lp = a->archive_extract_dir_list->next;
		chmod(a->archive_extract_dir_list->name,
		    a->archive_extract_dir_list->mode);
		/*
		 * TODO: Consider using this hook to restore dir
		 * timestamps as well.  However, dir timestamps don't
		 * really matter, and it would be a memory issue to
		 * record timestamps for every directory
		 * extracted... Ugh.
		 */
		if (a->archive_extract_dir_list->name)
			free(a->archive_extract_dir_list->name);
		free(a->archive_extract_dir_list);
		a->archive_extract_dir_list = lp;
	}
}

static int
archive_read_extract_regular(struct archive *a, struct archive_entry *entry,
    int flags)
{
	int fd, r;
	ssize_t s;

	r = ARCHIVE_OK;
	fd = archive_read_extract_regular_open(a,
	    archive_entry_pathname(entry), archive_entry_stat(entry)->st_mode,
	    flags);
	if (fd < 0) {
		archive_set_error(a, errno, "Can't open");
		return (ARCHIVE_WARN);
	}
	s = archive_read_data_into_fd(a, fd);
	if (s < archive_entry_size(entry)) {
		/* Didn't read enough data?  Complain but keep going. */
		archive_set_error(a, EIO, "Archive data truncated");
		r = ARCHIVE_WARN;
	}
	set_ownership(a, entry, flags);
	set_time(a, entry, flags);
	/* set_perm(a, entry, mode, flags); */ /* Handled implicitly by open.*/
	set_extended_perm(a, entry, flags);
	close(fd);
	return (r);
}

/*
 * Keep trying until we either open the file or run out of tricks.
 *
 * Note: the GNU tar 'unlink first' option seems redundant
 * with this strategy, since we never actually write over an
 * existing file.  (If it already exists, we remove it.)
 */
static int
archive_read_extract_regular_open(struct archive *a,
    const char *name, int mode, int flags)
{
	int fd;

	fd = open(name, O_WRONLY | O_CREAT | O_EXCL, mode);
	if (fd >= 0)
		return (fd);

	/* Try removing a pre-existing file. */
	if (!(flags & ARCHIVE_EXTRACT_NO_OVERWRITE)) {
		unlink(name);
		fd = open(name, O_WRONLY | O_CREAT | O_EXCL, mode);
		if (fd >= 0)
			return (fd);
	}

	/* Might be a non-existent parent dir; try fixing that. */
	mkdirpath(a, name);
	fd = open(name, O_WRONLY | O_CREAT | O_EXCL, mode);
	if (fd >= 0)
		return (fd);

	return (-1);
}

static int
archive_read_extract_dir(struct archive *a, struct archive_entry *entry,
    int flags)
{
	int mode, ret, ret2;

	mode = archive_entry_stat(entry)->st_mode;

	if (archive_read_extract_dir_create(a, archive_entry_pathname(entry),
		mode, flags)) {
		/* Unable to create directory; just use the existing dir. */
		return (ARCHIVE_WARN);
	}

	set_ownership(a, entry, flags);
	/*
	 * There is no point in setting the time here.
	 *
	 * Note that future extracts into this directory will reset
	 * the times, so to get correct results, the client has to
	 * track timestamps for directories and update them at the end
	 * of the run anyway.
	 */
	/* set_time(t, flags); */

	/*
	 * This next line may appear redundant, but it's not.  If the
	 * directory already exists, it won't get re-created by
	 * mkdir(), so we have to manually set permissions to get
	 * everything right.
	 */
	ret = set_perm(a, entry, mode, flags);
	ret2 = set_extended_perm(a, entry, flags);

	/* XXXX TODO: Fix this to work the right way. XXXX */
	if (ret == ARCHIVE_OK)
		return (ret2);
	else
		return (ret);
}

/*
 * Create the directory: try until something works or we run out of magic.
 */
static int
archive_read_extract_dir_create(struct archive *a, const char *name, int mode,
    int flags)
{
	/* Don't try to create '.' */
	if (name[0] == '.' && name[1] == 0)
		return (ARCHIVE_OK);
	if (mkdir(name, mode) == 0)
		return (ARCHIVE_OK);
	if (errno == ENOENT) {	/* Missing parent directory. */
		mkdirpath(a, name);
		if (mkdir(name, mode) == 0)
			return (ARCHIVE_OK);
	}

	if (errno != EEXIST)
		return (ARCHIVE_WARN);
	if ((flags & ARCHIVE_EXTRACT_NO_OVERWRITE)) {
		archive_set_error(a, EEXIST, "Directory already exists");
		return (ARCHIVE_WARN);
	}

	/* Could be a file; try unlinking. */
	if (unlink(name) == 0 &&
	    mkdir(name, mode) == 0)
		return (ARCHIVE_OK);

	/* Unlink failed. It's okay if it failed because it's already a dir. */
	if (errno != EPERM) {
		archive_set_error(a, errno, "Couldn't create dir");
		return (ARCHIVE_WARN);
	}

	/* Try removing the directory and recreating it from scratch. */
	if (rmdir(name)) {
		/* Failure to remove a non-empty directory is not a problem. */
		if (errno == ENOTEMPTY)
			return (ARCHIVE_OK);
		/* Any other failure is a problem. */
		archive_set_error(a, errno,
		    "Error attempting to remove existing directory");
		return (ARCHIVE_WARN);
	}

	/* We successfully removed the directory; now recreate it. */
	if (mkdir(name, mode) == 0)
		return (ARCHIVE_OK);

	archive_set_error(a, errno, "Failed to create dir");
	return (ARCHIVE_WARN);
}

static int
archive_read_extract_hard_link(struct archive *a, struct archive_entry *entry,
    int flags)
{
	/* Just remove any pre-existing file with this name. */
	if (!(flags & ARCHIVE_EXTRACT_NO_OVERWRITE))
		unlink(archive_entry_pathname(entry));

	if (link(archive_entry_hardlink(entry),
	    archive_entry_pathname(entry))) {
		archive_set_error(a, errno, "Can't restore hardlink");
		return (ARCHIVE_WARN);
	}

	/* Set ownership, time, permission information. */
	set_ownership(a, entry, flags);
	set_time(a, entry, flags);
	set_perm(a, entry, archive_entry_stat(entry)->st_mode, flags);
	set_extended_perm(a, entry, flags);

	return (ARCHIVE_OK);
}

static int
archive_read_extract_symbolic_link(struct archive *a,
    struct archive_entry *entry, int flags)
{
	/* Just remove any pre-existing file with this name. */
	if (!(flags & ARCHIVE_EXTRACT_NO_OVERWRITE))
		unlink(archive_entry_pathname(entry));

	if (symlink(archive_entry_symlink(entry),
		archive_entry_pathname(entry))) {
		/* XXX Better error message here XXX */
		archive_set_error(a, errno, "Can't restore symlink to '%s'",
		    archive_entry_symlink(entry));
		return (ARCHIVE_WARN);
	}

	/* Set ownership, time, permission information. */
	set_ownership(a, entry, flags);
	set_time(a, entry, flags);
	set_perm(a, entry, archive_entry_stat(entry)->st_mode, flags);
	set_extended_perm(a, entry, flags);

	return (ARCHIVE_OK);
}

static int
archive_read_extract_device(struct archive *a, struct archive_entry *entry,
    int flags, mode_t mode)
{
	int r;

	/* Just remove any pre-existing file with this name. */
	if (!(flags & ARCHIVE_EXTRACT_NO_OVERWRITE))
		unlink(archive_entry_pathname(entry));

	r = mknod(archive_entry_pathname(entry), mode,
	    archive_entry_stat(entry)->st_rdev);

	/* Might be a non-existent parent dir; try fixing that. */
	if (r != 0 && errno == ENOENT) {
		mkdirpath(a, archive_entry_pathname(entry));
		r = mknod(archive_entry_pathname(entry), mode,
		    archive_entry_stat(entry)->st_rdev);
	}

	if (r != 0) {
		archive_set_error(a, errno, "Can't recreate device node");
		return (ARCHIVE_WARN);
	}

	/* Set ownership, time, permission information. */
	set_ownership(a, entry, flags);
	set_time(a, entry, flags);
	set_perm(a, entry, archive_entry_stat(entry)->st_mode, flags);
	set_extended_perm(a, entry, flags);

	return (ARCHIVE_OK);
}

static int
archive_read_extract_char_device(struct archive *a,
    struct archive_entry *entry, int flags)
{
	mode_t mode;

	mode = (archive_entry_stat(entry)->st_mode & ~S_IFMT) | S_IFCHR;
	return (archive_read_extract_device(a, entry, flags, mode));
}

static int
archive_read_extract_block_device(struct archive *a,
    struct archive_entry *entry, int flags)
{
	mode_t mode;

	mode = (archive_entry_stat(entry)->st_mode & ~S_IFMT) | S_IFBLK;
	return (archive_read_extract_device(a, entry, flags, mode));
}

static int
archive_read_extract_fifo(struct archive *a,
    struct archive_entry *entry, int flags)
{
	int r;

	/* Just remove any pre-existing file with this name. */
	if (!(flags & ARCHIVE_EXTRACT_NO_OVERWRITE))
		unlink(archive_entry_pathname(entry));

	r = mkfifo(archive_entry_pathname(entry),
	    archive_entry_stat(entry)->st_mode);

	/* Might be a non-existent parent dir; try fixing that. */
	if (r != 0 && errno == ENOENT) {
		mkdirpath(a, archive_entry_pathname(entry));
		r = mkfifo(archive_entry_pathname(entry),
		    archive_entry_stat(entry)->st_mode);
	}

	if (r != 0) {
		archive_set_error(a, errno, "Can't restore fifo");
		return (ARCHIVE_WARN);
	}

	/* Set ownership, time, permission information. */
	set_ownership(a, entry, flags);
	set_time(a, entry, flags);
	/* Done by mkfifo. */
	/* set_perm(a, entry, archive_entry_stat(entry)->st_mode, flags); */
	set_extended_perm(a, entry, flags);

	return (ARCHIVE_OK);
}

/*
 * Returns 0 if it successfully created necessary directories.
 * Otherwise, returns ARCHIVE_WARN.
 */

static int
mkdirpath(struct archive *a, const char *path)
{
	char *p;

	/* Copy path to mutable storage, then call mkdirpath_recursive. */
	archive_strcpy(&(a->extract_mkdirpath), path);
	/* Prune a trailing '/' character. */
	p = a->extract_mkdirpath.s;
	if (p[strlen(p)-1] == '/')
		p[strlen(p)-1] = 0;
	/* Recursively try to build the path. */
	return (mkdirpath_recursive(p));
}

/*
 * For efficiency, just try creating longest path first (usually,
 * archives walk through directories in a reasonable order).  If that
 * fails, prune the last element and recursively try again.
 */
static int
mkdirpath_recursive(char *path)
{
	char * p;
	int r;

	p = strrchr(path, '/');
	if (!p) return (0);

	*p = 0;			/* Terminate path name. */
	r = mksubdir(path);	/* Try building path. */
	*p = '/';		/* Restore the '/' we just overwrote. */
	return (r);
}

static int
mksubdir(char *path)
{
	int mode = 0755;

	if (mkdir(path, mode) == 0) return (0);

	if (errno == EEXIST) /* TODO: stat() here to verify it is dir */
		return (0);
	if (mkdirpath_recursive(path))
		return (ARCHIVE_WARN);
	if (mkdir(path, mode) == 0)
		return (0);
	return (ARCHIVE_WARN); /* Still failed.  Harumph. */
}

/*
 * Note that I only inspect entry->ae_uid and entry->ae_gid here; if
 * the client wants POSIX compat, they'll need to do uname/gname
 * lookups themselves.  I don't do it here because of the potential
 * performance issues: if uname/gname lookup is expensive, then the
 * results should be aggressively cached; if they're cheap, then we
 * shouldn't waste memory on cache tables.
 *
 * Returns 0 if UID/GID successfully restored; ARCHIVE_WARN otherwise.
 */
static int
set_ownership(struct archive *a, struct archive_entry *entry, int flags)
{
	/* If UID/GID are already correct, return 0. */
	/* TODO: Fix this; need to stat() to find on-disk GID <sigh> */
	if (a->user_uid == archive_entry_stat(entry)->st_uid)
		return (0);

	/* Not changed. */
	if ((flags & ARCHIVE_EXTRACT_OWNER) == 0)
		return (ARCHIVE_WARN);

	/*
	 * Root can change owner/group; owner can change group;
	 * otherwise, bail out now.
	 */
	if ((a->user_uid != 0)
	    && (a->user_uid != archive_entry_stat(entry)->st_uid))
		return (ARCHIVE_WARN);

	if (lchown(archive_entry_pathname(entry),
		archive_entry_stat(entry)->st_uid,
		archive_entry_stat(entry)->st_gid)) {
		archive_set_error(a, errno,
		    "Can't set user=%d/group=%d for %s",
		    archive_entry_stat(entry)->st_uid,
		    archive_entry_stat(entry)->st_gid,
		    archive_entry_pathname(entry));
		return (ARCHIVE_WARN);
	}
	return (ARCHIVE_OK);
}

static int
set_time(struct archive *a, struct archive_entry *entry, int flags)
{
	const struct stat *st;
	struct timeval times[2];

	(void)a; /* UNUSED */
	st = archive_entry_stat(entry);

	if ((flags & ARCHIVE_EXTRACT_TIME) == 0)
		return (ARCHIVE_OK);

	times[1].tv_sec = st->st_mtime;
	times[1].tv_usec = st->st_mtimespec.tv_nsec / 1000;

	times[0].tv_sec = st->st_atime;
	times[0].tv_usec = st->st_atimespec.tv_nsec / 1000;

	if (lutimes(archive_entry_pathname(entry), times) != 0) {
		archive_set_error(a, errno, "Can't update time for %s",
		    archive_entry_pathname(entry));
		return (ARCHIVE_WARN);
	}

	/*
	 * Note: POSIX does not provide a portable way to restore ctime.
	 * So, any restoration of ctime will necessarily be OS-specific.
	 */

	/* TODO: Can FreeBSD restore ctime? */

	return (ARCHIVE_OK);
}

static int
set_perm(struct archive *a, struct archive_entry *entry, int mode, int flags)
{
	const char *name;

	if ((flags & ARCHIVE_EXTRACT_PERM) == 0)
		return (ARCHIVE_OK);

	name = archive_entry_pathname(entry);
	if (lchmod(name, mode) != 0) {
		archive_set_error(a, errno, "Can't set permissions");
		return (ARCHIVE_WARN);
	}
	return (0);
}

static int
set_extended_perm(struct archive *a, struct archive_entry *entry, int flags)
{
	int		 ret;

	if ((flags & ARCHIVE_EXTRACT_PERM) == 0)
		return (ARCHIVE_OK);

	ret = set_fflags(a, entry);
	if (ret == ARCHIVE_OK)
		ret = set_acls(a, entry);
	return (ret);
}

static int
set_fflags(struct archive *a, struct archive_entry *entry)
{
	char		*fflags;
	const char	*fflagsc;
	char		*fflags_p;
	const char	*name;
	int		 ret;
	unsigned long	 set, clear;
	struct stat	 st;

	name = archive_entry_pathname(entry);

	ret = ARCHIVE_OK;
	fflagsc = archive_entry_fflags(entry);
	if (fflagsc == NULL)
		return (ARCHIVE_OK);

	fflags = strdup(fflagsc);
	if (fflags == NULL)
		return (ARCHIVE_WARN);

	fflags_p = fflags;
	if (strtofflags(&fflags_p, &set, &clear) != 0  &&
	    stat(name, &st) == 0) {
		st.st_flags &= ~clear;
		st.st_flags |= set;
		if (chflags(name, st.st_flags) != 0) {
			archive_set_error(a, errno,
			    "Failed to set file flags");
			ret = ARCHIVE_WARN;
		}
	}
	free(fflags);
	return (ret);
}

/*
 * XXX TODO: What about ACL types other than ACCESS and DEFAULT?
 */
static int
set_acls(struct archive *a, struct archive_entry *entry)
{
	const char	*acldesc;
	acl_t		 acl;
	const char	*name;
	int		 ret;

	ret = ARCHIVE_OK;
	name = archive_entry_pathname(entry);
	acldesc = archive_entry_acl(entry);
	if (acldesc != NULL) {
		acl = acl_from_text(acldesc);
		if (acl == NULL) {
			archive_set_error(a, errno, "Error parsing acl '%s'",
			    acldesc);
			ret = ARCHIVE_WARN;
		} else {
			if (acl_set_file(name, ACL_TYPE_ACCESS, acl) != 0) {
				archive_set_error(a, errno,
				    "Failed to set acl");
				ret = ARCHIVE_WARN;
			}
			acl_free(acl);
		}
	}

	acldesc = archive_entry_acl_default(entry);
	if (acldesc != NULL) {
		acl = acl_from_text(acldesc);
		if (acl == NULL) {
			archive_set_error(a, errno, "error parsing acl '%s'",
			    acldesc);
			ret = ARCHIVE_WARN;
		} else {
			if (acl_set_file(name, ACL_TYPE_DEFAULT, acl) != 0) {
				archive_set_error(a, errno,
				    "Failed to set acl");
				ret = ARCHIVE_WARN;
			}
			acl_free(acl);
		}
	}

	return (ret);
}
