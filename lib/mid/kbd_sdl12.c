// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
#include <SDL/SDL_events.h>
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