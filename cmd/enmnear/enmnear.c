// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
#include "../../include/mid.h"
#include "../../include/log.h"
#include <stdlib.h>

enum { Startx = 2, Starty = 2 };

extern _Bool enemyinit(Enemy *, EnemyID id, int x, int y);
static _Bool goodloc(Zone *, int, Point);
static int cmp(const void*, const void*);

int main(int argc, char *argv[])
{
	int id, num = 1;

	loginit(NULL);

	if (argc < 2 || argc > 3)
		fatal("usage: enmnear <enemy ID> [<num>]");

	id = strtol(argv[1], NULL, 10);
	if (argc == 3)
		num = strtol(argv[2], NULL, 10);

	Zone *zn = zoneread(stdin);
	if (!zn)
		fatal("Failed to read the zone: %s", miderrstr());

	int sz = zn->lvl->w * zn->lvl->h;
	Point pts[sz];
	int n = zonelocs(zn, 0, goodloc, pts, sz);
	if (!n)
		fatal("No available locations for enemy ID %d", id);

	qsort(pts, n, sizeof(*pts), cmp);
	for (int i = 0; i < num; i++) {
		Enemy enm;
		if (!enemyinit(&enm, id, pts[i].x, pts[i].y))
			fatal("Failed to initialize enemy ID %d", id);
		zoneaddenemy(zn, 0, enm);
	}

	zonewrite(stdout, zn);
	zonefree(zn);
	return 0;
}

static _Bool goodloc(Zone *zn, int z, Point pt)
{
	return (pt.x != Startx || pt.y != Starty)
		&& zonefits(zn, z, pt, (Point) { Twidth, Theight })
		&& zoneonground(zn, z, pt, (Point) { Twidth, Theight })
		&& !zoneoverlap(zn, z, pt, (Point) { Twidth, Theight });
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
