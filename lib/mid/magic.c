/* © 2013 the Mid Authors under the MIT license. See AUTHORS for the list of authors.*/

#include "../../include/mid.h"

static Img *sheet;

typedef struct MagicOps MagicOps;
struct MagicOps{
	int cost;
	void (*cast)(Magic*,Player*);
	void (*affect)(Magic*,Player*,Enemy*);
	void (*update)(Magic*,Zone*,Body*);
};

static void bubblecast(Magic*,Player*);
static void bubbleaffect(Magic*,Player*,Enemy*);
static void noupdate(Magic*,Zone*,Body*);
static void zapcast(Magic*,Player*);
static void zapaffect(Magic*,Player*,Enemy*);
static void zapupdate(Magic*,Zone*,Body*);

static MagicOps ops[] = {
	[ItemBubble] = {
		MaxMP/5,
		bubblecast,
		bubbleaffect,
		noupdate,
	},
	[ItemZap] = {
		MaxMP/10,
		zapcast,
		zapaffect,
		zapupdate,
	},
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
	Body old = m->body;
	bodyupdate(&m->body, z->lvl);
	animupdate(&m->anim);
	ops[m->id].update(m, z, &old);
}

void magicaffect(Magic *m, Player *p, Enemy *e){
	ops[m->id].affect(m, p, e);
}

void itemcast(Magic *m, ItemID id, Player *p){
	if(!id || p->curmp < ops[id].cost)
		return;
	ops[id].cast(m, p);
	m->id = id;
	p->curmp -= ops[id].cost;
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

static void bubbleaffect(Magic *m, Player *p, Enemy *e){
	e->hp -= p->stats[StatMag];
	m->id = 0;
}

static void zapcast(Magic *m, Player *p){
	bubblecast(m, p);
	m->anim.row = 1;
	m->body.vel.y = 0;
	m->body.vel.x = 6;
	m->hp /= 2;
	if(p->dir == Left)
		m->body.vel.x = -m->body.vel.x;
}

static void zapaffect(Magic *m, Player *p, Enemy *e){
	e->hp -= p->stats[StatMag]/2;
	m->id = 0;
}

static void noupdate(Magic *m, Zone *z, Body *b){
	// nada
}

static void zapupdate(Magic *m, Zone *z, Body *b){
	if(m->body.bbox.a.x == b->bbox.a.x)
		m->id = 0;
}
