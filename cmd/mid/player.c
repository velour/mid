#include "../../include/mid.h"
#include "../../include/log.h"
#include "resrc.h"
#include "game.h"
#include <stdlib.h>

struct Player {
	Rect bbox;
	Point scrloc;
	Anim *walkl, *walkr, *cur;
};

enum { Tall = 32, Wide = 32 };

Player *playernew(int x, int y)
{
	Player *p = malloc(sizeof(*p));
	if (!p)
		return NULL;
	p->walkl = p->walkr = p->cur = resrcacq(anim, "anim/walk/anim", NULL);
	if (!p->walkl)
		fatal("Failed to load the player animation: %s", miderrstr());
	p->bbox = (Rect){ { x, y }, { x + Wide, y - Tall } };
	p->scrloc = (Point) { x, y - Tall };
	return p;
}

void playerfree(Player *p)
{
	free(p);
}

void playerupdate(Player *p)
{
	animupdate(p->cur, 1);
}

void playerdraw(Gfx *g, Player *p, Point tr)
{
	animdraw(g, p->cur, p->scrloc);
}

enum { Dx = 2, Dy = 2 };

void playermv(Player *p, Point *tr, int dx, int dy)
{
	if ((dx < 0 && p->scrloc.x < 112) || (dx > 0 && p->scrloc.x > 412))
		tr->x = tr->x - dx;
	else
		p->scrloc.x += dx;

	if ((dy > 0 && p->scrloc.y > 412) || (dy < 0 && p->scrloc.y < 112))
		tr->y = tr->y - dy;
	else
		p->scrloc.y += dy;
	rectmv(&p->bbox, dx, dy);
}

void playerhandle(Player *p, Point *tr, Event *e)
{
	if (e->type != Keychng)
		return;
	if (!e->down)
		return;
	switch(e->key){
	case 's':
		playermv(p, tr, -Dx, 0); break;
	case 'f':
		playermv(p, tr, Dx, 0); break;
	case 'e':
		playermv(p, tr, 0, -Dy); break;
	case 'd':
		playermv(p, tr, 0, Dy); break;
	}
}
