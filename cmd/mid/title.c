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
	_Bool havesave;
	_Bool loaded;
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

	Point titdims = imgdims(t.title);
	titdims.x /= 2;
	titdims.y /= 2;
	t.titlepos = (Point){
		gfxdims(g).x / 2 - titdims.x / 2,
		0
	};

	Point sd = txtdims(t.f, "Press 'x' to Start a new game");
	t.startpos = (Point){
		gfxdims(g).x / 2 - sd.x / 2,
		t.titlepos.y + titdims.y
	};

	Point ld = txtdims(t.f, "Press 'x' to Load the saved game");
	t.loadpos = (Point){
		gfxdims(g).x / 2 - ld.x / 2,
		t.startpos.y + sd.y + ti.size/2
	};

	Point od = txtdims(t.f, "Press 'x' for Options");
	t.optspos = (Point){
		gfxdims(g).x / 2 - od.x / 2,
		t.loadpos.y + ld.y + ti.size/2
	};

	Point copydims = imgdims(t.copy);
	copydims.x /= 2;
	copydims.y /= 2;
	t.copypos = (Point){
		gfxdims(g).x / 2 - copydims.x / 2,
		gfxdims(g).y - copydims.y - 8
	};

	t.havesave = saveavailable();

	s.mt = &titmt;
	s.data = &t;
	return &s;
}

static void update(Scrn *s, Scrnstk *stk){
}

static void draw(Scrn *s, Gfx *g){
	gfxclear(g, (Color){ 240, 240, 240 });
	Tit *t = s->data;
	imgdrawscale(g, t->title, t->titlepos, 0.5);
	txtdraw(g, t->f, t->startpos, "Press '%c' to Start a new game", kmap[Mvinv]);
	if (t->havesave)
		txtdraw(g, t->f, t->loadpos, "Press '%c' to Load the saved game", kmap[Mvjump]);
	txtdraw(g, t->f, t->optspos, "Press '%c' for Options", kmap[Mvact]);
	imgdraw(g, t->copy, t->copypos);
	gfxflip(g);
}

static Scrn gms = { &gamemt };

static void handle(Scrn *s, Scrnstk *stk, Event *e){
	Tit *t = s->data;
	if(t->loaded){
		t->havesave = saveavailable();
		t->loaded = 0;
	}

	if(e->type != Keychng || e->repeat)
		return;

	if(e->down && e->key == kmap[Mvinv]){
		rmsave(); 
		t->havesave = 0;
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
		t->loaded = 1;
		scrnstkpush(stk, &gms);
		return;
	}
}

static void titfree(Scrn *s){
	Tit *t = s->data;
	imgfree(t->copy);
}
