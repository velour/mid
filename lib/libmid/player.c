#include "../../include/mid.h"
#include "../../include/log.h"
#include <stdbool.h>
#include <stddef.h>

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
	Player *p = xalloc(1, sizeof(*p));

	if(bodyinit(&p->body, "knight", x, y)){
		xfree(p);
		return NULL;
	}

	p->bi.x = p->bi.y = p->bi.z = -1;
	return p;
}

void playerfree(Player *p)
{
	xfree(p);
}

void playerupdate(Player *p, Lvl *l, Point *tr)
{
	Blkinfo bi = lvlmajorblk(l, p->body.bbox);

	if (bi.x != p->bi.x || bi.y != p->bi.x || bi.z != p->bi.z)
		lvlvis(l, bi.x, bi.y);
	p->bi = bi;

	double olddx = p->body.vel.x;
	if(olddx)
		p->body.vel.x = (olddx < 0 ? -1 : 1) * blkdrag(bi.flags) * Dx;

	double oldddy = p->body.a.y;
	p->body.a.y = blkgrav(bi.flags);


	if (p->door && bi.flags & Tilebdoor)
		l->z += 1;
	else if (p->door && bi.flags & Tilefdoor)
		l->z -= 1;
	p->door = false;

	bodyupdate(&p->body, l, tr);
	p->body.vel.x = olddx;
	p->body.a.y = oldddy;
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
			p->body.a.y = Grav;
			p->body.fall = 1;
		}
	}else if(k == kmap[Mvdoor] && e->down){
		p->door = true;
	}
}

Point playerpos(Player *p)
{
	return p->body.bbox.a;
}

Rect playerbox(Player *p)
{
	return p->body.bbox;
}
