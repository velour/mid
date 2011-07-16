// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
#include "../../include/mid.h"
#include "../../include/log.h"
#include <assert.h>
#include <stdlib.h>

enum { Startx = 2, Starty = 2 };

static _Bool goodloc(Zone *zn, int z, Point pt);
static int cmp(const void*, const void*);

Point wh;
Rect start;

int main(int argc, char *argv[])
{
	int id, num = 1;

	loginit(NULL);

	if (argc < 2 || argc > 3)
		fatal("usage: envnear <env ID> [<num>]");

	id = strtol(argv[1], NULL, 10);
	if (argc == 3)
		num = strtol(argv[2], NULL, 10);

	Zone *zn = zoneread(stdin);

	wh = envsize(id);
	start = (Rect) { (Point) { Startx * Twidth, Starty * Theight },
		(Point) { (Startx+1) * Twidth, (Starty+1) * Theight } };

	int sz = zn->lvl->w * zn->lvl->h;
	Point pts[sz];
	int n = zonelocs(zn, 0, goodloc, pts, sz);
	if (!n)
		fatal("No locations available to place env ID: %d\n", id);

	qsort(pts, n, sizeof(*pts), cmp);
	for (int i = 0; i < num; i++) {
		Env env;
		envinit(&env, id, pts[i]);
		zoneaddenv(zn, 0, env);
	}

	zonewrite(stdout, zn);
	zonefree(zn);
	return 0;
}

static _Bool goodloc(Zone *zn, int z, Point pt)
{
	Rect r = (Rect) { (Point) { pt.x * Twidth, pt.y * Theight },
		(Point) { pt.x * Twidth + wh.x, pt.y * Theight + wh.y } };
	return !isect(start, r)
		&& zonefits(zn, z, pt, wh)
		&& zoneonground(zn, z, pt, wh)
		&& !zoneoverlap(zn, z, pt, wh);
}

static int cmp(const void *_a, const void *_b)
{
	const Point *a = (const Point*) _a;
	const Point *b = (const Point*) _b;

	double da = ptsqdist(*a, (Point){Startx, Starty});
	double db = ptsqdist(*b, (Point){Startx, Starty});

	if (da < db)
		return -1;
	else if (da > db)
		return 1;
	return 0;
}
