#include "../../include/mid.h"
#include "../../include/log.h"
#include "game.h"
#include "body.h"
#include <stdlib.h>

enum { Dx = 3, Dy = 8 };
static const float Dxwater = 0.6f, Dywater = 0.2f;

struct Player {
	Body body;
	int dz;
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
	return p;
}

void playerfree(Player *p)
{
	free(p);
}

void playerupdate(Player *p, Lvl *l, int *z, Point *tr)
{
	Blkinfo bi = lvlmajorblk(l, *z, p->body.curdir->bbox[p->body.curact]);

	float olddx = p->body.vel.x;
	if (bi.flags & Blkwater && p->body.vel.x)
		p->body.vel.x = (p->body.vel.x < 0 ? -1 : 1) * Dxwater * Dx;

	float oldddy = p->body.ddy;
	if(bi.flags & Blkwater && p->body.ddy)
		p->body.ddy = (p->body.ddy < 0 ? -1 : 1) * Dywater * Grav;


	if (p->dz > 0 && bi.flags & Blkbdoor)
		*z += 1;
	else if (p->dz < 0 && bi.flags & Blkfdoor)
		*z -= 1;
	p->body.z = *z;
	p->dz = 0;

	bodyupdate(&p->body, l, *z, tr);
	p->body.vel.x = olddx;
	p->body.ddy = oldddy;
}

void playerdraw(Gfx *g, Player *p, Point tr)
{
	bodydraw(g, &p->body, p->body.z, tr);
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
	}else if(k == kmap[Mvbak] && e->down){
		p->dz += 1;
	}else if(k == kmap[Mvfwd] && e->down){
		p->dz -= 1;
	}
}

Point playerpos(Player *p)
{
	return p->body.curdir->bbox[p->body.curact].a;
}
