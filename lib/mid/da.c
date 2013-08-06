/* © 2013 the Mid Authors under the MIT license. See AUTHORS for the list of authors.*/

#include "../../include/mid.h"
#include "enemy.h"
#include <stdio.h>

Img *daimg;

Info dainfo = {
	.stats = {
		[StatHp] = 12,
		[StatDex] = 3,
		[StatStr] = 1,
	},
	.drops = {
		.item = { ItemNone, ItemBroadSwd },
		.prob = { 80, 20 }
	},
	.death = EnemySplat
};

_Bool dainit(Enemy *e, int x, int y){
	e->hp = 12;
	e->data = 0;
	return 1;
}

void dafree(Enemy *e){
}

void daupdate(Enemy *e, Player *p, Zone *z){
	enemygenupdate(e, p, z, &dainfo);
}

void dadraw(Enemy *e, Gfx *g){
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
	camdrawreg(g, daimg, clip, e->body.bbox.a);
}

_Bool dascan(char *buf, Enemy *e){
	*e = (Enemy){};
	if (!defaultscan(buf, e))
		return 0;

	aipatroller(&e->ai, 3);

	return 1;
}

_Bool daprint(char *buf, size_t sz, Enemy *e){
	return defaultprint(buf, sz, e);
}
