#include "../../include/mid.h"
#include "../../include/log.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include "game.h"

enum {
	Maxenms = 32,
	Maxitms = 32,
	Maxenvs = 16,
	Maxz = 5,
};

struct Game {
	Point transl;
	Lvl *lvl;
	Player *player;
	Item itms[Maxz][Maxitms];
	Env envs[Maxz][Maxenvs];
	Enemy enms[Maxz][Maxenms];
};

Game *gamenew(void)
{
	lvlinit();
	int seed = rand();
	Lvl *lvl = lvlgen(50, 50, Maxz, seed);
	if (!lvl)
		fatal("Failed to load level lvl/0.lvl: %s", miderrstr());

	Game *gm = xalloc(1, sizeof(*gm));

	gm->lvl = lvl;
	gm->player = playernew(2, 2);
	if(!enemyinit(&gm->enms[0][0], 'u', 4, 2))
		goto oops;

	if(!iteminit(gm->itms[0], ItemStatup, (Point){3,1})){
		gm->enms[0][0].mt->free(&gm->enms[0][0]);
		goto oops;
	}

	for(int j = 5; j < 11; j++)
		iteminit(&gm->itms[0][j], ItemCopper, (Point){j,1});

	envinit(&gm->envs[0][0], EnvShrempty, (Point){2,1});

	return gm;

oops:
	lvlfree(lvl);
	playerfree(gm->player);
	free(gm);
	return NULL;
}

void gamefree(Scrn *s)
{
	Game *gm = s->data;
	playerfree(gm->player);

	for (int z = 0; z < gm->lvl->d; z++) {
		Enemy *e = gm->enms[z];
		for(size_t i = 0; i < Maxenms; i++)
			if(e[i].mt) e[i].mt->free(&e[i]);
	}

	lvlfree(gm->lvl);
	free(gm);
}

void gameupdate(Scrn *s, Scrnstk *stk)
{
	Game *gm = s->data;
	int z = gm->lvl->z;

	lvlupdate(gm->lvl);
	playerupdate(gm->player, gm->lvl, &gm->transl);

	itemupdateanims();

	Item *itms = gm->itms[z];
	for(size_t i = 0; i < Maxitms; i++)
		if(itms[i].id)
			itemupdate(&itms[i], gm->player, gm->lvl);

	envupdateanims();

	Env *en = gm->envs[z];
	for(size_t i = 0; i < Maxenvs; i++)
		if(en[i].id) envupdate(&en[i], gm->lvl);

	Enemy *e = gm->enms[z];
	for(size_t i = 0; i < Maxenms; i++)
		if(e[i].mt) e[i].mt->update(&e[i], gm->player, gm->lvl);
}

void gamedraw(Scrn *s, Gfx *g)
{
	Game *gm = s->data;
	int z = gm->lvl->z;

	gfxclear(g, (Color){ 0, 0, 0, 0 });
	lvldraw(g, gm->lvl, true, gm->transl);

	Env *en = gm->envs[z];
	for(size_t i = 0; i < Maxenvs; i++)
		if(en[i].id) envdraw(&en[i], g, gm->transl);

	playerdraw(g, gm->player, gm->transl);

	Item *itms = gm->itms[z];
	for(size_t i = 0; i < Maxitms; i++)
		if(itms[i].id)
			itemdraw(&itms[i], g, gm->transl);

	Enemy *e = gm->enms[z];
	for(size_t i = 0; i < Maxenms; i++)
		if(e[i].mt) e[i].mt->draw(&e[i], g, gm->transl);

	lvldraw(g, gm->lvl, false, gm->transl);

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
		scrnstkpush(stk, invscrnnew(gm->player, gm->lvl));
		return;
	}

	playerhandle(gm->player, e);

	if(gm->player->acting){
		int z = gm->lvl->z;
		for(Env *ev = gm->envs[z]; ev != gm->envs[z] + Maxenvs; ev++){
			if(!ev->id)
				continue;

			envact(ev, gm->player, gm->lvl);
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

