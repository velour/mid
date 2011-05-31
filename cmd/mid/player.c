#include "../../include/mid.h"
#include "../../include/log.h"
#include "game.h"
#include "body.h"
#include <stdlib.h>
#include <stdbool.h>

enum { Dxwater = 1, Dxair = 3, Dy = 8 };

struct Player {
	Body body;
	bool water;
	bool left, right;
	int dz;
};

static int curdx(Player *p);

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

	p->water = bi.flags & Blkwater;

	p->body.vel.x = 0;
	if (p->left)
		p->body.vel.x -= curdx(p);
	if (p->right)
		p->body.vel.x += curdx(p);

	if (p->dz > 0 && bi.flags & Blkbdoor)
		*z += 1;
	else if (p->dz < 0 && bi.flags & Blkfdoor)
		*z -= 1;
	p->body.z = *z;
	p->dz = 0;

	bodyupdate(&p->body, l, *z, tr);
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
		p->left = e->down;
	}else if(k == kmap[Mvright]){
		p->right = e->down;
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

static int curdx(Player *p)
{
	if (p->water)
		return Dxwater;
	return Dxair;
}
