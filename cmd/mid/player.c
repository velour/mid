#include "../../include/mid.h"
#include "../../include/log.h"
#include "game.h"
#include "body.h"
#include <stdlib.h>

enum { Dx = 3, Dy = 8 };

struct Player {
	Body body;
};

Player *playernew(int x, int y)
{
	Player *p = malloc(sizeof(*p));
	if (!p)
		return NULL;
	if(bodynew(&p->body, "knight", x, y)){
		free(p);
		return NULL;
	}
	return p;
}

void playerfree(Player *p)
{
	free(p);
}

void playerupdate(Player *p, Lvl *l, int z, Point *tr)
{
	bodyupdate(&p->body, l, z, tr);
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
	}
}
