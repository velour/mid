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
	Point imgloc;
	Point v;		/* velocity */
	float ddy;
	Anim *stand, *walk, *jump, *cur;
	_Bool fall;
	char *km;
};

Player *playernew(int x, int y, char km[])
{
	Player *p = malloc(sizeof(*p));
	if (!p)
		return NULL;
	p->stand = p->cur = resrcacq(anim, "anim/stand/anim", NULL);
	if (!p->stand)
		fatal("Failed to load the player stand anim: %s", miderrstr());
	p->walk = resrcacq(anim, "anim/walk/anim", NULL);
	if (!p->walk)
		fatal("Failed to load the player walk anim: %s", miderrstr());
	p->jump = resrcacq(anim, "anim/jump/anim", NULL);
	if (!p->jump)
		fatal("Failed to load the player jump anim: %s", miderrstr());
	p->bbox = (Rect){ { x, y }, { x + Wide, y - Tall } };
	p->imgloc = (Point) { x, y - Tall };
	p->v = (Point) { 0, 0 };
	p->ddy =  Grav;
	p->fall = 1;
	p->km = km;
	return p;
}

void playerfree(Player *p)
{
	free(p);
}

/* Move the player's image or scroll the screen. */
static void imgmvscroll(Player *p, Point *transl, float dx, float dy)
{
	float imgx = p->imgloc.x, imgy = p->imgloc.y;
	if ((dx < 0 && imgx < Scrlbuf) || (dx > 0 && imgx > Scrnw - Scrlbuf))
		transl->x -= dx;
	else
		p->imgloc.x += dx;

	if ((dy > 0 && imgy > Scrnh - Scrlbuf) || (dy < 0 && imgy < Scrlbuf))
		transl->y -= dy;
	else
		p->imgloc.y += dy;
}

static void dofall(Player *p, Isect is)
{
	if(p->v.y > 0 && is.dy > 0 && p->fall) { /* hit the ground */
		/* Constantly try to fall in order to test ground
		 * beneath us. */
		p->v.y = Grav;
		p->ddy = Grav;
		p->fall = 0;
	} else if (p->v.y < 0 && is.dy > 0) { /* hit my head on something */
		p->v.y = 0;
		p->ddy = Grav;
		p->fall = 1;
	}
	if (p->v.y > 0 && is.dy <= 0 && !p->fall) { /* are we falling now? */
		p->v.y = 0;
		p->ddy = Grav;
		p->fall = 1;
	}
}

static void updateanim(Player *p)
{
	Anim *cur = p->cur;
	if (p->fall)
		p->cur = p->jump;
	else if (p->v.x != 0)
		p->cur = p->walk;
	else
		p->cur = p->stand;
	if (p->cur != cur)
		animreset(p->cur);
}

static void playermv(Player *p, Lvl *l, int z, Point *transl)
{
	float xmul = p->v.x < 0 ? 1.0 : -1.0;
	float ymul = p->v.y < 0 ? 1.0 : -1.0;
	Isect is = lvlisect(l, z, p->bbox, p->v);
	float dx = p->v.x + xmul * is.dx;
	float dy = p->v.y + ymul * is.dy;
	dofall(p, is);
	rectmv(&p->bbox, dx, dy);
	imgmvscroll(p, transl, dx, dy);
}

void playerupdate(Player *p, Lvl *l, int z, Point *tr)
{
	animupdate(p->cur, 1);
	playermv(p, l, z, tr);

	if(p->fall && p->v.y < Maxdy)
		p->v.y += p->ddy;

	updateanim(p);
}

void playerdraw(Gfx *g, Player *p, Point tr)
{
	Rect bbox = p->bbox;
	rectmv(&bbox, tr.x, tr.y);
	gfxfillrect(g, bbox, (Color){255,0,0,255});

	animdraw(g, p->cur, p->imgloc);
}

void playerhandle(Player *p, Event *e)
{
	if (e->type != Keychng || e->repeat)
		return;

	char k = e->key;
	if(k == p->km[Mvleft]){
		if (e->down && p->v.x > -Dx)
			p->v.x -= Dx;
		else if (!e->down)
			p->v.x += Dx;
	}else if(k == p->km[Mvright]){
		if (e->down && p->v.x < Dx)
			p->v.x += Dx;
		else if (!e->down)
			p->v.x -= Dx;
	}else if(k == p->km[Mvjump]){
		if(!p->fall){
			p->v.y = (e->down ? -Dy : 0.0);
			p->ddy = Grav;
			p->fall = 1;
		}
	}
}
