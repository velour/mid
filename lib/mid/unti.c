// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
#include "../../include/mid.h"
#include "enemy.h"
#include <stdio.h>

Sfx *untihit;
Img *untiimg;

typedef struct Unti Unti;
struct Unti{
	Color c;
	Img *img;
};

_Bool untiinit(Enemy *e, int x, int y){
	e->hp = 10;
	e->hitback = 0;
	e->iframes = 0;

	Unti *u = xalloc(1, sizeof(*u));
	u->c = (Color){ 255, 55, 55, 255 };

	e->data = u;
	return 1;
}

void untifree(Enemy *e){
	xfree(e->data);
}

void untiupdate(Enemy *e, Player *p, Lvl *l){
	Unti *u = e->data;

	e->ai.update(e, p, l);

	if(e->iframes > 0){
		e->body.vel.x = e->hitback;
		e->iframes--;
	}else
		e->body.vel.x = 0;

	if(e->iframes < 250.0/Ticktm)
		e->hitback = 0;

	bodyupdate(&e->body, l);

	if(isect(e->body.bbox, playerbox(p))){
		int dir = e->body.bbox.a.x > p->body.bbox.a.x ? -1 : 1;
		u->c.b = 255;
		playerdmg(p, 3, dir);
	}else
		u->c.b = 55;

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
		e->hitback = swbb.a.x < e->body.bbox.a.x ? mhb : -mhb;
		e->iframes = 500.0 / Ticktm; // 0.5s

		if(e->hp <= 0){
			Enemy splat = {};
			enemyinit(&splat, EnemySplat, 0, 0);
			splat.body = e->body;
			untifree(e);
			*e = splat;
			return;
		}
	}

	u->c.r++;
}

void untidraw(Enemy *e, Gfx *g){
	if(e->iframes % 4 == 0)
		camdrawimg(g, untiimg, e->body.bbox.a);
}

_Bool untiscan(char *buf, Enemy *e){
	int r, g, b, a;

	if (!scangeom(buf, "dyddddd", &e->id, &e->body, &e->hp, &r, &g, &b, &a))
		return 0;

	e->hitback = 0;
	e->iframes = 0;

	Unti *u = xalloc(1, sizeof(*u));
	u->c = (Color){ r, g, b, a };
	e->data = u;
	aijumper(&e->ai, 8);

	return 1;
}

_Bool untiprint(char *buf, size_t sz, Enemy *e){
	Unti *u = e->data;
	Color c = u->c;
	return printgeom(buf, sz, "dyddddd", e->id, e->body, e->hp, c.r, c.g, c.b, c.a);
}
