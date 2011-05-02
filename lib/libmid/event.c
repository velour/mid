#include "../../include/mid.h"
#include <SDL/SDL.h>

static int prevtm = 0;

void framestart(void){
	prevtm = SDL_GetTicks();
}

void framefinish(void){
	int delay = prevtm + Ticktm - SDL_GetTicks();
	if(delay > 0)
		SDL_Delay(delay);
}
