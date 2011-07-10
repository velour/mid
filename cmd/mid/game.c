#include "../../include/mid.h"
#include "../../include/log.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include "game.h"

struct Game {
	Player player;
	Point transl;
	Zone *zone;
};

Game *gamenew(void)
{
	lvlinit();
	Game *gm = xalloc(1, sizeof(*gm));


	int seed = rand();
	gm->zone= zonegen(seed);
	if (!gm->zone)
		fatal("Failed to load zone: %s", miderrstr());

	playerinit(&gm->player, 2, 2);

	_Bool ok = itemldresrc();
	if (!ok)
		fatal("Failed to load item resources: %s", miderrstr());
	ok = envldresrc();
	if (!ok)
		fatal("Failed to load env resources: %s", miderrstr());

	return gm;

oops:
	zonefree(gm->zone);
	free(gm);
	return NULL;
}

void gamefree(Scrn *s)
{
	Game *gm = s->data;
	zonefree(gm->zone);
	free(gm);
}

void gameupdate(Scrn *s, Scrnstk *stk)
{
	Game *gm = s->data;
	zoneupdate(gm->zone, &gm->player, &gm->transl);
}

void gamedraw(Scrn *s, Gfx *g)
{
	Game *gm = s->data;

	gfxclear(g, (Color){ 0, 0, 0, 0 });

	cammove(g, gm->transl.x, gm->transl.y);

	zonedraw(g, gm->zone, &gm->player);

	Rect hp = { { 1, 1 }, { gm->player.stats[StatHp] * 5, 16 } };
	Rect curhp = hp;
	curhp.b.x = gm->player.eqp[StatHp] * 5;
	gfxfillrect(g, hp, (Color){ 200 });
	gfxfillrect(g, curhp, (Color){ 0, 200, 200 });
	gfxdrawrect(g, hp, (Color){0});

	gfxflip(g);
}

void gamehandle(Scrn *s, Scrnstk *stk, Event *e)
{
	if(e->type != Keychng || e->repeat)
		return;

	Game *gm = s->data;

	if(e->down && e->key == kmap[Mvinv]){
		scrnstkpush(stk, invscrnnew(&gm->player, gm->zone->lvl));
		return;
	}

	playerhandle(&gm->player, e);

	if(gm->player.acting){
		int z = gm->zone->lvl->z;
		Env *ev = gm->zone->envs[z];
		for(int i = 0; i < Maxenvs; i++) {
			if (!ev[i].id)
				continue;
			envact(&ev[i], &gm->player, gm->zone->lvl);
			if(gm->player.statup){
				scrnstkpush(stk, statscrnnew(&gm->player, &ev[i]));
				return;
			}
		}
	}
}

Scrnmt gamemt = {
	gameupdate,
	gamedraw,
	gamehandle,
	gamefree,
};

