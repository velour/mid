// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
#include "../../include/mid.h"
#include "enemy.h"
#include <stdio.h>

Img *nousimg;

Info nousinfo = {
	.stats = {
		[StatHp] = 1,
		[StatDex] = 5,
		[StatStr] = 1,
	},
	.drops = {
		.item = { ItemCopper, ItemSilver },
		.prob = { 85, 15 }
	},
	.death = EnemySplat
};

_Bool nousinit(Enemy *e, int x, int y){
	e->hp = 1;
	e->data = 0;
	e->hitback = 0;
	e->iframes = 0;

	bodyinit(&e->body, x*Twidth+3, y*Theight, Twidth-3, Theight);
	return 1;
}

void nousfree(Enemy *e){
}

void nousupdate(Enemy *e, Player *p, Zone *z){
	enemygenupdate(e, p, z, &nousinfo);
}

void nousdraw(Enemy *e, Gfx *g){
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

	Point p = e->body.bbox.a;
	p.x -= 3;
	camdrawreg(g, nousimg, clip, p);
}

_Bool nousscan(char *buf, Enemy *e){
	if (!defaultscan(buf, e))
		return 0;

	aiwalker(&e->ai, 2);

	return 1;
}

_Bool nousprint(char *buf, size_t sz, Enemy *e){
	return defaultprint(buf, sz, e);
}
