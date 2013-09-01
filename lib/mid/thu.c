/* © 2013 the Mid Authors under the MIT license. See AUTHORS for the list of authors.*/

#include "../../include/mid.h"
#include "enemy.h"
#include <stdio.h>

Img *thuimg;

Info thuinfo = {
	.stats = {
		[StatHp] = 7,
		[StatDex] = 4,
		[StatStr] = 2,
	},
	.drops = {
		.item = { ItemNone, ItemNone },
		.prob = { 99, 1 }
	},
	.death = EnemySplat
};

_Bool thuinit(Enemy *e, int x, int y){
	e->hp = 7;
	Anim *a = xalloc(1, sizeof(*a));
	a->sheet = thuimg;
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

void thufree(Enemy *e){
	xfree(e->data);
}

void thuupdate(Enemy *e, Player *p, Zone *z){
	enemygenupdate(e, p, z, &thuinfo);
	Anim *a = e->data;
	if(e->body.vel.x == 0)
		a->f = 0;
	else
		animupdate(a);
}

void thudraw(Enemy *e, Gfx *g){
	if(e->iframes % 4 != 0)
		return;

	Anim *a = e->data;
	if(e->body.vel.x < 0)
		a->row = 0;
	else
		a->row = 1;
	a->sheet = thuimg; // TODO: why is this necessary???
	camdrawanim(g, a, e->body.bbox.a);
}

_Bool thuscan(char *buf, Enemy *e){
	*e = (Enemy){};
	thuinit(e, 0, 0);
	if (!defaultscan(buf, e))
		return 0;

	aichaser(&e->ai, 4, 32*3);

	return 1;
}

_Bool thuprint(char *buf, size_t sz, Enemy *e){
	return defaultprint(buf, sz, e);
}
