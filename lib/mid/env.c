// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
#include "../../include/mid.h"
#include <assert.h>

typedef struct EnvOps EnvOps;
struct EnvOps{
	char *animname;
	void (*act)(Env*,Player*,Lvl*);
	Point wh;
	Anim anim;
};

static void shremptyact(Env*,Player*,Lvl*);
static void shrusedact(Env*,Player*,Lvl*);

static EnvOps ops[] = {
	[EnvShrempty] = {
		"img/shrine.png",
		shremptyact,
		{ 32, 64 },
		{ .row = 0, .len = 1, .delay = 1, .w = 32, .h = 64, .d = 1}
	},
	[EnvShrused] = {
		"img/shrine.png",
		shrusedact,
		{ 32, 64 },
		{ .row = 1, .len = 1, .delay = 1, .w = 32, .h = 64, .d = 1 }
	},
};

_Bool envinit(Env *e, EnvID id, Point p){
	assert(id >= 0 && id < EnvMax);

	e->id = id;
	bodyinit(&e->body, p.x * Twidth, p.y * Theight, Twidth, Theight);
	e->body.bbox.b.x = e->body.bbox.a.x + ops[id].wh.x;
	e->body.bbox.b.y = e->body.bbox.a.y + ops[id].wh.y;

	return 1;
}

_Bool envldresrc(){
	for (int id = 1; id < sizeof(ops)/sizeof(ops[0]); id++) {
		char *n = ops[id].animname;
		assert(n != NULL);

		Img *i = resrcacq(imgs, n, NULL);
		if(!i)
			return 0;
		ops[id].anim.sheet = i;
	}
	return 1;
}

_Bool envprint(char *buf, size_t sz, Env *env){
	return printgeom(buf, sz, "dyb", env->id, env->body, env->gotit);
}

_Bool envscan(char *buf, Env *env){
	return scangeom(buf, "dyb", &env->id, &env->body, &env->gotit);
}

Point envsize(EnvID id){
	return ops[id].wh;
}

void envupdateanims(void){
	for(size_t i = 1; i < EnvMax; i++)
		animupdate(&ops[i].anim);
}

void envupdate(Env *e, Lvl *l){
	bodyupdate(&e->body, l);
}

void envdraw(Env *e, Gfx *g){
	camdrawanim(g, &ops[e->id].anim, e->body.bbox.a);
}

void envact(Env *e, Player *p, Lvl *l){
	ops[e->id].act(e, p, l);
}

static void shremptyact(Env *e, Player *p, Lvl *l){
	if(isect(e->body.bbox, p->body.bbox))
		p->statup = 1;
}

static void shrusedact(Env *e, Player *p, Lvl *l){
	// nothing
}
