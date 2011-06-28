#include "../../include/mid.h"
#include <assert.h>

typedef struct ItemOps ItemOps;
struct ItemOps{
	char *name;
	char *animname;
	void (*update)(Item*,Player*,Lvl*);
	Anim *anim;
};

static void statupupdate(Item*,Player*,Lvl*);
static void copperupdate(Item*,Player*,Lvl*);

static ItemOps ops[] = {
	[ItemStatup] = { "Orb of Power", "anim/item/statup", statupupdate },
	[ItemCopper] = { "c", "anim/coin/coin1.anim", copperupdate },
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

void itemupdateanims(void){
	for(size_t i = 0; i < sizeof(ops)/sizeof(ops[0]); i++)
		if(ops[i].anim)
			animupdate(ops[i].anim, 1);
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
	animdraw(g, ops[i->id].anim, pt);
}

void iteminvdraw(Item *i, Gfx *g, Point p){
	animdraw(g, ops[i->id].anim, p);
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

_Bool itemscan(char *buf, Item *it)
{
	return scangeom(buf, "dyb", &it->id, &it->bod, &it->gotit);
}

_Bool itemprint(char *buf, size_t sz, Item *it)
{
	return printgeom(buf, sz, "dyb", it->id, it->bod, it->gotit);
}
