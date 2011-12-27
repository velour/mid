// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
#include "../../include/mid.h"
#include <stdbool.h>
#include <math.h>

static void bodymv(Body *b, Lvl *l);
static double tillwhole(double loc, double vel);
static Point velstep(Body *b, Point p);
static void dofall(Body *b, Lvl*, Isect is);

void bodyinit(Body *b, int x, int y, int w, int h)
{
	*b = (Body){
		.bbox = { { x, y }, { x + w, y + h } },
		.vel = { 0, 0 },
		.acc = { 0, 0 },
		.fall = false
	};
}

void bodyupdate(Body *b, Lvl *l)
{
	bodymv(b, l);
	if (b->fall && b->vel.y < Maxdy)
		b->vel.y += b->acc.y;
}

static void bodymv(Body *b, Lvl *l)
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
	}
	dofall(b, l, fallis);
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
	if (vel > 0)
		return ceil(loc) - loc;
	return floor(loc) - loc;
}

static void dofall(Body *b, Lvl *l, Isect is)
{
	double g = blkgrav(lvlmajorblk(l, b->bbox).flags);
	if(b->vel.y > 0 && is.dy > 0 && b->fall) { /* hit the ground */
		/* Constantly try to fall in order to test ground
		 * beneath us. */
		b->acc.y = g;
		b->fall = false;
	} else if (b->vel.y < 0 && is.dy > 0) { /* hit my head on something */
		b->vel.y = 0;
		b->acc.y = g;
		b->fall = true;
	}
	if (!is.is && !b->fall) { /* are we falling now? */
		b->vel.y = 0;
		b->acc.y = g;
		b->fall = true;
	}
}
