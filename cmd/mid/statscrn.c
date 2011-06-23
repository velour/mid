#include "../../include/mid.h"
#include "../../include/log.h"
#include "game.h"

typedef struct Statup Statup;
struct Statup{
	Player *p;
	Env *shrine;
	Txt *txt;
	int norbs, uorbs;
	Point mouse;
	_Bool inc;
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

Scrn *statscrnnew(Player *p, Env *sh){
	Statup *sup = xalloc(1, sizeof(*sup));
	sup->p = p;
	sup->shrine = sh;

	Txtinfo ti = { 32 };
	sup->txt = resrcacq(txt, "txt/retganon.ttf", &ti);
	if(!sup->txt)
		die("Failed to load stat screen font");

	for(size_t i = 0; i < Maxinv; i++)
		if(p->inv[i] && p->inv[i]->id == ItemStatup)
			sup->norbs++;

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

	gfxclear(g, (Color){ 127, 200, 255 });

	Color hilit = { 255, 219, 0 };
	enum { Bufsz = 256 };
	char buf[Bufsz];

	snprintf(buf, Bufsz, "HP: %d", sup->p->hp);
	Point hploc = txtdims(sup->txt, buf);

	Rect r = { { 1, 1 } };
	r.b = vecadd(r.a, hploc);

	if(rectcontains(r, sup->mouse)){ //TODO: generalize this for all stats
		gfxfillrect(g, r, hilit);
		if(sup->inc){
			sup->p->hp += 5;
			sup->norbs--;
			sup->uorbs++;
			sup->inc = 0;
		}
	}
	txtdraw(g, sup->txt, (Point){1,1}, buf);

	snprintf(buf, Bufsz, "Dexterity: %d", sup->p->dex);
	Point dexloc = txtdims(sup->txt, buf);

	r.a = (Point){ 1, hploc.y };
	r.b = vecadd(r.a, dexloc);

	if(rectcontains(r, sup->mouse)){ //TODO: generalize this for all stats
		gfxfillrect(g, r, hilit);
		if(sup->inc){
			sup->p->dex++;
			sup->norbs--;
			sup->uorbs++;
			sup->inc = 0;
		}
	}
	txtdraw(g, sup->txt, (Point){ 1, hploc.y }, buf);

	snprintf(buf, Bufsz, "Orbs: %d", sup->norbs);
	Point o = txtdims(sup->txt, buf);
	txtdraw(g, sup->txt, (Point){ Scrnw - o.x, 1 }, buf);

	gfxflip(g);
}

static void handle(Scrn *s, Scrnstk *stk, Event *e){
	Statup *sup = s->data;

	if(e->type == Mousemv){
		sup->mouse = (Point){ e->x, e->y };
		sup->inc = 0;
		return;
	}

	if(e->type == Mousebt && sup->uorbs == 0){
		if(sup->norbs == 0)
			return;
		sup->mouse = (Point){ e->x, e->y };
		sup->inc = 1;
		return;
	}

	if(e->type != Keychng || e->repeat)
		return;

	if(e->down && e->key == kmap[Mvdoor]){
		Player *p = sup->p;
		for(Item **i = p->inv; i != p->inv + Maxinv && sup->uorbs > 0; i++){
			if(*i && (*i)->id == ItemStatup){
				*i = NULL;
				sup->norbs--;
			}
		}

		scrnstkpop(stk);
		return;
	}
}

static void statupfree(Scrn *s){
	Statup *sup = s->data;
	if(sup->uorbs > 0)
		sup->shrine->id = EnvShrused;
	sup->p->statup = 0;
	xfree(s->data);
}

