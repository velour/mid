#include "../../include/mid.h"
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
	if (!s)
		return NULL;
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
	if (stk->cur == &stk->scrns[Stkmax - 1])
		abort();
	stk->cur++;
	*(stk->cur) = s;
}

Scrn *scrnstktop(Scrnstk *s){
	return *(s->cur);
}

void scrnstkpop(Scrnstk *stk){
	if (stk->cur < &stk->scrns[0])
		abort();
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

		Event e;
		while(pollevent(&e)){
			if(e.type == Quit)
				break;
			s->mt->handle(s, stk, &e);
		}

		framefinish();
	}
}
