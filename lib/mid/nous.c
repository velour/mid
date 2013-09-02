/* © 2013 the Mid Authors under the MIT license. See AUTHORS for the list of authors.*/

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
	e->hitback = 0;
	e->iframes = 0;

	bodyinit(&e->body, x*Twidth+3, y*Theight, Twidth-3, Theight);

	Anim *a = xalloc(1, sizeof(*a));
	a->sheet = nousimg;
	a->row = 0;
	a->len = 4;
	a->delay = 100 / Ticktm;
	a->w = 32;
	a->h = 32;
	a->f = 0;
	a->d = a->delay;
	e->data = a;
	return 1;
}

void nousfree(Enemy *e){
	xfree(e->data);
}

void nousupdate(Enemy *e, Player *p, Zone *z){
	enemygenupdate(e, p, z, &nousinfo);
	animupdate((Anim*)e->data);
}

void nousdraw(Enemy *e, Gfx *g){
	Anim *a = e->data;
	if(e->body.vel.x < 0)
		a->row = 0;
	else
		a->row = 1;
	a->sheet = nousimg; // TODO: why is this necessary???
	Point p = e->body.bbox.a;
	p.x -= 3;
	camdrawanim(g, a, p);
}

_Bool nousscan(char *buf, Enemy *e){
	nousinit(e, 0, 0);
	if (!defaultscan(buf, e))
		return 0;

	aiwalker(&e->ai, 2);

	return 1;
}

_Bool nousprint(char *buf, size_t sz, Enemy *e){
	return defaultprint(buf, sz, e);
}
