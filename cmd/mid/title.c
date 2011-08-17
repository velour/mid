// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
#include "../../include/mid.h"
#include "game.h"
#include <stdlib.h>

typedef struct Tit Tit;
struct Tit{
	Img *title;
	Img *start;
	Img *opts;
	Img *copy;
	Point titlepos;
	Point startpos;
	Point optspos;
	Point copypos;
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

	Txtinfo ti = { 32, {0} };
	Txt *f = resrcacq(txt, "txt/retganon.ttf", &ti);
	if(!f)
		return NULL;

	t.start = txt2img(g, f, "Press '%c' to Start", kmap[Mvinv]);
	if(!t.start)
		return NULL;

	t.opts = txt2img(g, f, "Press '%c' for Options", kmap[Mvact]);
	if(!t.opts){
		imgfree(t.start);
		return NULL;
	}

	t.copy = txt2img(g, f, "Copyright 2011 Steve McCoy and Ethan Burns");
	if(!t.copy){
		imgfree(t.opts);
		imgfree(t.start);
		return NULL;
	}

	t.titlepos = (Point){
		gfxdims(g).x / 2 - imgdims(t.title).x / 2,
		0
	};

	t.startpos = (Point){
		gfxdims(g).x / 2 - imgdims(t.start).x / 2,
		t.titlepos.y + imgdims(t.title).y
	};

	t.optspos = (Point){
		gfxdims(g).x / 2 - imgdims(t.opts).x / 2,
		t.startpos.y + imgdims(t.start).y + 16
	};

	t.copypos = (Point){
		gfxdims(g).x / 2 - imgdims(t.copy).x / 2,
		gfxdims(g).y - imgdims(t.copy).y - 8
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
	imgdraw(g, t->start, t->startpos);
	imgdraw(g, t->opts, t->optspos);
	imgdraw(g, t->copy, t->copypos);
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
	}else if(e->down && e->key == kmap[Mvact]){
		//TODO
	}
}

static void titfree(Scrn *s){
	Tit *t = s->data;
	imgfree(t->copy);
	imgfree(t->opts);
	imgfree(t->start);
}
