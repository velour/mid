#include "../../include/mid.h"
#include "../../include/log.h"
#include <stdlib.h>

enum { Startx = 2, Starty = 2 };

static _Bool empty(Zone *, int, Point);
static int cmp(const void*, const void*);

int main(int argc, char *argv[])
{
	int id, num = 1;

	if (argc < 2 || argc > 3)
		fatal("usage: enmnear <enemy ID> [<num>]");

	id = argv[1][0];
	if (argc == 3)
		num = strtol(argv[2], NULL, 10);

	initresrc();

	Zone *zn = zoneread(stdin);
	int sz = zn->lvl->w * zn->lvl->h;
	Point pts[sz];
	int n = zonelocs(zn, 0, empty, pts, sz);
	qsort(pts, n, sizeof(*pts), cmp);

	for (int i = 0; i < num; i++) {
		Enemy enm;
		enemyinit(&enm, id, pts[i].x, pts[i].y);
		blk(zn->lvl, pts[i].x, pts[i].y, 0)->tile = '.';
		zoneaddenemy(zn, 0, enm);
	}

	zonewrite(stdout, zn);
	zonefree(zn);
	return 0;
}

static _Bool empty(Zone *zn, int z, Point pt)
{
	return (pt.x != Startx || pt.y != Starty)
		&& blk(zn->lvl, pt.x, pt.y, z)->tile == ' '
		&& pt.y < zn->lvl->h - 1
		&& blk(zn->lvl, pt.x, pt.y+1, z)->tile == '#';
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
