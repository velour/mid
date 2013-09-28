/* © 2013 the Mid Authors under the MIT license. See AUTHORS for the list of authors.*/

#include "../../include/mid.h"
#include "enemy.h"
#include <stdio.h>

Img *tihgtimg;

Info tihgtinfo = {
	.stats = {
		[StatHp] = 30,
		[StatDex] = 4,
		[StatStr] = 4,
	},
	.drops = {
		.item = { ItemNone, ItemPlotBody },
		.prob = { 95, 5 }
	},
	.death = EnemySplat
};

_Bool tihgtinit(Enemy *e, int x, int y){
	e->hp = tihgtinfo.stats[StatHp];
	Anim *a = xalloc(1, sizeof(*a));
	a->sheet = tihgtimg;
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

void tihgtfree(Enemy *e){
	xfree(e->data);
}

void tihgtupdate(Enemy *e, Player *p, Zone *z){
	enemygenupdate(e, p, z, &tihgtinfo);
	animupdate((Anim*)e->data);
}

void tihgtdraw(Enemy *e, Gfx *g){
	if(e->iframes % 4 != 0)
		return;

	Anim *a = e->data;
	if(e->body.vel.x < 0)
		a->row = 0;
	else
		a->row = 1;

	if(e->body.vel.y < 0){ // jumping?
		a->f = 0;
		a->row += 2;
	}

	a->sheet = tihgtimg; // TODO: why is this necessary???
	camdrawanim(g, a, e->body.bbox.a);
}

_Bool tihgtscan(char *buf, Enemy *e){
	*e = (Enemy){};
	tihgtinit(e, 0, 0);
	if (!defaultscan(buf, e))
		return 0;

	aihunter(&e->ai, 8, 2, 32*6);

	return 1;
}

_Bool tihgtprint(char *buf, size_t sz, Enemy *e){
	return defaultprint(buf, sz, e);
}
