#include "../../include/mid.h"
#include "../../include/log.h"
#include <stdlib.h>
#include <stdbool.h>
#include "game.h"
#include "resrc.h"

struct Game {
	Point transl;
	int z;			/* current level z-level */
	Lvl *lvl;
	Player *player;
};

Game *gamenew(void)
{
	Game *gm = malloc(sizeof(*gm));
	if (!gm)
		return NULL;

	gm->transl = (Point) { 0, 0 };
	gm->z = 0;
	gm->lvl = resrcacq(lvls, "lvl/0.lvl", NULL);
	if (!gm->lvl)
		fatal("Failed to load level lvl/0.lvl: %s", miderrstr());
	gm->player = playernew(64, 96);

	return gm;
}

void gamefree(Scrn *s)
{
	Game *gm = s->data;
	playerfree(gm->player);
	resrcrel(lvls, "lvl/0.lvl", NULL);
	free(gm);
}

void gameupdate(Scrn *s, Scrnstk *stk)
{
	Game *gm = s->data;
	lvlupdate(anim, gm->lvl);
	playerupdate(gm->player, gm->lvl, &gm->z, &gm->transl);
}

void gamedraw(Scrn *s, Gfx *g)
{
	Game *gm = s->data;
	gfxclear(g, (Color){ 0, 0, 0, 0 });
	lvldraw(g, anim, gm->lvl, gm->z, true, gm->transl);
	playerdraw(g, gm->player, gm->transl);
	lvldraw(g, anim, gm->lvl, gm->z, false, gm->transl);
	gfxflip(g);
}

void gamehandle(Scrn *s, Scrnstk *stk, Event *e)
{
	if(e->type != Keychng || e->repeat)
		return;

	Game *gm = s->data;

	if(e->down && e->key == kmap[Mvinv]){
		scrnstkpush(stk, invscrnnew(gm->player, gm->lvl, gm->z));
		return;
	}

	playerhandle(gm->player, e);
}

Scrnmt gamemt = {
	gameupdate,
	gamedraw,
	gamehandle,
	gamefree,
};

