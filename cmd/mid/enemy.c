#include "../../include/mid.h"
#include "game.h"
#include <string.h>
#include <stdlib.h>

static Enemy *untinew(Point);
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
	Point p;
	Color c;
};

static Rect untibox(Unti*,Point);

static Enemy *(*spawns[])(Point) = {
	['u'] = untinew,
};

Enemy *enemynew(unsigned char id, Point loc){
	if(id > sizeof(spawns)/sizeof(spawns[0]))
		return NULL;

	return spawns[id](loc);
}

static Enemy *untinew(Point p){
	Enemy *e = calloc(1, sizeof(*e));
	if(!e)
		return NULL;

	Unti *u = calloc(1, sizeof(*u));
	if(!u)
		return NULL;

	u->p = p;
	u->c = (Color){ 255, 55, 55, 255 };

	e->mt = &untimt;
	e->data = u;
	return e;
}

static void untifree(Enemy *e){
	free(e->data);
	free(e);
}

static void untiupdate(Enemy *e, Player *p, Lvl *l){
	// Real enemies will do AI and the usual physics
	Unti *u = e->data;
	if(isect(untibox(u, (Point){0}), playerbox(p)))
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
