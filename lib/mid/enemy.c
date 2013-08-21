/* © 2013 the Mid Authors under the MIT license. See AUTHORS for the list of authors.*/

#include "../../include/mid.h"
#include "../../include/rng.h"
#include "enemy.h"
#include <stdio.h>

static Rng rng;

_Bool enemyldresrc(void){
	untihit = resrcacq(sfx, "sfx/hit.wav", 0);
	if(!untihit) return 0;
	untiinfo.hit = untihit;

	untiimg = resrcacq(imgs, "img/unti.png", 0);
	if(!untiimg) return 0;

	thuimg = resrcacq(imgs, "img/thu.png", 0);
	if(!thuimg) return 0;
	thuinfo.hit = untihit;

	splatimg = resrcacq(imgs, "img/splat.png", 0);
	if(!splatimg) return 0;
	splatinfo.hit = untihit;

	nousimg = resrcacq(imgs, "img/nous.png", 0);
	if(!nousimg) return 0;
	nousinfo.hit = untihit;

	grenduimg = resrcacq(imgs, "img/grendu.png", 0);
	if(!grenduimg) return 0;
	grenduinfo.hit = untihit;

	daimg = resrcacq(imgs, "img/da.png", 0);
	if(!daimg) return 0;
	dainfo.hit = untihit;

	rnginit(&rng, 666); //TODO: use the game seed

	return 1;
}

typedef struct Enemymt Enemymt;
struct Enemymt{
	_Bool (*init)(Enemy *, int, int);
	void (*free)(Enemy*);
	void (*update)(Enemy*, Player*, Zone*);
	void (*draw)(Enemy*, Gfx*);
	_Bool (*scan)(char *, Enemy *);
	_Bool (*print)(char *, size_t, Enemy *);
};

#define ENEMYMT(e) e##init, e##free, e##update, e##draw, e##scan, e##print

static Enemymt mt[] = {
	[EnemyUnti] = { ENEMYMT(unti) },
	[EnemyNous] = { ENEMYMT(nous) },
	[EnemyDa] = { ENEMYMT(da) },
	[EnemyThu] = { ENEMYMT(thu) },
	[EnemyGrendu] = { ENEMYMT(grendu) },
	[EnemySplat] = { ENEMYMT(splat) },
};

_Bool enemyinit(Enemy *e, EnemyID id, int x, int y){
	if(id == 0 || id >= EnemyMax)
		return 0;

	e->id = id;
	bodyinit(&e->body, x * Twidth, y * Theight, Twidth, Theight);

	return mt[id].init(e, x, y);
}

void enemyfree(Enemy *e){
	if(!e->id)
		return;
	mt[e->id].free(e);
	e->id = 0;
	e->hp = 0;
}

void enemyupdate(Enemy *e, Player *p, Zone *z){
	if(e->id) mt[e->id].update(e, p, z);
}

void enemydraw(Enemy *e, Gfx *g){
	if(e->id) {
		if(debugging)
			camfillrect(g, e->body.bbox, (Color){255,0,0,255});
		mt[e->id].draw(e, g);	
	}
}

_Bool enemyscan(char *buf, Enemy *e){
	*e = (Enemy){};
	int id;
	// need to take a peek at the ID to dispatch the correct scan method.
	if (sscanf(buf, " %d", &id) != 1)
		return 0;
	if (!mt[id].scan)
		return defaultscan(buf, e); 
	return mt[id].scan(buf, e);
}

_Bool enemyprint(char *buf, size_t s, Enemy *e){
	if (!mt[e->id].print)
		return defaultprint(buf, s, e); 
	return mt[e->id].print(buf, s, e);
}

_Bool defaultscan(char *buf, Enemy *e){
	return scangeom(buf, "dyd", &e->id, &e->body, &e->hp);
}

_Bool defaultprint(char *buf, size_t sz, Enemy *e){
	return printgeom(buf, sz, "dyd", e->id, e->body, e->hp);
}

void enemygenupdate(Enemy *e, Player *p, Zone *z, Info *i){
	e->ai.update(e, p, z);

	if(e->iframes > 0){
		e->body.vel.x = e->hitback;
		e->iframes--;
	}

	if(e->iframes <= 0)
		e->hitback = 0;

	bodyupdate(&e->body, z->lvl);

	Rect pbbox = playerbox(p);

	if(isect(e->body.bbox, pbbox)){
		int dir = e->body.bbox.a.x > pbbox.a.x ? -1 : 1;
		playerdmg(p, i->stats[StatStr], dir);
	}

	Rect swbb = swordbbox(&p->sw);
	if(e->iframes == 0 && p->sframes > 0 && isect(e->body.bbox, swbb)){
		sfxplay(i->hit);
		int pstr = swordstr(&p->sw, p);
		e->hp -= pstr;

		int mhb = 3;
		if(pstr > mhb * 2)
			mhb = pstr/2;
		if(mhb > 32)
			mhb = 32;
		e->hitback = pbbox.a.x < e->body.bbox.a.x ? mhb : -mhb;
		e->iframes = 500.0 / Ticktm; // 0.5s

		if(e->hp <= 0){
			Enemy splat = {};
			enemyinit(&splat, i->death, 0, 0);
			splat.body = e->body;
			enemyfree(e);
			*e = splat;

			int n = rngintincl(&rng, 0, 100);
			Drops *d = &i->drops;
			ItemID id = d->item[n > d->prob[DropCommon]];
			if(id == ItemNone)
				return;
			Item drop = {};
			Point gridcoord = { // BARF
				e->body.bbox.a.x / Twidth,
				e->body.bbox.a.y / Theight
			};
			iteminit(&drop, id, gridcoord);
			drop.body.vel.y = - 8;
			zoneadditem(z, z->lvl->z, drop);

			return;
		}
	}
}
