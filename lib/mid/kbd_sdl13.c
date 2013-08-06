#include <SDL/SDL_events.h>
#include <assert.h>
#include "../../include/mid.h"

_Bool keyrpt(SDL_Event *e){
	return e->key.repeat != 0;
}

static Uint8 *keystate;
static int nkeys;

_Bool iskeydown(Action act){
	if (!keystate)
		keystate = SDL_GetKeyboardState(&nkeys);
	int keysym = SDL_GetScancodeFromKey(kmap[act]);
	assert (keysym >= 0);
	assert (keysym < nkeys);
	return keystate[keysym];
}