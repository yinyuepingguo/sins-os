#include <sins/fs.h>
#include <sins/kernel.h>
#include <sins/irq.h>
#include <string.h>
#include <sins/sched.h>

static LIST_HEAD(fs_types); 
struct dentry droot = { .ref_count = {1} };
static struct inode iroot = { .ref_count = {1} };



result_t register_filesystem(struct file_system_type *fst)
{
	unsigned long flags;
	struct file_system_type *pos;

	if (fst->name == NULL || fst->get_super == NULL)
		return -EINVAL;
	if (fst->sops == NULL || fst->sops->read_inode == NULL ||
		fst->sops->lookup == NULL)
		return -EINVAL;
	if (fst->fops == NULL) 
		return -EINVAL;


	irq_save(flags);	
	list_for_each_entry(pos, &fs_types, list) {
		if (strcmp(pos->name, fst->name) == 0) {
			irq_restore(flags);
			return -EINVAL;
		}
	}
	list_add(&fst->list ,&fs_types);
	INIT_LIST_HEAD(&fst->sb_list);
	irq_restore(flags);
	return SUCCESS;
}

result_t unregister_filesystem(const char *name)
{
	unsigned long flags;
	struct list_head *pos, *tmp;
	struct file_system_type *fst;

	irq_save(flags);
	list_for_each_safe(pos, tmp, &fs_types) {
		fst = list_entry(pos, struct file_system_type, list);
		if (strcmp(fst->name, name) == 0) {
			if (list_empty(&fst->sb_list)) {
				list_del(&fst->list);
				irq_restore(flags);
				return SUCCESS;
			} else {
				irq_restore(flags);
				return -EBUSY;
			}
		}
	}
	irq_restore(flags);
	return -EINVAL;
}

struct super_block *alloc_super_block(dev_t dev, struct dentry *de)
{
	struct super_block *sb=
		(struct super_block *)kmalloc(sizeof(struct super_block));

	if (sb) {
		INIT_LIST_HEAD(&sb->sb_list);
		sb->fs_type = NULL;
		sb->magic = 0;
		sb->dev = dev;
		sb->parent = de;
		BUG_ON(de->mount != NULL);
		dref(de);
		de->mount = sb;
		sb->root = NULL;
		INIT_LIST_HEAD(&sb->inodes);
		INIT_LIST_HEAD(&sb->dentries);
	}
	return sb;
}
void dealloc_super_block(struct super_block *sb)
{
	BUG_ON(sb == NULL);
	dput(sb->parent);
	kfree(sb);
}

static void init_droot()
{
	dref(&droot);
	iref(&iroot);
	droot.inode = &iroot;
	droot.parent = &droot;
	dref(&droot);
	SET_DIR(iroot.mode);
}

void mount_root(dev_t dev)
{
	init_droot();

	if (FAILED(do_mount(dev, &droot)))
		panic("Mount root failed!");
	
	current->root = current->pwd = &droot;
	dref(&droot);
	dref(&droot);
}

result_t do_mount(dev_t dev, struct dentry *de)
{
	unsigned long flags;
	struct file_system_type *fst;
	struct super_block *sb;

	sb = alloc_super_block(dev, de);
	if (sb == NULL)
		return -ENOMEM;

	irq_save(flags);
	list_for_each_entry(fst, &fs_types, list) {
		sb->fs_type = fst;

		if (FAILED(fst->get_super(sb)))
			continue;
		else {
			list_add(&sb->sb_list, &fst->sb_list);
			printk("Mount %s file system as root ok!\n", fst->name);
			irq_restore(flags);
			return SUCCESS;
		}
	}
	irq_restore(flags);
	return -ERROR;
}
