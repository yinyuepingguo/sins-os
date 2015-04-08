#include <user/io.h>
#include <user/syscalls.h>
#include <sins/kernel.h>
#include <string.h>

void count_file(const char *path, char line, char name)
{
	int file;
	byte buf[1025];
	int count = 0;
	int ret;
	int i;

	file = open(path, 0);
	if (file < 0) {
		printf("open %s failed!\n", path);
	}
	while (1) {
		ret = read(file, buf, 1024);
		if (FAILED(ret))
			break;
		if (line) {
			for (i = 0; i != ret; ++i) {
				if (buf[i] == '\n')
					count++;
			}
		} else
			count += ret;
	}
	printf("%s%s%d\n", (name? path:""), (name? ":": ""), count);
	close(file);
}

int main(int argc, const char *argv[])
{
	int i;
	char out_line = 0;
	char out_name = 0;
	int count;

	for (i = 1; i < argc; ++i) {
		if (strcmp("-l", argv[i]) == 0) {
			out_line = 1;
		} else {
			count++;
		}
	}

	if (count == 0) {
		printf("It need at least one file.");
		return -1;
	}
	if (count > 1)
		out_name = 1;

	for (i = 1; i != argc; ++i) {
		if (strcmp("-l", argv[i]) != 0) {
			count_file(argv[i], out_line, out_name);
		}
	}

	return 0;
}
