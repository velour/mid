#include "../../include/mid.h"
#include "game.h"
#include <stdlib.h>

typedef struct Inv Inv;
struct Inv{
	Player *p;
	Lvl *lvl;
};

static void update(Scrn*,Scrnstk*);
static void draw(Scrn*,Gfx*);
static void handle(Scrn*,Scrnstk*,Event*);
static void invfree(Scrn*);

static Scrnmt invmt = {
	update,
	draw,
	handle,
	invfree,
};

Scrn *invscrnnew(Player *p, Lvl *lvl){
	Inv *inv = malloc(sizeof(*inv));
	if(!inv)
		return NULL;
	inv->p = p;
	inv->lvl = lvl;

	Scrn *s = malloc(sizeof(*s));
	if(!s)
		return NULL;
	s->mt = &invmt;
	s->data = inv;
	return s;
}

static void update(Scrn *s, Scrnstk *stk){
}

static void draw(Scrn *s, Gfx *g){
	gfxclear(g, (Color){ 127, 255, 127 });
	Inv *i = s->data;
	lvlminidraw(g, i->lvl, 0, (Point){0,0});
	gfxflip(g);
}

static void handle(Scrn *s, Scrnstk *stk, Event *e){
	if(e->type != Keychng || e->repeat)
		return;

	if(e->down && e->key == kmap[Mvinv]){
		scrnstkpop(stk);
		return;
	}
}

static void invfree(Scrn *s){
	free(s->data);
	free(s);
}
