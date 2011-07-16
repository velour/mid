// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
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

_Bool fsexists(const char *path);

#endif  // !_FS_H_
