/* © 2013 the Mid Authors under the MIT license. See AUTHORS for the list of authors.*/

#include "../../include/mid.h"
#include "../../include/rng.h"
#include <assert.h>

typedef struct EnvOps EnvOps;
struct EnvOps{
	char *animname;
	void (*act)(Env*,Player*,Zone*);
	Point wh;
	Anim anim;
};

static void shremptyact(Env*,Player*,Zone*);
static void shrusedact(Env*,Player*,Zone*);
static void stonehpact(Env*,Player*,Zone*);
static void stonedexact(Env*,Player*,Zone*);
static void stonestract(Env*,Player*,Zone*);
static void stonehpact2(Env*,Player*,Zone*);
static void stonedexact2(Env*,Player*,Zone*);
static void stonestract2(Env*,Player*,Zone*);
static void stonehpact3(Env*,Player*,Zone*);
static void stonedexact3(Env*,Player*,Zone*);
static void stonestract3(Env*,Player*,Zone*);

static void stonegenact(Env *e, Player *p, Zone *z, int stat, ItemID *drops, int dsz);

static Rng rng;

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
	[EnvSwdStoneHp] = {
		"img/swstones.png",
		stonehpact,
		{ 32, 32 },
		{ .row = 0, .len = 1, .delay = 1, .w = 32, .h = 32, .d = 1 }
	},
	[EnvSwdStoneDex] = {
		"img/swstones.png",
		stonedexact,
		{ 32, 32 },
		{ .row = 1, .len = 1, .delay = 1, .w = 32, .h = 32, .d = 1 }
	},
	[EnvSwdStoneStr] = {
		"img/swstones.png",
		stonestract,
		{ 32, 32 },
		{ .row = 2, .len = 1, .delay = 1, .w = 32, .h = 32, .d = 1 }
	},
	[EnvSwdStoneHp2] = {
		"img/swstones.png",
		stonehpact2,
		{ 32, 32 },
		{ .row = 0, .len = 1, .delay = 1, .w = 32, .h = 32, .d = 1 }
	},
	[EnvSwdStoneDex2] = {
		"img/swstones.png",
		stonedexact2,
		{ 32, 32 },
		{ .row = 1, .len = 1, .delay = 1, .w = 32, .h = 32, .d = 1 }
	},
	[EnvSwdStoneStr2] = {
		"img/swstones.png",
		stonestract2,
		{ 32, 32 },
		{ .row = 2, .len = 1, .delay = 1, .w = 32, .h = 32, .d = 1 }
	},
	[EnvSwdStoneHp3] = {
		"img/swstones.png",
		stonehpact3,
		{ 32, 32 },
		{ .row = 0, .len = 1, .delay = 1, .w = 32, .h = 32, .d = 1 }
	},
	[EnvSwdStoneDex3] = {
		"img/swstones.png",
		stonedexact3,
		{ 32, 32 },
		{ .row = 1, .len = 1, .delay = 1, .w = 32, .h = 32, .d = 1 }
	},
	[EnvSwdStoneStr3] = {
		"img/swstones.png",
		stonestract3,
		{ 32, 32 },
		{ .row = 2, .len = 1, .delay = 1, .w = 32, .h = 32, .d = 1 }
	},
};

_Bool envinit(Env *e, EnvID id, Point p){
	assert(id >= 0 && id < EnvMax);

	e->id = id;
	bodyinit(&e->body, p.x * Twidth, p.y * Theight, Twidth, Theight);
	e->body.bbox.b.x = e->body.bbox.a.x + ops[id].wh.x;
	e->body.bbox.b.y = e->body.bbox.a.y + ops[id].wh.y;

	e->min = rngintincl(&rng, 5, 30);

	return 1;
}

_Bool envldresrc(void){
	for (int id = 1; id < sizeof(ops)/sizeof(ops[0]); id++) {
		char *n = ops[id].animname;
		assert(n != NULL);

		Img *i = resrcacq(imgs, n, NULL);
		if(!i)
			return 0;
		ops[id].anim.sheet = i;
	}
	rnginit(&rng, 666);
	return 1;
}

_Bool envprint(char *buf, size_t sz, Env *env){
	return printgeom(buf, sz, "dybd", env->id, env->body, env->gotit, env->min);
}

_Bool envscan(char *buf, Env *env){
	return scangeom(buf, "dybd", &env->id, &env->body, &env->gotit, &env->min);
}

