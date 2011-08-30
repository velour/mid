// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
#include "../../include/mid.h"
#include "enemy.h"
#include <stdio.h>

Img *daimg;

_Bool dainit(Enemy *e, int x, int y){
	e->hp = 12;
	e->data = 0;
	return 1;
}

void dafree(Enemy *e){
}

void daupdate(Enemy *e, Player *p, Zone *z){
	e->ai.update(e, p, z);
	bodyupdate(&e->body, z->lvl);

	if(e->iframes > 0)
		e->iframes--;

	if(isect(e->body.bbox, playerbox(p))){
		int dir = e->body.bbox.a.x > p->body.bbox.a.x ? -1 : 1;
		playerdmg(p, 1, dir);
	}

	if(e->iframes == 0 && p->sframes > 0 && isect(e->body.bbox, swordbbox(&p->sw))){
		sfxplay(untihit);
		e->hp -= swordstr(&p->sw, p);

		if(e->hp <= 0){
			Enemy splat = {};
			enemyinit(&splat, EnemySplat, 0, 0);
			splat.body = e->body;
			dafree(e);
			*e = splat;
			return;
		}

		e->iframes = 500.0/Ticktm;
	}
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
