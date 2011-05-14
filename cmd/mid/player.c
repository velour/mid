#include "../../include/mid.h"
#include "../../include/log.h"
#include "resrc.h"
#include "game.h"
#include <stdlib.h>

/* Size of the bbox */
enum { Tall = 32, Wide = 32 };

/* Movement speed. */
enum { Dx = 3, Dy = 3, Maxdy = 12 };

struct Player {
	Rect bbox;
	Point scrloc;
	float dx, dy, ddy;
	Anim *walkl, *walkr, *cur;
};

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
	p->dx = p->dy = p->ddy =  0.0;
	return p;
}

void playerfree(Player *p)
{
	free(p);
}

void playermv(Player *p, Lvl *l, int z, Point *tr, float dx, float dy)
{
	rectmv(&p->bbox, dx, dy);
	Isect is = lvlisect(l, z, p->bbox);
	if (is.is) {
		float ddx = dx < 0 ? is.dx : -is.dx;
		float ddy = dy < 0 ? is.dy : -is.dy;
		dx += ddx;
		dy += ddy;
		if(ddy) p->ddy = 0;
		rectmv(&p->bbox, ddx, ddy);
	}
	if ((dx < 0 && p->scrloc.x < Scrlbuf) || (dx > 0 && p->scrloc.x > Scrnw - Scrlbuf))
		tr->x = tr->x - dx;
	else
		p->scrloc.x += dx;

	if ((dy > 0 && p->scrloc.y > Scrnh - Scrlbuf) || (dy < 0 && p->scrloc.y < Scrlbuf))
		tr->y = tr->y - dy;
	else
		p->scrloc.y += dy;
}

void playerupdate(Player *p, Lvl *l, int z, Point *tr)
{
	animupdate(p->cur, 1);
	playermv(p, l, z, tr, p->dx, p->dy);
	p->dy += p->ddy;
}

void playerdraw(Gfx *g, Player *p, Point tr)
{
	/* draw the player's bbox. */
	Rect bbox = p->bbox;
	rectmv(&bbox, tr.x, tr.y);
	gfxfillrect(g, bbox, (Color){255,0,0,255});

	animdraw(g, p->cur, p->scrloc);
}

void playerhandle(Player *p, Event *e)
{
	if (e->type != Keychng || e->repeat)
		return;
	switch(e->key){
	case 's':
		p->dx = (e->down ? -Dx : 0.0); break;
	case 'f':
		p->dx = (e->down ? Dx : 0.0); break;
	case 'e':
		p->dy = (e->down ? -Dy : 0.0);
		p->ddy = 1.0;
		break;
	}
}
