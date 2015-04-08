#include <user/io.h>
#include <user/syscalls.h>

int main(int argc, const char *argv[])
{
	struct tm now;

	time(&now);
	printf("%04d-%02d-%02d %02d:%02d:%02d\n",
		now.year, now.mon+1, now.mday,
		now.hour, now.min, now.sec);
	return 0;
}
