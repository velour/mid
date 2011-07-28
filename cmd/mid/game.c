// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
#include "../../include/mid.h"
#include "../../include/log.h"
#include "../../include/rng.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include "game.h"

struct Game {
	Player player;
	Point transl;
	_Bool died;
	int znum, zmax;
	Zone *zone;
	Rng rng;
};

Game *gamenew(void)
{
	static Game gm = {};

	lvlinit();

	unsigned int seed = time(NULL) ^ getpid();
	rnginit(&gm.rng, seed);
	pr("game seed: %u", seed);

	gm.zone = zonegen(&gm.rng);
	if (!gm.zone)
		fatal("Failed to load zone: %s", miderrstr());

	playerinit(&gm.player, 2, 2);

	_Bool ok = itemldresrc();
	if (!ok)
		fatal("Failed to load item resources: %s", miderrstr());
	ok = envldresrc();
	if (!ok)
		fatal("Failed to load env resources: %s", miderrstr());
	if(!swordldresrc())
		fatal("Failed to load item resrouces: %s", miderrstr());

	return &gm;
}

void gamefree(Scrn *s)
{
	Game *gm = s->data;
	zonefree(gm->zone);
	zonecleanup(gm->zmax);
	*gm = (Game){};
}

static void trystairs(Scrnstk *stk, Game *gm)
{
	if (gm->zone->updown == Gonone)
		return;

	Point loc0 = gm->player.body.bbox.a;
	zoneput(gm->zone, gm->znum);

	if (gm->zone->updown == Goup) {
		gm->znum--;
		if (gm->znum < 0) {
			pr("You just left the dungeon");
			scrnstkpush(stk, goverscrnnew(&gm->player, gm->znum));
			return;
		}
		gm->zone = zoneget(gm->znum);

		Tileinfo bi = zonedstairs(gm->zone);
		gm->zone->lvl->z = bi.z;
		playersetloc(&gm->player, bi.x, bi.y);
	} else if (gm->zone->updown == Godown) {
		zoneput(gm->zone, gm->znum);
		gm->znum++;
		if (gm->znum > gm->zmax) {
			gm->zmax = gm->znum;
			gm->zone = zonegen(&gm->rng);
		} else {
			gm->zone = zoneget(gm->znum);
		}

		playersetloc(&gm->player, 2, 2);
	}

	Point loc1 = gm->player.body.bbox.a;
	gm->transl = (Point) {
		gm->transl.x + loc0.x - loc1.x,
		gm->transl.y + loc0.y - loc1.y
	};
}

void gameupdate(Scrn *s, Scrnstk *stk)
{
	Game *gm = s->data;

	zoneupdate(gm->zone, &gm->player, &gm->transl);
	trystairs(stk, gm);
	if(gm->player.eqp[StatHp] <= 0 && !debugging){
		if(gm->player.lives == 0)
			scrnstkpush(stk, goverscrnnew(&gm->player, gm->znum));
		else{
			gm->died = 1;
			gm->player.eqp[StatHp] = gm->player.stats[StatHp];
			Player p = gm->player;
			playerinit(&p, 2, 2);
			gm->player.body = p.body;
			gm->player.imgloc = p.imgloc;
			gm->player.hitback = 0;
			gm->player.sframes = 0;
			gm->player.lives--;
			gm->transl.x = 0;
			gm->transl.y = 0;
			gm->zone->lvl->z = 0;

			int lose = rngintincl(&gm->rng, 0, Maxinv-1);
			gm->player.inv[lose] = (Invit){};
		}
	}
}

void gamedraw(Scrn *s, Gfx *g)
{
	Game *gm = s->data;

	gfxclear(g, (Color){ 0, 0, 0, 0 });

	if(gm->died){
		gm->died = 0;
		camreset(g);
	}else
		cammove(g, gm->transl.x, gm->transl.y);

	zonedraw(g, gm->zone, &gm->player);

	Rect hp = { { 1, 1 }, { gm->player.stats[StatHp] * 5, 16 } };
	Rect curhp = hp;
	curhp.b.x = gm->player.eqp[StatHp] * 5;
	gfxfillrect(g, hp, (Color){ 200 });
	gfxfillrect(g, curhp, (Color){ 0, 200, 200 });
	gfxdrawrect(g, hp, (Color){0});

	for(int i = 0; i < gm->player.lives; i++){
		Rect life = {
			{ 1 + i*16 + i*1, 16 + 2 },
			{ 1 + i*16 + i*1 + 16, 16 + 2 + 16}
		};
		gfxfillrect(g, life, (Color){200});
		gfxdrawrect(g, life, (Color){0});
	}

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

