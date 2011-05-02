#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../include/mid.h"

void test_fscat()
{
	char c[PATH_MAX + 1];
	char *d, *f, *res = "directory/file";

	d = "directory";
	f = "file";
	fscat(d, f, c);
	if (strcmp(res, c) != 0) {
		fprintf(stderr, "%s + %s = %s\n", d, f, c);
		fprintf(stderr, "Invalid fscatination\n");
		abort();
	}

	d = "directory/";
	f = "file";
	fscat(d, f, c);
	if (strcmp(res, c) != 0) {
		fprintf(stderr, "%s + %s = %s\n", d, f, c);
		fprintf(stderr, "Invalid fscatination\n");
		abort();
	}

	d = "directory";
	f = "/file";
	fscat(d, f, c);
	if (strcmp(res, c) != 0) {
		fprintf(stderr, "%s + %s = %s\n", d, f, c);
		fprintf(stderr, "Invalid fscatination\n");
		abort();
	}

	d = "directory///";
	f = "file";
	fscat(d, f, c);
	if (strcmp(res, c) != 0) {
		fprintf(stderr, "%s + %s = %s\n", d, f, c);
		fprintf(stderr, "Invalid fscatination\n");
		abort();
	}

	d = "directory";
	f = "///file";
	fscat(d, f, c);
	if (strcmp(res, c) != 0) {
		fprintf(stderr, "%s + %s = %s\n", d, f, c);
		fprintf(stderr, "Invalid fscatination\n");
		abort();
	}

	d = "directory///";
	f = "///file";
	fscat(d, f, c);
	if (strcmp(res, c) != 0) {
		fprintf(stderr, "%s + %s = %s\n", d, f, c);
		fprintf(stderr, "Invalid fscatination\n");
		abort();
	}
}

void touch(const char *p)
{
	FILE *f = fopen(p, "w+");
	fclose(f);
}

void test_fsfind()
{
	mkdir("a", 0777);
	mkdir("a/b", 0777);
	mkdir("a/b/c", 0777);
	touch("a/afile");
	touch("a/b/bfile");
	touch("a/b/c/cfile");

	char buf[PATH_MAX + 1];
	if (!fsfind("a", "afile", buf)) {
		fprintf(stderr, "afile not found\n");
		abort();
	}
	if (strcmp(buf, "a/afile") != 0) {
		fprintf(stderr, "afile found at %s\n", buf);
		abort();
	}
	if (!fsfind("a", "bfile", buf)) {
		fprintf(stderr, "bfile not found\n");
		abort();
	}
	if (strcmp(buf, "a/b/bfile") != 0) {
		fprintf(stderr, "bfile found at %s\n", buf);
		abort();
	}
	if (!fsfind("a", "cfile", buf)) {
		fprintf(stderr, "cfile not found\n");
		abort();
	}
	if (strcmp(buf, "a/b/c/cfile") != 0) {
		fprintf(stderr, "cfile found at %s\n", buf);
		abort();
	}
	if (fsfind("a", "dfile", buf)) {
		fprintf(stderr, "dfile was found\n");
		abort();
	}

	unlink("a/afile");
	unlink("a/b/bfile");
	unlink("a/b/c/cfile");
	rmdir("a/b/c");
	rmdir("a/b");
	rmdir("a");
}

int main()
{
	test_fscat();
	test_fsfind();
	return EXIT_SUCCESS;
}
