#include <sins/fs.h>
#include <sins/kernel.h>
#include <string.h>
#include <sins/irq.h>
#include <string.h>
#include <sins/init.h>
#include <sins/sched.h>

extern struct dentry droot;
static struct dentry *lookup(const char *pathname, char fromroot);
static struct dentry *find_entry(struct dentry *root,
	const char *name, unsigned long len);


result_t open_namei(const char *__user filename, int flags,
	struct inode **inode)
{
	struct dentry *find = lookup(filename, 0);
	
	if (find == NULL) {
		*inode = NULL;
		return -ERROR;
	}
	
	*inode = find->inode;
	iref(find->inode);
	dput(find);

	return SUCCESS;
}

asmlinkage result_t sys_chdir(const char *__user filename)
{
	struct dentry *find = lookup(filename, 0);
	struct dentry *iter = find, *root = current->root;
	
	if (find == NULL)
		return -ERROR;
	
	if (root == NULL)
		root = &droot;

	while (iter != &droot) {
		if (iter == root ) {
			goto finish;
		}
		if (iter->parent == NULL) {
			iter = iter->sb->parent;
		} else {
			iter = iter->parent;
		}
	}
	if (iter == root)
		goto finish;
	dput(find);
	return -ERROR;
finish:
	if (IS_DIR(find->inode->mode)) {
		dput(current->pwd);
		current->pwd = find;
		return SUCCESS;
	} else {
		dput(find);
		return -ERROR;
	}
}

asmlinkage result_t sys_chroot(const char *__user filename)
{
	struct dentry *find = lookup(filename, 1);
	
	if (find == NULL)
		return -ERROR;
	
	if (IS_DIR(find->inode->mode)) {
		if (current->root != NULL)
			dput(current->root);
		current->root = find;

		dref(find);
		if (current->pwd != NULL)
			dput(current->pwd);
		current->pwd = find;
		return SUCCESS;
	} else {
		dput(find);
		return -ERROR;
	}
}

asmlinkage result_t sys_getcwd(char *__user buf, unsigned long len)
{
	struct dentry *iter;
	struct list_head name_list, *pos, *tmp;
	result_t ret;
	struct cwd_name {
		struct list_head list;
		struct dentry *entry;
	};
	unsigned long need;
	char *begin;
	struct cwd_name *cwd = NULL;
	
	INIT_LIST_HEAD(&name_list);
	iter = current->pwd;
	while (iter != current->root) {
		cwd = (struct cwd_name *)kmalloc(sizeof(struct cwd_name));
	
		if (cwd == NULL) {
			ret = -ENOMEM;
			goto finish;
		}
	
		cwd->entry = iter;
		list_add(&cwd->list, &name_list);
		if (iter->parent == NULL && iter->sb != NULL) {
			iter = iter->sb->parent;
		} else {
			iter = iter->parent;
		}
	}

	if (len == 0) {
		ret = -EINVAL;
		goto finish;
	}

	need = 0;
	begin = buf;
	*begin = 0;
	list_for_each(pos, &name_list) {
		cwd = list_entry(pos, struct cwd_name, list);
	
		if (cwd->entry->parent == NULL)
			continue;
		/* root inode not print */
		need += cwd->entry->name_len+1;
		if (need < len-1) {
			*begin = '/';
			begin++;
			strncpy(begin, cwd->entry->name, cwd->entry->name_len);
			begin += cwd->entry->name_len;
		} else {
			*begin = 0;
			ret = begin - buf;
			goto finish;
		}
	} 
	if (begin == buf)
		*begin++ = '/';
	*begin = 0;
	ret = begin - buf;
finish:
	list_for_each_safe(pos, tmp, &name_list) {
		list_del(pos);
		kfree(list_entry(pos, struct cwd_name, list));
	}
	return ret;
}

static struct dentry *find_entry(struct dentry *root,
	const char *name, unsigned long len)
{
	struct dentry *iter = root;
	unsigned long flags;
	struct dentry *find, *entry;
	struct list_head *pos, *tmp;

	irq_save(flags);
	while (iter->mount != NULL) {
		iter = iter->mount->root;
	}
	dref(iter);
	/* find entry in memory */
	list_for_each_safe(pos, tmp, &iter->children) {
		entry = list_entry(pos, struct dentry, child);
#ifdef DEBUG
		printk("--- %s ---", entry->name );
#endif
		if (strncmp(entry->name, name, len) == 0) {
			dref(entry);
			dput(iter);
			irq_restore(flags);
			return entry;
		}
	}
	irq_restore(flags);

	/* find entry in disk */
	find = iter->sb->fs_type->sops->lookup(iter, name, len);
	dput(iter);
	return find;
}

static struct dentry *lookup(const char *pathname, char fromroot)
{
	struct dentry *iter = current->pwd, *prev_iter = current->pwd;
	struct dentry *root = current->root;
	const char *begin = pathname;
	const char *end;
	unsigned long flags;

	BUG_ON(pathname == NULL);
	
	if (*begin == '/') {
		iter = current->root;
		++begin;
	}
	
	/* if one process has no dentry, select root dentry */
	if (iter == NULL || fromroot) {
		iter = &droot;
		root = &droot;
	}

	dref(iter);	
	while (1) {
		end = begin;
		while (*end != '/' && *end != 0) {
			end++;
		}
		if (begin == end) {
			if (*end == 0)
				goto finish;
			else {
				begin++;
				continue;
			}
		} else if (end - begin == 1 && *begin == '.') {
			/* no action */
			if (*end == 0)
				goto finish;
			else {
				begin = end;
				continue;
			}
		} else if (end - begin == 2
			&& *begin == '.' && *(begin+1) == '.') {
			
			if (iter == root) {
				if (*end == 0)
					goto finish;
				else {
					begin = end;
					continue;
				}
			}
			irq_save(flags);
			prev_iter = iter;
			if (iter->parent == NULL) {
				iter = iter->sb->parent;
			} else {
				iter = iter->parent;
			}
			dref(iter);
			dput(prev_iter);
			irq_restore(flags);

			if (*end == 0)
				goto finish;
			else {
				begin = end;
				continue;
			}
		} else {
			irq_save(flags)
			prev_iter = iter;
			if (IS_DIR(iter->inode->mode)) {
				iter = find_entry(iter, begin, end-begin);
			} else {
				iter = NULL;
			}
			dput(prev_iter);
			irq_restore(flags);
			if (iter == NULL)
				return NULL;
			if (*end == 0)
				goto finish;
			else {
				begin = end;
				continue;
			}
		}
	}
finish:
	while (iter->mount != NULL) {
		prev_iter = iter->mount->root;
		dref(prev_iter);
		dput(iter);
		iter = prev_iter;
	}
	return iter;
}



