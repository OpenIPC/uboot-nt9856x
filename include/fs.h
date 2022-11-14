/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2012, NVIDIA CORPORATION.  All rights reserved.
 */
#ifndef _FS_H
#define _FS_H

#include <common.h>

#define FS_TYPE_ANY	0
#define FS_TYPE_FAT	1
#define FS_TYPE_EXT	2
#define FS_TYPE_SANDBOX	3
#define FS_TYPE_UBIFS	4
#define FS_TYPE_BTRFS	5

/*
 * Tell the fs layer which block device an partition to use for future
 * commands. This also internally identifies the filesystem that is present
 * within the partition. The identification process may be limited to a
 * specific filesystem type by passing FS_* in the fstype parameter.
 *
 * Returns 0 on success.
 * Returns non-zero if there is an error accessing the disk or partition, or
 * no known filesystem type could be recognized on it.
 */
int fs_set_blk_dev(const char *ifname, const char *dev_part_str, int fstype);

/*
 * fs_set_blk_dev_with_part - Set current block device + partition
 *
 * Similar to fs_set_blk_dev(), but useful for cases where you already
 * know the blk_desc and part number.
 *
 * Returns 0 on success.
 * Returns non-zero if invalid partition or error accessing the disk.
 */
int fs_set_blk_dev_with_part(struct blk_desc *desc, int part);

/**
 * fs_get_type_name() - Get type of current filesystem
 *
 * Return: Pointer to filesystem name
 *
 * Returns a string describing the current filesystem, or the sentinel
 * "unsupported" for any unrecognised filesystem.
 */
const char *fs_get_type_name(void);

/*
 * Print the list of files on the partition previously set by fs_set_blk_dev(),
 * in directory "dirname".
 *
 * Returns 0 on success. Returns non-zero on error.
 */
int fs_ls(const char *dirname);

/*
 * Determine whether a file exists
 *
 * Returns 1 if the file exists, 0 if it doesn't exist.
 */
int fs_exists(const char *filename);

/*
 * fs_size - Determine a file's size
 *
 * @filename: Name of the file
 * @size: Size of file
 * @return 0 if ok with valid *size, negative on error
 */
int fs_size(const char *filename, loff_t *size);

/*
 * fs_read - Read file from the partition previously set by fs_set_blk_dev()
 * Note that not all filesystem types support either/both offset!=0 or len!=0.
 *
 * @filename: Name of file to read from
 * @addr: The address to read into
 * @offset: The offset in file to read from
 * @len: The number of bytes to read. Maybe 0 to read entire file
 * @actread: Returns the actual number of bytes read
 * @return 0 if ok with valid *actread, -1 on error conditions
 */
int fs_read(const char *filename, ulong addr, loff_t offset, loff_t len,
	    loff_t *actread);

/*
 * fs_write - Write file to the partition previously set by fs_set_blk_dev()
 * Note that not all filesystem types support offset!=0.
 *
 * @filename: Name of file to read from
 * @addr: The address to read into
 * @offset: The offset in file to read from. Maybe 0 to write to start of file
 * @len: The number of bytes to write
 * @actwrite: Returns the actual number of bytes written
 * @return 0 if ok with valid *actwrite, -1 on error conditions
 */
int fs_write(const char *filename, ulong addr, loff_t offset, loff_t len,
	     loff_t *actwrite);

/*
 * Directory entry types, matches the subset of DT_x in posix readdir()
 * which apply to u-boot.
 */
#define FS_DT_DIR  4         /* directory */
#define FS_DT_REG  8         /* regular file */
#define FS_DT_LNK  10        /* symbolic link */

/*
 * A directory entry, returned by fs_readdir().  Returns information
 * about the file/directory at the current directory entry position.
 */
struct fs_dirent {
	unsigned type;       /* one of FS_DT_x (not a mask) */
	loff_t size;         /* size in bytes */
	char name[256];
};

/* Note: fs_dir_stream should be treated as opaque to the user of fs layer */
struct fs_dir_stream {
	/* private to fs. layer: */
	struct blk_desc *desc;
	int part;
};

