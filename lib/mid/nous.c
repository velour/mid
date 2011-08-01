// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
#include "../../include/mid.h"
#include "enemy.h"
#include <stdio.h>

Img *nousimg;

_Bool nousinit(Enemy *e, int x, int y){
	e->hp = 1;
	e->data = 0;
	return 1;
}

void nousfree(Enemy *e){
}

void nousupdate(Enemy *e, Player *p, Lvl *l){
	if(!untihit)
		untihit = resrcacq(sfx, "sfx/hit.wav", 0);

	e->ai.update(e, p, l);
	bodyupdate(&e->body, l);

	if(isect(e->body.bbox, playerbox(p))){
		int dir = e->body.bbox.a.x > p->body.bbox.a.x ? -1 : 1;
		playerdmg(p, 1, dir);
	}

	if(p->sframes > 0 && isect(e->body.bbox, swordbbox(&p->sw))){
		sfxplay(untihit);
		e->hp--;

		if(e->hp <= 0){
			Enemy splat = {};
			enemyinit(&splat, EnemySplat, 0, 0);
			splat.body = e->body;
			nousfree(e);
			*e = splat;
			return;
		}
	}
}

void nousdraw(Enemy *e, Gfx *g){
	if(!nousimg) nousimg = resrcacq(imgs, "img/nous.png", 0);
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
	camdrawreg(g, nousimg, clip, e->body.bbox.a);
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
