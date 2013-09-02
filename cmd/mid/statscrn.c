/* © 2013 the Mid Authors under the MIT license. See AUTHORS for the list of authors.*/

#include "../../include/mid.h"
#include "../../include/log.h"
#include "game.h"

typedef struct Statup Statup;
struct Statup{
	Game *g;
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

Scrn *statscrnnew(Game *g, Player *p, Env *sh){
	static Statup sup  = {0};
	static Scrn s  = {0};

	sup.g = g;
	sup.p = p;
	sup.shrine = sh;

	Txtinfo ti = { TxtSzMedium };
	sup.txt = resrcacq(txt, TxtStyleMenu, &ti);
	if(!sup.txt)
		die("Failed to load stat screen font");

	for(size_t i = 0; i < Maxinv; i++)
		if(p->inv[i].id == ItemStatup)
			sup.norbs++;

	s.mt = &statupmt;
	s.data = &sup;
	return &s;
}

static void update(Scrn *s, Scrnstk *stk){
	// nothing
}

static void draw(Scrn *s, Gfx *g){
	extern char *statname[];

	Statup *sup = s->data;

	gfxclear(g, (Color){ 127, 200, 255 });

	enum { Bufsz = 256 };
	char buf[Bufsz];

	int mh = TxtSzMedium/2;
	int Pad = 4;
	Point sloc = { Pad, Pad };
	for(int i = StatHp; i < StatMax; i++){
		Meter meter = {
			.base = sup->p->stats[i],
			.extra = sup->p->eqp[i],
			.max = statmax[i],
			.xscale = 2,
			.h = mh,
			.cbg = {0x65, 0x65, 0x65},
			.cbase = {0x1E, 0x94, 0x22},
			.cextra = {0x1B, 0xAF, 0xE0},
			.cborder = {}
		};

		Point mloc = { sloc.x + mh*2, sloc.y };
		Rect ma = meterarea(&meter, mloc);

		if(rectcontains(ma, sup->mouse))
			meter.cbg = (Color){ 255, 219, 0 };

		txtdraw(g, sup->txt, sloc, statname[i]);
		meterdraw(g, &meter, mloc);

		if(rectcontains(ma, sup->mouse) && sup->inc && sup->p->stats[i] < statmax[i]){
			if(i == StatHp){
				sup->p->stats[i] += 5;
				sup->p->curhp += 5;
			}else
				sup->p->stats[i]++;

			sup->norbs--;
			sup->uorbs++;
			sup->inc = 0;
		}

		sloc = vecadd(sloc, (Point){0, mh + Pad});
	}

	snprintf(buf, Bufsz, "Orbs: %d", sup->norbs);
	Point o = txtdims(sup->txt, buf);
	txtdraw(g, sup->txt, (Point){ Scrnw - o.x, 1 }, buf);

	gfxflip(g);
}

static void handle(Scrn *s, Scrnstk *stk, Event *e){
	Statup *sup = s->data;

	if(e->type == Mousemv){
		sup->mouse = projpt((Point){ e->x, e->y });
		sup->inc = 0;
		return;
	}

	if(e->type == Mousebt && sup->uorbs == 0){
		if(sup->norbs == 0)
			return;
		sup->mouse = projpt((Point){ e->x, e->y });
		sup->inc = 1;
		return;
	}

	if(e->type != Keychng || e->repeat)
		return;

	if(e->down && e->key == kmap[Mvinv]){
		Player *p = sup->p;
		int u = 0;
		for(Invit *i = p->inv; i != p->inv + Maxinv && u < sup->uorbs; i++){
			if(i->id == ItemStatup){
				i->id = 0;
				sup->norbs--;
				u++;
			}
		}
		gamesave(sup->g);
		scrnstkpop(stk);
		return;
	}
}

static void statupfree(Scrn *s){
	Statup *sup = s->data;
	if(sup->uorbs > 0)
		sup->shrine->id = EnvShrused;
	*sup = (Statup){0};
}

