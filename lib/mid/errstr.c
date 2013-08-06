/* © 2013 the Mid Authors under the MIT license. See AUTHORS for the list of authors.*/

#include "../../include/mid.h"
#include <SDL/SDL_error.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>

enum { Bufsz = 1024 };

static char curerr[Bufsz];

void seterrstr(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(curerr, Bufsz, fmt, ap);
	va_end(ap);
}

const char *miderrstr(void){
	int err = errno;

	if (curerr[0] != '\0') {
		static char retbuf[Bufsz];
		strncpy(retbuf, curerr, Bufsz - 1);
		retbuf[Bufsz - 1] = '\0';
		curerr[0] = '\0';
		return retbuf;
	}

	const char *e = SDL_GetError();
	if(e[0] != '\0')
		return e;

	return strerror(err);
}
