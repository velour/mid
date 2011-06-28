#include "../../include/mid.h"
#include <assert.h>

typedef struct EnvOps EnvOps;
struct EnvOps{
	char *animname;
	void (*act)(Env*,Player*,Lvl*);
	Point wh;
	Anim *anim;
};

static void shremptyact(Env*,Player*,Lvl*);
static void shrusedact(Env*,Player*,Lvl*);

static EnvOps ops[] = {
	[EnvShrempty] = { "anim/shrine/empty.anim", shremptyact, { 32, 64 } },
	[EnvShrused] = { "anim/shrine/used.anim", shrusedact, { 32, 64 } },
};

_Bool envinit(Env *e, EnvID id, Point p){
	assert(id >= 0 && id < EnvMax);

	e->id = id;
	bodyinit(&e->body, p.x * Twidth, p.y * Theight);
	e->body.bbox.b.x = e->body.bbox.a.x + ops[id].wh.x;
	e->body.bbox.b.y = e->body.bbox.a.y + ops[id].wh.y;

	return 1;
}

_Bool envldresrc()
{
	for (int id = 0; id < sizeof(ops)/sizeof(ops[0]); id++) {
		char *n = ops[id].animname;
		if (!n)
			continue;
		Anim *a = resrcacq(anims, n, NULL);
		if(!a)
			return 0;
		ops[id].anim = a;
	}
	return 1;
}

Point envsize(EnvID id)
{
	return ops[id].wh;
}

void envupdateanims(void){
	for(size_t i = 1; i < EnvMax; i++){
		if(!ops[i].anim)
			ops[i].anim = resrcacq(anims, ops[i].animname, NULL);
		animupdate(ops[i].anim, 1);
	}
}

void envupdate(Env *e, Lvl *l){
	bodyupdate(&e->body, l);
}

void envdraw(Env *e, Gfx *g, Point tr){
	Point pt = { e->body.bbox.a.x + tr.x, e->body.bbox.a.y + tr.y };
	animdraw(g, ops[e->id].anim, pt);
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

void envprint(char *buf, size_t sz, Env *env)
{
	printgeom(buf, sz, "dyb", env->id, env->body, env->gotit);
}

void envscan(char *buf, Env *env)
{
	scangeom(buf, "dyb", &env->id, &env->body, &env->gotit);
}
