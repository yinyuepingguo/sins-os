#include <user/io.h>
#include <user/syscalls.h>
#include <sins/error.h>
#include <string.h>
#include <sins/kernel.h>
#include <sins/fs.h>

static struct dir buf[128];
static char name_buf[MAX_NAME_LEN+1];

static void print_dir(const char *path, char multipath)
{
	int i = 0;
	result_t ret = 0;
	int file;

	file = open(path, 0);
	if (file < 0) {
		printf("open %s failed!\n", path);
		return;
	}

	if (multipath)
		printf("%s:\n", path);

	while (1) {
		ret = readdir(file, buf, array_size(buf));
		if (ret == -EIO)
			break;
		else if (ret < 0) {
			printf("read %s failed!", path);
			break;
		}
		for (i = 0; i != ret; ++i) {
			strncpy(name_buf, buf[i].name, MAX_NAME_LEN);
			name_buf[MAX_NAME_LEN] = 0;
			if (i == ret-1)
				printf("%s", name_buf);
			else	
				printf("%s \t", name_buf);
		}
		/* we don't need to read again */
		if (ret < array_size(buf))
			break;
	}
	close(file);

}

/* Just a simple ls implement. Support Minix directory(bad system call!) */
int main(int argc, const char *argv[])
{
	char path_buf[128];
	int i;

	if (argc == 1) {
		if (getcwd(path_buf, 128) >= 0) {
			print_dir(path_buf, 0);
			printf("\n");
		}
	}

	for (i = 1; i < argc; ++i) {
		print_dir(argv[i], argc>=3? 1: 0);
		printf("\n");
	}
	return 0;
}
