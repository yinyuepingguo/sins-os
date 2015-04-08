#include <user/io.h>
#include <user/syscalls.h>
#include <sins/kernel.h>
#include <sins/sched.h>
#include <types.h>

static struct task_info buf[10];

result_t read_file(const char *path)
{
	int file;
	int ret;

	file = open(path, 0);
	if (file < 0) {
		printf("open %s failed%d!\n", path, file);
		return file;
	}
	ret = read(file, (byte *)buf, sizeof(buf));
	if (FAILED(ret))
		return ret;
	close(file);
	return ret;
}
void output(unsigned long count)
{
	unsigned long i;
	char *state;

	printf(" %-10s%-6s%-7s%-6s%-10s%-6s%-6s%-13s%-13s\n",
		"CMD", "PID", "STATE", "PPID", "PCMD",
		"SLICE", "FILES", "ROOT", "PWD");
	for (i = 0; i != count; ++i) {
		switch (buf[i].state) {
		case TASK_RUNNABLE:
			state = "Run";
			break;
		case TASK_WAIT:
			state = "Sleep";
			break;
		case TASK_ZOMBIE:
			state = "Zombie";
			break;
		case TASK_DEAD:
			state = "Dead";
			break;
		default:
			state = "?";
		}
		printf("%c%-10s%-6d%-7s%-6d%-10s%-6d%-6d%-13s%-13s\n",
			(buf[i].flags&TASK_KTHREAD? '*': ' '),
			buf[i].cmd, buf[i].pid, state, buf[i].ppid, buf[i].pcmd,
			buf[i].time_slice, buf[i].files, buf[i].root, buf[i].pwd);
	}
}

int main(int argc, const char *argv[])
{
	result_t ret;

	ret = read_file("/dev/proc");
	if (ret < 0)
		return ret;
	output(ret / sizeof(struct task_info));
	return 0;
}
