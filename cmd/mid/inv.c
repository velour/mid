#include "../../include/mid.h"
#include "../../include/log.h"
#include "resrc.h"
#include "game.h"
#include <stdlib.h>

enum { Iconw = 32, Iconh = 32 };
enum { Pad = 1 };
enum { Width = Iconw * Invcols + Pad * (Invcols - 1),
       Height = Iconh * Invrows + Pad * (Invrows - 1) };
enum { Xmin = Scrnw - Width - 1, Ymin = 15 };

static Txt *invtxt;
static Txtinfo txtinfo = (Txtinfo) { .size = 12, .color = (Color) {0} };

static void griddraw(Gfx *g, Inv *inv);
static void entrydraw(Gfx *g, Inv *inv, int r, int c);

void invdraw(Gfx *g, Inv *inv)
{
	if (!invtxt) {
		invtxt = resrcacq(txt, "txt/FreeSans.ttf", &txtinfo);
		if (!invtxt)
			fatal("Failed to load inventory text");
	}
	Point d = txtdims(invtxt, "gold");
	txtdraw(g, invtxt, (Point) { Scrnw - d.x, 1 }, "gold");
	d.x += txtdims(invtxt, "%d ", inv->money).x;
	txtdraw(g, invtxt, (Point) { Scrnw - d.x , 1 }, "%d ", inv->money);
	griddraw(g, inv);
}

static void griddraw(Gfx *g, Inv *inv)
{
	for (int r = 0; r < Invrows; r++) {
		for (int c = 0; c < Invcols; c++) {
			entrydraw(g, inv, r, c);
		}
	}
}

static void entrydraw(Gfx *g, Inv *inv, int r, int c)
{
	int x0 = Xmin + r * Pad;
	int y0 = Ymin + c * Pad;
	Point a = (Point) { r * Iconw + x0, c * Iconh + y0 };
	Point b = (Point) { (r + 1) * Iconw + x0, (c + 1) * Iconh + y0 };
	Rect rect = (Rect){ a, b };
	gfxdrawrect(g, rect, (Color){0});

	Item *it = inv->items[r * Invcols + c];
	if (it)
		animdraw(g, it->icon, a);
}

void invupdate(Inv *inv)
{
	for (int i = 0; i < Invcols * Invrows; i++) {
		Item *it = inv->items[i];
		if (!it)
			continue;
		animupdate(it->icon, 1);
	}
}

Item *invmod(Inv *inv, Item *new, int x, int y)
{
	int i = x * Invcols + y;
	Item *old = inv->items[i];
	inv->items[i] = new;
	return old;
}

Item *itemnew(const char *name, const char *resrc)
{
	Item *item = malloc(sizeof(*item));
	if (!item)
		fatal("malloc failed: %s", miderrstr());

	item->icon = resrcacq(anim, resrc, NULL);
	if (!item->icon)
		fatal("Failed to load icon for item %s: %s", name, miderrstr());
	item->name = name;

	return item;
}

void itemfree(Item *item)
{
	free(item);
}

