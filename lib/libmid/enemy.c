#include "../../include/mid.h"

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
	Color c;
	Img *orb;
};

static _Bool (*spawns[])(Enemy*,Point) = {
	['u'] = untiinit,
};

_Bool enemyinit(Enemy *e, unsigned char id, Point loc){
	if(id >= sizeof(spawns)/sizeof(spawns[0]))
		return 0;

	return spawns[id](e, loc);
}

static _Bool untiinit(Enemy *e, Point p){
	if(bodyinit(&e->b, p.x, p.y))
		return 0;

	Unti *u = xalloc(1, sizeof(*u));
	u->c = (Color){ 255, 55, 55, 255 };
	u->orb = resrcacq(imgs, "img/orb.png", 0);

	e->mt = &untimt;
	e->data = u;
	return 1;
}

static void untifree(Enemy *e){
	resrcrel(imgs, "img/orb.png", 0);
	xfree(e->data);
}

static void untiupdate(Enemy *e, Player *p, Lvl *l){
	// Real enemies will do AI
	Unti *u = e->data;

	bodyupdate(&e->b, l);

	if(isect(e->b.bbox, playerbox(p))){
		u->c.b = 255;
		playerdmg(p, 3);
	}else
		u->c.b = 55;
	u->c.r++;
}

static void untidraw(Enemy *e, Gfx *g, Point tr){
	Unti *u = e->data;

	Rect r = {
		{e->b.bbox.a.x + tr.x, e->b.bbox.a.y + tr.y},
		{e->b.bbox.b.x + tr.x, e->b.bbox.b.y + tr.y}
	};
	//gfxfillrect(g, r, u->c);
	imgdraw(g, u->orb, (Point){r.a.x, r.a.y - Tall});
}
