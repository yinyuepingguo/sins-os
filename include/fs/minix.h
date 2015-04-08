#ifndef _FS_MINIX_H
#define _FS_MINIX_H

#include <types.h>
#include <sins/fs.h>

struct super_block;

#define MINIX_SUPER_MAGIC 0x137F

#define MINIX_I_MAP_SLOTS	8
#define MINIX_Z_MAP_SLOTS	8

#define MINIX_BLOCK_SIZE 1024

#define MINIX_ROOT_INR	1

#define MINIX_NAME_LEN 14

struct minix_block {
	byte *data;
	struct block *blk;
	unsigned char dirty;
};

struct minix_super_block {
	u16 ninodes;
	u16 nzones;
	u16 imap_blocks;
	u16 zmap_blocks;
	u16 firstdatazone;
	u16 log_zone_size;
	u32 max_size;
	u16 magic;
	struct minix_block *imap[MINIX_I_MAP_SLOTS];
	struct minix_block *zmap[MINIX_Z_MAP_SLOTS];
};

struct minix_inode {
	unsigned short zone[9];
};

struct minix_d_inode {
	unsigned short mode;
	unsigned short uid;
	unsigned long size;
	unsigned long time;
	unsigned char gid;
	unsigned char nlinks;
	unsigned short zone[9];
};

struct minix_dentry {
	unsigned short inr;
	char name[MINIX_NAME_LEN];
};

#define MINIX_IFMT 00170000

#define MINIX_ISREG(mode)	(((mode)& MINIX_IFMT) == 00100000)
#define MINIX_ISDIR(mode)	(((mode)& MINIX_IFMT) == 00040000)
#define MINIX_ISCHR(mode)	(((mode)& MINIX_IFMT) == 00020000)
#define MINIX_ISBLK(mode)	(((mode)& MINIX_IFMT) == 00060000)

extern struct minix_block *
	get_minix_block(dev_t dev, unsigned long minix_blocks);
extern void put_minix_block(struct minix_block *data);

extern result_t minix_get_super(struct super_block *sb);
extern void minix_kill_super(struct super_block *sb);

extern result_t minix_read_inode(struct inode *i);
extern result_t minix_write_inode(struct inode *i);
extern struct dentry *minix_lookup
	(struct dentry *parent, const char *name, unsigned long len);

extern unsigned long
	minix_bmap(struct inode * inode, unsigned long block);
extern unsigned long
	minix_create_block(struct inode * inode, unsigned long block);

extern ssize_t minix_file_read
	(struct file *, byte __user *, size_t, loff_t *);
extern ssize_t minix_file_write
	(struct file *, const byte __user *, size_t, loff_t *);	
extern ssize_t minix_file_readdir
	(struct file *, struct dir __user *, size_t, loff_t *);	


#endif
