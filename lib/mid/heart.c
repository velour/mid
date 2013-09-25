/* © 2013 the Mid Authors under the MIT license. See AUTHORS for the list of authors.*/

#include "../../include/mid.h"
#include "enemy.h"
#include <stdio.h>

static void die(Enemy *e, Zone *z);

Sfx *hearthit;
Img *heartimg;

Info heartinfo = {
	.stats = {
		[StatHp] = 100,
		[StatStr] = 10,
	},
	.drops = {
		.item = { ItemDjewel, ItemNone },
		.prob = { 100, 0 }
	},
	.death = EnemySplat
};

_Bool heartinit(Enemy *e, int x, int y){
	e->hp = 100;
	e->hitback = 0;
	e->iframes = 0;
	e->data = 0;
	return 1;
}

void heartfree(Enemy *e){
}

void heartupdate(Enemy *e, Player *p, Zone *z){
	if(e->iframes > 0)
		e->iframes--;

	Rect pbbox = playerbox(p);

	if(isect(e->body.bbox, pbbox)){
		int dir = e->body.bbox.a.x > pbbox.a.x ? -1 : 1;
		playerdmg(p, heartinfo.stats[StatStr], dir);
	}

	if(e->iframes > 0)
		return;

	Rect swbb = swordbbox(&p->sw);
	if(p->sframes > 0 && isect(e->body.bbox, swbb)){
		sfxplay(heartinfo.hit);
		int pstr = swordstr(&p->sw, p);
		e->hp -= pstr;

		int mhb = 3;
		if(pstr > mhb * 2)
			mhb = pstr/2;
		if(mhb > 32)
			mhb = 32;
		e->hitback = pbbox.a.x < e->body.bbox.a.x ? mhb : -mhb;
		e->iframes = 500.0 / Ticktm; // 0.5s

		if(e->hp <= 0)
			die(e, z);
	}

	for(int j = 0; j < Maxmagics; j++){
		Magic *m = &z->mags[z->lvl->z][j];
		Rect mbb = m->body.bbox;
		if(m->id == 0 || !isect(e->body.bbox, mbb))
			continue;

		int orighp = e->hp;
		magicaffect(m, p, e);

		if(orighp <= e->hp)
			continue;

		sfxplay(heartinfo.hit);
		int mstr = p->stats[StatMag];
		int mhb = 3;
		if(mstr > mhb * 2)
			mhb = mstr/2;
		if(mhb > 32)
			mhb = 32;
		e->hitback = mbb.a.x < e->body.bbox.a.x ? mhb : -mhb;
		e->iframes = 500.0 / Ticktm; // 0.5s

		if(e->hp <= 0)
			die(e, z);
	}
}

void heartdraw(Enemy *e, Gfx *g){
	if(e->iframes % 4 == 0)
		camdrawimg(g, heartimg, e->body.bbox.a);
}

_Bool heartscan(char *buf, Enemy *e){
	if (!scangeom(buf, "dyd", &e->id, &e->body, &e->hp))
		return 0;

	e->hitback = 0;
	e->iframes = 0;

	return 1;
}

_Bool heartprint(char *buf, size_t sz, Enemy *e){
	return printgeom(buf, sz, "dyd", e->id, e->body, e->hp);
}

static void die(Enemy *e, Zone *z){
	Enemy splat = {};
	enemyinit(&splat, heartinfo.death, 0, 0);
	splat.body = e->body;
	enemyfree(e);
	*e = splat;

	Item drop = {};
	Point gridcoord = { // BARF
		e->body.bbox.a.x / Twidth,
		e->body.bbox.a.y / Theight
	};
	iteminit(&drop, ItemDjewel, gridcoord);
	drop.body.vel.y = - 8;
	zoneadditem(z, z->lvl->z, drop);
}
