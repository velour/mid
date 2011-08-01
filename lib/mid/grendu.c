// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
#include "../../include/mid.h"
#include "enemy.h"
#include <stdio.h>

Img *grenduimg;

_Bool grenduinit(Enemy *e, int x, int y){
	e->hp = 7;
	e->data = 0;
	return 1;
}

void grendufree(Enemy *e){
}

void grenduupdate(Enemy *e, Player *p, Lvl *l){
	if(!untihit)
		untihit = resrcacq(sfx, "sfx/hit.wav", 0);

	e->ai.update(e, p, l);

	if(e->iframes > 0){
		e->b.vel.x = e->hitback;
		e->iframes--;
	}

	if(e->iframes <= 0)
		e->hitback = 0;

	bodyupdate(&e->b, l);

	if(isect(e->b.bbox, playerbox(p))){
		int dir = e->b.bbox.a.x > p->body.bbox.a.x ? -1 : 1;
		playerdmg(p, 2, dir);
	}

	Rect swbb = swordbbox(&p->sw);

	if(e->iframes == 0 && p->sframes > 0 && isect(e->b.bbox, swbb)){
		sfxplay(untihit);
		int pstr = swordstr(&p->sw, p);
		e->hp -= pstr;

		int mhb = 5;
		if(pstr > mhb*2)
			mhb = pstr/2;
		if(mhb > 32)
			mhb = 32;
		e->hitback = swbb.a.x < e->b.bbox.a.x ? mhb : -mhb;
		e->iframes = 500.0 / Ticktm; // 0.5s
		if(e->hp <= 0){
			Enemy splat = {};
			enemyinit(&splat, EnemySplat, 0, 0);
			splat.b = e->b;
			dafree(e);
			*e = splat;
			return;
		}
	}
}

void grendudraw(Enemy *e, Gfx *g){
	if(!grenduimg) grenduimg = resrcacq(imgs, "img/grendu.png", 0);

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
	camdrawreg(g, grenduimg, clip, e->b.bbox.a);
}

_Bool grenduscan(char *buf, Enemy *e){
	*e = (Enemy){};
	if (!defaultscan(buf, e))
		return 0;

	aihunter(&e->ai, 8, 4, 32*3);

	return 1;
}

_Bool grenduprint(char *buf, size_t sz, Enemy *e){
	return defaultprint(buf, sz, e);
}
