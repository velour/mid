/* © 2013 the Mid Authors under the MIT license. See AUTHORS for the list of authors.*/

#include "../../include/mid.h"
#include "enemy.h"
#include <stdio.h>

Img *thuimg;

Info thuinfo = {
	.stats = {
		[StatHp] = 7,
		[StatDex] = 4,
		[StatStr] = 2,
	},
	.drops = {
		.item = { ItemNone, ItemNone },
		.prob = { 99, 1 }
	},
	.death = EnemySplat
};

_Bool thuinit(Enemy *e, int x, int y){
	e->hp = 7;
	e->data = 0;
	return 1;
}

void thufree(Enemy *e){
}

void thuupdate(Enemy *e, Player *p, Zone *z){
	enemygenupdate(e, p, z, &thuinfo);
}

void thudraw(Enemy *e, Gfx *g){
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
	camdrawreg(g, thuimg, clip, e->body.bbox.a);
}

_Bool thuscan(char *buf, Enemy *e){
	*e = (Enemy){};
	if (!defaultscan(buf, e))
		return 0;

	aichaser(&e->ai, 4, 32*3);

	return 1;
}

_Bool thuprint(char *buf, size_t sz, Enemy *e){
	return defaultprint(buf, sz, e);
}
