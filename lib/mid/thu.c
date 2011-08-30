// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
#include "../../include/mid.h"
#include "enemy.h"
#include <stdio.h>

Img *thuimg;

_Bool thuinit(Enemy *e, int x, int y){
	e->hp = 7;
	e->data = 0;
	return 1;
}

void thufree(Enemy *e){
}

void thuupdate(Enemy *e, Player *p, Zone *z){
	e->ai.update(e, p, z);

	if(e->iframes > 0){
		e->body.vel.x = e->hitback;
		e->iframes--;
	}

	if(e->iframes <= 0)
		e->hitback = 0;

	bodyupdate(&e->body, z->lvl);

	Rect pbbox = playerbox(p);

	if(isect(e->body.bbox, pbbox)){
		int dir = e->body.bbox.a.x > pbbox.a.x ? -1 : 1;
		playerdmg(p, 2, dir);
	}

	Rect swbb = swordbbox(&p->sw);

	if(e->iframes == 0 && p->sframes > 0 && isect(e->body.bbox, swbb)){
		sfxplay(untihit);
		int pstr = swordstr(&p->sw, p);
		e->hp -= pstr;

		int mhb = 5;
		if(pstr > mhb*2)
			mhb = pstr/2;
		if(mhb > 32)
			mhb = 32;
		e->hitback = pbbox.a.x < e->body.bbox.a.x ? mhb : -mhb;
		e->iframes = 500.0 / Ticktm; // 0.5s
		if(e->hp <= 0){
			Enemy splat = {};
			enemyinit(&splat, EnemySplat, 0, 0);
			splat.body = e->body;
			dafree(e);
			*e = splat;
			return;
		}
	}
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
