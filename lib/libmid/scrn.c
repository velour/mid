#include "../../include/mid.h"
#include <SDL/SDL.h>
#include <stdlib.h>

void framestart(void);
void framefinish(void);

enum { Stkmax = 8 };

struct Scrnstk{
	Scrn *scrns[Stkmax];
	Scrn **cur;
};

Scrnstk *scrnstknew(void){
	Scrnstk *s = calloc(1, sizeof(*s));
	s->cur = s->scrns;
	return s;
}

void scrnstkfree(Scrnstk *stk){
	int n = stk->cur - stk->scrns;
	for(int i = 0; i < n; ++i){
		Scrn *s = stk->scrns[i];
		s->mt->free(s);
	}
	free(stk);
}

void scrnstkpush(Scrnstk *stk, Scrn *s){
	stk->cur++;
	*(stk->cur) = s;
}

Scrn *scrnstktop(Scrnstk *s){
	return *(s->cur);
}

void scrnstkpop(Scrnstk *stk){
	Scrn *s = *(stk->cur);
	s->mt->free(s);
	stk->cur--;
}

void scrnrun(Scrnstk *stk, Gfx *g){
	for(;;){
		Scrn *s = scrnstktop(stk);
		if(!s)
			return;

		framestart();
		s->mt->update(s, stk);
		s->mt->draw(s, g);

		SDL_Event e;
		while(SDL_PollEvent(&e)){
			if(e.type == SDL_QUIT)
				break;
			s->mt->handle(s, stk, &e);
		}

		framefinish();
	}
}
