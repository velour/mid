/* © 2013 the Mid Authors under the MIT license. See AUTHORS for the list of authors.*/

#include "../../include/mid.h"

static Img *sheet;

typedef struct MagicOps MagicOps;
struct MagicOps{
	void (*cast)(Magic*,Player*);
};

static void bubblecast(Magic*,Player*);
static void zapcast(Magic*,Player*);

static MagicOps ops[] = {
	[ItemBubble] = { bubblecast },
	[ItemZap] = { zapcast },
};

_Bool magicldresrc(void){
	sheet = resrcacq(imgs, "img/spells.png", NULL);
	return sheet != 0;
}

void magicdraw(Gfx *g, Magic *m){

	m->anim.sheet = sheet; //TODO: what the hell is going on with these anim sheets
	camdrawanim(g, &m->anim, m->body.bbox.a);
}

void magicupdate(Magic *m, Zone *z){
	m->hp--;
	if(m->hp == 0){
		m->id = 0;
		return;
	}
	bodyupdate(&m->body, z->lvl);
	animupdate(&m->anim);
	//TODO: do something to enemies…
}

void itemcast(Magic *m, ItemID id, Player *p){
	if(!id)
		return;
	ops[id].cast(m, p);
	m->id = id;
}

static void bubblecast(Magic *m, Player *p){
	*m = (Magic){
		.body = {
			.bbox = {
				playerpos(p),
				vecadd(playerpos(p), (Point){16,16}),
			},
			.vel = { 4, -4 },
			.fall = 1,
		},
		.anim = {
			.sheet = sheet,
			.row = 0,
			.len = 2,
			.delay = 200/Ticktm,
			.w = 16,
			.h = 16,
			.f = 0,
			.d = 200/Ticktm,
		},
		.hp = 2 * 1000 / Ticktm, // 2s
	};
	if(p->dir == Left)
		m->body.vel.x = -m->body.vel.x;
}

static void zapcast(Magic *m, Player *p){
	return bubblecast(m, p);
}
