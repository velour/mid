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
	Point v;		/* velocity */
	float ddy;
	Anim *walkl, *walkr, *cur;
	_Bool fall;
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
	p->v = (Point) { 0, 0 };
	p->ddy =  Grav;
	p->fall = 1;
	return p;
}

void playerfree(Player *p)
{
	free(p);
}

void playermv(Player *p, Lvl *l, int z, Point *tr, Point v)
{
	Isect is = lvlisect(l, z, p->bbox, v);
	float xmul = v.x < 0 ? 1.0 : -1.0;
	float ymul = v.y < 0 ? 1.0 : -1.0;
	float dx = v.x + xmul * is.dx, dy = v.y + ymul * is.dy;
	rectmv(&p->bbox, dx, dy);
	if(v.y >= 0 && is.dy > 0){
		p->fall = 0;
		p->ddy = 0;
	}else{
		p->fall = 1;
		p->ddy = Grav;
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
	playermv(p, l, z, tr, p->v);
	if(p->fall && p->v.y < Maxdy)
		p->v.y += p->ddy;
}

void playerdraw(Gfx *g, Player *p, Point tr)
{
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
		p->v.x = (e->down ? -Dx : 0.0); break;
	case 'f':
		p->v.x = (e->down ? Dx : 0.0); break;
	case 'e':
		if(!p->fall){
			p->v.y = (e->down ? -Dy : 0.0);
			p->ddy = Grav;
			p->fall = 1;
		}
		break;
	}
}
