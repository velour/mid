#include "../../include/mid.h"
#include "../../include/log.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include "game.h"

struct Game {
	Player *player;
	Point transl;
	Zone *zone;
};

Game *gamenew(void)
{
	lvlinit();
	int seed = rand();
	Lvl *lvl = lvlgen(50, 50, Maxz, seed);
	if (!lvl)
		fatal("Failed to load level lvl/0.lvl: %s", miderrstr());

	Game *gm = xalloc(1, sizeof(*gm));


	/* for now */
	gm->zone = xalloc(1, sizeof(*gm->zone));

	gm->zone->lvl = lvl;
	gm->player = playernew(2, 2);
	if(!enemyinit(&gm->zone->enms[0][0], 'u', 4, 2))
		goto oops;

	if(!iteminit(gm->zone->itms[0], ItemStatup, (Point){3,1})){
		gm->zone->enms[0][0].mt->free(&gm->zone->enms[0][0]);
		goto oops;
	}

	for(int j = 5; j < 11; j++)
		iteminit(&gm->zone->itms[0][j], ItemCopper, (Point){j,1});

	envinit(&gm->zone->envs[0][0], EnvShrempty, (Point){2,1});

	return gm;

oops:
	zonefree(gm->zone);
	playerfree(gm->player);
	free(gm);
	return NULL;
}

void gamefree(Scrn *s)
{
	Game *gm = s->data;
	playerfree(gm->player);

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
	int z = gm->zone->lvl->z;

	lvlupdate(gm->zone->lvl);
	playerupdate(gm->player, gm->zone->lvl, &gm->transl);

	itemupdateanims();

	Item *itms = gm->zone->itms[z];
	for(size_t i = 0; i < Maxitms; i++)
		if(itms[i].id)
			itemupdate(&itms[i], gm->player, gm->zone->lvl);

	envupdateanims();

	Env *en = gm->zone->envs[z];
	for(size_t i = 0; i < Maxenvs; i++)
		if(en[i].id) envupdate(&en[i], gm->zone->lvl);

	Enemy *e = gm->zone->enms[z];
	for(size_t i = 0; i < Maxenms; i++)
		if(e[i].mt) e[i].mt->update(&e[i], gm->player, gm->zone->lvl);

	if(gm->player->curhp <= 0)
		scrnstkpush(stk, goverscrnnew(gm->player));
}

void gamedraw(Scrn *s, Gfx *g)
{
	Game *gm = s->data;
	int z = gm->zone->lvl->z;

	gfxclear(g, (Color){ 0, 0, 0, 0 });
	lvldraw(g, gm->zone->lvl, true, gm->transl);

	Env *en = gm->zone->envs[z];
	for(size_t i = 0; i < Maxenvs; i++)
		if(en[i].id) envdraw(&en[i], g, gm->transl);

	playerdraw(g, gm->player, gm->transl);

	Item *itms = gm->zone->itms[z];
	for(size_t i = 0; i < Maxitms; i++)
		if(itms[i].id)
			itemdraw(&itms[i], g, gm->transl);

	Enemy *e = gm->zone->enms[z];
	for(size_t i = 0; i < Maxenms; i++)
		if(e[i].mt) e[i].mt->draw(&e[i], g, gm->transl);

	lvldraw(g, gm->zone->lvl, false, gm->transl);

	Rect hp = { { 1, 1 }, { gm->player->hp * 5, 16 } };
	Rect curhp = hp;
	curhp.b.x = gm->player->curhp * 5;
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
		scrnstkpush(stk, invscrnnew(gm->player, gm->zone->lvl));
		return;
	}

	playerhandle(gm->player, e);

	if(gm->player->acting){
		int z = gm->zone->lvl->z;
		for(Env *ev = gm->zone->envs[z]; ev != gm->zone->envs[z] + Maxenvs; ev++){
			if(!ev->id)
				continue;

			envact(ev, gm->player, gm->zone->lvl);
			if(gm->player->statup){
				scrnstkpush(stk, statscrnnew(gm->player, ev));
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

