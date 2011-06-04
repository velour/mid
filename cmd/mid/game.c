#include "../../include/mid.h"
#include "../../include/log.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include "game.h"
#include "resrc.h"

enum { Maxenms = 32 };

typedef struct Enms Enms;
struct Enms{
	Enemy *es;
	int n;
};

struct Game {
	Point transl;
	Lvl *lvl;
	Player *player;
	Inv inv;
	Enms enms[];
};

Game *gamenew(void)
{
	Lvl *lvl = resrcacq(lvls, "lvl/0.lvl", NULL);
	if (!lvl)
		fatal("Failed to load level lvl/0.lvl: %s", miderrstr());

	Game *gm = calloc(1, sizeof(*gm) + sizeof(Enms)*lvl->d);
	if (!gm){
		lvlfree(lvl);
		return NULL;
	}

	gm->lvl = lvl;
	gm->player = playernew(64, 96);
	gm->enms[0].es = calloc(1, sizeof(Enemy));
	gm->enms[0].n = 1;
	if(!enemyinit(&gm->enms[0].es[0], 'u', (Point){128,96})){
		lvlfree(lvl);
		playerfree(gm->player);
		free(gm->enms[0].es);
		free(gm);
		return NULL;
	}
	gm->enms[1].n = 0;

	// Testing items
	Item *axe = itemnew("Golden Pickaxe", "gaxe/anim");
	invmod(&gm->inv, axe, 0, 0);
	axe = itemnew("Golden Pickaxe", "gaxe/anim");
	invmod(&gm->inv, axe, 1, 0);
	axe = itemnew("Golden Pickaxe", "gaxe/anim");
	invmod(&gm->inv, axe, 1, 1);

	return gm;
}

void gamefree(Scrn *s)
{
	Game *gm = s->data;
	playerfree(gm->player);

	Enms es = gm->enms[gm->lvl->z];
	Enemy *e = es.es;
	int n = es.n;
	for(size_t i = 0; i < n; i++)
		e[i].mt->free(&e[i]);

	resrcrel(lvls, "lvl/0.lvl", NULL);
	free(gm);
}

void gameupdate(Scrn *s, Scrnstk *stk)
{
	Game *gm = s->data;
	lvlupdate(anim, gm->lvl);
	playerupdate(gm->player, gm->lvl, &gm->transl);

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
	lvldraw(g, anim, gm->lvl, true, gm->transl);
	playerdraw(g, gm->player, gm->transl);

	Enms es = gm->enms[gm->lvl->z];
	Enemy *e = es.es;
	int n = es.n;
	for(size_t i = 0; i < n; i++)
		e[i].mt->draw(&e[i], g, gm->transl);

	lvldraw(g, anim, gm->lvl, false, gm->transl);
	gfxflip(g);
}

void gamehandle(Scrn *s, Scrnstk *stk, Event *e)
{
	if(e->type != Keychng || e->repeat)
		return;

	Game *gm = s->data;

	if(e->down && e->key == kmap[Mvinv]){
		scrnstkpush(stk, invscrnnew(&gm->inv, gm->lvl, playerpos(gm->player)));
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

