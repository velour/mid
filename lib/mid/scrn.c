// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
#include "../../include/mid.h"
#include <assert.h>

void framestart(void);
void framefinish(void);

enum { Stkmax = 8 };

struct Scrnstk{
	Scrn *scrns[Stkmax];
	Scrn **nxt;
};

Scrnstk *scrnstknew(void){
	Scrnstk *s = xalloc(1, sizeof(*s));
	s->nxt = s->scrns;
	return s;
}

void scrnstkfree(Scrnstk *stk){
	int n = stk->nxt - stk->scrns;
	for(int i = 0; i < n; ++i){
		Scrn *s = stk->scrns[i];
		s->mt->free(s);
	}
	xfree(stk);
}

void scrnstkpush(Scrnstk *stk, Scrn *s){
	assert(stk->nxt != &stk->scrns[Stkmax - 1]);

	*(stk->nxt) = s;
	stk->nxt++;
}

Scrn *scrnstktop(Scrnstk *s){
	return s->nxt[-1];
}

void scrnstkpop(Scrnstk *stk){
	assert(stk->nxt >= &stk->scrns[0]);

	Scrn *s = stk->nxt[-1];
	s->mt->free(s);
	stk->nxt--;
}

void scrnrun(Scrnstk *stk, Gfx *g){
	for(;;){
		Scrn *s = scrnstktop(stk);
		if(!s)
			return;

		framestart();
		s->mt->update(s, stk);
		s->mt->draw(s, g);

		Event e;
		while(pollevent(&e)){
			if(e.type == Quit)
				return;
			s->mt->handle(s, stk, &e);
		}

		framefinish();
	}
}
