#include <sins/fs.h>
#include <sins/kernel.h>
#include <sins/irq.h>
#include <string.h>
#include <sins/atomic.h>

struct char_device {
	const char *name;
	struct file_operations *fops;
	atomic_t ref_count;
};

static struct char_device chrdevs[NR_CHRDEVS];

static result_t chrdev_open(struct inode *inode, struct file *filp);
static result_t chrdev_close(struct inode *inode, struct file *filp);
static ssize_t chrdev_read(struct file *, byte __user *, size_t, loff_t *);
static ssize_t chrdev_write(struct file *, const byte __user *,
	size_t, loff_t *);
static result_t chrdev_ioctl(struct file *, unsigned long, unsigned long);
static loff_t chrdev_lseek(struct file *filp, loff_t offset, int whence);


struct file_operations def_chr_fops = {
	.open	= chrdev_open,
	.read	= chrdev_read,
	.write	= chrdev_write,
	.ioctl	= chrdev_ioctl,
	.lseek	= chrdev_lseek,
	.close	= chrdev_close
};

result_t register_chrdev(unsigned int major, const char *name,
	struct file_operations *fops)
{
	unsigned long flags;

	if (fops == NULL)
		return -ERROR;
	if (major >= NR_CHRDEVS)
		return -EINVAL;

	irq_save(flags);
	if (major == 0) {
		for (major = NR_CHRDEVS - 1; major > 0; major --) {
			if (chrdevs[major].fops == NULL) {
				chrdevs[major].name = name;
				chrdevs[major].fops = fops;
				irq_restore(flags);
				return major;
			}
		}
		irq_restore(flags);
		return -EBUSY;
	}
	if (chrdevs[major].fops && chrdevs[major].fops != fops) {
		irq_restore(flags);
		return -EBUSY;
	}
	chrdevs[major].name = name;
	chrdevs[major].fops = fops;
	irq_restore(flags);
	return SUCCESS;
}

result_t unregister_chrdev(unsigned int major, const char *name)
{
	unsigned long flags;

	if (major >= NR_CHRDEVS)
		return -EINVAL;

	irq_save(flags);
	if (atomic_read(&chrdevs[major].ref_count) != 0) {
		irq_restore(flags);
		return -EBUSY;
	}
	if (!chrdevs[major].fops || strcmp(chrdevs[major].name, name) != 0) {
		irq_restore(flags);
		return -EINVAL;
	}
	chrdevs[major].name = NULL;
	chrdevs[major].fops = NULL;
	irq_restore(flags);
	return SUCCESS;
}

static result_t chrdev_open(struct inode *inode, struct file *filp)
{
	result_t ret = -ENODEV;
	struct char_device *device;
	
	BUG_ON(MAJOR(inode->rdev) >= NR_CHRDEVS);
	
	device = &chrdevs[MAJOR(inode->rdev)];
	atomic_inc(&device->ref_count);

	if (device->fops != NULL) {
		ret = SUCCESS;
		if (device->fops->open != NULL)
			ret = device->fops->open(inode, filp);
	}
	if (FAILED(ret))
		atomic_dec(&device->ref_count);
	return ret;
}

static ssize_t chrdev_read(struct file *filp, byte __user *buf,
	size_t size, loff_t *pos)
{
	ssize_t ret = -ENOSYS;
	struct char_device *device;

	device = &chrdevs[MAJOR(filp->inode->rdev)];
	if (device->fops->read != NULL)
		ret = device->fops->read(filp, buf, size, pos);
	return ret;
}

static ssize_t chrdev_write(struct file *filp, const byte __user *buf,
	size_t size, loff_t *pos)
{
	ssize_t ret = -ENOSYS;	
	struct char_device *device;

	device = &chrdevs[MAJOR(filp->inode->rdev)];
	if (device->fops->write != NULL)
		ret = device->fops->write(filp, buf, size, pos);
	return ret;
}

static result_t chrdev_ioctl(struct file *filp, unsigned long cmd,
	unsigned long arg)
{
	result_t ret = -ENOSYS;	
	struct char_device *device;

	device = &chrdevs[MAJOR(filp->inode->rdev)];
	if (device->fops->ioctl != NULL)
		ret = device->fops->ioctl(filp, cmd, arg);
	return ret;
}

static loff_t chrdev_lseek(struct file *filp, loff_t offset, int whence)
{
	loff_t ret = filp->pos;	
	struct char_device *device;

	device = &chrdevs[MAJOR(filp->inode->rdev)];
	if (device->fops->lseek != NULL)
		ret = device->fops->lseek(filp, offset, whence);
	else
		ret = -ENOSYS;
	if (ret >= 0)
		filp->pos = ret;
	return ret;
}

static result_t chrdev_close(struct inode *inode, struct file *filp)
{
	result_t ret = SUCCESS;
	struct char_device *device;

	device = &chrdevs[MAJOR(inode->rdev)];
	if (device->fops->close != NULL)
		ret = device->fops->close(inode, filp);
	atomic_dec(&device->ref_count);
	return ret;
}