/*
 * fs_opendir - Open a directory
 *
 * @filename: the path to directory to open
 * @return a pointer to the directory stream or NULL on error and errno
 *    set appropriately
 */
struct fs_dir_stream *fs_opendir(const char *filename);

/*
 * fs_readdir - Read the next directory entry in the directory stream.
 *
 * Works in an analogous way to posix readdir().  The previously returned
 * directory entry is no longer valid after calling fs_readdir() again.
 * After fs_closedir() is called, the returned directory entry is no
 * longer valid.
 *
 * @dirs: the directory stream
 * @return the next directory entry (only valid until next fs_readdir() or
 *    fs_closedir() call, do not attempt to free()) or NULL if the end of
 *    the directory is reached.
 */
struct fs_dirent *fs_readdir(struct fs_dir_stream *dirs);

/*
 * fs_closedir - close a directory stream
 *
 * @dirs: the directory stream
 */
void fs_closedir(struct fs_dir_stream *dirs);

/*
 * fs_unlink - delete a file or directory
 *
 * If a given name is a directory, it will be deleted only if it's empty
 *
 * @filename: Name of file or directory to delete
 * @return 0 on success, -1 on error conditions
 */
int fs_unlink(const char *filename);

/*
 * fs_mkdir - Create a directory
 *
 * @filename: Name of directory to create
 * @return 0 on success, -1 on error conditions
 */
int fs_mkdir(const char *filename);

/*
 * Common implementation for various filesystem commands, optionally limited
 * to a specific filesystem type via the fstype parameter.
 */
int do_size(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[],
		int fstype);
int do_load(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[],
		int fstype);
int do_ls(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[],
		int fstype);
int file_exists(const char *dev_type, const char *dev_part, const char *file,
		int fstype);
int do_save(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[],
		int fstype);
int do_rm(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[],
		int fstype);
int do_mkdir(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[],
		int fstype);
struct fstype_info *fs_get_info(int fstype);
void fs_close(void);

/*
 * Determine the UUID of the specified filesystem and print it. Optionally it is
 * possible to store the UUID directly in env.
 */
int do_fs_uuid(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[],
		int fstype);

/*
 * Determine the type of the specified filesystem and print it. Optionally it is
 * possible to store the type directly in env.
 */
int do_fs_type(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);

struct fstype_info {
	int fstype;
	char *name;
	/*
	 * Is it legal to pass NULL as .probe()'s  fs_dev_desc parameter? This
	 * should be false in most cases. For "virtual" filesystems which
	 * aren't based on a U-Boot block device (e.g. sandbox), this can be
	 * set to true. This should also be true for the dummy entry at the end
	 * of fstypes[], since that is essentially a "virtual" (non-existent)
	 * filesystem.
	 */
	bool null_dev_desc_ok;
	int (*probe)(struct blk_desc *fs_dev_desc,
		     disk_partition_t *fs_partition);
	int (*ls)(const char *dirname);
	int (*exists)(const char *filename);
	int (*size)(const char *filename, loff_t *size);
	int (*read)(const char *filename, void *buf, loff_t offset,
		    loff_t len, loff_t *actread);
	int (*write)(const char *filename, void *buf, loff_t offset,
		     loff_t len, loff_t *actwrite);
	void (*close)(void);
	int (*uuid)(char *uuid_str);
	/*
	 * Open a directory stream.  On success return 0 and directory
	 * stream pointer via 'dirsp'.  On error, return -errno.  See
	 * fs_opendir().
	 */
	int (*opendir)(const char *filename, struct fs_dir_stream **dirsp);
	/*
	 * Read next entry from directory stream.  On success return 0
	 * and directory entry pointer via 'dentp'.  On error return
	 * -errno.  See fs_readdir().
	 */
	int (*readdir)(struct fs_dir_stream *dirs, struct fs_dirent **dentp);
	/* see fs_closedir() */
	void (*closedir)(struct fs_dir_stream *dirs);
	int (*unlink)(const char *filename);
	int (*mkdir)(const char *dirname);
};

#endif /* _FS_H */
