#include <sins/fs.h>
#include <sins/irq.h>
#include <string.h>
#include <sins/sched.h>
#include <sins/kernel.h>
#include <sins/init.h>

#define NR_HASH_LISTS 20

static struct list_head inode_hash[NR_HASH_LISTS];

static unsigned long get_hash_index(struct super_block *sb, unsigned long i_nr)
{
	return ((unsigned long)sb+i_nr*301) % NR_HASH_LISTS;
}

static struct inode *icache(struct super_block *sb, unsigned long i_nr,
	struct inode *cached)
{
	unsigned long flags;
	struct inode *pos;
	unsigned long index = get_hash_index(sb, i_nr);

	irq_save(flags);
	list_for_each_entry(pos, &inode_hash[index], hash_list) {
		if (pos->sb == sb && pos->nr == i_nr) {
			BUG_ON(cached != NULL && cached != pos);
			irq_restore(flags);
			return pos;
		}
	}
	if (cached) {
		list_add(&cached->hash_list, &inode_hash[index]);
		irq_restore(flags);
		return cached;
	}
	irq_restore(flags);
	return NULL;
}

static void idecache(struct inode *cached)
{
	unsigned long flags;
	struct inode *pos;
	unsigned long index = get_hash_index(cached->sb, cached->nr);

	irq_save(flags);
	list_for_each_entry(pos, &inode_hash[index], hash_list) {
		if (pos->sb == cached->sb && pos->nr == cached->nr) {
			list_del(&cached->hash_list);
			irq_restore(flags);
			return;
		}
	}
	irq_restore(flags);
}

static volatile unsigned char iput_lock = 0;
static WAIT_QUEUE(wait_iput);

static inline void init_file_type_specific(struct inode *inode)
{
	if (IS_REG(inode->mode)) {
		inode->fops = &def_file_fops;
	} else if (IS_CHR(inode->mode)) {
		inode->fops = &def_chr_fops;
	} else if (IS_BLK(inode->mode)) {
		inode->fops = &def_blk_fops;
	} else {
		inode->fops = &def_file_fops;
	}
}


struct inode *iget(struct super_block *sb, unsigned long i_nr)
{
	unsigned long flags;
	struct inode *i, *tmp = NULL;
	struct super_operations *sops;

	irq_save(flags);
	if ((i = icache(sb, i_nr, NULL)) != NULL) {
		iref(i);
		irq_restore(flags);
		return i;
	}
	i = (struct inode *)kmalloc(sizeof(struct inode));
	if (i) {
		memset(i, 0, sizeof(*i));
		i->nr = i_nr;
		i->sb = sb;
		i->dirty = 0;
		list_add(&i->use_list, &sb->inodes);
		atomic_inc(&i->ref_count);
		sops = sb->fs_type->sops;
		/* may sleep */
		wait_event(&wait_iput, iput_lock == 0);
		if (FAILED(sops->read_inode(i)) ||
			(tmp = icache(sb, i_nr, NULL)) != NULL) {
			/* read error or already exists */
			list_del(&i->use_list);
			kfree(i);
			i = tmp;
		} else {
			init_file_type_specific(i);
			icache(sb, i_nr, i);
		}
	}
	irq_restore(flags);
	return i;
}

void iref(struct inode *i)
{
	BUG_ON(atomic_read(&i->ref_count) == 0);
	atomic_inc(&i->ref_count);
}

void iput(struct inode *i)
{
	unsigned long flags;
	struct super_operations *sops;
	result_t ret;

	irq_save(flags);
	BUG_ON(atomic_read(&i->ref_count) == 0);
	atomic_dec(&i->ref_count);
	if (atomic_read(&i->ref_count) == 0) {
		list_del(&i->use_list);
		idecache(i);
		if (i->dirty) {
			sops = i->sb->fs_type->sops;
			if (sops->write_inode) {
				iput_lock = 1;
				ret = sops->write_inode(i);
				iput_lock = 0;
				wake_up_all(&wait_iput);
				if (FAILED(ret))
					WARN("write_inode returns error!\n");
			}
		}
		kfree(i);
	}
	irq_restore(flags);
}

static result_t icache_setup()
{
	unsigned long i;

	for (i = 0; i != NR_HASH_LISTS; ++i)
		INIT_LIST_HEAD(&inode_hash[i]);

	return SUCCESS;
}

subsys_initcall(icache_setup);
