// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
#include "../../include/mid.h"
#include "game.h"

typedef struct Gover Gover;
struct Gover{
	Player *p;
	Txt *txt;
};

static void update(Scrn *s, Scrnstk *stk);
static void draw(Scrn *s, Gfx *g);
static void handle(Scrn *s, Scrnstk *stk, Event *e);
static void goverfree(Scrn *s);
static char *praise(int);

static Scrnmt govermt = {
	update,
	draw,
	handle,
	goverfree
};

Scrn *goverscrnnew(Player *p){
	static Gover go = {0};
	static Scrn s = {0};

	go.p = p;

	Txtinfo ti = { 32, { 255, 255, 255 } };
	go.txt = resrcacq(txt, "txt/retganon.ttf", &ti);

	s.mt = &govermt;
	s.data = &go;
	return &s;
}

static void update(Scrn *s, Scrnstk *stk){
	// nothing
}

static void draw(Scrn *s, Gfx *g){
	Gover *go = s->data;

	camreset(g);
	gfxclear(g, (Color){ 200, 0, 0 });

	int m = go->p->money;
	enum { Bufsz = 256 };
	char buf[Bufsz];

	char *d = "You are dead!";
	snprintf(buf, Bufsz, "With %d %s gold for your family.", m, praise(m));

	Point dp = txtdims(go->txt, d);
	txtdraw(g, go->txt, (Point){1,1}, d);
	txtdraw(g, go->txt, (Point){1,dp.y}, buf);

	gfxflip(g);
}

static void handle(Scrn *s, Scrnstk *stk, Event *e){
	if(e->type != Keychng || !e->down || e->repeat || e->key != kmap[Mvinv])
		return;

	scrnstkpop(stk); // this
	scrnstkpop(stk); // the game
}

static void goverfree(Scrn *s){
}

static char *praise(int m){
	if(m < 10)
		return "worthless";
	if(m < 100)
		return "pitiful";
	if(m < 1000)
		return "acceptable";
	if(m < 10000)
		return "amazing";
	return "glorious";
}
