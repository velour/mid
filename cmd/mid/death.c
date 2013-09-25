/* © 2013 the Mid Authors under the MIT license. See AUTHORS for the list of authors.*/

#include "../../include/mid.h"
#include "game.h"

typedef struct Gover Gover;
struct Gover{
	Player *p;
	int maxd;
	Txt *big;
	Txt *med;
};

static void update(Scrn *s, Scrnstk *stk);
static void draw(Scrn *s, Gfx *g);
static void handle(Scrn *s, Scrnstk *stk, Event *e);
static void goverfree(Scrn *s);
static char *praise(int);
static int hasjewel(Player *p);

static Scrnmt govermt = {
	update,
	draw,
	handle,
	goverfree
};

Scrn *goverscrnnew(Player *p, int maxd){
	static Gover go = {0};
	static Scrn s = {0};

	go.p = p;
	go.maxd = maxd;

	Txtinfo ti = { TxtSzLarge, { 255, 255, 255 } };
	go.big = resrcacq(txt, TxtStyleMenu, &ti);
	ti.size = TxtSzMedium;
	go.med = resrcacq(txt, TxtStyleMenu, &ti);

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
	gfxclear(g, BloodRed);

	int m = go->p->money;
	enum { Bufsz = 256 };
	char buf[Bufsz];

	char *d;
	if(hasjewel(go->p))
		d = "YOU ARE A HERO.";
	else if(go->maxd < 0)
		d = "YOU ARE A COWARD.";
	else if(go->maxd < 10)
		d = "You are dead!";
	else
		d = "You have died with HONOR!";
	snprintf(buf, Bufsz, "With %d %s gold for your family.", m, praise(m));

	Point dp = txtdims(go->big, d);
	Point dl = { gfxdims(g).x/2 - dp.x/2, dp.y };
	txtdraw(g, go->big, dl, d);

	Point mp = txtdims(go->med, buf);
	txtdraw(g, go->med, (Point){gfxdims(g).x/2 - mp.x/2, dl.y+dp.y+TxtSzMedium/2}, buf);

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

static int hasjewel(Player *p){
	for(int i = 0; i <= Maxinv; i++)
		if(p->inv[i].id == ItemDjewel)
			return 1;
	return 0;
}
