// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
#include "../../include/mid.h"
#include "../../include/log.h"
#include "game.h"
#include <stdlib.h>
#include <string.h>

typedef struct Invscr Invscr;
typedef struct Eloc Eloc;

struct Invscr{
	Player *p;
	Zone *zone;
	int depth, zvis;
	Invit *curitem;
	_Bool drag;
	Point mouse;
	Rect invgrid[Maxinv];
	Rect eqpgrid[EqpMax + 2];
};

struct Eloc{
	Invit *it;
	EqpLoc loc;
};

enum {
	Invitw = 32,
	Invith = 32,
	Pad = 4,
	Width = Invitw * Invcols + Pad * (Invcols - 1),
	Height = Invith * Invrows + Pad * (Invrows - 1),
	EqpXmin = Scrnw - Invitw * 5,
	Xmin = EqpXmin - Width,
	Ymin = 40,
	EqpEat = EqpMax,
	EqpDrop = EqpEat + 1,
};

static const char *moneystr = "gold";
static char *locname[] = {
	[EqpHead] = "Head",
	[EqpBody] = "Body",
	[EqpArms] = "Arms",
	[EqpLegs] = "Legs",
	[EqpWep] = "Wep.",
	[EqpEat] = "Eat",
	[EqpDrop] = "Drop",
};

static char *statname[] = {
	[StatHp] = "HP",
	[StatDex] = "Dex",
	[StatStr] = "Str",
};

static void update(Scrn*,Scrnstk*);
static void draw(Scrn*,Gfx*);
static void handle(Scrn*,Scrnstk*,Event*);
static Invit *invat(Invscr *i, int x, int y);
static void invfree(Scrn*);
static void curdraw(Gfx *g, Invit *inv);
static void moneydraw(Gfx *g, int m);
static Txt *gettxt(void);
static void invswap(Invit *, Invit *);
static Eloc eqpat(Invscr *i, int x, int y);
static void resetstats(Player*);

static Scrnmt invmt = {
	update,
	draw,
	handle,
	invfree,
};

Scrn *invscrnnew(Player *p, Zone *zone, int depth){
	static Invscr inv = {0};
	static Scrn s = {0};

	inv.p = p;
	inv.zone = zone;
	inv.depth = depth;
	inv.zvis = zone->lvl->z;
	inv.curitem = NULL;

	int i = 0;
	for(int r = 0; r < Invrows; r++){
	for(int c = 0; c < Invcols; c++){
		int x0 = Xmin + c * Pad;
		int y0 = Ymin + r * Pad;
		Point a = { c * Invitw + x0, r * Invith + y0 };
		inv.invgrid[i] = (Rect){
			{ a.x - 1, a.y - 1 },
			{ (c + 1) * Invitw + x0 + 1, (r + 1) * Invith + y0 + 1 }
		};
		i++;
	}
	}

	for(int j = EqpHead; j < EqpMax + 2; j++){
		Point a = {
			EqpXmin + Pad*3,
			Ymin + (j-1) * (Invith + Pad)
		};
		inv.eqpgrid[j] = (Rect){
			{ a.x - 1, a.y - 1 },
			{ a.x + Invitw + 1, a.y + Invith + 1}
		};
	}

	s.mt = &invmt;
	s.data = &inv;
	return &s;
}

static void update(Scrn *s, Scrnstk *stk){

}

enum { Scale = 6 };

static void draw(Scrn *s, Gfx *g){
	gfxclear(g, (Color){ 127, 127, 127 });

	Invscr *i = s->data;
	Txt *txt = gettxt();

	char buf[32];
	snprintf(buf, sizeof(buf), "%d-%d", i->depth, i->zvis);
	Point info = txtdims(txt, buf);
	txtdraw(g, txt, (Point){0}, buf);

	int realz = i->zone->lvl->z;
	i->zone->lvl->z = i->zvis;
	lvlminidraw(g, i->zone->lvl, (Point){0,info.y+1}, Scale);
	i->zone->lvl->z = realz;

	Point ppos = playerpos(i->p);
	double px = ppos.x / Twidth;
	double py = ppos.y / Theight;
	Rect r = {
		(Point){ px*Scale, info.y+1 + py*Scale },
		(Point){ px*Scale + Scale, info.y+1 + py*Scale + Scale }
	};
	gfxfillrect(g, r, (Color){ 255, 0, 0, 255 });

	moneydraw(g, i->p->money);
	if (i->curitem && i->curitem->id > 0 && !i->drag)
		curdraw(g, i->curitem);

	for(int j = 0; j < Maxinv; j++){
		Rect r = i->invgrid[j];
		gfxdrawrect(g, r, (Color){0});
		Invit *it = &i->p->inv[j];
		if(it->id > 0 && (!i->drag || it != i->curitem))
			invitdraw(it, g, (Point){r.a.x+1,r.a.y+1});
	}

	for (int j = EqpHead; j < EqpMax + 2; j++){
		Rect er = i->eqpgrid[j];
		gfxdrawrect(g, er, (Color){0});
		txtdraw(g, txt, (Point){ er.b.x + Pad, er.a.y }, locname[j]);
		if(j >= EqpMax)
			continue;
		Invit *it = &i->p->wear[j];
		if(it->id > 0 && (!i->drag || it != i->curitem))
			invitdraw(it, g, (Point){er.a.x+1,er.a.y+1});
	}

	if(i->drag && i->curitem && i->curitem->id > 0){
		Point moff = vecadd(i->mouse, (Point){-Invitw/2,-Invith/2});
		invitdraw(i->curitem, g, moff);
	}

	Point sloc = { Pad, Height + Ymin + Pad + TxtSzMedium };
	for(int j = StatHp; j < StatMax; j++){
		Meter meter = {
			.base = i->p->stats[j],
			.extra = i->p->eqp[j],
			.max = 30,
			.xscale = 3,
			.h = TxtSzMedium,
			.cbg = {0x65, 0x65, 0x65},
			.cbase = {0x1E, 0x94, 0x22},
			.cextra = {0x1B, 0xAF, 0xE0},
			.cborder = {}
		};

		txtdraw(g, gettxt(), sloc, statname[j]);
		meterdraw(g, &meter, (Point){ sloc.x + TxtSzMedium*2, sloc.y });

		sloc = vecadd(sloc, (Point){0, TxtSzMedium + Pad});
	}

	gfxflip(g);
}

