#include "../../include/mid.h"
#include "../../include/log.h"
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

const double Grav = 0.5;

static void loadanim(Anim **a, const char *name, const char *dir, const char *act);
static void bodymv(Body *b, Lvl *l, Point *transl);
static double tillwhole(double loc, double vel);
static Point velstep(Body *b, Point p);
static void dofall(Body *b, Isect is);
static void chngdir(Body *b);
static void chngact(Body *b);
static void imgmvscroll(Body *b, Point *transl, double dx, double dy);

_Bool bodyinit(Body *b, const char *name, int x, int y)
{
	loadanim(&b->left.anim[Stand], name, "left", "stand");
	loadanim(&b->left.anim[Walk], name, "left", "walk");
	loadanim(&b->left.anim[Jump], name, "left", "jump");
	loadanim(&b->right.anim[Stand], name, "right", "stand");
	loadanim(&b->right.anim[Walk], name, "right", "walk");
	loadanim(&b->right.anim[Jump], name, "right", "jump");

	/* Eventually we want to load this from the resrc directory. */
	b->bbox = (Rect){ { x, y }, { x + Wide, y - Tall } };

	b->vel = (Point) { 0, 0 };
	b->imgloc = (Point) { x, y - Tall };
	b->curdir = &b->right;
	b->curact = Stand;
	b->fall = true;
	b->a.y = Grav;

	return false;
}

void bodyfree(Body *b)
{
}

void bodyupdate(Body *b, Lvl *l, Point *transl)
{
	bodymv(b, l, transl);
	if (b->fall && b->vel.y < Maxdy)
		b->vel.y += b->a.y;
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
	double xmul = b->vel.x > 0 ? 1.0 : -1.0;
	double ymul = b->vel.y > 0 ? 1.0 : -1.0;
	Isect fallis = (Isect) { .is = false };
	Point v = b->vel;
	Point left = (Point) { fabs(v.x), fabs(v.y) };

	while (left.x > 0.0 || left.y > 0.0) {
		Point d = velstep(b, v);
		left.x -= fabs(d.x);
		left.y -= fabs(d.y);
		Isect is = lvlisect(l, b->bbox, d);
		if (is.is && is.dy != 0.0)
			fallis = is;

		d.x = d.x + -xmul * is.dx;
		d.y = d.y + -ymul * is.dy;
		v.x -= d.x;
		v.y -= d.y;

		rectmv(&b->bbox, d.x, d.y);
		imgmvscroll(b, transl, d.x, d.y);
	}
	dofall(b, fallis);
}

static Point velstep(Body *b, Point v)
{
	Point loc = b->bbox.a;
	Point d = (Point) { tillwhole(loc.x, v.x), tillwhole(loc.y, v.y) };
	if (d.x == 0.0 && v.x != 0.0)
		d.x = fabs(v.x) / v.x;
	if (fabs(d.x) > fabs(v.x))
		d.x = v.x;
	if (d.y == 0.0 && v.y != 0.0)
		d.y = fabs(v.y) / v.y;
	if (fabs(d.y) > fabs(v.y))
		d.y = v.y;
	return d;
}

static double tillwhole(double loc, double vel)
{
	if (vel > 0) {
		int l = loc + 0.5;
		return l - loc;
	} else {
		int l = loc;
		return l - loc;
	}
}

static void dofall(Body *b, Isect is)
{
	if(b->vel.y > 0 && is.dy > 0 && b->fall) { /* hit the ground */
		/* Constantly try to fall in order to test ground
		 * beneath us. */
		b->vel.y = Grav;
		b->a.y = Grav;
		b->fall = false;
	} else if (b->vel.y < 0 && is.dy > 0) { /* hit my head on something */
		b->vel.y = 0;
		b->a.y = Grav;
		b->fall = true;
	}
	if (b->vel.y > 0 && is.dy <= 0 && !b->fall) { /* are we falling now? */
		b->vel.y = 0;
		b->a.y = Grav;
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

static void imgmvscroll(Body *b, Point *tr, double dx, double dy)
{
	b->imgloc.x += dx;
	b->imgloc.y += dy;
	if (!tr)
		return;

	double imgx = b->imgloc.x + tr->x, imgy = b->imgloc.y + tr->y;
	if ((dx < 0 && imgx < Scrlbuf) || (dx > 0 && imgx > Scrnw - Scrlbuf))
		tr->x -= dx;

	if ((dy > 0 && imgy > Scrnh - Scrlbuf) || (dy < 0 && imgy < Scrlbuf))
		tr->y -= dy;
}


void bodydraw(Gfx *g, Body *b, Point tr)
{
	if(debugging){
		Rect bbox = b->bbox;
		rectmv(&bbox, tr.x, tr.y);
		gfxfillrect(g, bbox, (Color){255,0,0,255});
	}
	Point p = (Point) { b->imgloc.x + tr.x, b->imgloc.y + tr.y };
	animdraw(g, b->curdir->anim[b->curact], p);
}
