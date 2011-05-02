#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "resrc.h"

/* To deal with communication between test/load/free functions. */
bool loaded, freed;

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
	loaded = true;
	return strdup(p);
}

void test_load_free(void *p)
{
	freed = true;
	printf("freeing %s\n", p);
	free(p);
}

void test_load()
{
	struct rcache c;

	mkdir("a", 0777);
	touch("a/afile");

	rcache(&c, test_load_load, test_load_free);

	loaded = false;
	char *p0 = resrc(&c, "afile");
	loaded = false;
	char *p1 = resrc(&c, "afile");
	if (loaded) {
		fprintf(stderr, "Failed to find %s in the cache\n", p0);
		abort();
	}
	if (p0 != p1) {
		fprintf(stderr, "Found a different value in the cache\n");
		abort();
	}

	unlink("a/afile");
	rmdir("a");
}

void touchn(int n)
{
	char buf[PATH_MAX + 1];
	for (unsigned int i = 0; i < n; i += 1) {
		snprintf(buf, PATH_MAX+1, "a/file%d", i);
		touch(buf);
	}
}

void unlinkn(int n)
{
	char buf[PATH_MAX + 1];
	for (unsigned int i = 0; i < n; i += 1) {
		snprintf(buf, PATH_MAX+1, "a/file%d", i);
		unlink(buf);
	}
}

void test_bump()
{
	struct rcache c;

	mkdir("a", 0777);
	touchn(101);

	rcache(&c, test_load_load, test_load_free);

	loaded = false;
	resrc(&c, "file0");
	if (!loaded) {
		fprintf(stderr, "Didn't load file0\n");
		abort();
	}
	freed = loaded = false;
	resrc(&c, "file1");
	if (!loaded) {
		fprintf(stderr, "Didn't load file1\n");
		abort();
	}
	if (!freed) {
		fprintf(stderr, "Didn't free file0\n");
		abort();
	}
	loaded = false;
	resrc(&c, "file0");
	if (!loaded) {
		fprintf(stderr, "Didn't load file0\n");
		abort();
	}
	loaded = false;
	resrc(&c, "file0");
	if (loaded) {
		fprintf(stderr, "Loaded file0 again\n");
		abort();
	}
	freed = loaded = false;
	resrc(&c, "file1");
	if (!loaded) {
		fprintf(stderr, "Didn't load file1\n");
		abort();
	}
	if (!freed) {
		fprintf(stderr, "Didn't free file0\n");
		abort();
	}

	unlinkn(101);
	rmdir("a");
}

int main()
{
	test_load();
	test_bump();
	return EXIT_SUCCESS;
}
