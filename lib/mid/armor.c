/* © 2013 the Mid Authors under the MIT license. See AUTHORS for the list of authors.*/

#include "../../include/mid.h"
#include <assert.h>

typedef struct ArmorOps ArmorOps;
struct ArmorOps{
	void (*bonus)(Player *);
	char *sheetname;
	Img *invsheet;
};

static void nobonus(Player*);
static void ironbonus(Player*);

static ArmorOps ops[] = {
	[ArmorSetNone] = {
		.bonus = nobonus,
		.sheetname = "naked"
	},
	[ArmorSetIron] = {
		.bonus = ironbonus,
		.sheetname = "iron"
	},
};

Img *knightsheet;

void armorinit(void){
	char buf[128];

	if(!knightsheet){
		knightsheet = resrcacq(imgs, "img/knight.png", NULL);
		assert(knightsheet != NULL);
	}

	for(int i = ArmorSetIron; i < ArmorSetMax; i++){
		ArmorOps *op = &ops[i];
		snprintf(buf, sizeof(buf), "img/%s.png", op->sheetname);
		op->invsheet = resrcacq(imgs, buf, NULL);
		assert(op->invsheet != NULL);
	}
}

void applyarmorbonus(Player *p, ArmorSetID id){
	ops[id].bonus(p);
}

static void nobonus(Player *p){
	// nada
}

static void ironbonus(Player *p){
	p->eqp[StatHp] += 5;
	p->eqp[StatStr] += 1;
	p->eqp[StatDex] += 1;
}
