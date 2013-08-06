/* © 2013 the Mid Authors under the MIT license. See AUTHORS for the list of authors.*/

#include "../../include/mid.h"
#include "enemy.h"
#include <stdio.h>

Sfx *untihit;
Img *untiimg;

Info untiinfo = {
	.stats = {
		[StatHp] = 10,
		[StatDex] = 8,
		[StatStr] = 3,
	},
	.drops = {
		.item = { ItemNone, ItemTopHat },
		.prob = { 95, 5 }
	},
	.death = EnemySplat
};

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

void untiupdate(Enemy *e, Player *p, Zone *z){
	enemygenupdate(e, p, z, &untiinfo);
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
