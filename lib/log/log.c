/* © 2013 the Mid Authors under the MIT license. See AUTHORS for the list of authors.*/

#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>

enum { Bufsz = 256 };

static FILE *lfile;

int loginit(const char *fname){
	if(!fname){
		lfile = stderr;
		return 0;
	}

	lfile = fopen(fname, "w+");
	if(!lfile)
		return errno;
	return 0;
}

void logclose(void){
	if(lfile != stderr)
		fclose(lfile);
}

void prtime()
{
	time_t t = time(NULL);
	struct tm *tm = localtime(&t);
	if (!tm) {
		fprintf(stderr, "prtime: localtime() failed\n");
		return;
	}

	char buf[Bufsz];
	size_t ret = strftime(buf, Bufsz, "[%H:%M:%S]", tm);
	if (ret == 0) {
		fprintf(stderr,"prtime: strftime() failed for some reason...\n");
		return;
	}
	fprintf(lfile, "%s ", buf);
}


void prraw(const char *func, int err, const char *fmt, va_list ap)
{
	bool fname = func[0] != '\0';

	prtime();

	if (fname)
		fprintf(lfile, "%s: ", func);

	vfprintf(lfile, fmt, ap);

	if (err == 0)
		goto done;

	fprintf(lfile, "%s" , strerror(err));
done:
	fprintf(lfile, "\n");
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

void flushlog()
{
	fflush(lfile);
}
