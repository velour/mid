#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>

enum { LINE_MAX = 256 };

/* stderr is not constant so we can't use a static global here. */
#define lfile stderr

void prv(bool prtime, const char *fmt, va_list ap)
{
	if (prtime) {
		char str[LINE_MAX + 1];
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
		if (!strftime(str, LINE_MAX + 1, "[%T]", &tm)) {
			fprintf(stderr, "%s failed: strftime failed\n", __func__);
			abort();
		}
		fprintf(lfile, "%s", str);
	}
	vfprintf(lfile, fmt, ap);
}

void pr(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	prv(true, fmt, ap);
	va_end(ap);
	fprintf(lfile, "\n");
}

void prraw(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	prv(false, fmt, ap);
	va_end(ap);
	fprintf(lfile, "\n");
}

void prerr(int err, const char *fmt, ...)
{
	va_list ap;
	char str[LINE_MAX + 1];
	if (strerror_r(err, str, LINE_MAX + 1) == 0) {
		va_start(ap, fmt);
		prv(true, fmt, ap);
		va_end(ap);
		pr(false, "%s\n" , str);
	} else {
		perror("strerror_r");
		fprintf(stderr, "perr failed\n");
		abort();
	}
}
