#include "../../include/mid.h"

typedef struct Enemymt Enemymt;
struct Enemymt{
	_Bool (*init)(Enemy *, int, int);
	void (*free)(Enemy*);
	void (*update)(Enemy*, Player*, Lvl*);
	void (*draw)(Enemy*, Gfx*);
	_Bool (*scan)(char *, Enemy *);
	_Bool (*print)(char *, size_t, Enemy *);
};

static Sfx *untihit;
static _Bool untiinit(Enemy *e, int x, int y);
static void untifree(Enemy*);
static void untiupdate(Enemy*,Player*,Lvl*);
static void untidraw(Enemy*,Gfx*);
static _Bool untiscan(char *buf, Enemy *e);
static _Bool untiprint(char *buf, size_t sz, Enemy *e);

static _Bool defaultscan(char *, Enemy *);
static _Bool defaultprint(char *, size_t, Enemy *);

static Enemymt mt[] = {
	[EnemyUnti] = { untiinit, untifree, untiupdate, untidraw,
		untiscan, untiprint },
};

// NOTE: this function should only be called by the zone gen pipeline.
// It doesn't initialize any of the resources.
_Bool enemyinit(Enemy *e, EnemyID id, int x, int y){
	if(id >= 0 && id >= EnemyMax)
		return 0;

	e->id = id;
	return mt[id].init(e, x, y);
}

void enemyfree(Enemy *e){
	if(!e->id)
		return;
	mt[e->id].free(e);
	e->id = 0;
	e->hp = 0;
}

void enemyupdate(Enemy *e, Player *p, Lvl *l){
	if(e->id) mt[e->id].update(e, p, l);
}

void enemydraw(Enemy *e, Gfx *g){
	if(e->id) mt[e->id].draw(e, g);
}


typedef struct Unti Unti;
struct Unti{
	Color c;
	Img *img;
};

static _Bool untiinit(Enemy *e, int x, int y){
	bodyinit(&e->b, x * Twidth, y * Theight);
	e->hp = 1;

	Unti *u = xalloc(1, sizeof(*u));
	u->c = (Color){ 255, 55, 55, 255 };

	e->data = u;
	return 1;
}

static void untifree(Enemy *e){
	resrcrel(imgs, "img/unti.png", 0);
	xfree(e->data);
}

static void untiupdate(Enemy *e, Player *p, Lvl *l){
	if(!untihit)
		untihit = resrcacq(sfx, "sfx/hit.wav", 0);

	// Real enemies will do AI
	Unti *u = e->data;

	bodyupdate(&e->b, l);

	if(isect(e->b.bbox, playerbox(p))){
		u->c.b = 255;
		playerdmg(p, 3);
	}else
		u->c.b = 55;

	if(isect(e->b.bbox, p->sw.loc[p->sw.cur])){
		sfxplay(untihit);
		e->hp--;
	}

	u->c.r++;
}

static void untidraw(Enemy *e, Gfx *g){
	Unti *u = e->data;

	//gfxfillrect(g, r, u->c);
	camdrawimg(g, u->img, e->b.bbox.a);
}

static _Bool untiscan(char *buf, Enemy *e)
{
	int r, g, b, a;

	if (!scangeom(buf, "dyddddd", &e->id, &e->b, &e->hp, &r, &g, &b, &a))
		return 0;

	Unti *u = xalloc(1, sizeof(*u));
	u->c = (Color){ r, g, b, a };
	u->img = resrcacq(imgs, "img/unti.png", 0);
	e->data = u;

	return 1;
}

static _Bool untiprint(char *buf, size_t sz, Enemy *e)
{
	Unti *u = e->data;
	Color c = u->c;
	return printgeom(buf, sz, "dyddddd", e->id, e->b, e->hp, c.r, c.g, c.b, c.a);
}

static _Bool defaultscan(char *buf, Enemy *e)
{
	return scangeom(buf, "dyd", &e->id, &e->b, &e->hp);
}

static _Bool defaultprint(char *buf, size_t sz, Enemy *e)
{
	return printgeom(buf, sz, "dyd", e->id, e->b, e->hp);
}

_Bool enemyscan(char *buf, Enemy *e)
{
	int id;
	// need to take a peek at the ID to dispatch the correct scan method.
	if (sscanf(buf, " %d", &id) != 1)
		return 0;
	if (!mt[id].scan)
		return defaultscan(buf, e); 
	return mt[id].scan(buf, e);
}

_Bool enemyprint(char *buf, size_t s, Enemy *e)
{
	if (!mt[e->id].print)
		return defaultprint(buf, s, e); 
	return mt[e->id].print(buf, s, e);
}