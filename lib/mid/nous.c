// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
#include "../../include/mid.h"
#include "enemy.h"
#include <stdio.h>

_Bool nousinit(Enemy *e, int x, int y){
	e->hp = 1;
	e->data = 0;
	return 1;
}

void nousfree(Enemy *e){
	resrcrel(imgs, "img/unti.png", 0);
}

void nousupdate(Enemy *e, Player *p, Lvl *l){
	if(!untihit)
		untihit = resrcacq(sfx, "sfx/hit.wav", 0);

	e->ai.update(e, p, l);
	bodyupdate(&e->b, l);

	if(isect(e->b.bbox, playerbox(p))){
		int dir = e->b.bbox.a.x > p->body.bbox.a.x ? -1 : 1;
		playerdmg(p, 2, dir);
	}

	if(p->sframes > 0 && isect(e->b.bbox, swordbbox(&p->sw))){
		sfxplay(untihit);
		e->hp--;
	}
}

void nousdraw(Enemy *e, Gfx *g){
	if(!untiimg) untiimg = resrcacq(imgs, "img/unti.png", 0);
	camdrawimg(g, untiimg, e->b.bbox.a);
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
