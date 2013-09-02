/* © 2013 the Mid Authors under the MIT license. See AUTHORS for the list of authors.*/

#include "../../include/mid.h"
#include "enemy.h"
#include <stdio.h>

Img *grenduimg;

Info grenduinfo = {
	.stats = {
		[StatHp] = 7,
		[StatDex] = 2,
		[StatStr] = 2,
	},
	.drops = {
		.item = { ItemSilver, ItemGold },
		.prob = { 85, 15 }
	},
	.death = EnemySplat
};

_Bool grenduinit(Enemy *e, int x, int y){
	e->hp = 7;
	Anim *a = xalloc(1, sizeof(*a));
	a->sheet = grenduimg;
	a->row = 0;
	a->len = 4;
	a->delay = 100 / Ticktm;
	a->w = 32;
	a->h = 32;
	a->f = 0;
	a->d = a->delay;
	e->data = a;
	return 1;
}

void grendufree(Enemy *e){
	xfree(e->data);
}

void grenduupdate(Enemy *e, Player *p, Zone *z){
	enemygenupdate(e, p, z, &grenduinfo);
	animupdate((Anim*)e->data);
}

void grendudraw(Enemy *e, Gfx *g){
	if(e->iframes % 4 != 0)
		return;

	Anim *a = e->data;
	if(e->body.vel.x < 0)
		a->row = 0;
	else
		a->row = 1;

	if(e->body.vel.y < 0){ // jumping?
		a->f = 0;
		a->row += 2;
	}

	a->sheet = grenduimg; // TODO: why is this necessary???
	camdrawanim(g, a, e->body.bbox.a);
}

_Bool grenduscan(char *buf, Enemy *e){
	*e = (Enemy){};
	grenduinit(e, 0, 0);
	if (!defaultscan(buf, e))
		return 0;

	aihunter(&e->ai, 8, 2, 32*3);

	return 1;
}

_Bool grenduprint(char *buf, size_t sz, Enemy *e){
	return defaultprint(buf, sz, e);
}
