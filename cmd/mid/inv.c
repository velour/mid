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
	Img *img = txt2img(g, invtxt, "money: %d", inv->money);
	if (!img)
		fatal("Failed to create money text image");
	imgdraw(g, img, (Point){ 0, Scrnh - 12 });
	imgfree(img);
}

void invupdate(Inv *inv)
{
}

