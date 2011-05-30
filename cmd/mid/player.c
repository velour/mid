#include "../../include/mid.h"
#include "../../include/log.h"
#include "game.h"
#include "body.h"
#include <stdlib.h>

enum { Dx = 3, Dy = 8 };

struct Player {
	Body body;
	int dz;
};

Player *playernew(int x, int y)
{
	Player *p = malloc(sizeof(*p));
	if (!p)
		return NULL;
	if(bodyinit(&p->body, "knight", x, y)){
		free(p);
		return NULL;
	}
	p->dz = 0;
	return p;
}

void playerfree(Player *p)
{
	free(p);
}

void playerupdate(Player *p, Lvl *l, int *z, Point *tr)
{
	bodyupdate(&p->body, l, *z, tr);
	if (p->dz == 0)
		return;
	Blkinfo bi = lvlmajorblk(l, *z, p->body.curdir->bbox[p->body.curact]);
	if (p->dz > 0 && bi.flags & Blkbdoor)
		*z += 1;
	else if (p->dz < 0 && bi.flags & Blkfdoor)
		*z -= 1;
	p->dz = 0;
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
		if (e->down && p->body.vel.x > -Dx)
			p->body.vel.x -= Dx;
		else if (!e->down)
			p->body.vel.x += Dx;
	}else if(k == kmap[Mvright]){
		if (e->down && p->body.vel.x < Dx)
			p->body.vel.x += Dx;
		else if (!e->down)
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
