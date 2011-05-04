#include "../../include/mid.h"
#include <SDL/SDL_error.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>

const char *miderrstr(void){
	const char *e = SDL_GetError();
	if(e[0] != '\0')
		return e;

	e = IMG_GetError();
	if(e[0] != '\0')
		return e;

	e = TTF_GetError();
	if(e[0] != '\0')
		return e;

	return "";
}
