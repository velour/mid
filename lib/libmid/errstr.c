#include "../../include/mid.h"
#include <SDL/SDL_error.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <errno.h>
#include <string.h>

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

	return strerror(err);
}
