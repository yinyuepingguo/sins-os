#include <sins/sched.h>
#include <sins/kernel.h>
#include <string.h>
#include <sins/mm.h>
#include <sins/fs.h>
#include <sins/irq.h>
#include <sins/syscalls.h>


static inline int get_unused_fd(struct file *fdt[])
{
	unsigned long i;

	for (i = 0; i != NR_OPEN; ++i) {
		if (fdt[i] == NULL) {
			/* lock this fd */
			fdt[i] = (struct file *)fdt;
			return i;
		}
	}
	return -1;
}
static inline void put_used_fd(struct file *fdt[], unsigned int fd)
{
	fdt[fd] = NULL;	
}

asmlinkage int sys_open(const char *__user filename, int flags)
{
	int fd, i;
	struct inode *inode;
	struct file *f;
	result_t ret = SUCCESS;

	fd = get_unused_fd(current->fdt);
	if (FAILED(fd))
		return -ERROR;

	flags &= O_MASK;

	i = open_namei(filename, flags, &inode);
	/* already add inode reffrence, so need to dereferrence when closing */
	if (FAILED(i)) {
		ret = i;
		goto error_release_fd;
	}

	f = (struct file *)kmalloc(sizeof(struct file));
	if (f == NULL) {
		ret = -ENOMEM;
		goto error_release_inode;
	}
	memset(f, 0, sizeof(f));	
	f->flags = flags;
	f->inode = inode;
	f->pos = 0;
	atomic_set(&f->ref_count, 1);
	current->fdt[fd] = f;

	if (inode->fops != NULL && inode->fops->open != NULL) {
		ret = inode->fops->open(inode, f);
		if (FAILED(ret)) {
			goto error_release_file;
		}
	}

	ret = fd;
	goto ok;
	
error_release_file:
	kfree(f);
error_release_inode:
	iput(inode);
error_release_fd:
	put_used_fd(current->fdt, fd);
ok:
	return ret;
}

asmlinkage ssize_t sys_readdir(unsigned int fd,
	struct dir *__user buf, size_t len)
{
	struct file *filp;
	struct inode *inode;
	ssize_t ret = -ENOSYS;

	if (fd >= NR_OPEN)
		return -EINVAL;
	
	filp = current->fdt[fd];
	if (filp == NULL)
		return -EINVAL;

	if (!IS_DIR(filp->inode->mode))
		return -ERROR;

	inode = filp->inode;	
	if (inode->fops != NULL && inode->fops->readdir != NULL) {
		ret = inode->fops->readdir(filp, buf, len, &filp->pos);
	}
	return ret;
}

asmlinkage int sys_creat(const char *__user pathname)
{
	return sys_open(pathname, O_CREAT | O_TRUNC);
}

/* must close successfully */
asmlinkage int sys_close(unsigned int fd)
{
	struct file *f;
	unsigned long flags;
	struct inode *inode;

	result_t ret = SUCCESS;

	if (fd >= NR_OPEN)
		return -EINVAL;

	f = current->fdt[fd];
	if (f == NULL)
		return -EINVAL;

	put_used_fd(current->fdt, fd);

	inode = f->inode;

	irq_save(flags);
	atomic_dec(&f->ref_count);
	if (atomic_read(&f->ref_count) == 0) {
		irq_restore(flags);
		if (inode->fops != NULL && inode->fops->close != NULL) {
			ret = inode->fops->close(inode, f);
		}
		iput(inode);
		kfree(f);
		irq_save(flags);
	}
	irq_restore(flags);
	return ret;
}

/* NOTE:pos filed will be massed when two processes access one file */
asmlinkage ssize_t sys_read(unsigned int fd, byte __user *buf, size_t size)
{
	struct file *filp;
	struct inode *inode;
	ssize_t ret = -ENOSYS;

	if (fd >= NR_OPEN)
		return -EINVAL;

	filp = current->fdt[fd];
	if (filp == NULL) 
		return -EINVAL;

	inode = filp->inode;	
	if (inode->fops != NULL && inode->fops->read != NULL) {
		ret = inode->fops->read(filp, buf, size, &filp->pos);
	}
	return ret;
}

asmlinkage ssize_t sys_write(unsigned int fd, const byte __user *buf, size_t size)
{
	struct file *filp;
	struct inode *inode;
	ssize_t ret = -ENOSYS;

	if (fd >= NR_OPEN)
		return -EINVAL;

	filp = current->fdt[fd];
	if (filp == NULL)
		return -EINVAL;

	inode = filp->inode;	
	if (inode->fops != NULL && inode->fops->write != NULL) {
		ret = inode->fops->write(filp, buf, size, &filp->pos);
	}
	return ret;
}

asmlinkage result_t sys_ioctl(unsigned int fd,
	unsigned long cmd, unsigned long arg)
{
	struct file *filp;
	struct inode *inode;
	result_t ret = -ENOSYS;

	if (fd >= NR_OPEN)
		return -EINVAL;
	
	filp = current->fdt[fd];
	if (filp == NULL)
		return -EINVAL;

	inode = filp->inode;
	if (inode->fops != NULL && inode->fops->ioctl != NULL) {
		ret = inode->fops->ioctl(filp, cmd, arg);
	}
	return ret;
}

asmlinkage loff_t sys_lseek(unsigned int fd, loff_t offset, int whence)
{
	struct file *filp;
	struct inode *inode;
	result_t ret = -ENOSYS;

	if (fd >= NR_OPEN)
		return -EINVAL;
	
	filp = current->fdt[fd];
	if (filp == NULL)
		return -EINVAL;

	inode = filp->inode;
	if (inode->fops != NULL && inode->fops->lseek != NULL) {
		ret = inode->fops->lseek(filp, offset, whence);
	}
	return ret;
}

asmlinkage int sys_dup(unsigned int old_fd)
{
	result_t new_fd;
	struct file *f;

	if (old_fd >= NR_OPEN)
		return -EINVAL;

	f = current->fdt[old_fd];
	if (f == NULL)
		return -EINVAL;

	new_fd = get_unused_fd(current->fdt);
	if (FAILED(new_fd))
		return -ERROR;

	current->fdt[new_fd] = f;
	atomic_inc(&f->ref_count);

	return new_fd;
}

asmlinkage int sys_dup2(unsigned int old_fd, unsigned int new_fd)
{
	if (old_fd >= NR_OPEN || new_fd >= NR_OPEN)
		return -EINVAL;
	if (current->fdt[old_fd] == NULL || current->fdt[new_fd] == NULL)
		return -EINVAL;
	if (old_fd == new_fd)
		return old_fd;

	current->fdt[new_fd] = current->fdt[old_fd];	
	atomic_inc(&current->fdt[new_fd]->ref_count);

	return new_fd;
}
