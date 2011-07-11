#include "../../include/mid.h"
#include <assert.h>
#include <string.h>

typedef struct ItemOps ItemOps;
struct ItemOps{
	char *name;
	char *animname;
	void (*update)(Item*,Player*,Lvl*);
	Anim anim;
	int stats[StatMax];
	EqpLoc loc;
};

static void statupupdate(Item*,Player*,Lvl*);
static void copperupdate(Item*,Player*,Lvl*);

static Sfx *gengrab;

static ItemOps ops[] = {
	[ItemStatup] = {
		"Orb of Power",
		"img/items.png",
		statupupdate,
		{ .row = 0, .len = 2, .delay = 1200/Ticktm, .w = 32, .h = 32, .d = 1200/Ticktm }
	},
	[ItemCopper] = {
		"c",
		"img/items.png",
		copperupdate,
		{ .row = 1, .len = 8, .delay = 150/Ticktm, .w = 32, .h = 32, .d = 150/Ticktm }
	},
};

_Bool iteminit(Item *i, ItemID id, Point p){
	assert(id >= 0 && id < ItemMax);

	i->id = id;
	bodyinit(&i->bod, p.x * Twidth, p.y * Theight);
	i->gotit = 0;

	return 1;
}

_Bool itemldresrc()
{
	gengrab = resrcacq(sfx, "sfx/gold.wav", NULL);
	if(!gengrab)
		return 0;

	for (int id = 1; id < sizeof(ops)/sizeof(ops[0]); id++) {
		char *n = ops[id].animname;
		assert(n != NULL);

		Img *a = resrcacq(imgs, n, NULL);
		if(!a)
			return 0;

		ops[id].anim.sheet = a;
	}
	return 1;
}

_Bool itemscan(char *buf, Item *it){
	return scangeom(buf, "dyb", &it->id, &it->bod, &it->gotit);
}

_Bool itemprint(char *buf, size_t sz, Item *it){
	return printgeom(buf, sz, "dyb", it->id, it->bod, it->gotit);
}

void itemupdateanims(void){
	for(size_t i = 1; i < sizeof(ops)/sizeof(ops[0]); i++)
		animupdate(&ops[i].anim);
}

void itemupdate(Item *i, Player *p, Lvl *l){
	if(i->gotit)
		return;
	ops[i->id].update(i, p, l);
}

void itemdraw(Item *i, Gfx *g){
	if(i->gotit)
		return;
	camdrawanim(g, &ops[i->id].anim, i->bod.bbox.a);
}

char *itemname(ItemID id){
	return ops[id].name;
}

EqpLoc itemeqploc(ItemID id){
	return ops[id].loc;
}

void invitinit(Invit *it, ItemID id){
	it->id = id;
	//TODO: randomize stats
	memcpy(it->stats, ops[id].stats, sizeof(ops[id].stats));
}

void invitdraw(Invit *it, Gfx *g, Point p){
	animdraw(g, &ops[it->id].anim, p);
}

static void statupupdate(Item *i, Player *p, Lvl *l){
	bodyupdate(&i->bod, l);

	if(isect(i->bod.bbox, playerbox(p)) && playertake(p, i)){
		sfxplay(gengrab);
		i->gotit = 1;
	}
}

static void copperupdate(Item *i, Player *p, Lvl *l){
	bodyupdate(&i->bod, l);

	if(isect(i->bod.bbox, playerbox(p))){
		sfxplay(gengrab);
		p->money++;
		i->gotit = 1;
	}
}
