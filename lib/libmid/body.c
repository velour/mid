#include "../../include/mid.h"
#include "../../include/log.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

const float Grav = 0.5;

static void loadanim(Anim **a, const char *name, const char *dir, const char *act);
static void bodymv(Body *b, Lvl *l, Point *transl);
static void dofall(Body *b, Isect is);
static void chngdir(Body *b);
static void chngact(Body *b);
static void imgmvscroll(Body *b, Point *transl, float dx, float dy);

_Bool bodyinit(Body *b, const char *name, int x, int y, int z)
{
	loadanim(&b->left.anim[Stand], name, "left", "stand");
	loadanim(&b->left.anim[Walk], name, "left", "walk");
	loadanim(&b->left.anim[Jump], name, "left", "jump");
	loadanim(&b->right.anim[Stand], name, "right", "stand");
	loadanim(&b->right.anim[Walk], name, "right", "walk");
	loadanim(&b->right.anim[Jump], name, "right", "jump");

	/* Eventually we want to load this from the resrc directory. */
	b->left.bbox[Stand] = (Rect){ { x, y }, { x + Wide, y - Tall } };
	b->left.bbox[Walk] = (Rect){ { x, y }, { x + Wide, y - Tall } };
	b->left.bbox[Jump] = (Rect){ { x, y }, { x + Wide, y - Tall } };
	b->right.bbox[Stand] = (Rect){ { x, y }, { x + Wide, y - Tall } };
	b->right.bbox[Walk] = (Rect){ { x, y }, { x + Wide, y - Tall } };
	b->right.bbox[Jump] = (Rect){ { x, y }, { x + Wide, y - Tall } };

	b->vel = (Point) { 0, 0 };
	b->z = z;
	b->imgloc = (Point) { x, y - Tall };
	b->curdir = &b->right;
	b->curact = Stand;
	b->fall = true;
	b->ddy = Grav;

	return false;
}

void bodyfree(Body *b)
{
	free(b);
}

void bodyupdate(Body *b, Lvl *l, Point *transl)
{
	bodymv(b, l, transl);
	if (b->fall && b->vel.y < Maxdy)
		b->vel.y += b->ddy;
	Anim *prevanim = b->curdir->anim[b->curact];
	chngdir(b);
	chngact(b);
	if (b->curdir->anim[b->curact] != prevanim)
		animreset(b->curdir->anim[b->curact]);
	else
		animupdate(b->curdir->anim[b->curact], 1);
}

enum { Buflen = 256 };

static void loadanim(Anim **a, const char *name, const char *dir, const char *act)
{
	char buf[Buflen];
	snprintf(buf, Buflen, "%s/%s/%s/anim", name, dir, act);
	*a = resrcacq(anims, buf, NULL);
	if (!*a)
		fatal("Failed to load %s: %s", buf, miderrstr());
}

static void bodymv(Body *b, Lvl *l, Point *transl)
{
	float xmul = b->vel.x < 0 ? 1.0 : -1.0;
	float ymul = b->vel.y < 0 ? 1.0 : -1.0;
	Isect is = lvlisect(l, b->curdir->bbox[b->curact], b->vel);
	float dx = b->vel.x + xmul * is.dx;
	float dy = b->vel.y + ymul * is.dy;
	dofall(b, is);
	for (int i = 0; i < Nacts; i++) {
		rectmv(&b->left.bbox[i], dx, dy);
		rectmv(&b->right.bbox[i], dx, dy);
	}
	imgmvscroll(b, transl, dx, dy);
}

static void dofall(Body *b, Isect is)
{
	if(b->vel.y > 0 && is.dy > 0 && b->fall) { /* hit the ground */
		/* Constantly try to fall in order to test ground
		 * beneath us. */
		b->vel.y = Grav;
		b->ddy = Grav;
		b->fall = false;
	} else if (b->vel.y < 0 && is.dy > 0) { /* hit my head on something */
		b->vel.y = 0;
		b->ddy = Grav;
		b->fall = true;
	}
	if (b->vel.y > 0 && is.dy <= 0 && !b->fall) { /* are we falling now? */
		b->vel.y = 0;
		b->ddy = Grav;
		b->fall = true;
	}
}

static void chngdir(Body *b)
{
	if (b->vel.x < 0)
		b->curdir = &b->left;
	else if (b->vel.x > 0)
		b->curdir = &b->right;
}

static void chngact(Body *b)
{
	if (b->fall)
		b->curact = Jump;
	else if (b->vel.x != 0)
		b->curact = Walk;
	else
		b->curact = Stand;
}

static void imgmvscroll(Body *b, Point *transl, float dx, float dy)
{
	if (!transl) {
		b->imgloc.x += dx;
		b->imgloc.y += dy;
		return;
	}

	float imgx = b->imgloc.x, imgy = b->imgloc.y;
	if ((dx < 0 && imgx < Scrlbuf) || (dx > 0 && imgx > Scrnw - Scrlbuf))
		transl->x -= dx;
	else
		b->imgloc.x += dx;

	if ((dy > 0 && imgy > Scrnh - Scrlbuf) || (dy < 0 && imgy < Scrlbuf))
		transl->y -= dy;
	else
		b->imgloc.y += dy;
}


void bodydraw(Gfx *g, Body *b, Point tr)
{
	if(lvlgridon){
		Rect bbox = b->curdir->bbox[b->curact];
		rectmv(&bbox, tr.x, tr.y);
		gfxfillrect(g, bbox, (Color){255,0,0,255});
	}
	animdraw(g, b->curdir->anim[b->curact], b->imgloc);
}
