#include <sins/fs.h>
#include <sins/kernel.h>
#include <math.h>
#include <string.h>

ssize_t minix_file_read
	(struct file *filp, byte __user *buf, size_t size, loff_t *pos)
{
	unsigned long left,chars, remains;
	unsigned long nr;
	struct inode *inode = filp->inode;
	struct minix_block *mb;

	left = size;
	while (left) {
		nr = minix_bmap(inode, *pos/MINIX_BLOCK_SIZE);
		if (nr != 0) {
			mb = get_minix_block(inode->sb->dev, nr); 
			if (mb == NULL)
				break;
		} else
			mb = NULL;

		nr = *pos % MINIX_BLOCK_SIZE;
		chars = min(MINIX_BLOCK_SIZE - nr, left);
		if (*pos < filp->inode->size)
			remains = filp->inode->size - *pos;
		else
			remains = 0;
		remains = min(remains, chars);
		*pos += remains;
		left -= remains;
		if (mb) {
			byte *p = nr + mb->data;
			while (remains-->0) {
				*buf++ = *p++;
			}
			put_minix_block(mb);
			if (*pos >= filp->inode->size) {
				break;
			}
		} else {
			if (*pos < filp->inode->size) {
				while (remains-->0)
					*buf++ = 0;
			} else {
				left += remains;
			}
			break;
		}
	}
	return (size-left)?(size-left):-EIO;
}

ssize_t minix_file_write
	(struct file *filp, const byte __user *buf, size_t size, loff_t *pos)
{
	return -ERROR;
}

ssize_t minix_file_readdir
	(struct file *filp, struct dir __user *buf, size_t len, loff_t *pos)
{
	unsigned long size;
	struct minix_dentry *tmp = get_free_page(GFP_NORMAL | GFP_DMA);
	result_t ret;
	unsigned long i = 0, j;

	if (tmp == NULL)
		return -ENOMEM;

	while (i < len) {
		size = min((len - i)* sizeof(struct minix_dentry),
			(unsigned long)PAGE_SIZE);
		ret = minix_file_read(filp, (byte *)tmp, size, pos);
		if (ret > 0) {
			for (j = 0; j != ret/sizeof(struct minix_dentry);
				++j, ++i) {

				strncpy(buf[i].name,
					tmp[j].name, MINIX_NAME_LEN);
				buf[i].name[MINIX_NAME_LEN] = 0;
			}
			/* check align */
			if (ret % sizeof(struct minix_dentry)) {
				ret = -ERROR;
				goto finish;
			}
			/* if next time is confirm to return EIO, finish directly */
			if (ret < size) {
				ret = i;
				goto finish;
			}
		} else if (ret == -EIO && i == 0) {
			goto finish;
		} else {
			ret = i;
			goto finish; /* EIO */
		}
	}
	ret = i;
finish:
	free_page(tmp);
	return ret;
}
