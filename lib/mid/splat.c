// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
#include "../../include/mid.h"
#include "enemy.h"
#include <stdio.h>
#include <assert.h>

Img *splatimg;

Info splatinfo = {
	// unused
};

typedef struct Splat Splat;
struct Splat{
	Anim anim;
};

_Bool splatinit(Enemy *e, int x, int y){
	e->hp = 600/Ticktm;

	Splat *sp = xalloc(1, sizeof(*sp));
	sp->anim = (Anim){
		.row = 0,
		.len = 3,
		.delay = 200/Ticktm,
		.w = 32,
		.h = 32,
		.d = 200/Ticktm
	};

	e->data = sp;
	return 1;
}

void splatfree(Enemy *e){
	xfree(e->data);
}

void splatupdate(Enemy *e, Player *p, Zone *z){
	bodyupdate(&e->body, z->lvl);

	Splat *sp = e->data;
	sp->anim.sheet = splatimg;
	animupdate(&sp->anim);

	e->hp--;
	if(e->hp <= 0)
		e->id = 0;
}

void splatdraw(Enemy *e, Gfx *g){
	Splat *sp = e->data;
	sp->anim.sheet = splatimg;
	camdrawanim(g, &sp->anim, e->body.bbox.a);
}

_Bool splatscan(char *buf, Enemy *e){
	if(!defaultscan(buf, e))
		return 0;

	e->hitback = 0;
	e->iframes = 0;

	Splat *sp = xalloc(1, sizeof(*sp));
	sp->anim = (Anim){
		.row = 0,
		.len = 3,
		.delay = 200/Ticktm,
		.w = 32,
		.h = 32,
		.d = 200/Ticktm
	};

	e->data = sp;
	return 1;
}

_Bool splatprint(char *buf, size_t sz, Enemy *e){
	return defaultprint(buf, sz, e);
}
