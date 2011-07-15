#include "../../include/mid.h"
#include "../../include/log.h"
#include "game.h"
#include <stdlib.h>

typedef struct Invscr Invscr;
struct Invscr{
	Player *p;
	Lvl *lvl;
	Invit *curitem;
};

enum { Invitw = 32, Invith = 32 };
enum { Pad = 4 };
enum {
	Width = Invitw * Invcols + Pad * (Invcols - 1),
	Height = Invith * Invrows + Pad * (Invrows - 1),
};
enum { EqpXmin = Scrnw - Invitw * 3 };
enum { Xmin = EqpXmin - Width, Ymin = 40 };

static const char *moneystr = "gold";
static char *locname[] = {
	[EqpHead] = "Head",
	[EqpBody] = "Body",
	[EqpWep] = "Wep.",
	[EqpAcc] = "Acc.",
};

static void update(Scrn*,Scrnstk*);
static void draw(Scrn*,Gfx*);
static void handle(Scrn*,Scrnstk*,Event*);
static Invit *invat(Invit inv[], int x, int y);
static void invfree(Scrn*);
static void curdraw(Gfx *g, Invit *inv);
static void moneydraw(Gfx *g, int m);
static void entrydraw(Gfx *g, Invit inv[], Invit *cur, int r, int c);
static void griddraw(Gfx *g, Invit inv[], Invit *cur);
static Txt *gettxt(void);

static Scrnmt invmt = {
	update,
	draw,
	handle,
	invfree,
};

Scrn *invscrnnew(Player *p, Lvl *lvl){
	static Invscr inv = {0};
	static Scrn s = {0};

	inv.p = p;
	inv.lvl = lvl;
	inv.curitem = NULL;

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
	lvlminidraw(g, i->lvl, (Point){0,0}, Scale);

	Point ppos = playerpos(i->p);
	double px = ppos.x / Twidth;
	double py = ppos.y / Theight;
	Rect r = {
		(Point){ px*Scale, py*Scale },
		(Point){ px*Scale + Scale, py*Scale + Scale }
	};
	gfxfillrect(g, r, (Color){ 255, 0, 0, 255 });

	moneydraw(g, i->p->money);
	griddraw(g, i->p->inv, i->curitem);
	if (i->curitem && i->curitem->id > 0)
		curdraw(g, i->curitem);

	for (int j = 1; j < EqpMax; j++){
		Point a = { EqpXmin + Pad*3, Ymin + (j-1) * (Invith + Pad) };
		Rect er = {
			{ a.x - 1, a.y - 1 },
			{ a.x + Invitw + 1, a.y + Invith + 1}
		};
		gfxdrawrect(g, er, (Color){0});
		Txt *txt = gettxt();
		txtdraw(g, txt, (Point){ er.b.x + Pad, er.a.y }, locname[j]);
		if(i->p->inv[j].id)
			invitdraw(&i->p->wear[j], g, a);
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

static void griddraw(Gfx *g, Invit inv[], Invit *cur)
{
	for (int r = 0; r < Invrows; r++) {
	for (int c = 0; c < Invcols; c++) {
		entrydraw(g, inv, cur, r, c);
	}
	}
}

static void entrydraw(Gfx *g, Invit inv[], Invit *cur, int r, int c)
{
	int x0 = Xmin + c * Pad;
	int y0 = Ymin + r * Pad;
	Point a = { c * Invitw + x0, r * Invith + y0 };
	Rect rect = {
		{ a.x - 1, a.y - 1 },
		{ (c + 1) * Invitw + x0 + 1, (r + 1) * Invith + y0 + 1 }
	};
	Invit *it = &inv[r * Invcols + c];
	if (cur && it == cur)
		gfxfillrect(g, rect, (Color){0x99,0x66,0,0xFF});
	gfxdrawrect(g, rect, (Color){0});

	if (it->id > 0)
		invitdraw(it, g, a);
}

static void curdraw(Gfx *g, Invit *inv)
{
	Txt *invtxt = gettxt();
	Point d = txtdims(invtxt, itemname(inv->id));
	Point p = (Point) { .x = Scrnw - d.x, .y = Height + Ymin + Pad};
	txtdraw(g, invtxt, p, itemname(inv->id));
}

static Txt *gettxt(void)
{
	static Txt *invtxt;
	static Txtinfo txtinfo = { 32, {0} };
	if (!invtxt) {
		invtxt = resrcacq(txt, "txt/retganon.ttf", &txtinfo);
		if (!invtxt)
			fatal("Failed to load inventory text");
	}
	return invtxt;
}

static void handle(Scrn *s, Scrnstk *stk, Event *e){
	if (e->type == Mousemv) {
		Invscr *i = s->data;
		i->curitem = invat(i->p->inv, e->x, e->y);
	}

	if(e->type != Keychng || e->repeat)
		return;

	if(e->down && e->key == kmap[Mvinv]){
		scrnstkpop(stk);
		return;
	}
}

static Invit *invat(Invit inv[], int x, int y)
{
	if (x < Xmin || x > Xmin + Width || y < Ymin || y > Ymin + Height)
		return NULL;

	int i = (x - Xmin) / (Invitw + Pad);
	int j = (y - Ymin) / (Invith + Pad);

	if (x > Xmin + i * (Invitw + Pad) + Invitw
	    || y > Ymin + j * (Invith + Pad) + Invith)
		return NULL;	/* In padding */

	return &inv[j * Invcols + i];
}

static void invfree(Scrn *s){
	// nothing
}
