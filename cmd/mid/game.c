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
	gm->zone= zonegen(50, 50, Maxz, seed);
	if (!gm->zone)
		fatal("Failed to load zone: %s", miderrstr());

	playerinit(&gm->player, 2, 2);
	if(!enemyinit(&gm->zone->enms[0][0], 'u', 4, 2))
		goto oops;

	_Bool ok = itemldresrc();
	if (!ok)
		fatal("Failed to load item resources: %s", miderrstr());

	return gm;

oops:
	zonefree(gm->zone);
	free(gm);
	return NULL;
}

void gamefree(Scrn *s)
{
	Game *gm = s->data;

	for (int z = 0; z < gm->zone->lvl->d; z++) {
		Enemy *e = gm->zone->enms[z];
		for(size_t i = 0; i < Maxenms; i++)
			if(e[i].mt) e[i].mt->free(&e[i]);
	}

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

	zonedraw(g, gm->zone, &gm->player, gm->transl);

	Rect hp = { { 1, 1 }, { gm->player.hp * 5, 16 } };
	Rect curhp = hp;
	curhp.b.x = gm->player.curhp * 5;
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
		for(Env *ev = gm->zone->envs[z]; ev != gm->zone->envs[z] + Maxenvs; ev++){
			if(!ev->id)
				continue;

			envact(ev, &gm->player, gm->zone->lvl);
			if(gm->player.statup){
				scrnstkpush(stk, statscrnnew(&gm->player, ev));
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

