#include "../../include/mid.h"
#include "../../include/log.h"
#include "resrc.h"
#include "game.h"
#include <stdlib.h>
#include <math.h>

/* Size of the bbox */
enum { Tall = 32, Wide = 32 };

/* Movement speed. */
enum { Dx = 3, Dy = 8, Maxdy = 12 };
const float Grav = 0.5;

struct Player {
	Rect bbox;
	Point scrloc;
	float dx, dy, ddy;
	Anim *walkl, *walkr, *cur;
	_Bool jmp;
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
	p->ddy =  Grav;
	return p;
}

void playerfree(Player *p)
{
	free(p);
}

void playermv(Player *p, Lvl *l, int z, Point *tr, float dx, float dy)
{
	Rect b = lvltrace(l, z, p->bbox, (Point) { dx, dy });
	dy = b.a.y - p->bbox.a.y;
	dx = b.a.x - p->bbox.a.x;
	p->bbox = b;
	if(dy) p->jmp = 0;

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
	float step = p->dy < 0 ? -1.0 : 1.0;
	for (float i = 1; i < fabs(p->dy); i++) {
		float y = p->bbox.a.y;
		playermv(p, l, z, tr, 0.0, step);
		if (y == p->bbox.a.y) /* done moving */
			break;
	}
	playermv(p, l, z, tr, p->dx, 0.0);
	if(p->dy < Maxdy)
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
		if(!p->jmp){
			p->dy = (e->down ? -Dy : 0.0);
			p->ddy = Grav;
			p->jmp = 1;
		}
		break;
/*
	case 'd':
		p->dy = (e->down ? Dy : 0.0); break;
	case 'e':
		p->dy = (e->down ? -Dy : 0.0); break;
*/
	}
}
