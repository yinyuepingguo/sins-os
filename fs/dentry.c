#include <sins/fs.h>
#include <sins/irq.h>
#include <string.h>
#include <sins/kernel.h>
#include <sins/init.h>

#define NR_HASH_LISTS	20

static struct list_head dentry_hash[NR_HASH_LISTS];

static unsigned long get_hash_index(struct inode *i, struct dentry *parent)
{
	return ((unsigned long)i+ (unsigned long)parent*301) % NR_HASH_LISTS;
}

static struct dentry *dcache(struct inode *i, struct dentry *parent,
	struct dentry *cached)
{
	unsigned long flags;
	struct dentry *pos;
	unsigned long index = get_hash_index(i, parent);

	irq_save(flags);
	list_for_each_entry(pos, &dentry_hash[index], hash_list) {
		if (pos->inode == i && pos->parent == parent) {
			BUG_ON(cached != NULL && cached != pos);
			irq_restore(flags);
			return pos;
		}
	}
	if (cached) {
		list_add(&cached->hash_list, &dentry_hash[index]);
		irq_restore(flags);
		return cached;
	}
	irq_restore(flags);
	return NULL;
}

static void ddecache(struct dentry *cached)
{
	unsigned long flags;
	struct dentry *pos;
	unsigned long index = get_hash_index(cached->inode, cached->parent);

	irq_save(flags);
	list_for_each_entry(pos, &dentry_hash[index], hash_list) {
		if (pos->inode == cached->inode && 
				pos->parent == cached->parent) {
			list_del(&cached->hash_list);
			irq_restore(flags);
			return;
		}
	}
	irq_restore(flags);
}

struct dentry *dget(struct inode *i, struct dentry *parent)
{
	unsigned long flags;
	struct dentry *d;

	BUG_ON(i == NULL);
	irq_save(flags);
	if ((d = dcache(i, parent, NULL)) != NULL) {
		dref(d);
		irq_restore(flags);
		return d;	
	}
	d = (struct dentry *)kmalloc(sizeof(struct dentry));
	if (d) {
		memset(d, 0, sizeof(*d));
		if (parent) {
			dref(parent);
			list_add(&d->child, &parent->children);
		}
		d->parent = parent;
		d->sb = i->sb;
		iref(i);
		d->inode = i;
		INIT_LIST_HEAD(&d->children);
		d->mount = NULL;
		atomic_inc(&d->ref_count);
		list_add(&d->use_list, &d->sb->dentries);
		dcache(i, parent, d);
	}
	irq_restore(flags);
	return d;
}

void dref(struct dentry *de)
{
	BUG_ON(atomic_read(&de->ref_count) == 0);
	atomic_inc(&de->ref_count);
}

void dput(struct dentry *de)
{
	unsigned long flags;

	irq_save(flags);
	BUG_ON(atomic_read(&de->ref_count) == 0);
	atomic_dec(&de->ref_count);
	if (atomic_read(&de->ref_count) == 0) {
		list_del(&de->use_list);
		if (de->parent) {
			list_del(&de->child);
			dput(de->parent);
		}
		iput(de->inode);
		BUG_ON(!list_empty(&de->children));
		ddecache(de);
		kfree(de);
	}
	irq_restore(flags);
}

static result_t dcache_setup()
{
	unsigned long i;

	for (i = 0; i != NR_HASH_LISTS; ++i)
		INIT_LIST_HEAD(&dentry_hash[i]);

	return SUCCESS;
}

subsys_initcall(dcache_setup);
