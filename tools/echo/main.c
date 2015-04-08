#include <user/io.h>

int main(int argc, const char *argv[])
{
	int i;

	for (i = 1; i < argc; ++i) {
		printf("%s%c", argv[i], (i != (argc-1)? ' ': '\n'));
	}
	return 0;
}
