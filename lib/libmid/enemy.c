#include "../../include/mid.h"
#include <stdlib.h>

static _Bool untiinit(Enemy *, Point);
static void untifree(Enemy*);
static void untiupdate(Enemy*,Player*,Lvl*);
static void untidraw(Enemy*,Gfx*,Point);
static Enemymt untimt = {
	untifree,
	untiupdate,
	untidraw,
};

typedef struct Unti Unti;
struct Unti{
	Body b;
	Color c;
};

enum { Dx = 3, Dy = 8 };
static const double Dxwater = 1.2f, Dywater = 1.2f;

static _Bool (*spawns[])(Enemy*,Point) = {
	['u'] = untiinit,
};

_Bool enemyinit(Enemy *e, unsigned char id, Point loc){
	if(id >= sizeof(spawns)/sizeof(spawns[0]))
		return 0;

	return spawns[id](e, loc);
}

static _Bool untiinit(Enemy *e, Point p){
	Unti *u = calloc(1, sizeof(*u));
	if(!u)
		return 0;

	if(bodyinit(&u->b, "knight", p.x, p.y, 0)){
		free(u);
		return 0;
	}
	u->c = (Color){ 255, 55, 55, 255 };

	e->mt = &untimt;
	e->data = u;
	return 1;
}

static void untifree(Enemy *e){
	free(e->data);
}

static void untiupdate(Enemy *e, Player *p, Lvl *l){
	// Real enemies will do AI
	Unti *u = e->data;

	bodyupdate(&u->b, l, NULL);

	if(isect(u->b.curdir->bbox[u->b.curact], playerbox(p)))
		u->c.b = 255;
	else
		u->c.b = 55;
	u->c.r++;
}

static void untidraw(Enemy *e, Gfx *g, Point tr){
	Unti *u = e->data;
	
	bodydraw(g, &u->b, tr);
}
