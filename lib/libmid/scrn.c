#include "../../include/mid.h"
#include <stdlib.h>

void framestart(void);
void framefinish(void);

enum { Stkmax = 8 };

struct Scrnstk{
	Scrn *scrns[Stkmax];
	Scrn **nxt;
};

Scrnstk *scrnstknew(void){
	Scrnstk *s = calloc(1, sizeof(*s));
	if (!s)
		return NULL;
	s->nxt = s->scrns;
	return s;
}

void scrnstkfree(Scrnstk *stk){
	int n = stk->nxt - stk->scrns;
	for(int i = 0; i < n; ++i){
		Scrn *s = stk->scrns[i];
		s->mt->free(s);
	}
	free(stk);
}

void scrnstkpush(Scrnstk *stk, Scrn *s){
	if (stk->nxt == &stk->scrns[Stkmax - 1])
		abort();
	*(stk->nxt) = s;
	stk->nxt++;
}

Scrn *scrnstktop(Scrnstk *s){
	return s->nxt[-1];
}

void scrnstkpop(Scrnstk *stk){
	if (stk->nxt < &stk->scrns[0])
		abort();
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
