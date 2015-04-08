#include <sins/kernel.h>
#include <sins/init.h>
#include <sins/fs.h>

static struct file_operations minix_fops = {
	.read	= minix_file_read,
	.readdir= minix_file_readdir,
	.write	= minix_file_write
};

static struct super_operations minix_sops = {
	.lookup		= minix_lookup,
	.read_inode 	= minix_read_inode,
	.write_inode 	= minix_write_inode
};


static struct file_system_type file_system = {
	.name		= "minix",
	.get_super	= minix_get_super,
	.kill_super	= minix_kill_super,
	.sops		= &minix_sops,
	.fops		= &minix_fops
};

result_t minix_fs_setup()
{
	result_t ret;

	ret = register_filesystem(&file_system);
	return ret;
}

fs_initcall(minix_fs_setup);
