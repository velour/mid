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

#if !defined(LINE_MAX)
#if !defined(_POSIX_LINE_MAX)
#define LINE_MAX 4096
#else
#define LINE_MAX _POSIX_LINE_MAX
#endif
#endif

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

void pr(bool prtime, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	prv(prtime, fmt, ap);
	va_end(ap);
}

void prerr(bool prtime, int err, const char *fmt, ...)
{
	va_list ap;
	char str[LINE_MAX + 1];
	if (strerror_r(err, str, LINE_MAX + 1) == 0) {
		va_start(ap, fmt);
		prv(prtime, fmt, ap);
		va_end(ap);
		pr(false, "%s" , str);
	} else {
		perror("strerror_r");
		fprintf(stderr, "perr failed\n");
		abort();
	}
}
