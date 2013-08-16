/* © 2013 the Mid Authors under the MIT license. See AUTHORS for the list of authors.*/

#include "../../include/mid.h"
#include "game.h"
#include <stdlib.h>

typedef struct Tit Tit;
struct Tit{
	Img *title;
	Txt *f;
	Img *copy;
	Point titlepos;
	Point startpos;
	Point loadpos;
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

	Txtinfo ti = { TxtSzMedium };
	t.f = resrcacq(txt, TxtStyleMenu, &ti);
	if(!t.f)
		return NULL;

	ti.size = TxtSzSmall;
	Txt *f = resrcacq(txt, TxtStyleMenu, &ti);
	if(!f)
		return NULL;

	t.copy = txt2img(g, f, "Copyright 2011 Steve McCoy and Ethan Burns");
	if(!t.copy)
		return NULL;

	t.titlepos = (Point){
		gfxdims(g).x / 2 - imgdims(t.title).x / 2,
		0
	};

	Point sd = txtdims(t.f, "Press 'x' to Start a new game");
	t.startpos = (Point){
		gfxdims(g).x / 2 - sd.x / 2,
		t.titlepos.y + imgdims(t.title).y
	};

	Point ld = txtdims(t.f, "Press 'x' to Load the saved game");
	t.loadpos = (Point){
		gfxdims(g).x / 2 - ld.x / 2,
		t.startpos.y + sd.y + 16
	};

	Point od = txtdims(t.f, "Press 'x' for Options");
	t.optspos = (Point){
		gfxdims(g).x / 2 - od.x / 2,
		t.loadpos.y + ld.y + 16
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
	txtdraw(g, t->f, t->startpos, "Press '%c' to Start a new game", kmap[Mvinv]);
	if (saveavailable())
		txtdraw(g, t->f, t->loadpos, "Press '%c' to Load the saved game", kmap[Mvjump]);
	txtdraw(g, t->f, t->optspos, "Press '%c' for Options", kmap[Mvact]);
	imgdraw(g, t->copy, t->copypos);
	gfxflip(g);
}

static Scrn gms = { &gamemt };

static void handle(Scrn *s, Scrnstk *stk, Event *e){
	if(e->type != Keychng || e->repeat)
		return;

	if(e->down && e->key == kmap[Mvinv]){
		Game *g = gamenew();
		lvlsetpallet(lvlpallet(g));
		gms.data = g;
		scrnstkpush(stk, &gms);
		return;
	}else if(e->down && e->key == kmap[Mvact]){
		scrnstkpush(stk, optscrnnew());
		return;
	}else if(e->down && e->key == kmap[Mvjump]){
		Game *g = gameload();
		lvlsetpallet(lvlpallet(g));
		gms.data = g;
		scrnstkpush(stk, &gms);
		return;
	}
}

static void titfree(Scrn *s){
	Tit *t = s->data;
	imgfree(t->copy);
}
