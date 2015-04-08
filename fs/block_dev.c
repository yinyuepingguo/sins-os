#include <sins/fs.h>
#include <sins/kernel.h>
#include <sins/irq.h>
#include <string.h>
#include <sins/atomic.h>
#include <list.h>
#include <sins/init.h>
#include <sins/mm.h>
#include <sins/sched.h>

struct block_device {
	const char *name;
	atomic_t ref_count;
	unsigned long block_size;
	void (*rw_handler)(int rw, struct block *blk);
};

static struct block_device blkdevs[NR_BLKDEVS];

static result_t blkdev_open(struct inode *inode, struct file *filp);
static result_t blkdev_close(struct inode *inode, struct file *filp);
static ssize_t blkdev_read(struct file *, byte __user *, size_t, loff_t *);
static ssize_t blkdev_write(struct file *, const byte __user *,
	size_t, loff_t *);
static result_t blkdev_ioctl(struct file *, unsigned long, unsigned long);
static loff_t blkdev_lseek(struct file *filp, loff_t offset, int whence);


struct file_operations def_blk_fops = {
	.open	= blkdev_open,
	.read	= blkdev_read,
	.write	= blkdev_write,
	.ioctl	= blkdev_ioctl,
	.lseek	= blkdev_lseek,
	.close	= blkdev_close
};

struct block *get_empty_blk(dev_t dev, unsigned long block_nr)
{
	struct block *blk;

	blk = (struct block *)kmalloc(sizeof(struct block));
	if (likely(blk != NULL)) {
		blk->block_nr = block_nr;
		blk->dev = dev;
		blk->data = get_free_page(GFP_NORMAL | GFP_DMA);
		blk->dirty = 0;
		blk->uptodate = 0;
		if (unlikely(blk->data == NULL)) {
			kfree(blk);
			blk = NULL;
		}
	}
	return blk;
}
void put_blk(struct block *blk)
{
	BUG_ON(blk == NULL);

	free_page(blk->data);
	kfree(blk);
}

/* not use buffer */
struct block *bread(dev_t dev, unsigned long block_nr)
{
	struct block *blk;

	if (blkdevs[MAJOR(dev)].rw_handler == NULL)
		panic("No such device!");
		
	blk = get_empty_blk(dev, block_nr);
	if (blk) {
		blkdevs[MAJOR(dev)].rw_handler(BLK_READ, blk);
		if (blk->uptodate)
			return blk;
		else {
			put_blk(blk);
			return NULL;
		}
	}
	return NULL;
}

void brelse(struct block *blk)
{
	BUG_ON(blk == NULL);

	if (blk->dirty) {
		blkdevs[MAJOR(blk->dev)].rw_handler(BLK_WRITE, blk);
	}
	BUG_ON(blk->dirty);
	put_blk(blk);
}


result_t register_blkdev(unsigned int major, const char *name,
	void (*rw_handler)(int rw, struct block *blk),
		unsigned long block_size)
{
	unsigned long flags;

	if (name == NULL || rw_handler == NULL)
		return -EINVAL;

	if (major >= NR_BLKDEVS)
		return -EINVAL;

	irq_save(flags);
	if (major == 0) {
		for (major = NR_BLKDEVS - 1; major > 0; major --) {
			if (blkdevs[major].name == NULL) {
				goto find_major;	
			}
		}
		irq_restore(flags);
		return -EBUSY;
	}
	if (blkdevs[major].name != NULL) {
		irq_restore(flags);
		return -EBUSY;
	}
find_major:
	blkdevs[major].name = name;
	blkdevs[major].rw_handler = rw_handler;
	blkdevs[major].block_size = block_size;
	irq_restore(flags);
	return SUCCESS;
}

result_t unregister_blkdev(unsigned int major, const char *name)
{
	unsigned long flags;

	if (major >= NR_BLKDEVS)
		return -EINVAL;

	irq_save(flags);
	if (atomic_read(&blkdevs[major].ref_count) != 0) {
		irq_restore(flags);
		return -EBUSY;
	}
	if (strcmp(blkdevs[major].name, name) != 0) {
		irq_restore(flags);
		return -EINVAL;
	}
	blkdevs[major].name = NULL;
	blkdevs[major].rw_handler = NULL; 
	blkdevs[major].block_size = 0;
	irq_restore(flags);
	return SUCCESS;
}

static result_t blkdev_open(struct inode *inode, struct file *filp)
{
	result_t ret = -ENODEV;
	struct block_device *device;
	
	BUG_ON(MAJOR(inode->rdev) >= NR_BLKDEVS);
	
	device = &blkdevs[MAJOR(inode->rdev)];
	atomic_inc(&device->ref_count);

	if (device->rw_handler != NULL) {
		ret = SUCCESS;
	}	
	if (FAILED(ret))
		atomic_dec(&device->ref_count);
	return ret;
}

static ssize_t blkdev_read(struct file *filp, byte __user *buf,
	size_t size, loff_t *pos)
{
	long block = *pos / BLOCK_SIZE;
	long offset = *pos % BLOCK_SIZE;
	long chars;
	long read = 0;
	struct block *blk;
	register byte *p;

	while (size > 0) {
		blk = bread(filp->inode->rdev, block);
		if (!blk)
			return read? read: -EIO;
		chars = (size < BLOCK_SIZE)? size: BLOCK_SIZE;
		p = offset + blk->data;
		offset = 0;
		block++;
		*pos += chars;
		read += chars;
		size -= chars;
		while (chars-- > 0) {
			*buf = *p;
			p++;
			buf++;
		}
		brelse(blk);	
	}

	return read;
}

static ssize_t blkdev_write(struct file *filp, const byte __user *buf,
	size_t size, loff_t *pos)
{
	long block = *pos / BLOCK_SIZE;
	long offset = *pos % BLOCK_SIZE;
	long chars;
	long written = 0;
	struct block *blk;
	register byte *p;

	while (size > 0) {
		blk = bread(filp->inode->rdev, block);
		if (!blk)
			return written? written: -EIO;
		chars = (size < BLOCK_SIZE)? size: BLOCK_SIZE;
		p = offset + blk->data;
		offset = 0;
		block++;
		*pos += chars;
		written += chars;
		size -= chars;
		while (chars-- > 0) {
			*p = *buf;
			p++;
			buf++;
		}
		blk->dirty = 1;
		brelse(blk);	
	}

	return written;
}

static result_t blkdev_ioctl(struct file *filp, unsigned long cmd,
	unsigned long arg)
{
	return -ENOSYS;
}

static loff_t blkdev_lseek(struct file *filp, loff_t offset, int whence)
{
	switch (whence)	{
		case 0:
			if (offset < 0)
				return -EINVAL;
			filp->pos = offset;
			break;
		case 1:
			if (filp->pos + offset < 0)
				return -EINVAL;
			filp->pos += offset;
			break;
		/* we don't know device's size */
		default:
			return -EINVAL;
		
	}
	return filp->pos;
}

static result_t blkdev_close(struct inode *inode, struct file *filp)
{
	result_t ret = SUCCESS;
	struct block_device * device;

	device = &blkdevs[MAJOR(inode->rdev)];
	atomic_dec(&device->ref_count);
	return ret;
}
