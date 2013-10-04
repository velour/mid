/* © 2013 the Mid Authors under the MIT license. See AUTHORS for the list of authors.*/

#include "../../include/mid.h"
#include <assert.h>
#include <string.h>

typedef struct ItemOps ItemOps;
struct ItemOps{
	char *name;
	char *animname;
	ItemStatus (*update)(Item*,Player*,Zone*);
	void (*eat)(Invit*,Player*,Zone*);
	Anim anim;
	int stats[StatMax];
	EqpLoc loc;
	ArmorSetID set;
};

static ItemStatus statupupdate(Item*,Player*,Zone*);
static ItemStatus copperupdate(Item*,Player*,Zone*);
static ItemStatus healthupdate(Item *, Player *, Zone *);
static ItemStatus silverupdate(Item*,Player*,Zone*);
static ItemStatus goldupdate(Item*,Player*,Zone*);
static ItemStatus carrotupdate(Item*,Player*,Zone*);
static ItemStatus tophatupdate(Item*,Player*,Zone*);
static ItemStatus silverswdupdate(Item*,Player*,Zone*);
static void hamcaneat(Invit*,Player*,Zone*);

static Sfx *goldgrab;
static Sfx *gengrab;

static ItemOps ops[] = {
	[ItemStatup] = {
		"Orb of Power",
		"img/food.png",
		statupupdate,
		NULL,
		{ .row = 0, .len = 2, .delay = 120/Ticktm, .w = 16, .h = 16, .d = 120/Ticktm }
	},
	[ItemCopper] = {
		"Copper Coin",
		"img/coins.png",
		copperupdate,
		NULL,
		{ .row = 0, .len = 4, .delay = 150/Ticktm, .w = 16, .h = 16, .d = 150/Ticktm }
	},
	[ItemHealth] = {
		"Broccoli",
		"img/food.png",
		healthupdate,
		NULL,
		{ .row = 1, .len = 1, .delay = 1, .w = 16, .h = 16, .d = 1 }
	},
	[ItemSilver] = {
		"Silver Coin",
		"img/coins.png",
		silverupdate,
		NULL,
		{ .row = 1, .len = 4, .delay = 150/Ticktm, .w = 16, .h = 16, .d = 150/Ticktm }
	},
	[ItemGold] = {
		"Gold Coin",
		"img/coins.png",
		goldupdate,
		NULL,
		{ .row = 2, .len = 4, .delay = 150/Ticktm, .w = 16, .h = 16, .d = 150/Ticktm }
	},
	[ItemCarrot] = {
		"Carrot",
		"img/food.png",
		carrotupdate,
		NULL,
		{ .row = 2, .len = 1, .delay = 1, .w = 16, .h = 16, .d = 1 }
	},
	[ItemHamCan] = {
		"Ham Can",
		"img/food.png",
		statupupdate,
		hamcaneat,
		{ .row = 3, .len = 1, .delay = 1, .w = 16, .h = 16, .d = 1 }
	},
	[ItemTopHat] = {
		"Top Hat",
		"img/tux.png",
		tophatupdate,
		NULL,
		{.row = 0, .len = 1, .delay = 1, .w = 16, .h = 16, .d = 1},
		.stats = { [StatDex] = 1 },
		.loc = EqpHead,
		.set = ArmorSetTux
	},
	[ItemSilkGlove] = {
		"Silk Gloves",
		"img/tux.png",
		tophatupdate,
		NULL,
		{.row = 2, .len = 1, .delay = 1, .w = 16, .h = 16, .d = 1},
		.stats = { [StatDex] = 1 },
		.loc = EqpArms,
		.set = ArmorSetTux
	},
	[ItemNavyBlazer] = {
		"Navy Blazer",
		"img/tux.png",
		tophatupdate,
		NULL,
		{.row = 1, .len = 1, .delay = 1, .w = 16, .h = 16, .d = 1},
		.stats = { [StatDex] = 1 },
		.loc = EqpBody,
		.set = ArmorSetTux
	},
	[ItemFineShoe] = {
		"Fine Shoes",
		"img/tux.png",
		tophatupdate,
		NULL,
		{.row = 3, .len = 1, .delay = 1, .w = 16, .h = 16, .d = 1},
		.stats = { [StatDex] = 1 },
		.loc = EqpLegs,
		.set = ArmorSetTux
	},
	[ItemIronHelm] = {
		"Iron Helm",
		"img/iron.png",
		tophatupdate,
		NULL,
		{.row = 0, .len = 1, .delay = 1, .w = 16, .h = 16, .d = 1},
		.stats = { [StatHp] = 1 },
		.loc = EqpHead,
		.set = ArmorSetIron
	},
	[ItemIronGlove] = {
		"Iron Gloves",
		"img/iron.png",
		tophatupdate,
		NULL,
		{.row = 2, .len = 1, .delay = 1, .w = 16, .h = 16, .d = 1},
		.stats = { [StatHp] = 1 },
		.loc = EqpArms,
		.set = ArmorSetIron
	},
	[ItemIronBody] = {
		"Iron Plate",
		"img/iron.png",
		tophatupdate,
		NULL,
		{.row = 1, .len = 1, .delay = 1, .w = 16, .h = 16, .d = 1},
		.stats = { [StatHp] = 1 },
		.loc = EqpBody,
		.set = ArmorSetIron
	},
	[ItemIronBoot] = {
		"Iron Boots",
		"img/iron.png",
		tophatupdate,
		NULL,
		{.row = 3, .len = 1, .delay = 1, .w = 16, .h = 16, .d = 1},
		.stats = { [StatHp] = 1, [StatDex] = -1 },
		.loc = EqpLegs,
		.set = ArmorSetIron
	},
	[ItemSteelHelm] = {
		"Steel Helm",
		"img/steel.png",
		tophatupdate,
		NULL,
		{.row = 0, .len = 1, .delay = 1, .w = 16, .h = 16, .d = 1},
		.stats = { [StatHp] = 2 },
		.loc = EqpHead,
		.set = ArmorSetSteel
	},
	[ItemSteelGlove] = {
		"Steel Gloves",
		"img/steel.png",
		tophatupdate,
		NULL,
		{.row = 2, .len = 1, .delay = 1, .w = 16, .h = 16, .d = 1},
		.stats = { [StatHp] = 2 },
		.loc = EqpArms,
		.set = ArmorSetSteel
	},
	[ItemSteelBody] = {
		"Steel Plate",
		"img/steel.png",
		tophatupdate,
		NULL,
		{.row = 1, .len = 1, .delay = 1, .w = 16, .h = 16, .d = 1},
		.stats = { [StatHp] = 2 },
		.loc = EqpBody,
		.set = ArmorSetSteel
	},
	[ItemSteelBoot] = {
		"Steel Boots",
		"img/steel.png",
		tophatupdate,
		NULL,
		{.row = 3, .len = 1, .delay = 1, .w = 16, .h = 16, .d = 1},
		.stats = { [StatHp] = 2, [StatDex] = -1 },
		.loc = EqpLegs,
		.set = ArmorSetSteel
	},
	[ItemGoldHelm] = {
		"Gold Helm",
		"img/gold.png",
		tophatupdate,
		NULL,
		{.row = 0, .len = 1, .delay = 1, .w = 16, .h = 16, .d = 1},
		.stats = { [StatHp] = 1, [StatLuck] = 1 },
		.loc = EqpHead,
		.set = ArmorSetGold
	},
	[ItemGoldGlove] = {
		"Gold Gloves",
		"img/gold.png",
		tophatupdate,
		NULL,
		{.row = 2, .len = 1, .delay = 1, .w = 16, .h = 16, .d = 1},
		.stats = { [StatHp] = 1, [StatLuck] = 1 },
		.loc = EqpArms,
		.set = ArmorSetGold
	},
	[ItemGoldBody] = {
		"Gold Plate",
		"img/gold.png",
		tophatupdate,
		NULL,
		{.row = 1, .len = 1, .delay = 1, .w = 16, .h = 16, .d = 1},
		.stats = { [StatHp] = 1, [StatLuck] = 1 },
		.loc = EqpBody,
		.set = ArmorSetGold
	},
	[ItemGoldBoot] = {
		"Gold Boots",
		"img/gold.png",
		tophatupdate,
		NULL,
		{.row = 3, .len = 1, .delay = 1, .w = 16, .h = 16, .d = 1},
		.stats = { [StatHp] = 1, [StatLuck] = 1 },
		.loc = EqpLegs,
		.set = ArmorSetGold
	},
	[ItemRockHelm] = {
		"Rock Helm",
		"img/rock.png",
		tophatupdate,
		NULL,
		{.row = 0, .len = 1, .delay = 1, .w = 16, .h = 16, .d = 1},
		.stats = { [StatHp] = 4, [StatDex] = -1 },
		.loc = EqpHead,
		.set = ArmorSetRock
	},
	[ItemRockGlove] = {
		"Rock Gloves",
		"img/rock.png",
		tophatupdate,
		NULL,
		{.row = 2, .len = 1, .delay = 1, .w = 16, .h = 16, .d = 1},
		.stats = { [StatHp] = 4, [StatDex] = -1 },
		.loc = EqpArms,
		.set = ArmorSetRock
	},
	[ItemRockBody] = {
		"Rock Plate",
		"img/rock.png",
		tophatupdate,
		NULL,
		{.row = 1, .len = 1, .delay = 1, .w = 16, .h = 16, .d = 1},
		.stats = { [StatHp] = 4, [StatDex] = -1 },
		.loc = EqpBody,
		.set = ArmorSetRock
	},
	[ItemRockBoot] = {
		"Rock Boots",
		"img/rock.png",
		tophatupdate,
		NULL,
		{.row = 3, .len = 1, .delay = 1, .w = 16, .h = 16, .d = 1},
		.stats = { [StatHp] = 4, [StatDex] = -1 },
		.loc = EqpLegs,
		.set = ArmorSetRock
	},
	[ItemLavaHelm] = {
		"Lava Helm",
		"img/lava.png",
		tophatupdate,
		NULL,
		{.row = 0, .len = 1, .delay = 1, .w = 16, .h = 16, .d = 1},
		.stats = { [StatHp] = 8, [StatDex] = -1 },
		.loc = EqpHead,
		.set = ArmorSetLava
	},
	[ItemLavaGlove] = {
		"Lava Gloves",
		"img/lava.png",
		tophatupdate,
		NULL,
		{.row = 2, .len = 1, .delay = 1, .w = 16, .h = 16, .d = 1},
		.stats = { [StatHp] = 8, [StatDex] = -1 },
		.loc = EqpArms,
		.set = ArmorSetLava
	},
	[ItemLavaBody] = {
		"Lava Plate",
		"img/lava.png",
		tophatupdate,
		NULL,
		{.row = 1, .len = 1, .delay = 1, .w = 16, .h = 16, .d = 1},
		.stats = { [StatHp] = 8, [StatDex] = -1 },
		.loc = EqpBody,
		.set = ArmorSetLava
	},
	[ItemLavaBoot] = {
		"Lava Boots",
		"img/lava.png",
		tophatupdate,
		NULL,
		{.row = 3, .len = 1, .delay = 1, .w = 16, .h = 16, .d = 1},
		.stats = { [StatHp] = 8, [StatDex] = -1 },
		.loc = EqpLegs,
		.set = ArmorSetLava
	},
	[ItemPlotHelm] = {
		"Plot Helm",
		"img/plot.png",
		tophatupdate,
		NULL,
		{.row = 0, .len = 1, .delay = 1, .w = 16, .h = 16, .d = 1},
		.stats = { [StatMag] = 0 },
		.loc = EqpHead,
		.set = ArmorSetPlot
	},
	[ItemPlotGlove] = {
		"Plot Gloves",
		"img/plot.png",
		tophatupdate,
		NULL,
		{.row = 2, .len = 1, .delay = 1, .w = 16, .h = 16, .d = 1},
		.stats = { [StatStr] = 0 },
		.loc = EqpArms,
		.set = ArmorSetPlot
	},
	[ItemPlotBody] = {
		"Plot Plate",
		"img/plot.png",
		tophatupdate,
		NULL,
		{.row = 1, .len = 1, .delay = 1, .w = 16, .h = 16, .d = 1},
		.stats = { [StatHp] = 0 },
		.loc = EqpBody,
		.set = ArmorSetPlot
	},
	[ItemPlotBoot] = {
		"Plot Boots",
		"img/plot.png",
		tophatupdate,
		NULL,
		{.row = 3, .len = 1, .delay = 1, .w = 16, .h = 16, .d = 1},
		.stats = { [StatDex] = 0 },
		.loc = EqpLegs,
		.set = ArmorSetPlot
	},
	[ItemSilverSwd] = {
		"Silver Sword",
		"img/sword-icons.png",
		silverswdupdate,
		NULL,
		{ .row = 0, .len = 1, .delay = 1, .w = 16, .h = 16, .d = 1},
		.stats = { 0, 0, 1 },
		.loc = EqpWep
	},
	[ItemBroadSwd] = {
		"Lady Sword",
		"img/sword-icons.png",
		silverswdupdate,
		NULL,
		{ .row = 1, .len = 1, .delay = 1, .w = 16, .h = 16, .d = 1},
		.stats = { 0, 0, 3 },
		.loc = EqpWep
	},
	[ItemWindSwd] = {
		"Kaze",
		"img/sword-icons.png",
		silverswdupdate,
		NULL,
		{ .row = 2, .len = 1, .delay = 1, .w = 16, .h = 16, .d = 1 },
		.stats = { 0, 1, 2 },
		.loc = EqpWep
	},
	[ItemBubble] = {
		"Bubble Magic",
		"img/spells.png",
		silverswdupdate,
		NULL,
		{ .row = 0, .len = 1, .delay = 1, .w = 16, .h = 16, .d = 1 },
		.stats = { [StatMag] = 0 },
		.loc = EqpMag
	},
	[ItemZap] = {
		"Zap Magic",
		"img/spells.png",
		silverswdupdate,
		NULL,
		{ .row = 1, .len = 1, .delay = 1, .w = 16, .h = 16, .d = 1 },
		.stats = { [StatMag] = 1 },
		.loc = EqpMag
	},
	[ItemLead] = {
		"Lead Magic",
		"img/spells.png",
		silverswdupdate,
		NULL,
		{ .row = 2, .len = 1, .delay = 1, .w = 16, .h = 16, .d = 1 },
		.stats = { [StatMag] = 0 },
		.loc = EqpMag
	},
	[ItemDjewel] = {
		"Dragon Jewel",
		"img/coins.png",
		silverswdupdate,
		NULL,
		{ .row = 3, .len = 1, .delay = 1, .w = 16, .h = 16, .d = 1 },
	},
	[ItemWandSwd] = {
		"Magic Wand",
		"img/sword-icons.png",
		silverswdupdate,
		NULL,
		{ .row = 3, .len = 1, .delay = 1, .w = 16, .h = 16, .d = 1},
		.stats = { [StatMag] = 5 },
		.loc = EqpWep
	},
	[ItemSoulSwd] = {
		"Soul Sword",
		"img/sword-icons.png",
		silverswdupdate,
		NULL,
		{ .row = 4, .len = 1, .delay = 1, .w = 16, .h = 16, .d = 1},
		.stats = { [StatHp] = 5 }, // TODO: really I want the power to scale with HP
		.loc = EqpWep
	},
	[ItemGoldSwd] = {
		"Gold Sword",
		"img/sword-icons.png",
		silverswdupdate,
		NULL,
		{ .row = 5, .len = 1, .delay = 1, .w = 16, .h = 16, .d = 1},
		.stats = { [StatStr] = -1, [StatLuck] = 5 },
		.loc = EqpWep
	},
	[ItemShiningSwd] = {
		"Shining Sword",
		"img/sword-icons.png",
		silverswdupdate,
		NULL,
		{ .row = 6, .len = 1, .delay = 1, .w = 16, .h = 16, .d = 1},
		.stats = { [StatStr] = 4, [StatLuck] = 2 },
		.loc = EqpWep
	},
	[ItemThickSwd] = {
		"Thick Sword",
		"img/sword-icons.png",
		silverswdupdate,
		NULL,
		{ .row = 7, .len = 1, .delay = 1, .w = 16, .h = 16, .d = 1},
		.stats = { [StatStr] = 6, [StatHp] = 1 },
		.loc = EqpWep
	},
	[ItemDarkSwd] = {
		"Dark Sword",
		"img/sword-icons.png",
		silverswdupdate,
		NULL,
		{ .row = 8, .len = 1, .delay = 1, .w = 16, .h = 16, .d = 1},
		.stats = { [StatStr] = 4, [StatDex] = 3 },
		.loc = EqpWep
	},
	[ItemYgSwd] = {
		"Ygdrasil Wand",
		"img/sword-icons.png",
		silverswdupdate,
		NULL,
		{ .row = 9, .len = 1, .delay = 1, .w = 16, .h = 16, .d = 1},
		.stats = { [StatMag] = 10 },
		.loc = EqpWep
	},
	[ItemUtmaSwd] = {
		"Utma Sword",
		"img/sword-icons.png",
		silverswdupdate,
		NULL,
		{ .row = 10, .len = 1, .delay = 1, .w = 16, .h = 16, .d = 1},
		.stats = { [StatHp] = 10 }, // TODO: really I want the power to scale with HP
		.loc = EqpWep
	},
	[ItemSproutSwd] = {
		"Sprout Sword",
		"img/sword-icons.png",
		silverswdupdate,
		NULL,
		{ .row = 11, .len = 1, .delay = 1, .w = 16, .h = 16, .d = 1},
		.stats = {
			[StatStr] = 2,
			[StatDex] = 2,
			[StatMag] = 2,
		},
		.loc = EqpWep
	},
	[ItemRedwoodSwd] = {
		"Redwood Sword",
		"img/sword-icons.png",
		silverswdupdate,
		NULL,
		{ .row = 12, .len = 1, .delay = 1, .w = 16, .h = 16, .d = 1},
		.stats = {
			[StatHp] = 5,
			[StatStr] = 5,
			[StatDex] = 5,
			[StatMag] = 5,
			[StatLuck] = 5,
		},
		.loc = EqpWep
	},
};

