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

void *test_load_load(const char *p)
{
	printf("loading %s\n", p);
	loaded = strdup(p);
	return loaded;
}

void test_load_free(void *p)
{
	strncpy(freed, p, PATH_MAX + 1);
	freed[PATH_MAX] = '\0';
	printf("freeing %s\n", p);
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

	c = rcachenew(test_load_load, test_load_free);

	char buf[PATH_MAX + 1];
	for (unsigned int i = 0; i < n - 1; i += 1) {
		snprintf(buf, PATH_MAX+1, "file%d", i);
		loaded = NULL;
		freed[0] = '\0';
		resrc(c, buf);
		if (!loaded) {
			fprintf(stderr, "%d: Didn't load %s\n", i, buf);
			abort();
		}
		if (freed[0] != '\0') {
			fprintf(stderr, "%d: Freed %s\n", i, freed);
			abort();
		}
	}
	unlinkn(n);
	rmdir("resrc");

	rcachefree(c);
	return EXIT_SUCCESS;
}
