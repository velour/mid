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

void daupdate(Enemy *e, Player *p, Lvl *l){
	if(!untihit)
		untihit = resrcacq(sfx, "sfx/hit.wav", 0);

	e->ai.update(e, p, l);
	bodyupdate(&e->b, l);

	if(e->iframes > 0)
		e->iframes--;

	if(isect(e->b.bbox, playerbox(p))){
		int dir = e->b.bbox.a.x > p->body.bbox.a.x ? -1 : 1;
		playerdmg(p, 1, dir);
	}

	if(e->iframes == 0 && p->sframes > 0 && isect(e->b.bbox, swordbbox(&p->sw))){
		sfxplay(untihit);
		e->hp -= swordstr(&p->sw, p);

		if(e->hp <= 0){
			Enemy splat = {};
			enemyinit(&splat, EnemySplat, 0, 0);
			splat.b = e->b;
			dafree(e);
			*e = splat;
			return;
		}

		e->iframes = 500.0/Ticktm;
	}
}

void dadraw(Enemy *e, Gfx *g){
	if(!daimg) daimg = resrcacq(imgs, "img/da.png", 0);

	if(e->iframes % 4 != 0)
		return;

	Rect clip;
	if(e->b.vel.x < 0)
		clip = (Rect){
			{ 0, 0 },
			{ 32, 32 }
		};
	else
		clip = (Rect){
			{ 32, 0 },
			{ 64, 32 }
		};
	camdrawreg(g, daimg, clip, e->b.bbox.a);
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
