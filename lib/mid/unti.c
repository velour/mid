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
	resrcrel(imgs, "img/unti.png", 0);
	xfree(e->data);
}

void untiupdate(Enemy *e, Player *p, Lvl *l){
	if(!untihit)
		untihit = resrcacq(sfx, "sfx/hit.wav", 0);

	Unti *u = e->data;

	e->ai.update(e, p, l);

	if(e->iframes > 0){
		e->b.vel.x = e->hitback;
		e->iframes--;
	}else
		e->b.vel.x = 0;

	if(e->iframes < 250.0/Ticktm)
		e->hitback = 0;

	bodyupdate(&e->b, l);

	if(isect(e->b.bbox, playerbox(p))){
		int dir = e->b.bbox.a.x > p->body.bbox.a.x ? -1 : 1;
		u->c.b = 255;
		playerdmg(p, 3, dir);
	}else
		u->c.b = 55;

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
	}

	u->c.r++;
}

void untidraw(Enemy *e, Gfx *g){
	if(!untiimg) untiimg = resrcacq(imgs, "img/unti.png", 0);

	if(e->iframes % 4 == 0)
		camdrawimg(g, untiimg, e->b.bbox.a);
}

_Bool untiscan(char *buf, Enemy *e){
	int r, g, b, a;

	if (!scangeom(buf, "dyddddd", &e->id, &e->b, &e->hp, &r, &g, &b, &a))
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
	return printgeom(buf, sz, "dyddddd", e->id, e->b, e->hp, c.r, c.g, c.b, c.a);
}
