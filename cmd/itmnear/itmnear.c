#include "../../include/mid.h"
#include "../../include/log.h"
#include <stdlib.h>

enum { Startx = 2, Starty = 2 };

static _Bool empty(Zone *, int, Point);
static _Bool hasitm(Zone *zn, int z, Point pt);
static int cmp(const void*, const void*);

int main(int argc, char *argv[])
{
	int id, num = 1;

	if (argc < 2 || argc > 3)
		fatal("usage: itmnear <item ID> [<num>]");

	id = strtol(argv[1], NULL, 10);
	if (argc == 3)
		num = strtol(argv[2], NULL, 10);

	Zone *zn = zoneread(stdin);
	int sz = zn->lvl->w * zn->lvl->h;
	Point pts[sz];
	int n = zonelocs(zn, 0, empty, pts, sz);
	qsort(pts, n, sizeof(*pts), cmp);

	for (int i = 0; i < num; i++) {
		Item it;
		iteminit(&it, id, pts[i]);
		zoneadditem(zn, 0, it);
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
		&& blk(zn->lvl, pt.x, pt.y+1, z)->tile == '#'
		&& !hasitm(zn, z, pt);
}

static _Bool hasitm(Zone *zn, int z, Point pt)
{
	Item *itms = zn->itms[z];
	for (int i = 0; i < Maxitms; i++) {
		if (!itms[i].id)
			continue;
		Point a = itms[i].bod.bbox.a;
		if (a.x == pt.x * Twidth && a.y == pt.y * Theight)
			return 1;
	}

	return 0;
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
