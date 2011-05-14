#include "../../include/log.h"
#include "../../include/mid.h"
#include "resrc.h"
#include "game.h"
#include <stdlib.h>
#include <stdbool.h>

Gfx *gfx;

bool init()
{
	loginit(0);

	pr("%s", "Let's rock.");

	gfx = gfxinit(Scrnw, Scrnh);
	if(!gfx)
		return false;

	if (!sndinit())
		fatal("Failed to initialze sound: %s\n", miderrstr());

	initresrc();

	return true;
}

void deinit()
{
	freeresrc();
	sndfree();
	gfxfree(gfx);
}

int main(int argc, char *argv[])
{
	if (!init())
		fatal("Failed to initialize: %s", miderrstr());

	Game *gm = gamenew();
	Scrn mainscrn = { &gamemt, gm };

	Music *m = resrcacq(music, "music/bgm_placeholder.ogg", NULL);
	if (!m)
		fatal("Failed to load bgm_placeholder.ogg");
	musicstart(m, 0);

	Scrnstk *stk = scrnstknew();
	scrnstkpush(stk, &mainscrn);

	scrnrun(stk, gfx);

	deinit();
	return 0;
}
