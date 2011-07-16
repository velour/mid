// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "../../lib/libmid/fs.h"

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

int main()
{
	test_fscat();
	return EXIT_SUCCESS;
}
