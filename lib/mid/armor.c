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
static void steelbonus(Player*);
static void goldbonus(Player*);
static void tuxbonus(Player*);
static void rockbonus(Player*);
static void lavabonus(Player*);
static void plotbonus(Player *p);

static ArmorOps ops[] = {
	[ArmorSetNone] = {
		.bonus = nobonus,
		.sheetname = "naked"
	},
	[ArmorSetIron] = {
		.bonus = ironbonus,
		.sheetname = "iron"
	},
	[ArmorSetTux] = {
		.bonus = tuxbonus,
		.sheetname = "tux"
	},
	[ArmorSetSteel] = {
		.bonus = steelbonus,
		.sheetname = "steel"
	},
	[ArmorSetGold] = {
		.bonus = goldbonus,
		.sheetname = "gold"
	},
	[ArmorSetRock] = {
		.bonus = rockbonus,
		.sheetname = "rock"
	},
	[ArmorSetLava] = {
		.bonus = lavabonus,
		.sheetname = "lava"
	},
	[ArmorSetPlot] = {
		.bonus = plotbonus,
		.sheetname = "plot"
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
	p->eqp[StatStr] += 1;
	p->eqp[StatDex] += 1;
}

static void steelbonus(Player *p){
	p->eqp[StatStr] += 2;
	p->eqp[StatDex] += 1;
}

static void goldbonus(Player *p){
	p->eqp[StatLuck] += 2;
}

static void tuxbonus(Player *p){
	p->eqp[StatDex] += 1;
	p->eqp[StatLuck] += 1;
}

static void rockbonus(Player *p){
	p->eqp[StatHp] += 4;
	p->eqp[StatStr] += 1;
}

static void lavabonus(Player *p){
	p->eqp[StatStr] += 2;
}

static void plotbonus(Player *p){
	p->eqp[StatLuck] = statmax[StatLuck];
}