Point envsize(EnvID id){
	return ops[id].wh;
}

void envupdateanims(void){
	for(size_t i = 1; i < EnvMax; i++)
		animupdate(&ops[i].anim);
}

void envupdate(Env *e, Zone *z){
	bodyupdate(&e->body, z->lvl);
}

void envdraw(Env *e, Gfx *g){
	if(e->id && debugging)
		camfillrect(g, e->body.bbox, (Color){255,0,0,255});
	camdrawanim(g, &ops[e->id].anim, e->body.bbox.a);
}

void envact(Env *e, Player *p, Zone *z){
	ops[e->id].act(e, p, z);
}

static void shremptyact(Env *e, Player *p, Zone *z){
	if(isect(e->body.bbox, p->body.bbox))
		p->statup = 1;
}

static void shrusedact(Env *e, Player *p, Zone *z){
	// nothing
}

static void stonehpact(Env *e, Player *p, Zone *z){
	static ItemID hpswds[] = {
		ItemSilverSwd,
		ItemWandSwd,
	};

	stonegenact(e, p, z, StatHp, hpswds, sizeof(hpswds)/sizeof(hpswds[0]));
}

static void stonedexact(Env *e, Player *p, Zone *z){
	static ItemID dexswds[] = {
		ItemWindSwd,
		ItemGoldSwd,
	};

	stonegenact(e, p, z, StatDex, dexswds, sizeof(dexswds)/sizeof(dexswds[0]));
}

static void stonestract(Env *e, Player *p, Zone *z){
	static ItemID strswds[] = {
		ItemBroadSwd,
		ItemSoulSwd,
	};

	stonegenact(e, p, z, StatStr, strswds, sizeof(strswds)/sizeof(strswds[0]));
}

static void stonehpact2(Env *e, Player *p, Zone *z){
	static ItemID hpswds[] = {
		ItemWandSwd,
		ItemShiningSwd,
	};

	stonegenact(e, p, z, StatHp, hpswds, sizeof(hpswds)/sizeof(hpswds[0]));
}

static void stonedexact2(Env *e, Player *p, Zone *z){
	static ItemID dexswds[] = {
		ItemWindSwd,
		ItemGoldSwd,
		ItemSproutSwd,
	};

	stonegenact(e, p, z, StatDex, dexswds, sizeof(dexswds)/sizeof(dexswds[0]));
}

static void stonestract2(Env *e, Player *p, Zone *z){
	static ItemID strswds[] = {
		ItemBroadSwd,
		ItemSoulSwd,
		ItemThickSwd,
	};

	stonegenact(e, p, z, StatStr, strswds, sizeof(strswds)/sizeof(strswds[0]));
}

static void stonehpact3(Env *e, Player *p, Zone *z){
	static ItemID hpswds[] = {
		ItemYgSwd,
		ItemShiningSwd,
		ItemRedwoodSwd,
	};

	stonegenact(e, p, z, StatHp, hpswds, sizeof(hpswds)/sizeof(hpswds[0]));
}

static void stonedexact3(Env *e, Player *p, Zone *z){
	static ItemID dexswds[] = {
		ItemWindSwd,
		ItemGoldSwd,
		ItemDarkSwd,
	};

	stonegenact(e, p, z, StatDex, dexswds, sizeof(dexswds)/sizeof(dexswds[0]));
}

static void stonestract3(Env *e, Player *p, Zone *z){
	static ItemID strswds[] = {
		ItemBroadSwd,
		ItemUtmaSwd,
		ItemThickSwd,
	};

	stonegenact(e, p, z, StatStr, strswds, sizeof(strswds)/sizeof(strswds[0]));
}

static void stonegenact(Env *e, Player *p, Zone *z, int stat, ItemID *drops, int dsz){
	if(p->stats[stat] >= e->min && isect(e->body.bbox, p->body.bbox)){
		ItemID id = drops[rngintincl(&rng, 0, dsz - 1)];
		Item drop = {};
		Point gridcoord = { // BARF
			e->body.bbox.a.x / Twidth,
			e->body.bbox.a.y / Theight
		};
		iteminit(&drop, id, gridcoord);
		drop.body.vel.y = -8;
		if(zoneadditem(z, z->lvl->z, drop))
			*e = (Env){};
	}
}
