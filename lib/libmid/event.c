#include "../../include/mid.h"
#include <SDL/SDL_events.h>
#include <SDL/SDL_timer.h>

// This will probably never change in SDL, but just in case...
enum { assert_keychar_eq = 1/!!('a' == SDLK_a) };

static int prevtm = 0;

void framestart(void){
	prevtm = SDL_GetTicks();
}

void framefinish(void){
	int delay = prevtm + Ticktm - SDL_GetTicks();
	if(delay > 0)
		SDL_Delay(delay);
}

_Bool pollevent(Event *event){
	SDL_Event e;
	int p = SDL_PollEvent(&e);
	if(!p)
		return 0;

	switch(e.type){
	case SDL_QUIT:
		event->type = Quit;
		return 1;
	case SDL_KEYDOWN:
	case SDL_KEYUP:
		event->type = Keychng;
		event->down = e.type == SDL_KEYDOWN;
		event->repeat = e.key.repeat != 0;
		event->key = e.key.keysym.sym;
		return 1;
	case SDL_MOUSEMOTION:
		event->type = Mousemv;
		event->x = e.motion.x;
		event->y = e.motion.y;
		event->dx = e.motion.xrel;
		event->dy = e.motion.yrel;
		return 1;
	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP:
		event->type = Mousebt;
		event->down = e.type == SDL_MOUSEBUTTONDOWN;
		event->x = e.button.x;
		event->y = e.button.y;
		event->butt = e.button.button;
		return 1;
	default:
		return 0;
	}
}
