#if !defined(_FS_H_)
#define _FS_H_

#include <limits.h>
/* ick. */
#if !defined(PATH_MAX)
#if !defined(_POSIX_PATH_MAX)
#define PATH_MAX 4096
#else
#define PATH_MAX _POSIX_PATH_MAX
#endif
#endif

/* Concatinate path names.  'cat' must be of size PATH_MAX + 1. */
void fscat(const char *d, const char *f, char cat[]);
_Bool isdir(const char *p);
/* Find a file in the subtree rooted by 'root'.  'out' is the
 * resulting file, it must be of size PATH_MAX + 1. */
_Bool fsfind(const char *root, const char *fname, char out[]);

#endif  // !_FS_H_
