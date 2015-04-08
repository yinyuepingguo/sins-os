#include <sins/fs.h>
#include <sins/mm.h>
#include <sins/kernel.h>
#include <sins/error.h>
#include <sins/syscalls.h>
#include <sins/sched.h>
#include <string.h>
#include <sins/processor.h>

struct exec_wrapper_params {
	wait_queue_t wait;
	int file;
	result_t ret;
	unsigned long argc;
	void *arg_page;
};

static result_t copy_argv(const char *filename, char *dest, unsigned long size,
	unsigned long argc, const char *__user argv[])
{
	int total_size = 0;
	int start = 0;
	int i = 0;

	total_size += strlen(filename);
	if (total_size >= size)
		return -ERROR;
	memcpy(dest + start, filename, total_size - start);
	dest[++total_size] = 0;

	for (i = 0; i != argc; ++i) {
		if (argv[i] == NULL)
			return -ERROR;
		start = total_size;
		total_size += strlen(argv[i]);
		if (total_size+1 >= size)
			return -ERROR;
		memcpy(dest+start, argv[i], total_size-start);
		dest[++total_size] = 0;
	}
	return SUCCESS;
}

static result_t write_program_to_user(struct exec_wrapper_params *params)
{
	struct page *pg;
	long load_count;
	void *user_addr = 0;
	result_t read_count;
	
	for (load_count = 0; ; ++load_count) {
		BUG_ON(((unsigned long)load_count * PAGE_SIZE) >= PAGE_OFFSET);

		pg = load_page(current->page_dir,
				load_count * PAGE_SIZE);
		user_addr = (void *)(load_count * PAGE_SIZE);
		if (pg == NULL) {
			params->ret = -ERROR;
			break;
		}
		read_count = sys_read(params->file, user_addr, PAGE_SIZE);
		if (read_count > 0) {
			memset(user_addr + read_count, 0, PAGE_SIZE - read_count);
			/* check exec magic */
			if (load_count == 0 && (read_count < 8 ||
				*(unsigned long *)0x04 != SINS_EXEC_MAGIC)) {
				params->ret = -ENOEXEC;
				break;
			}
		} else if (read_count == -EIO) {
			params->ret = SUCCESS;
			break;	
		} else {
			params->ret = -ERROR;
			break;
		}
	}
	return params->ret;
}

static result_t write_argv_to_user(struct exec_wrapper_params *params)
{
	struct page *argc_pg, *argv_pg;
	char *user_addr;
	char *iter;
	long count = 0;
	long pos;
	
	BUG_ON(params->argc == 0);

	argv_pg = load_page(current->page_dir, PAGE_OFFSET - 2*PAGE_SIZE);
	if (argv_pg == NULL) {
		params->ret = -ERROR;
		return -ERROR;
	}
	argc_pg = load_page(current->page_dir, PAGE_OFFSET - PAGE_SIZE);
	if (argc_pg == NULL) {
		params->ret = -ERROR;
		return -ERROR;
	}
	user_addr = (char *)(PAGE_OFFSET - 2*PAGE_SIZE);
	memcpy(user_addr, params->arg_page, PAGE_SIZE);
	iter = (char *)(PAGE_OFFSET - PAGE_SIZE);
	pos = 0;
	while (count != params->argc) {
		*(unsigned long *)iter = (unsigned long)(user_addr + pos);
		iter += sizeof(unsigned long);
		pos += strlen(user_addr + pos)+1;
		++count;
	}

	params->ret = SUCCESS;
	return SUCCESS;
}

static result_t create_user_stack(struct exec_wrapper_params *params)
{
	struct page *pg;

	pg = load_page(current->page_dir, PAGE_OFFSET - 3*PAGE_SIZE);
	if (pg == NULL) {
		params->ret = -ERROR;
		return -ERROR;
	}
	memset((void *)(PAGE_OFFSET-3*PAGE_SIZE), 0, PAGE_SIZE);
	params->ret = SUCCESS;
	return SUCCESS;
}

static int exec_wrapper(unsigned long data)
{
	struct exec_wrapper_params *params;
	int ok = 0;

	params = (struct exec_wrapper_params *)data;
	if (write_program_to_user(params) >= 0) {
		if (write_argv_to_user(params) >= 0) {
			ok = 1;
		}
	}
	if (ok == 1) {
		if (FAILED(create_user_stack(params)))
			ok = 0;
	}
	/* we clear allocated pages in 'exit' system call */
	sys_close(params->file);

	/*
		we want sys_exec can return immediately,
		so we pass run result early.
	*/
	wake_up(&params->wait);
	if (ok) {
		/* we must clear TASK_KTRHEAD flag. */
		current->flags &= ~TASK_KTHREAD;
		/* pass argc, argv which can be accessed by user mode */
		move_to_user_mode(0, PAGE_OFFSET - 2*PAGE_SIZE, params->argc,
			(const char **)(PAGE_OFFSET - PAGE_SIZE));
	}
	/* never go here */
	return -ERROR;


}

static void init_task_cmd(char cmd[], const char *filename,
	unsigned long argc, const char *argv[])
{
	int count = 0;
	int i;

	strncpy(cmd, filename, TASK_CMD_LENGTH);
	for (i = 0; i != argc; ++i) {
		count = strnlen(cmd, TASK_CMD_LENGTH);
		if (count >= TASK_CMD_LENGTH)
			break;
		strncpy(cmd + count, argv[i], TASK_CMD_LENGTH-count);
	}
	cmd[TASK_CMD_LENGTH] = 0;
}

/* argc must less than 1024 now. and argv's size need to less than PAGE_SIZE */
asmlinkage int sys_exec(const char *__user filename,
	unsigned long argc, const char *__user argv[])
{
	int file;
	struct exec_wrapper_params params;
	unsigned long flags;
	int pid;
	void *arg_page;
	int ret;
	char cmd[TASK_CMD_LENGTH];

	file = sys_open(filename, O_RDONLY);
	if (FAILED(file))
		return file;

	if (argc > PAGE_SIZE/sizeof(const char *)) {
		ret = -ERROR;
		goto release_file;
	}

	arg_page = get_zeroed_page(GFP_NORMAL | GFP_DMA);
	if (arg_page) {
		if (FAILED(copy_argv(filename, arg_page, PAGE_SIZE, argc, argv)))
		{
			ret = -ERROR;
			goto release_page;
		}
	} else {
		ret = -ENOMEM;
		goto release_file;
	}

	INIT_WAIT_QUEUE(&params.wait);
	params.file = file;
	/* kernel add path to argv[0], so argc++ */
	params.argc = argc + 1;
	params.arg_page = arg_page;

	init_task_cmd(cmd, filename, argc, argv);
	/* promise wait called before wake_up */
	irq_save(flags);
	pid = kernel_thread(filename ,exec_wrapper, (unsigned long)&params);
	if (FAILED(pid)) {
		ret = pid;
		goto restore_flags;
	}
	wait(&params.wait);
	if (FAILED(params.ret)) {
		if (params.ret == -ENOEXEC)
			printk("%s is not a executable file.", filename);
		sys_wait_pid(pid);
		ret = params.ret;
		goto restore_flags;
	}
	ret = pid;
restore_flags:
	irq_restore(flags);
release_page:
	free_page(arg_page);
release_file:
	sys_close(file);
	return ret;
}
