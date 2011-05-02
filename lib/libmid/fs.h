#include <limits.h>
#include <stdbool.h>

#if !defined(PATH_MAX)
#define PATH_MAX 4096
#endif

/* Concatinate path names.  'cat' must be of size PATH_MAX + 1. */
void concat(const char *d, const char *f, char cat[]);

bool is_dir(const char *p);

/* Find a file in the subtree rooted by 'root'.  'out' is the
 * resulting file, it must be of size PATH_MAX + 1. */
bool find(const char *root, const char *fname, char out[]);
