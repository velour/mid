/* © 2013 the Mid Authors under the MIT license. See AUTHORS for the list of authors.*/

#include "../../include/mid.h"
#include <assert.h>

void framestart(void);
void framefinish(void);

enum { Stkmax = 8 };

struct Scrnstk{
	Scrn *scrns[Stkmax];
	Scrn **nxt;
	Gfx *g;
};

Scrnstk *scrnstknew(Gfx *g){
	Scrnstk *s = xalloc(1, sizeof(*s));
	s->nxt = s->scrns;
	s->g = g;
	return s;
}

void scrnstkfree(Scrnstk *stk){
	int n = stk->nxt - stk->scrns;
	for(int i = n-1; i >= 0; i--){
		Scrn *s = stk->scrns[i];
		s->mt->free(s);
	}
	xfree(stk);
}

void scrnstkpush(Scrnstk *stk, Scrn *s){
	assert(stk->nxt != &stk->scrns[Stkmax - 1]);

	if (stk->nxt != &stk->scrns[0])
		scrnstktop(stk)->cam = camget(stk->g);
	camreset(stk->g);
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

	camreset(stk->g);
	Point p = scrnstktop(stk)->cam;
	cammove(stk->g, p.x, p.y);
}

void scrnrun(Scrnstk *stk){
	for(;;){
		Scrn *s = scrnstktop(stk);
		if(!s)
			return;

		framestart();
		s->mt->update(s, stk);
		s->mt->draw(s, stk->g);

		Event e;
		while(pollevent(&e)){
			if(e.type == Quit)
				return;
			s->mt->handle(s, stk, &e);
		}

		framefinish();
	}
}
