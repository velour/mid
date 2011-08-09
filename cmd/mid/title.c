// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
#include "../../include/mid.h"
#include "game.h"
#include <stdlib.h>

typedef struct Tit Tit;
struct Tit{
	Img *title;
	Point titlepos;
};

static void update(Scrn*,Scrnstk*);
static void draw(Scrn*,Gfx*);
static void handle(Scrn*,Scrnstk*,Event*);
static void titfree(Scrn*);

static Scrnmt titmt = {
	update,
	draw,
	handle,
	titfree,
};

Scrn *titlescrnnew(Gfx *g){
	static Tit t = {0};
	static Scrn s = {0};

	t.title = resrcacq(imgs, "img/title.png", 0);
	if(!t.title)
		return NULL;

	t.titlepos = (Point){
		gfxdims(g).x / 2 - imgdims(t.title).x / 2,
		0
	};

	s.mt = &titmt;
	s.data = &t;
	return &s;
}

static void update(Scrn *s, Scrnstk *stk){
}

static void draw(Scrn *s, Gfx *g){
	gfxclear(g, (Color){ 240, 240, 240 });
	Tit *t = s->data;
	imgdraw(g, t->title, t->titlepos);
	gfxflip(g);
}

static Scrn gms = { &gamemt };

static void handle(Scrn *s, Scrnstk *stk, Event *e){
	if(e->type != Keychng || e->repeat)
		return;

	if(e->down && e->key == kmap[Mvinv]){
		Game *g = gamenew();
		gms.data = g;
		scrnstkpush(stk, &gms);
		return;
	}
}

static void titfree(Scrn *s){
}
