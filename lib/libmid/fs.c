#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

#include "fs.h"

const char DIRSEP = '/';

/* clang can't seem to find this on Ubuntu 10.10. */
extern int readdir_r(DIR *dirp, struct dirent *entry, struct dirent **result);

/* cat must be of size PATH_MAX + 1. */
void concat(const char *d, const char *f, char cat[])
{
	unsigned int dl = strlen(d);
	while (dl > 0 && d[dl - 1] == DIRSEP)
		dl -= 1;
	unsigned int fl = strlen(f);
	while (fl > 0 && f[0] == DIRSEP) {
		f += 1;
		fl -= 1;
	}
	if (dl + fl >= PATH_MAX) {
		fprintf(stderr, "Paths too long to concat\n");
		abort();
	}
	strncpy(cat, d, dl);
	if (dl > 0) {
		cat[dl] = DIRSEP;
		dl += 1;
	}
	strncpy(cat + dl, f, PATH_MAX - dl);
	cat[PATH_MAX] = '\0';
}

bool is_dir(const char *p)
{
	struct stat s;
	int err = stat(p, &s);
	if (err) {
		perror("stat");
		fprintf(stderr, "is_dir: stat failed on %s\n", p);
		abort();
	}
	return S_ISDIR(s.st_mode);
}

struct dirent *alloc_dent(const char *root)
{
	unsigned int len = offsetof(struct dirent, d_name)
		+ pathconf(root, _PC_NAME_MAX) + 1;
	return malloc(len);
}

/* Recursively find the given file beneath the given root.  out must
 * be of size PATH_MAX + 1. */
bool find(const char *root, const char *fname, char out[])
{
	assert(is_dir(root));
	DIR *dir = opendir(root);
	struct dirent *dent = alloc_dent(root);
	struct dirent *res;
	int err;
	do {
		err = readdir_r(dir, dent, &res);
		if (err) {
			perror("readdir_r");
			fprintf(stderr, "find: readdir_r failed\n");
			abort();
		}
		if (!res)
			break;
		if (strcmp(dent->d_name, ".") == 0 || strcmp(dent->d_name, "..") == 0)
			continue;

		char ent[PATH_MAX + 1];
		concat(root, dent->d_name, ent);
		if (strcmp(fname, dent->d_name) == 0) {
			strncpy(out, ent, PATH_MAX + 1);
			free(dent);
			return true;
		} else if (is_dir(ent)) {
			if (find(ent, fname, out)) {
				free(dent);
				return true;
			}
		}
	} while (res);

	free(dent);
	return false;
}
