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
	Point p, v;
	double ddy;
	Color c;
};

enum { Dx = 3, Dy = 8 };
static const double Dxwater = 1.2f, Dywater = 1.2f;

static Rect untibox(Unti*,Point);

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

	u->p = p;
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

	Rect box = untibox(u, (Point){0});
/*
	Blkinfo bi = lvlmajorblk(l, box);

	double olddx = u->v.x;
	if(bi.flags & Tilewater && u->v.x)
		u->v.x = (u->v.x < 0 ? -1 : 1) * Dxwater * Dx;

	double oldddy = u->ddy;
	if(bi.flags & Tilewater && u->ddy)
		u->ddy = (u->ddy < 0 ? -1 : 1) * Dywater * Grav;

	Isect i = lvlisect(l, box, u->v);
	if(i.is) u->p.y += u->v.x + i.dy;
*/
	if(isect(box, playerbox(p)))
		u->c.b = 255;
	else
		u->c.b = 55;
	u->c.r++;
}

static void untidraw(Enemy *e, Gfx *g, Point tr){
	Unti *u = e->data;
	
	gfxfillrect(g, untibox(u, tr), u->c);
}

static Rect untibox(Unti *u, Point tr){
	Point a = { u->p.x + tr.x, u->p.y + tr.y };
	Point b = { a.x + Wide, a.y + Tall };
	return (Rect){ a, b };
}