_Bool iteminit(Item *i, ItemID id, Point p){
	assert(id >= 0 && id < ItemMax);

	ItemOps *iop = &ops[id];

	i->id = id;
	bodyinit(&i->body, p.x * Twidth, p.y * Theight, iop->anim.w, iop->anim.h);

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

	ops[ItemPlotHelm].stats[StatMag] = statmax[StatMag];
	ops[ItemPlotGlove].stats[StatStr] = statmax[StatStr];
	ops[ItemPlotBody].stats[StatHp] = statmax[StatHp];
	ops[ItemPlotBoot].stats[StatDex] = statmax[StatDex];
	return 1;
}

_Bool itemscan(char *buf, Item *it){
	return scangeom(buf, "dy", &it->id, &it->body);
}

_Bool itemprint(char *buf, size_t sz, Item *it){
	return printgeom(buf, sz, "dy", it->id, it->body);
}

void itemupdateanims(void){
	for(size_t i = 1; i < sizeof(ops)/sizeof(ops[0]); i++)
		animupdate(&ops[i].anim);
}

ItemStatus itemupdate(Item *i, Player *p, Zone *l){
	if(i->id)
		return ops[i->id].update(i, p, l);
	return ItemStatusNormal;
}

void itemdraw(Item *i, Gfx *g){
	if(!i->id)
		return;
	if(debugging)
		camfillrect(g, i->body.bbox, (Color){255,0,0,255});
	camdrawanim(g, &ops[i->id].anim, i->body.bbox.a);
}

