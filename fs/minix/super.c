#include <sins/kernel.h>
#include <sins/fs.h>
#include <string.h>

result_t minix_get_super(struct super_block *sb)
{
	dev_t dev;
	struct minix_super_block *minix_sb;
	result_t ret = SUCCESS;
	long i;
	struct minix_block *blk;
	unsigned long block;
	unsigned long free;
	struct inode *root_inode;

	dev = sb->dev;
	if (!(blk = get_minix_block(dev, 1)))
		return -ERROR;

	minix_sb = (struct minix_super_block *)(blk->data);
	if (minix_sb->magic != MINIX_SUPER_MAGIC) {
		ret = -ERROR;
		goto release_blk;
	}
	memcpy(&sb->sb_minix, minix_sb, sizeof(struct minix_super_block));
	minix_sb = &sb->sb_minix;
#ifdef DEBUG
	printk("MINIX: magic:%p, %d ninodes, %d zones.\n"
		"MINIX: firstdatazone %d, %d imap_blocks, %d zmap_blocks\n"
		"MINIX: log_zone_size: %d, max_size: %d\n",
			minix_sb->magic, minix_sb->ninodes,
			minix_sb->nzones, minix_sb->firstdatazone,
			minix_sb->imap_blocks, minix_sb->zmap_blocks,
			minix_sb->log_zone_size, minix_sb->max_size);
#endif
	for (i = 0; i != MINIX_I_MAP_SLOTS; ++i)
		minix_sb->imap[i] = NULL;
	for (i = 0; i != MINIX_Z_MAP_SLOTS; ++i)
		minix_sb->zmap[i] = NULL;
	
	block = 2;
	for (i = 0; i < minix_sb->imap_blocks; ++i) {
		if ((minix_sb->imap[i] = get_minix_block(dev, block)) != NULL)
			block++;
		else
			break;	
	}
	for (i = 0; i < minix_sb->zmap_blocks; ++i) {
		if ((minix_sb->zmap[i] = get_minix_block(dev, block)) != NULL)
			block++;
		else
			break;	
	}
	if (block != minix_sb->imap_blocks + minix_sb->zmap_blocks + 2) {
		goto release_map;
	}
	minix_sb->imap[0]->data[0] |= 1;
	minix_sb->zmap[0]->data[0] |= 1;

	free = 0;
	i = minix_sb->nzones;
	while (--i >= 0) {
		byte *t = minix_sb->zmap[i>>13]->data + i/8%1024;
		if (!test_bit(*t, i%8))
			++free;
	}
	printk("%d/%d free blocks\n", free, minix_sb->nzones);
	free = 0;
	i = minix_sb->ninodes + 1;
	while (--i >= 0) {
		byte *t = minix_sb->imap[i>>13]->data + i/8%1024;
		if (!test_bit(*t, i%8))
			++free;
	}
	printk("%d/%d free inodes\n", free, minix_sb->ninodes);

	root_inode = iget(sb, MINIX_ROOT_INR);
	if (root_inode) {
		/* root dentry */
		sb->root = dget(root_inode, NULL);
		if (!sb->root) {
			dput(sb->root);
			iput(root_inode);
			goto release_map;
		}
		ret = SUCCESS;
		goto ok;
	}

	ret = -ERROR;

release_map:
	for (i = 0; i < MINIX_I_MAP_SLOTS; ++i)
		if (minix_sb->imap[i])
			put_minix_block(minix_sb->imap[i]);
	for (i = 0; i < MINIX_Z_MAP_SLOTS; ++i)
		if (minix_sb->zmap[i])
			put_minix_block(minix_sb->zmap[i]);
ok:
release_blk:
	put_minix_block(blk);
	return ret;
}
void minix_kill_super(struct super_block *sb)
{
	
}
