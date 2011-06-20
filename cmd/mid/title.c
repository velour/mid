#include "../../include/mid.h"
#include "game.h"
#include <stdlib.h>

typedef struct Tit Tit;
struct Tit{
	Img *title;
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
	Tit *t = xalloc(1, sizeof(*t));
	Txt *txt = txtnew("resrc/txt/prstartk.ttf", 72, (Color){0});
	if(!txt){
		xfree(t);
		return NULL;
	}

	t->title = txt2img(g, txt, "MID");
	txtfree(txt);
	if(!t->title){
		xfree(t);
		return NULL;
	}

	Scrn *s = xalloc(1, sizeof(*s));
	s->mt = &titmt;
	s->data = t;
	return s;
}

static void update(Scrn *s, Scrnstk *stk){
}

static void draw(Scrn *s, Gfx *g){
	gfxclear(g, (Color){ 240, 240, 240 });
	Tit *t = s->data;
	imgdraw(g, t->title, (Point){0});
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
	Tit *t = s->data;
	imgfree(t->title);
	free(t);
	free(s);
}
