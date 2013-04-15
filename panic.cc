#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include "panic.h"

void
panic(const char *fmt, ...)
{
	va_list ap;

	fprintf(stderr, "FATAL: ");

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	fputc('\n', stderr);

	exit(1);
}
