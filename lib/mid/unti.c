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
	e->hp = 1;

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
	bodyupdate(&e->b, l);

	if(isect(e->b.bbox, playerbox(p))){
		int dir = e->b.bbox.a.x > p->body.bbox.a.x ? -1 : 1;
		u->c.b = 255;
		playerdmg(p, 3, dir);
	}else
		u->c.b = 55;

	if(p->sframes > 0 && isect(e->b.bbox, swordbbox(&p->sw))){
		sfxplay(untihit);
		e->hp--;
	}

	u->c.r++;
}

void untidraw(Enemy *e, Gfx *g){
	if(!untiimg) untiimg = resrcacq(imgs, "img/unti.png", 0);

	//gfxfillrect(g, r, u->c);
	camdrawimg(g, untiimg, e->b.bbox.a);
}

_Bool untiscan(char *buf, Enemy *e){
	int r, g, b, a;

	if (!scangeom(buf, "dyddddd", &e->id, &e->b, &e->hp, &r, &g, &b, &a))
		return 0;

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
