#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../lib/libmid/fs.h"
#include "../../include/mid.h"

/* To deal with communication between test/load/free functions. */
char *loaded;
char freed[PATH_MAX + 1];;

char *strdup(const char *s)
{
	char *n = calloc(strlen(s), sizeof(*s));
	strcpy(n, s);
	return n;
}

void touch(const char *p)
{
	FILE *f = fopen(p, "w+");
	fclose(f);
}

void *test_load_load(const char *p, void *ignore)
{
	loaded = strdup(p);
	return loaded;
}

void test_load_free(void *p, void *_info)
{
	strncpy(freed, p, PATH_MAX + 1);
	freed[PATH_MAX] = '\0';
	free(p);
}


void touchn(int n)
{
	char buf[PATH_MAX + 1];
	for (unsigned int i = 0; i < n; i += 1) {
		snprintf(buf, PATH_MAX+1, "resrc/file%d", i);
		touch(buf);
	}
}

void unlinkn(int n)
{
	char buf[PATH_MAX + 1];
	for (unsigned int i = 0; i < n; i += 1) {
		snprintf(buf, PATH_MAX+1, "resrc/file%d", i);
		unlink(buf);
	}
}

int main()
{
	Rcache *c;
	static const int n = 101;

	mkdir("resrc", 0777);
	touchn(n);

	c = rcachenew(test_load_load, test_load_free, NULL, NULL);

	char buf[PATH_MAX + 1];
	for (unsigned int i = 0; i < n - 1; i += 1) {
		snprintf(buf, PATH_MAX+1, "file%d", i);
		loaded = NULL;
		freed[0] = '\0';
		char *r = resrc(c, buf, NULL);
		if (!loaded) {
			fprintf(stderr, "%d: Didn't load %s ", i, buf);
			fprintf(stderr, "got %s instead\n", r);
			abort();
		}
		if (freed[0] != '\0') {
			fprintf(stderr, "%d: Freed %s\n", i, freed);
			abort();
		}
	}
	strcpy(buf, "file100");
	loaded = NULL;
	freed[0] = '\0';
	char *r = resrc(c, buf, NULL);
	if (!loaded) {
		fprintf(stderr, "Didn't load %s ", buf);
		fprintf(stderr, "got %s instead\n", r);
		abort();
	}
	if (freed[0] == '\0') {
		fprintf(stderr, "Didn't free anything\n");
		abort();
	}
	unlinkn(n);
	rmdir("resrc");

	rcachefree(c);
	return EXIT_SUCCESS;
}