char *itemname(ItemID id){
	return ops[id].name;
}

EqpLoc itemeqploc(ItemID id){
	return ops[id].loc;
}

ArmorSetID itemarmorset(ItemID id){
	return ops[id].set;
}

void invitinit(Invit *it, ItemID id){
	it->id = id;
	//TODO: randomize stats
	memcpy(it->stats, ops[id].stats, sizeof(ops[id].stats));
}

void invitdraw(Invit *it, Gfx *g, Point p){
	animdraw(g, &ops[it->id].anim, p);
}

_Bool inviteat(Invit *it, Player *p, Zone *z){
	if(!ops[it->id].eat)
		return 0;

	ops[it->id].eat(it, p, z);
	return 1;
}

static ItemStatus statupupdate(Item *i, Player *p, Zone *z){
	bodyupdate(&i->body, z->lvl);

	if(isect(i->body.bbox, playerbox(p))){
		if(!playertake(p, i))
			return ItemStatusNoRoom;
		sfxplay(gengrab);
		i->id = ItemNone;
		return ItemStatusPicked;
	}
	return ItemStatusNormal;
}

static ItemStatus copperupdate(Item *i, Player *p, Zone *z){
	bodyupdate(&i->body, z->lvl);

	if(isect(i->body.bbox, playerbox(p))){
		sfxplay(goldgrab);
		p->money++;
		i->id = ItemNone;
		return ItemStatusPicked;
	}
	return ItemStatusNormal;
}

