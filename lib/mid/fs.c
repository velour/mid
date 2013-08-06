/* © 2013 the Mid Authors under the MIT license. See AUTHORS for the list of authors.*/

#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include "fs.h"

const char DIRSEP = '/';

/* cat must be of size PATH_MAX + 1. */
void fscat(const char *d, const char *f, char cat[])
{
	unsigned int dl = strlen(d);
	while (dl > 0 && d[dl - 1] == DIRSEP)
		dl -= 1;
	unsigned int fl = strlen(f);
	while (fl > 0 && f[0] == DIRSEP) {
		f += 1;
		fl -= 1;
	}
	assert(dl + fl < PATH_MAX && "Paths too long to fscat\n");

	strncpy(cat, d, dl);
	if (dl > 0) {
		cat[dl] = DIRSEP;
		dl += 1;
	}
	strncpy(cat + dl, f, PATH_MAX - dl);
	cat[PATH_MAX] = '\0';
}

bool fsexists(const char *path)
{
	struct stat sb;
	return stat(path, &sb) == 0;
}

