#include <user/io.h>
#include <user/syscalls.h>
#include <sins/kernel.h>
#include <types.h>

void output_file(const char *path)
{
	int file;
	byte buf[1024];
	int ret;
	int i = 0;
	unsigned long addr = 0;

	file = open(path, 0);
	if (file < 0) {
		printf("open %s failed!\n", path);
	}
	while (1) {
		ret = read(file, buf, 1024);
		if (FAILED(ret))
			break;
		for (i = 0; i < ret/4; i++, addr+=4) {
			if (i % 4 == 0) {
				printf("%p:    ", addr);
			}
			printf("%08x  ", ((u32 *)buf)[i]);
			if (i % 4 == 3) {
				printf("\n");
			}
		}
		if (ret % 4) {
			if (i % 4 == 3)
				printf("%p: ", addr);
			printf("%08x\n", ((u32 *)buf)[i]);
		}
		if (i < 4)
			printf("\n");
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
