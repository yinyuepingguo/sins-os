#include <sins/error.h>
#include <user/syscalls.h>
#include <string.h>
#include <user/io.h>
#include <ctype.h>
#include <sins/kernel.h>

/*
	I just made a tiny toy shell.
	If you have any idea, do it!
*/

static volatile int task_done = 0;
static volatile int run_block = 0;

static void sh_help()
{
	printf("It's a shell parser.You can use it by type"
		" 'COMMAND arg1 arg2 ...'\n");
}

static void sh_hello()
{
	printf("\n\tHello guy.It's a SINS program.\n"
		"\tYou can use it like bash(a UNIX world program)."
		"Type 'help' for some useful information.\n"
		"\t\t\t\t\t\t\t\tGOOD LUCK.\n");
}

static void sh_exit(int argc, const char *argv[])
{
	long res;
	char *endp;

	if (argc == 0)
		exit(0);
	else if (argc == 1) {
		res = simple_strtol(argv[0], &endp, 10);
		if (endp == argv[0] + strlen(argv[0]))
			exit(res);
		else
			printf("invalid argument '%s'\n", argv[0]);
	} else {
		printf("invalid arguments num.\n");
	}
}

static void sh_cd(int argc, const char *argv[])
{
	if (argc == 0) {
		printf("you need an argument.\n");
	} else if (argc != 1) {
		printf("invalid arguments num.\n");
	} else {
		if (FAILED(chdir(argv[0]))) {
			printf("change working directory failed.\n");
		}
	}
}

static void sh_kill(int argc, const char *argv[])
{
	long pid;
	long signal;
	char *endp;

	if (argc == 0) {
		printf("kill need at least one parameters");
	} else if (argc == 1) {
		pid = simple_strtol(argv[0], &endp, 10);
		if (endp == argv[0] + strlen(argv[0]) && pid >= 0) {
			pid = kill(pid, SIGKILL);
			if (pid < 0) {
				printf("signal send failed!\n");
			}
		} else
			printf("invalid argument '%s'\n", argv[0]);
	} else if (argc == 2) {
		pid = simple_strtol(argv[0], &endp, 10);
		if (endp != argv[0] + strlen(argv[0])) {
			printf("invalid argument '%s'\n", argv[0]);
			return;
		}
		if (pid < 0) {
			signal = pid;
			pid = simple_strtol(argv[1], &endp, 10);
			if (endp != argv[1] + strlen(argv[1]) || pid < 0) {
				printf("invalid argument '%s'\n", argv[1]);
			}
		} else {
			signal = simple_strtol(argv[1], &endp, 10);
			if (endp != argv[1] + strlen(argv[1]) || signal > 0) {
				printf("invalid argument '%s'\n", argv[1]);
			}
		}
		pid = kill(pid, -signal);
		if (pid < 0) {
			printf("signal send failed!\n");
		}
	}else {
		printf("invalid arguments num.\n");
	}
}

static void sh_root(int argc, const char *argv[])
{
	if (argc == 0) {
		printf("you need an argument.\n");
	} else if (argc != 1) {
		printf("invalid arguments num.\n");
	} else {
		if (FAILED(chroot(argv[0]))) {
			printf("change root directory failed.\n");
		}
	}
}

static void sh_exec(const char *command, int argc, const char *argv[], int noblock)
{
	unsigned long len = strlen(command);
	result_t pid;
	char exec_path[128];

	if (strlen("/bin/") + len + 1 >= 128) {
		printf("command:%s is too long.\n", command);
		return;
	}

	if (len >= 2 && command[0] == '.' && command[1] == '/') {
		if (len == 2) {
			printf("what you want to execute?\n");
			return;
		} else {
			sprintf(exec_path, "%s", command+2);
		}
	} else {
		sprintf(exec_path, "/bin/%s", command);
	}

	pid = exec(exec_path, argc, argv);	
	if (FAILED(pid)) {
		printf("execute %s failed.\n", exec_path);
	} else {
		if (!noblock) {
			wait_pid(pid);
			task_done--;
		}
	}
}

static char buf[1024];
static char arg_buf[4096];
static const char *arg_ptrs[1024];
static char path[128];
static unsigned long argc;

static result_t fetch_command()
{
	unsigned int i = 0;
	unsigned int start = 0;
	result_t ret;

	ret = getcwd(path, 128);
	if (FAILED(ret)) {
		printf("can't get current working directory.\n");
		exit(0);
	}
	printf("%s>", path);
continue_read:
	/* we use 1023 because buf[1023] must be zero */
	i = read(STDIN, (byte *)buf + start, 1023);
	start += i;
	if (start >= 1023) {
		printf("command is too large.(max size = 1024)\n");
		return -ERROR;
	}

	if (i < 0) {
		printf("read command from STDIN failed!\n");
		buf[0] = 0;
		return -ERROR;
	} else if (i == 0) {
		buf[0] = 0;
		return -ERROR;
	}

	if (buf[start-1] == '\\') {
		start--;
		goto continue_read;
	}
	buf[1023] = 0;
	return SUCCESS;
}

static void add_to_arg_buf(char *begin, char *end)
{
	if (begin != end) {
		char *dest = arg_buf + (begin - buf);

		strcpy(dest, begin);
		arg_ptrs[argc] = dest;
		argc++;
	}

}

static void _execute_command(const char *command, int argc, const char *argv[], int noblock)
{
	/* list built-in commands and external command */
	if (strcmp(command, "help") == 0) {
		sh_help();
	} else if (strcmp(command, "hello") == 0) {
		sh_hello();
	} else if (strcmp(command, "exit") == 0) {
		sh_exit(argc, argv);
	} else if (strcmp(command, "cd") == 0) {
		sh_cd(argc, argv);
	} else if (strcmp(command, "kill") == 0) {
		sh_kill(argc, argv);
	} else if (strcmp(command, "root") == 0) {
		sh_root(argc, argv);
	}else {
		/* run external command */
		sh_exec(command, argc, argv, noblock);
	}
}

static void execute_command()
{
	char *iter = buf;
	char *begin = buf;
	
	argc = 0;
	memset(arg_ptrs, 0, sizeof(arg_ptrs));
	while (*iter != 0) {
		if (isspace(*iter)) {
			*iter = 0;
			add_to_arg_buf(begin, iter);
			begin = iter+1;
		}
		iter++;
	}
	add_to_arg_buf(begin, iter);

	if (argc != 0) {
		if (strcmp(arg_ptrs[argc-1], "&") == 0) {
			_execute_command(arg_ptrs[0], argc-2, arg_ptrs+1, 1);
		} else {
			_execute_command(arg_ptrs[0], argc-1, arg_ptrs+1, 0);
		}
	}
}

static asmlinkage void sigchld_handler()
{
	task_done++;
}
static void setup_signal()
{
	sigmask(SIGCHLD, 0);
	sigaction(SIGCHLD, sigchld_handler);
}

int main(int argc, const char *argv[])
{
	int done;
	int i;

	sh_hello();
	setup_signal();

	while (1) {
		if (fetch_command() >= 0) {
			execute_command();
		}
		done = task_done;
		for (i = 0; i != done; ++i)
			wait_chld(NULL);
		task_done -= i;
	}

	return SUCCESS;
}
