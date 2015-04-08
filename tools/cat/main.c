#include <user/io.h>
#include <user/syscalls.h>
#include <sins/kernel.h>

void output_file(const char *path)
{
	int file;
	byte buf[1025];
	int ret;

	file = open(path, 0);
	if (file < 0) {
		printf("open %s failed!\n", path);
	}
	while (1) {
		ret = read(file, buf, 1024);
		if (FAILED(ret))
			break;
		buf[ret] = 0;
		printf("%s", buf);
	}
	close(file);
}

int main(int argc, const char *argv[])
{
	int i;

	if (argc <= 1)
		return -1;

	for (i = 1; i != argc; ++i) {
		output_file(argv[i]);	
	}	
	return 0;
}
