#include "../../include/mid.h"
#include "game.h"
#include <stdlib.h>

typedef struct Invscr Invscr;
struct Invscr{
	Inv *inv;
	Lvl *lvl;
	int z;
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

Scrn *invscrnnew(Inv *i, Lvl *lvl, int z){
	Invscr *inv = malloc(sizeof(*inv));
	if(!inv)
		return NULL;
	inv->inv = i;
	inv->lvl = lvl;
	inv->z = z;

	Scrn *s = malloc(sizeof(*s));
	if(!s)
		return NULL;
	s->mt = &invmt;
	s->data = inv;
	return s;
}

static void update(Scrn *s, Scrnstk *stk){
	Invscr *i = s->data;
	invupdate(i->inv);
}

static void draw(Scrn *s, Gfx *g){
	gfxclear(g, (Color){ 127, 255, 127 });
	Invscr *i = s->data;
	lvlminidraw(g, i->lvl, i->z, (Point){0,0});
	invdraw(g, i->inv);
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
