#include <user/syscalls.h>
#include <user/io.h>
#include <string.h>

static char buf[4096];

int printf(const char *fmt, ...)
{
	va_list args;
	int i;
	
	va_start(args, fmt);
	i = vsprintf(buf, fmt, args);
	va_end(args);
	if (i > 0)
		write(STDOUT, (byte *)buf, i);
	return i;
}

int scanf(const char *fmt, ...)
{
	va_list args;
	int i;

	i = read(STDIN, (byte *)buf, 4095);
	if (i < 0)
		return i;
	buf[i] = 0;
	va_start(args, fmt);
	i = vsscanf(buf, fmt, args);
	va_end(args);
	return i;
}
