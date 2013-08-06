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
	e->data = 0;
	return 1;
}

void grendufree(Enemy *e){
}

void grenduupdate(Enemy *e, Player *p, Zone *z){
	enemygenupdate(e, p, z, &grenduinfo);
}

void grendudraw(Enemy *e, Gfx *g){
	if(e->iframes % 4 != 0)
		return;

	Rect clip;
	if(e->body.vel.x < 0)
		clip = (Rect){
			{ 0, 0 },
			{ 32, 32 }
		};
	else
		clip = (Rect){
			{ 32, 0 },
			{ 64, 32 }
		};
	camdrawreg(g, grenduimg, clip, e->body.bbox.a);
}

_Bool grenduscan(char *buf, Enemy *e){
	*e = (Enemy){};
	if (!defaultscan(buf, e))
		return 0;

	aihunter(&e->ai, 8, 2, 32*3);

	return 1;
}

_Bool grenduprint(char *buf, size_t sz, Enemy *e){
	return defaultprint(buf, sz, e);
}