static void moneydraw(Gfx *g, int m)
{
	Txt *invtxt = gettxt();
	Point d = txtdims(invtxt, moneystr);
	txtdraw(g, invtxt, (Point) { Scrnw - d.x, 1 }, moneystr);
	d.x += txtdims(invtxt, "%d ", m).x;
	txtdraw(g, invtxt, (Point) { Scrnw - d.x , 1 }, "%d ", m);
}

static void curdraw(Gfx *g, Invit *inv)
{
	Txt *invtxt = gettxt();
	Point p = { .x = Xmin, .y = Height + Ymin + Pad + TxtSzMedium + Pad };
	txtdraw(g, invtxt, p, itemname(inv->id));
}

static Txt *gettxt(void)
{
	static Txt *invtxt;
	static Txtinfo txtinfo = { TxtSzMedium, {0} };
	if (!invtxt) {
		invtxt = resrcacq(txt, TxtStyleMenu, &txtinfo);
		if (!invtxt)
			fatal("Failed to load inventory text");
	}
	return invtxt;
}

static void handle(Scrn *s, Scrnstk *stk, Event *e){
	Invscr *i = s->data;

	if (e->type == Mousemv) {
		if(!i->drag)
			i->curitem = invat(i, e->x, e->y);
		if(!i->curitem)
			i->curitem = eqpat(i, e->x, e->y).it;
		i->mouse.x = e->x;
		i->mouse.y = e->y;
		return;
	}

	if(e->type == Mousebt && e->down){
		i->curitem = invat(i, e->x, e->y);
		if(!i->curitem)
			i->curitem = eqpat(i, e->x, e->y).it;
		i->drag = i->curitem != NULL;
		return;
	}

	if(e->type == Mousebt && !e->down && i->drag){
		i->drag = 0;
		Invit *s = invat(i, e->x, e->y);
		if(i->curitem == s)
			return;
		if(s){
			invswap(i->curitem, s);
			i->curitem = s;
			resetstats(i->p);
		}else{
			Eloc el = eqpat(i, e->x, e->y);
			s = el.it;
			if(el.loc == EqpDrop){
				int dpos = i->p->anim == i->p->leftas ? -1 : 1;
				Item drop = {};
				Point gridloc = { // BARF
					i->p->body.bbox.a.x / Twidth + dpos,
					i->p->body.bbox.a.y / Theight
				};
				iteminit(&drop, i->curitem->id, gridloc);
				zoneadditem(i->zone, i->zone->lvl->z, drop);
				*i->curitem = (Invit){};
				return;
			}
			if(el.loc == EqpEat){
				if(inviteat(i->curitem, i->p, i->zone))
					*i->curitem = (Invit){};
				return;
			}
			if(!s || s == i->curitem || el.loc != itemeqploc(i->curitem->id))
				return;
			invswap(i->curitem, s);
			i->curitem = s;
			resetstats(i->p);
		}
	}

	if(e->type != Keychng || e->repeat)
		return;

	if(e->down && e->key == kmap[Mvinv]){
		scrnstkpop(stk);
		return;
	}

	if(e->down && e->key == kmap[Mvleft] && i->zvis > 0)
			i->zvis--;
	else if(e->down && e->key == kmap[Mvright] && i->zvis < i->zone->lvl->seenz)
			i->zvis++;
}

static Invit *invat(Invscr *i, int x, int y)
{
	Point p = { x, y };
	for(int j = 0; j < Maxinv; j++)
		if(rectcontains(i->invgrid[j], p))
			return &i->p->inv[j];
	return NULL;
}

static void invswap(Invit *a, Invit *b){
	Invit c = *a;
	*a = *b;
	*b = c;
}

static Eloc eqpat(Invscr *i, int x, int y){
	Point p = { x, y };
	for (int j = 1; j < EqpMax; j++)
		if(rectcontains(i->eqpgrid[j], p))
			return (Eloc){ &i->p->wear[j], j };

	if(rectcontains(i->eqpgrid[EqpEat], p))
		return (Eloc){ NULL, EqpEat };

	if(rectcontains(i->eqpgrid[EqpDrop], p))
		return (Eloc){ NULL, EqpDrop };

	return (Eloc){ NULL, -1 };
}

static void invfree(Scrn *s){
	Invscr *inv = s->data;
	resetstats(inv->p);
}

static void resetstats(Player *p){
	memset(p->eqp, 0, sizeof(p->eqp));
	for(int i = EqpHead; i < EqpMax; i++)
		if(p->wear[i].id > 0) for(int j = 0; j < StatMax; j++)
			p->eqp[j] += p->wear[i].stats[j];

	//TODO: swd fun for this
	if(p->wear[EqpWep].id == 0)
		p->sw.row = 0;
	else
		p->sw.row = p->wear[EqpWep].id - ItemSilverSwd;
}
