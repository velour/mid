#include "../../include/mid.h"
#include <assert.h>

typedef struct ItemOps ItemOps;
struct ItemOps{
	char *animname;
	void (*update)(Item*,Player*,Lvl*);
	Anim *anim;
};

static void statupupdate(Item*,Player*,Lvl*);

static ItemOps ops[] = {
	[ItemStatup] = { "anim/item/statup", statupupdate },
};

_Bool iteminit(Item *i, ItemID id, Point p){
	assert(id >= 0 && id < ItemMax);

	i->id = id;
	bodyinit(&i->bod, p.x * Twidth, p.y * Theight);

	if(ops[id].anim)
		return 1;

	char *n = ops[id].animname;
	Anim *a = resrcacq(anims, n, NULL);
	if(!a)
		return 0;

	ops[id].anim = a;
	return 1;
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

static void statupupdate(Item *i, Player *p, Lvl *l){
	bodyupdate(&i->bod, l);

	if(isect(i->bod.bbox, playerbox(p))){
		i->gotit = 1;
		//TODO: add to player's inventory
	}
}
