#include "../../include/mid.h"
#include "../../include/log.h"
#include <stdlib.h>
#include <stdbool.h>

enum { Dx = 3, Dy = 8 };
static const double Dxwater = 0.6f, Dywater = 0.2f;

struct Player {
	Body body;
	bool door;

	/* if changed, update visibility. */
	Blkinfo bi;
};

Player *playernew(int x, int y)
{
	Player *p = calloc(1, sizeof(*p));
	if (!p)
		return NULL;
	if(bodyinit(&p->body, "knight", x, y, 0)){
		free(p);
		return NULL;
	}

	p->bi.x = p->bi.y = p->bi.z = -1;
	return p;
}

void playerfree(Player *p)
{
	free(p);
}

void playerupdate(Player *p, Lvl *l, Point *tr)
{
	Blkinfo bi = lvlmajorblk(l, p->body.curdir->bbox[p->body.curact]);

	if (bi.x != p->bi.x || bi.y != p->bi.x || bi.z != p->bi.z)
		lvlvis(l, bi.x, bi.y);
	p->bi = bi;

	double olddx = p->body.vel.x;
	if (bi.flags & Tilewater && p->body.vel.x)
		p->body.vel.x = (p->body.vel.x < 0 ? -1 : 1) * Dxwater * Dx;

	double oldddy = p->body.ddy;
	if(bi.flags & Tilewater && p->body.ddy)
		p->body.ddy = (p->body.ddy < 0 ? -1 : 1) * Dywater * Grav;


	if (p->door && bi.flags & Tilebdoor)
		l->z += 1;
	else if (p->door && bi.flags & Tilefdoor)
		l->z -= 1;
	p->body.z = l->z;
	p->door = false;

	bodyupdate(&p->body, l, tr);
	p->body.vel.x = olddx;
	p->body.ddy = oldddy;
}

void playerdraw(Gfx *g, Player *p, Point tr)
{
	bodydraw(g, &p->body, tr);
}

void playerhandle(Player *p, Event *e)
{
	if (e->type != Keychng || e->repeat)
		return;

	char k = e->key;
	if(k == kmap[Mvleft]){
		if(e->down && p->body.vel.x > -Dx)
			p->body.vel.x -= Dx;
		else if(!e->down)
			p->body.vel.x += Dx;
	}else if(k == kmap[Mvright]){
		if(e->down && p->body.vel.x < Dx)
			p->body.vel.x += Dx;
		else if(!e->down)
			p->body.vel.x -= Dx;
	}else if(k == kmap[Mvjump]){
		if(!p->body.fall){
			p->body.vel.y = (e->down ? -Dy : 0.0);
			p->body.ddy = Grav;
			p->body.fall = 1;
		}
	}else if(k == kmap[Mvdoor] && e->down){
		p->door = true;
	}
}

Point playerpos(Player *p)
{
	return p->body.curdir->bbox[p->body.curact].a;
}

Rect playerbox(Player *p)
{
	return p->body.curdir->bbox[p->body.curact];
}
