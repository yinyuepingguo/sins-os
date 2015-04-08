#include <string.h>
#include <sins/fs.h>
#include <sins/mm.h>
#include <sins/kernel.h>

static unsigned long _bmap
	(struct inode *inode, unsigned long block, int create);

struct minix_block *get_minix_block(dev_t dev, unsigned long minix_blocks)
{
	unsigned long block =
		minix_blocks / (BLOCK_SIZE / MINIX_BLOCK_SIZE);
	unsigned long offset =
		(minix_blocks%(BLOCK_SIZE/MINIX_BLOCK_SIZE))*MINIX_BLOCK_SIZE;
	struct block *blk;
	struct minix_block *mb = NULL;

	if ((blk = bread(dev, block)) != NULL) {
		mb = (struct minix_block *)
			kmalloc(sizeof(struct minix_block));
		
		if (mb) {
			mb->data = blk->data + offset;
			mb->blk = blk; 
			mb->dirty = 0;
		}
		return mb;
	}
	else
		return NULL;
}

void put_minix_block(struct minix_block *mb)
{
	BUG_ON(mb == NULL);
	BUG_ON(mb->data == NULL);

	if (mb->dirty) {
		mb->blk->dirty = 1;
	}
	brelse(mb->blk);
	kfree(mb);
}

unsigned long minix_bmap(struct inode * inode, unsigned long block)
{
	return _bmap(inode, block, false);
}

unsigned long minix_create_block(struct inode * inode, unsigned long block)
{
	return _bmap(inode, block, true);
}

static unsigned long _bmap
	(struct inode *inode, unsigned long block, int create)
{
	struct minix_block *blk = NULL;
	unsigned long i;

	BUG_ON(block >= 7 + 512 + 512*512);
	
	if (block < 7) {
		if (create && !inode->i_minix.zone[block]) {
			/* do create a new block */
		}
		return inode->i_minix.zone[block];
	}
	block -= 7;

	if (block < 512) {
		if (create && !inode->i_minix.zone[7]) {
			/* do create a new block */
		}
		if (!inode->i_minix.zone[7])
			return 0;
		blk = get_minix_block(inode->sb->dev, inode->i_minix.zone[7]);
		if (blk == NULL)
			return 0;
		i = ((unsigned short *)blk->data)[block];
		if (create && !i) {
			/* do create a new block */
		}
		put_minix_block(blk);
		return i;
	}
	block -= 512;
	if (create && !inode->i_minix.zone[8]) {
		/* do create */
	}
	
	if (!inode->i_minix.zone[8])
		return 0;
	blk = get_minix_block(inode->sb->dev, inode->i_minix.zone[8]);
	if (blk == NULL)
		return 0;
	i = ((unsigned short *)blk->data)[block>>9];
	if (create && !i) {
		/* do create */
	}
	put_minix_block(blk);

	if (!i)
		return 0;

	blk = get_minix_block(inode->sb->dev, i);
	if (blk == NULL)
		return 0;
	i = ((unsigned short *)blk->data)[block&511];
	if (create && !i) {
		/* do create */
	}
	put_minix_block(blk);
	return i;
}








