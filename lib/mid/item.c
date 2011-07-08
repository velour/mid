#include "../../include/mid.h"
#include <assert.h>

typedef struct ItemOps ItemOps;
struct ItemOps{
	char *name;
	char *animname;
	void (*update)(Item*,Player*,Lvl*);
	Anim anim;
};

static void statupupdate(Item*,Player*,Lvl*);
static void copperupdate(Item*,Player*,Lvl*);

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

void itemdraw(Item *i, Gfx *g, Point tr){
	if(i->gotit)
		return;
	Point pt = { i->bod.bbox.a.x + tr.x, i->bod.bbox.a.y + tr.y };
	animdraw(g, &ops[i->id].anim, pt);
}

void iteminvdraw(Item *i, Gfx *g, Point p){
	animdraw(g, &ops[i->id].anim, p);
}

char *itemname(Item *i){
	return ops[i->id].name;
}

static void statupupdate(Item *i, Player *p, Lvl *l){
	bodyupdate(&i->bod, l);

	if(isect(i->bod.bbox, playerbox(p)) && playertake(p, i))
		i->gotit = 1;
}

static void copperupdate(Item *i, Player *p, Lvl *l){
	bodyupdate(&i->bod, l);

	if(isect(i->bod.bbox, playerbox(p))){
		p->money++;
		i->gotit = 1;
	}
}
