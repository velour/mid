/* © 2013 the Mid Authors under the MIT license. See AUTHORS for the list of authors.*/

#include <SDL_events.h>
#include <assert.h>
#include "../../include/mid.h"

_Bool keyrpt(SDL_Event *e){
	return 0;
}

static Uint8 *keystate;
static int nkeys;

_Bool iskeydown(Action act) {
	if (!keystate)
		keystate = SDL_GetKeyState(&nkeys);
	int keysym = kmap[act];
	assert (keysym >= 0);
	assert (keysym < nkeys);
	return keystate[keysym];
}
