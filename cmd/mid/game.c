#include "../../include/mid.h"
#include "../../include/log.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include "game.h"

enum {
	Maxenms = 32,
	Maxitms = 1, //TODO: more than 1
};

extern Lvl *lvlgen(int w, int h, int d, int sd);

typedef struct Enms Enms;
struct Enms{
	Enemy *es;
	int n;
};

struct Game {
	Point transl;
	Lvl *lvl;
	Player *player;
	Item itms[Maxitms];
	Enms enms[];
};

Game *gamenew(void)
{
	lvlinit();
	int seed = rand();
	Lvl *lvl = lvlgen(50, 50, 5, seed);
	if (!lvl)
		fatal("Failed to load level lvl/0.lvl: %s", miderrstr());

	Game *gm = calloc(1, sizeof(*gm) + sizeof(Enms[lvl->d]));
	if (!gm){
		lvlfree(lvl);
		return NULL;
	}

	gm->lvl = lvl;
	gm->player = playernew(2, 2);
	gm->enms[0].es = calloc(1, sizeof(Enemy));
	gm->enms[0].n = 1;
	if(!enemyinit(&gm->enms[0].es[0], 'u', 4, 2))
		goto oops;

	if(!iteminit(gm->itms, ItemStatup, (Point){3,1})){
		gm->enms[0].es[0].mt->free(&gm->enms[0].es[0]);
		goto oops;
	}

	return gm;

oops:
	lvlfree(lvl);
	playerfree(gm->player);
	free(gm->enms[0].es);
	free(gm);
	return NULL;
}

void gamefree(Scrn *s)
{
	Game *gm = s->data;
	playerfree(gm->player);

	for (int z = 0; z < gm->lvl->d; z++) {
		Enms es = gm->enms[z];
		Enemy *e = es.es;
		if (!e)
			continue;
		for(size_t i = 0; i < es.n; i++)
			e[i].mt->free(&e[i]);
	}

	lvlfree(gm->lvl);
	free(gm);
}

void gameupdate(Scrn *s, Scrnstk *stk)
{
	Game *gm = s->data;
	lvlupdate(gm->lvl);
	playerupdate(gm->player, gm->lvl, &gm->transl);

	for(size_t i = 0; i < Maxitms; i++)
		itemupdate(&gm->itms[i], gm->player, gm->lvl);

	Enms es = gm->enms[gm->lvl->z];
	Enemy *e = es.es;
	int n = es.n;
	for(size_t i = 0; i < n; i++)
		e[i].mt->update(&e[i], gm->player, gm->lvl);
}

void gamedraw(Scrn *s, Gfx *g)
{
	Game *gm = s->data;
	gfxclear(g, (Color){ 0, 0, 0, 0 });
	lvldraw(g, gm->lvl, true, gm->transl);
	playerdraw(g, gm->player, gm->transl);

	for(size_t i = 0; i < Maxitms; i++)
		itemdraw(&gm->itms[i], g, gm->transl);

	Enms es = gm->enms[gm->lvl->z];
	Enemy *e = es.es;
	int n = es.n;
	for(size_t i = 0; i < n; i++)
		e[i].mt->draw(&e[i], g, gm->transl);

	lvldraw(g, gm->lvl, false, gm->transl);
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
}

Scrnmt gamemt = {
	gameupdate,
	gamedraw,
	gamehandle,
	gamefree,
};