static ItemStatus healthupdate(Item *i, Player *p, Zone *z){
	bodyupdate(&i->body, z->lvl);

	int maxhp = playerstat(p, StatHp);
	if(isect(i->body.bbox, playerbox(p)) && p->curhp < maxhp){
		sfxplay(gengrab);
		playerheal(p, 1);
		i->id = ItemNone;
		return ItemStatusPicked;
	}
	return ItemStatusNormal;
}

static ItemStatus silverupdate(Item *i, Player *p, Zone *z){
	bodyupdate(&i->body, z->lvl);

	if(isect(i->body.bbox, playerbox(p))){
		sfxplay(goldgrab);
		p->money += 5;
		i->id = ItemNone;
		return ItemStatusPicked;
	}
	return ItemStatusNormal;
}

static ItemStatus goldupdate(Item *i, Player *p, Zone *z){
	bodyupdate(&i->body, z->lvl);

	if(isect(i->body.bbox, playerbox(p))){
		sfxplay(goldgrab);
		p->money += 25;
		i->id = ItemNone;
		return ItemStatusPicked;
	}
	return ItemStatusNormal;
}

static ItemStatus carrotupdate(Item *i, Player *p, Zone *z){
	bodyupdate(&i->body, z->lvl);

	int maxhp = playerstat(p, StatHp);
	if(isect(i->body.bbox, playerbox(p)) && p->curhp < maxhp){
		sfxplay(gengrab);
		playerheal(p, 5);
		i->id = ItemNone;
		return ItemStatusPicked;
	}
	return ItemStatusNormal;
}

static ItemStatus tophatupdate(Item *i, Player *p, Zone *z){
	bodyupdate(&i->body, z->lvl);

	if(isect(i->body.bbox, playerbox(p))){
		if(!playertake(p, i))
			return ItemStatusNoRoom;
		sfxplay(gengrab);
		i->id = ItemNone;
		return ItemStatusPicked;
	}
	return ItemStatusNormal;
}

static ItemStatus silverswdupdate(Item *i, Player *p, Zone *z){
	bodyupdate(&i->body, z->lvl);

	if(isect(i->body.bbox, playerbox(p))){
		if(!playertake(p, i))
			return ItemStatusNoRoom;
		sfxplay(gengrab);
		i->id = ItemNone;
		return ItemStatusPicked;
	}
	return ItemStatusNormal;
}

static void hamcaneat(Invit *i, Player *p, Zone *z){
	sfxplay(gengrab);
	playerheal(p, 10);
}
