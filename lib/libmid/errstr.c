#define _POSIX_C_SOURCE 200112L
#include "../../include/mid.h"
#include <SDL/SDL_error.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <stdio.h>
#include <errno.h>

enum { Bufsz = 256 };

const char *miderrstr(void){
	int err = errno;

	const char *e = SDL_GetError();
	if(e[0] != '\0')
		return e;

	e = IMG_GetError();
	if(e[0] != '\0')
		return e;

	e = TTF_GetError();
	if(e[0] != '\0')
		return e;

	static char str[Bufsz];
	if (strerror_r(err, str, Bufsz) != 0) {
		perror("strerror_r");
		fprintf(stderr, "perr failed\n");
		abort();
	}

	return str;
}
