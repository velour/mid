// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
#include "../../include/mid.h"
#include <assert.h>
#include <string.h>

typedef struct ItemOps ItemOps;
struct ItemOps{
	char *name;
	char *animname;
	void (*update)(Item*,Player*,Zone*);
	Anim anim;
	int stats[StatMax];
	EqpLoc loc;
};

static void statupupdate(Item*,Player*,Zone*);
static void copperupdate(Item*,Player*,Zone*);
static void healthupdate(Item *, Player *, Zone *);
static void silverupdate(Item*,Player*,Zone*);
static void goldupdate(Item*,Player*,Zone*);
static void carrotupdate(Item*,Player*,Zone*);
static void tophatupdate(Item*,Player*,Zone*);
static void silverswdupdate(Item*,Player*,Zone*);

static Sfx *goldgrab;
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
	[ItemHealth] = {
		"Broccoli",
		"img/items.png",
		healthupdate,
		{ .row = 2, .len = 2, .delay = 600/Ticktm, .w = 32, .h = 32, .d = 600/Ticktm }
	},
	[ItemSilver] = {
		"s",
		"img/items.png",
		silverupdate,
		{ .row = 3, .len = 8, .delay = 150/Ticktm, .w = 32, .h = 32, .d = 150/Ticktm }
	},
	[ItemGold] = {
		"g",
		"img/items.png",
		goldupdate,
		{ .row = 4, .len = 8, .delay = 150/Ticktm, .w = 32, .h = 32, .d = 150/Ticktm }
	},
	[ItemCarrot] = {
		"Carrot",
		"img/items.png",
		carrotupdate,
		{ .row = 5, .len = 2, .delay = 600/Ticktm, .w = 32, .h = 32, .d = 600/Ticktm }
	},
	[ItemTopHat] = {
		"Top Hat",
		"img/items.png",
		tophatupdate,
		{.row = 6, .len = 1, .delay = 1, .w = 32, .h = 32, .d = 1},
		.stats = { 0, 5, 0 },
		.loc = EqpHead
	},
	[ItemSilverSwd] = {
		"Silver Sword",
		"img/items.png",
		silverswdupdate,
		{ .row = 7, .len = 1, .delay = 1, .w = 32, .h = 32, .d = 1},
		.stats = { 0, 0, 1 },
		.loc = EqpWep
	},
	[ItemBroadSwd] = {
		"Lady Sword",
		"img/items.png",
		silverswdupdate,
		{ .row = 8, .len = 1, .delay = 1, .w = 32, .h = 32, .d = 1},
		.stats = { 0, 0, 3 },
		.loc = EqpWep
	},
};

_Bool iteminit(Item *i, ItemID id, Point p){
	assert(id >= 0 && id < ItemMax);

	i->id = id;
	bodyinit(&i->body, p.x * Twidth, p.y * Theight, Twidth, Theight);
	i->gotit = 0;

	return 1;
}

_Bool itemldresrc()
{
	goldgrab = resrcacq(sfx, "sfx/gold.wav", NULL);
	if(!goldgrab)
		return 0;

	gengrab = resrcacq(sfx, "sfx/yum.wav", NULL);
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
	return scangeom(buf, "dyb", &it->id, &it->body, &it->gotit);
}

_Bool itemprint(char *buf, size_t sz, Item *it){
	return printgeom(buf, sz, "dyb", it->id, it->body, it->gotit);
}

void itemupdateanims(void){
	for(size_t i = 1; i < sizeof(ops)/sizeof(ops[0]); i++)
		animupdate(&ops[i].anim);
}

void itemupdate(Item *i, Player *p, Zone *l){
	if(i->gotit)
		return;
	ops[i->id].update(i, p, l);
}

void itemdraw(Item *i, Gfx *g){
	if(i->gotit)
		return;
	camdrawanim(g, &ops[i->id].anim, i->body.bbox.a);
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

static void statupupdate(Item *i, Player *p, Zone *z){
	bodyupdate(&i->body, z->lvl);

	if(isect(i->body.bbox, playerbox(p)) && playertake(p, i)){
		sfxplay(gengrab);
		i->gotit = 1;
	}
}

static void copperupdate(Item *i, Player *p, Zone *z){
	bodyupdate(&i->body, z->lvl);

	if(isect(i->body.bbox, playerbox(p))){
		sfxplay(goldgrab);
		p->money++;
		i->gotit = 1;
	}
}

static void healthupdate(Item *i, Player *p, Zone *z){
	bodyupdate(&i->body, z->lvl);

	if(isect(i->body.bbox, playerbox(p))){
		sfxplay(gengrab);
		playerheal(p, 1);
		i->gotit = 1;
	}
}

static void silverupdate(Item *i, Player *p, Zone *z){
	bodyupdate(&i->body, z->lvl);

	if(isect(i->body.bbox, playerbox(p))){
		sfxplay(goldgrab);
		p->money += 5;
		i->gotit = 1;
	}
}

static void goldupdate(Item *i, Player *p, Zone *z){
	bodyupdate(&i->body, z->lvl);

	if(isect(i->body.bbox, playerbox(p))){
		sfxplay(goldgrab);
		p->money += 25;
		i->gotit = 1;
	}
}

static void carrotupdate(Item *i, Player *p, Zone *z){
	bodyupdate(&i->body, z->lvl);

	if(isect(i->body.bbox, playerbox(p))){
		sfxplay(gengrab);
		playerheal(p, 5);
		i->gotit = 1;
	}
}

static void tophatupdate(Item *i, Player *p, Zone *z){
	bodyupdate(&i->body, z->lvl);

	if(isect(i->body.bbox, playerbox(p)) && playertake(p, i)){
		sfxplay(gengrab);
		i->gotit = 1;
	}
}

static void silverswdupdate(Item *i, Player *p, Zone *z){
	bodyupdate(&i->body, z->lvl);

	if(isect(i->body.bbox, playerbox(p)) && playertake(p, i)){
		sfxplay(gengrab);
		i->gotit = 1;
	}
}
