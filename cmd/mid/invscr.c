#include "../../include/mid.h"
#include "../../include/log.h"
#include "game.h"
#include <stdlib.h>

typedef struct Invscr Invscr;
struct Invscr{
	Player *p;
	Lvl *lvl;
	Item *curitem;
};

enum { Itemw = 64, Itemh = 64 };
enum { Pad = 1 };
enum { Width = Itemw * Invcols + Pad * (Invcols - 1),
       Height = Itemh * Invrows + Pad * (Invrows - 1),
       Imgh = 204, Imgw = 204 };
enum { Xmin = Scrnw - Imgw, Ymin = 25 };

static const char *moneystr = "gold";

static void update(Scrn*,Scrnstk*);
static void draw(Scrn*,Gfx*);
static void handle(Scrn*,Scrnstk*,Event*);
static Item *invat(Item *inv[], int x, int y);
static void invfree(Scrn*);
static void curdraw(Gfx *g, Item *inv);
static void moneydraw(Gfx *g, int m);
static void entrydraw(Gfx *g, Item *inv[], Item *cur, int r, int c);
static void griddraw(Gfx *g, Item *inv[], Item *cur);
static Txt *gettxt(void);

static Scrnmt invmt = {
	update,
	draw,
	handle,
	invfree,
};

Scrn *invscrnnew(Player *p, Lvl *lvl){
	Invscr *inv = xalloc(1, sizeof(*inv));
	inv->p = p;
	inv->lvl = lvl;
	inv->curitem = NULL;

	Scrn *s = xalloc(1, sizeof(*s));
	s->mt = &invmt;
	s->data = inv;
	return s;
}

static void update(Scrn *s, Scrnstk *stk){
	Invscr *i = s->data;
	for (int j = 0; j < Invcols * Invrows; j++) {
		Item *it = i->p->inv[j];
		if (!it)
			continue;
		itemupdate(it, i->p, i->lvl);
	}
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
	if (i->curitem)
		curdraw(g, i->curitem);

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

static void griddraw(Gfx *g, Item *inv[], Item *cur)
{
	Img *img = resrcacq(imgs, "img/inv.png", NULL);
	if (!img)
		fatal("Failed to load inventory img: %s", miderrstr());
	imgdraw(g, img, (Point){ .x = Xmin - 5, .y = Ymin - 5 });
	resrcrel(imgs, "img/inv.png", NULL);

	for (int r = 0; r < Invrows; r++) {
		for (int c = 0; c < Invcols; c++) {
			entrydraw(g, inv, cur, r, c);
		}
	}
}

static void entrydraw(Gfx *g, Item *inv[], Item *cur, int r, int c)
{
	int x0 = Xmin + r * Pad;
	int y0 = Ymin + c * Pad;
	Point a = (Point) { r * Itemw + x0, c * Itemh + y0 };
	Point b = (Point) { (r + 1) * Itemw + x0, (c + 1) * Itemh + y0 };
	Rect rect = (Rect){ a, b };
	Item *it = inv[r * Invcols + c];
	if (cur && it == cur)
		gfxfillrect(g, rect, (Color){0x99,0x66,0,0xFF});
	gfxdrawrect(g, rect, (Color){0});

	if (it)
		iteminvdraw(it, g, a);
}

static void curdraw(Gfx *g, Item *inv)
{
	Txt *invtxt = gettxt();
	Point d = txtdims(invtxt, itemname(inv));
	Point p = (Point) { .x = Scrnw - d.x, .y = Imgh + Ymin + Pad };
	txtdraw(g, invtxt, p, itemname(inv));
}

static Txt *gettxt(void)
{
	static Txt *invtxt;
	static Txtinfo txtinfo = { 12, {0} };
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

static Item *invat(Item *inv[], int x, int y)
{
	if (x < Xmin || x > Xmin + Width || y < Ymin || y > Ymin + Height)
		return NULL;

	int i = (x - Xmin) / (Itemw + Pad);
	int j = (y - Ymin) / (Itemh + Pad);

	if (x > Xmin + i * (Itemw + Pad) + Itemw
	    || y > Ymin + j * (Itemh + Pad) + Itemh)
		return NULL;	/* In padding */

	return inv[i * Invcols + j];
}

static void invfree(Scrn *s){
	free(s->data);
	free(s);
}
