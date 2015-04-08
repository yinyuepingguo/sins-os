#include <sins/kernel.h>
#include <sins/fs.h>
#include <string.h>

#define INODES_PER_BLOCK (MINIX_BLOCK_SIZE/sizeof(struct minix_d_inode))
#define DENTRIES_PER_BLOCK (MINIX_BLOCK_SIZE/sizeof(struct minix_dentry))

result_t minix_read_inode(struct inode *i)
{
	unsigned long blk_nr = 2 + i->sb->sb_minix.imap_blocks
		+ i->sb->sb_minix.zmap_blocks + (i->nr-1)/INODES_PER_BLOCK;
	struct minix_block *block = get_minix_block(i->sb->dev, blk_nr);
	struct minix_d_inode *d_inode =
		(struct minix_d_inode *)block->data + (i->nr-1)%INODES_PER_BLOCK;
#ifdef DEBUG
	unsigned long j;
#endif

	if (block == NULL)
		return -ERROR;

	i->uid = d_inode->uid;
	i->size = d_inode->size;
	i->mtime = i->atime = i->ctime = d_inode->time;
	i->gid = d_inode->gid;
	i->nlinks = d_inode->nlinks;

	if (MINIX_ISDIR(d_inode->mode)) {
		SET_DIR(i->mode);
	}
	else if (MINIX_ISREG(d_inode->mode)) {
		SET_REG(i->mode);
	}
	else if (MINIX_ISCHR(d_inode->mode)) {
		SET_CHR(i->mode);
		i->rdev = MKDEV(d_inode->zone[0]>>8, d_inode->zone[0]&255);
	}
	else if (MINIX_ISBLK(d_inode->mode)) {
		SET_BLK(i->mode);
		i->rdev = MKDEV(d_inode->zone[0]>>8, d_inode->zone[0]&255);
	}
	else {
		WARN("unknown file mode in minix\n");
		put_minix_block(block);
		return -ERROR;
	}
	memcpy(&i->i_minix.zone, &d_inode->zone, sizeof(i->i_minix.zone));
#ifdef DEBUG
	printk("nr: %d, mode:%p, uid:%d, size:%d, "
		" mtime:%d, gid:%d, nlinks:%d\n", i->nr, i->mode, i->uid,
		i->size, i->mtime, i->gid, i->nlinks);
	for (j = 0; j != 9; ++j)
		printk("[%d]:%d ", j, i->i_minix.zone[j]);
#endif
	put_minix_block(block);
	return SUCCESS;
}
result_t minix_write_inode(struct inode *i)
{
	printk("minix_write_inode%d", i->nr);
	return SUCCESS;
}

struct dentry *minix_lookup
	(struct dentry *parent, const char *name, unsigned long len)
{
	struct inode *pinode;
	unsigned long dentries;
	unsigned long i = 0;
	unsigned long block;
	struct minix_dentry *minix_d;
	struct minix_block *minix_block;
	struct inode *inode_find = NULL;
	struct dentry *dentry_ret = NULL;
	char tmp_name_buf[MINIX_NAME_LEN+1];

	pinode = parent->inode;
	dentries = pinode->size / sizeof(struct minix_dentry);
	block = pinode->i_minix.zone[0];
	if (!block)
		return NULL;
	minix_block = get_minix_block(pinode->sb->dev, block);
	if (minix_block == NULL)
		return NULL;
	minix_d = (struct minix_dentry *)minix_block->data;

#ifdef DEBUG
	printk("SEARCH:%s\n", name);
#endif
	while (i < dentries) {
	    if(minix_d->inr)
#ifdef DEBUG
		    printk("LOOKUP:%d/%d[%d]%s\n"
			,i, dentries, minix_d->inr, minix_d->name);
#endif
	    if ((byte *)minix_d >= MINIX_BLOCK_SIZE + minix_block->data) {
		if (minix_block)
			put_minix_block(minix_block);
		minix_block = NULL;
		if (!(block = minix_bmap(pinode, i/DENTRIES_PER_BLOCK))
			|| !(minix_block = 
				get_minix_block(pinode->sb->dev, block))) {
			i += DENTRIES_PER_BLOCK;
			continue;
		}
		minix_d = (struct minix_dentry *)minix_block->data;
	    }

//	printk("%d", strncmp(name, minix_d->name, MINIX_NAME_LEN));
	    strncpy(tmp_name_buf, name, len);
	    tmp_name_buf[len] = 0;
	    /* we need a tmp_name_buf because 'name' is not NULL-terminated*/
	    if (minix_d->inr != 0 && 
		strncmp(tmp_name_buf, minix_d->name, MINIX_NAME_LEN) == 0) {

		inode_find = iget(pinode->sb, minix_d->inr);
		if (inode_find) {
		    dentry_ret = dget(inode_find, parent);
		    if (dentry_ret == NULL) {
		        iput(inode_find);
		    } else {
		        strncpy(dentry_ret->name,
				minix_d->name, MINIX_NAME_LEN);
		        dentry_ret->name_len
				= strnlen(minix_d->name, MINIX_NAME_LEN);
		        dentry_ret->name[dentry_ret->name_len] = 0;
		    } /*dentry_ret == NULL*/
		    goto find;
	    	} else {
		    dentry_ret = NULL;
		    goto find;
		} /*inode find*/
	    } /* strncmp */
	    minix_d++;
	    i++;
	}/* while */
	dentry_ret = NULL;
find:
	if (minix_block)
		put_minix_block(minix_block);
	return dentry_ret;
}
