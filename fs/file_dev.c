#include <sins/fs.h>
#include <sins/kernel.h>

static result_t file_open(struct inode *inode, struct file *filp);
static ssize_t file_read(struct file *filp, byte __user *buf,
	size_t size, loff_t *pos);
static ssize_t file_readdir(struct file *filp, struct dir __user *buf,
	size_t len, loff_t *pos);
static ssize_t file_write(struct file *filp, const byte __user *buf,
	size_t size, loff_t *pos);
static result_t file_ioctl(struct file *filp, unsigned long cmd,
	unsigned long arg);
static loff_t file_lseek(struct file *filp, loff_t offset, int whence);
static result_t file_close(struct inode *inode, struct file *filp);

struct file_operations def_file_fops = {
	.open	= file_open,
	.read	= file_read,
	.readdir= file_readdir,
	.write	= file_write,
	.ioctl	= file_ioctl,
	.lseek	= file_lseek,
	.close	= file_close
};

static result_t file_open(struct inode *inode, struct file *filp)
{
	struct file_operations *fops;
	result_t ret = SUCCESS;

	fops = filp->inode->sb->fs_type->fops;
	if (fops->open)
		ret = fops->open(inode, filp);
	return ret;
}

static ssize_t file_read(struct file *filp, byte __user *buf,
	size_t size, loff_t *pos)
{
	struct file_operations *fops;
	ssize_t ret = -ENOSYS;

	fops = filp->inode->sb->fs_type->fops;
	if (fops->read)
		ret = fops->read(filp, buf, size, pos);
	return ret;
}

static ssize_t file_readdir(struct file *filp, struct dir __user *buf,
	size_t len, loff_t *pos)
{
	struct file_operations *fops;
	ssize_t ret = -ENOSYS;

	fops = filp->inode->sb->fs_type->fops;
	if (fops->readdir)
		ret = fops->readdir(filp, buf, len, pos);
	return ret;
}
static ssize_t file_write(struct file *filp, const byte __user *buf,
	size_t size, loff_t *pos)
{
	struct file_operations *fops;
	ssize_t ret = -ENOSYS;

	fops = filp->inode->sb->fs_type->fops;
	if (fops->write)
		ret = fops->write(filp, buf, size, pos);
	return ret;
}

static result_t file_ioctl(struct file *filp, unsigned long cmd,
	unsigned long arg)
{
	struct file_operations *fops;
	result_t ret = -ENOSYS;

	fops = filp->inode->sb->fs_type->fops;
	if (fops->ioctl)
		ret = fops->ioctl(filp, cmd, arg);
	return ret;
}

static loff_t file_lseek(struct file *filp, loff_t offset, int whence)
{
	struct file_operations *fops;
	loff_t ret = filp->pos;	

	fops = filp->inode->sb->fs_type->fops;
	if (fops->lseek)
		ret = fops->lseek(filp, offset, whence);
	else
		ret = -ENOSYS;
	if (ret >= 0)
		filp->pos = ret;
	return ret;
}

static result_t file_close(struct inode *inode, struct file *filp)
{
	struct file_operations *fops;
	result_t ret = SUCCESS;

	fops = filp->inode->sb->fs_type->fops;
	if (fops->close)
		ret = fops->close(inode, filp);
	return ret;
}
