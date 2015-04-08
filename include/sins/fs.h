#ifndef _SINS_FS_H
#define _SINS_FS_H

#include <list.h>
#include <sins/error.h>
#include <types.h>
#include <sins/kernel.h>
#include <sins/atomic.h>
#include <sins/mm.h>

typedef unsigned long dev_t;

#define MAJOR(dev)	((dev)>>20)
#define MINOR(dev)	((dev)&0x000FFFFFUL)
#define MKDEV(major, minor)	(((major)<<20) + ((minor)&0x000FFFFFUL))

#define MAX_NAME_LEN 20

#define NR_CHRDEVS 32
#define NR_BLKDEVS 32
#define NR_OPEN	20

#define FT_MASK	0x0F000
#define FT_CHR 0x01000
#define FT_BLK 0x02000
#define FT_REG 0x03000
#define FT_DIR 0x04000

#define IS_CHR(mode)	(((mode)&FT_MASK) == FT_CHR)
#define IS_BLK(mode)	(((mode)&FT_MASK) == FT_BLK)
#define IS_REG(mode)	(((mode)&FT_MASK) == FT_REG)
#define IS_DIR(mode)	(((mode)&FT_MASK) == FT_DIR)

#define SET_CHR(mode)	((mode) |= (FT_MASK&FT_CHR))	
#define SET_BLK(mode)	((mode) |= (FT_MASK&FT_BLK))
#define SET_REG(mode)	((mode) |= (FT_MASK&FT_REG))
#define SET_DIR(mode)	((mode) |= (FT_MASK&FT_DIR))

#define O_RDONLY	0x00
#define O_WRONLY	0x01
#define O_RDWR		0x02
#define O_CREAT		0x04
#define O_TRUNC		0x08
#define O_APPEND	0x10
#define O_MASK		(O_RDONLY | O_WRONLY | O_RDWR | O_CREAT | O_TRUNC \
	| O_APPEND)	

#define BLK_READ	0
#define BLK_WRITE	1

#define BLOCK_SIZE PAGE_SIZE

/* block device number */
#define DEV_HD	1

/* char device number */
#define DEV_TTY	1
#define DEV_KBD	2
#define DEV_PROC	8


struct dentry;
struct file;
struct inode;
struct file_system_type;
struct dir;

/* include specific fs headers hear*/
#include <fs/minix.h>

struct file_operations {
	result_t (*open)(struct inode *inode, struct file *flip);
	ssize_t (*read)(struct file *, byte __user *, size_t, loff_t *);
	ssize_t (*readdir)(struct file *, struct dir __user *,
		size_t, loff_t *);
	ssize_t (*write)(struct file *, const byte __user *,
		size_t, loff_t *);	
	result_t (*ioctl)(struct file *, unsigned long, unsigned long);
	loff_t (*lseek)(struct file *, loff_t offset, int whence);
	result_t (*close)(struct inode *inode, struct file *flip);	
};

struct super_operations {
	struct dentry *(*lookup)
		(struct dentry *parent, const char *name, unsigned long len);
	/* need fill mode and size(rdev) and file system specific */
	result_t (*read_inode)(struct inode *i);
	result_t (*write_inode)(struct inode *i); 
};

struct block {
	unsigned long block_nr;
	dev_t dev;
	byte *data;
	unsigned char dirty;
	unsigned char uptodate;
};

struct super_block {
	struct list_head sb_list;
	struct file_system_type *fs_type;
	unsigned long magic;
	dev_t dev;
	/* which dentry i mount */
	struct dentry *parent;
	/* my root dentry */
	struct dentry *root;
	struct list_head inodes;
	struct list_head dentries;

	union {
		struct minix_super_block sb_minix;
	};
};

struct file_system_type {
	const char *name;
	result_t (*get_super)(struct super_block *sb);
	void (*kill_super)(struct super_block *sb);
	struct super_operations *sops;
	struct file_operations *fops;
	struct list_head list;
	struct list_head sb_list;
};


struct inode {
	unsigned long nr;

	/* basic inode info,filled by real fs */
	unsigned long mode;
	unsigned long uid;
	union {
		loff_t size;
		/* record device. used for special devices */
		dev_t rdev;
	};
	unsigned long mtime;
	unsigned long atime;
	unsigned long ctime;
	unsigned long gid;
	unsigned long nlinks;

	/* initilize by vfs */
	struct file_operations *fops;

	struct super_block *sb;
	void *private_data;
	unsigned char dirty;
	
	/* reference count */
	atomic_t ref_count;

	struct list_head use_list;
	struct list_head hash_list;
	
	/* specific */
	union {
		struct minix_inode i_minix;
	};
};

struct dentry {
	char name[MAX_NAME_LEN];
	unsigned char name_len;

	struct dentry *parent;
	struct super_block *sb;
	/* inode which dentry belongs to */
	struct inode * inode;
	/* node in my parent's list */
	struct list_head child;
	struct list_head children;

	struct super_block *mount;
	/* reference count */
	atomic_t ref_count;
	struct list_head use_list;
	struct list_head hash_list;
};

struct file {
	unsigned long flags;
	loff_t pos;
	struct inode *inode;
	void *private_data;
	atomic_t ref_count;
};

struct dir {
	char name[MAX_NAME_LEN];
};


extern result_t register_chrdev(unsigned int major, const char *name,
	struct file_operations *fops);
extern result_t unregister_chrdev(unsigned int major, const char *name);

/* block size must be power of 2, between 512 and 4K*/
extern result_t register_blkdev(unsigned int major, const char *name,
	void (*rw_handler)(int rw, struct block *blk),
		unsigned long block_size);
extern result_t unregister_blkdev(unsigned int major, const char *name);

extern result_t register_filesystem(struct file_system_type *fst);
extern result_t unregister_filesystem(const char *name);

extern result_t do_mount(dev_t dev, struct dentry *de);
extern void mount_root(dev_t dev);
extern struct super_block *alloc_super_block(dev_t dev, struct dentry *de);
extern void dealloc_super_block(struct super_block *sb);


extern result_t open_namei(const char *__user filename, int flags,
	struct inode **inode);

extern struct inode *iget(struct super_block *sb, unsigned long i_nr);
extern void iref(struct inode *i);
extern void iput(struct inode *i);
extern struct dentry *dget(struct inode *i, struct dentry *parent);
extern void dref(struct dentry *de);
extern void dput(struct dentry *de);

extern struct block *bread(dev_t dev, unsigned long block_nr);
extern void brelse(struct block *blk);

extern struct file_operations def_file_fops;
extern struct file_operations def_chr_fops;
extern struct file_operations def_blk_fops;

#endif
