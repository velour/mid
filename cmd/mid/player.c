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
	p->dx = p->dy = 0.0;
	p->ddy =  1.0;
	return p;
}

void playerfree(Player *p)
{
	free(p);
}

void playermv(Player *p, Lvl *l, int z, Point *tr, float dx, float dy)
{
	Rect b = lvltrace(l, z, p->bbox, 0.0, dy);
	dy = b.a.y - p->bbox.a.y;
	p->bbox = b;
	b = lvltrace(l, z, p->bbox, dx, 0.0);
	dx = b.a.x - p->bbox.a.x;
	p->bbox = b;

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
	if (p->dy > 10)
		p->dy = 10;
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
		p->ddy = 5.0;
		break;
/*
	case 'd':
		p->dy = (e->down ? Dy : 0.0); break;
	case 'e':
		p->dy = (e->down ? -Dy : 0.0); break;
*/
	}
}
