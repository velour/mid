/* © 2013 the Mid Authors under the MIT license. See AUTHORS for the list of authors.*/

#include "../../include/mid.h"
#include <assert.h>

typedef struct ArmorOps ArmorOps;
struct ArmorOps{
	void (*bonus)(Player *);
	char *sheetname;
	Img *invsheet;
	Img *parts[ArmorMax];
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

static char *partname[] = {
	[ArmorBackArm] = "arm-back",
	[ArmorBody] = "body",
	[ArmorHead] = "helm",
	[ArmorFrontArm] = "arm-front",
	[ArmorLegs] = "legs"
};

void armorinit(void){
	char buf[128];

	for(int i = ArmorSetNone; i < ArmorSetMax; i++){
		ArmorOps *op = &ops[i];
		snprintf(buf, sizeof(buf), "img/%s.png", op->sheetname);
		op->invsheet = resrcacq(imgs, buf, NULL);
		assert(op->invsheet != NULL);

		for(int j = ArmorBackArm; j < ArmorMax; j++){
			snprintf(buf, sizeof(buf), "img/%s-%s.png", op->sheetname, partname[j]);
			op->parts[j] = resrcacq(imgs, buf, NULL);
			assert(op->parts[j] != NULL);
		}
	}
}

void applyarmorbonus(Player *p, ArmorSetID id){
	ops[id].bonus(p);
}

Img *armorsetsheet(ArmorSetID as, ArmorLoc loc){
	return ops[as].parts[loc];
}

Img *armorinvsheet(ArmorSetID id){
	return ops[id].invsheet;
}

static void nobonus(Player *p){
	// nada
}

static void ironbonus(Player *p){
	p->eqp[StatHp] += 5;
	p->eqp[StatStr] += 1;
	p->eqp[StatDex] += 1;
}
