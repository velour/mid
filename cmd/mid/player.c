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
	p->imgloc = (Point) { x, y - Tall };
	p->v = (Point) { 0, 0 };
	p->ddy =  Grav;
	p->fall = 1;
	return p;
}

void playerfree(Player *p)
{
	free(p);
}

/* Move the player's image or scroll the screen by the given
 * amount. */
static void imgmvscroll(Player *p, Point *transl, Point v)
{
	float imgx = p->imgloc.x, imgy = p->imgloc.y;
	if ((v.x < 0 && imgx < Scrlbuf) || (v.x > 0 && imgx > Scrnw - Scrlbuf))
		transl->x -= v.x;
	else
		p->imgloc.x += v.x;

	if ((v.y > 0 && imgy > Scrnh - Scrlbuf) || (v.y < 0 && imgy < Scrlbuf))
		transl->y -= v.y;
	else
		p->imgloc.y += v.y;
}

static void playermv(Player *p, Lvl *l, int z, Point *transl)
{
	float xmul = p->v.x < 0 ? 1.0 : -1.0;
	float ymul = p->v.y < 0 ? 1.0 : -1.0;
	Isect is = lvlisect(l, z, p->bbox, p->v);
	Point v = (Point) { p->v.x + xmul * is.dx, p->v.y + ymul * is.dy };

	if(p->v.y >= 0 && is.dy > 0){
		p->fall = 0;
		p->ddy = 0;
	}else{
		p->fall = 1;
		p->ddy = Grav;
	}

	rectmv(&p->bbox, v.x, v.y);
	imgmvscroll(p, transl, v);
}

void playerupdate(Player *p, Lvl *l, int z, Point *tr)
{
	animupdate(p->cur, 1);
	playermv(p, l, z, tr);
	if(p->fall && p->v.y < Maxdy)
		p->v.y += p->ddy;
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
