#include "../../include/mid.h"
#include "../../include/log.h"
#include "game.h"

typedef struct Statup Statup;
struct Statup{
	Player *p;
	Lvl *lvl;
	Txt *txt;
};

static void update(Scrn *s, Scrnstk *stk);
static void draw(Scrn *s, Gfx *g);
static void handle(Scrn *s, Scrnstk *stk, Event *e);
static void statupfree(Scrn *s);

static Scrnmt statupmt = {
	update,
	draw,
	handle,
	statupfree
};

Scrn *statscrnnew(Player *p, Lvl *l){
	Statup *sup = xalloc(1, sizeof(*sup));
	sup->p = p;
	sup->lvl = l;

	Txtinfo ti = { 32 };
	sup->txt = resrcacq(txt, "txt/retganon.ttf", &ti);
	if(!sup->txt)
		die("Failed to load stat screen font");

	Scrn *s = xalloc(1, sizeof(*s));
	s->mt = &statupmt;
	s->data = sup;
	return s;
}

static void update(Scrn *s, Scrnstk *stk){
	// nothing
}

static void draw(Scrn *s, Gfx *g){
	Statup *sup = s->data;

	gfxclear(g, (Color){ 200, 200, 255 });

	enum { Bufsz = 256 };
	char buf[Bufsz];

	snprintf(buf, Bufsz, "HP: %d", sup->p->hp);
//	Point hploc = txtdims(sup->txt, buf);
	txtdraw(g, sup->txt, (Point){1,1}, buf);

	gfxflip(g);
}

static void handle(Scrn *s, Scrnstk *stk, Event *e){
	if(e->type != Keychng || e->repeat)
		return;

	if(e->down && e->key == kmap[Mvdoor]){
		scrnstkpop(stk);
		return;
	}
}

static void statupfree(Scrn *s){
	xfree(s->data);
}

