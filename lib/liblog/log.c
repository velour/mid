#define _POSIX_SOURCE
#define _POSIX_C_SOURCE 200112L
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>

enum { Bufsz = 256 };

/* stderr is not constant so we can't use a static global here. */
#define lfile stderr

void prtime()
{
	char str[Bufsz];
	struct tm tm;
	time_t t = time(NULL);
	if (t == ((time_t) -1)) {
		perror("time");
		fprintf(stderr, "%s failed: time failed\n", __func__);
		abort();
	}
	if (!localtime_r(&t, &tm)) {
		fprintf(stderr, "%s failed: localtime failed\n", __func__);
		abort();
	}
	if (!strftime(str, Bufsz - 1, "[%T]", &tm)) {
		fprintf(stderr, "%s failed: strftime failed\n", __func__);
		abort();
	}
	fprintf(lfile, "%s", str);
}


void prraw(const char *func, int err, const char *fmt, va_list ap)
{
	bool fname = func[0] != '\0';

	prtime();

	if (fname)
		fprintf(lfile, "%s: ", func);

	vfprintf(lfile, fmt, ap);

	if (err == 0)
		return;

	char str[Bufsz];
	if (strerror_r(err, str, Bufsz) == 0) {
		fprintf(lfile, "%s\n" , str);
	} else {
		perror("strerror_r");
		fprintf(stderr, "perr failed\n");
		abort();
	}
}

void pr(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	prraw("", 0, fmt, ap);
	va_end(ap);
}

void prerr(int err, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	prraw("", err, fmt, ap);
	va_end(ap);
}

void prfn(const char *func, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	prraw(func, 0, fmt, ap);
	va_end(ap);
}

void prfnerr(const char *func, int err, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	prraw(func, err, fmt, ap);
	va_end(ap);
}
