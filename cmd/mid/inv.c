#include "../../include/mid.h"
#include "../../include/log.h"
#include "resrc.h"
#include "game.h"

static Txt *invtxt;
static Txtinfo txtinfo = (Txtinfo) { .size = 12, .color = (Color) {0} };

void invdraw(Gfx *g, Inv *inv)
{
	if (!invtxt) {
		invtxt = resrcacq(txt, "txt/FreeSans.ttf", &txtinfo);
		if (!invtxt)
			fatal("Failed to load inventory text");
	}
	txtdraw(g, invtxt, (Point){ 0, Scrnh - 12 }, "money: %d", inv->money);
}

void invupdate(Inv *inv)
{
}

