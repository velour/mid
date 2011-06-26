#include "../../include/mid.h"
#include "../../include/log.h"
#include <assert.h>
#include <stdlib.h>

enum { Startx = 2, Starty = 2 };

static _Bool fits(Zone *zn, int z, Point pt);
static void use(Lvl *l, int z, Point pt);
static int cmp(const void*, const void*);

Point size;

int main(int argc, char *argv[])
{
	int id, num = 1;

	if (argc < 2 || argc > 3)
		fatal("usage: envnear <env ID> [<num>]");

	id = strtol(argv[1], NULL, 10);
	if (argc == 3)
		num = strtol(argv[2], NULL, 10);

	size = envsize(id);
	size.x /= Twidth;
	size.y /= Theight;

	Zone *zn = zoneread(stdin);
	int sz = zn->lvl->w * zn->lvl->h;
	Point pts[sz];
	int n = zonelocs(zn, 0, fits, pts, sz);
	qsort(pts, n, sizeof(*pts), cmp);

	for (int i = 0; i < num; i++) {
		Env env;
		envinit(&env, id, pts[i]);
		use(zn->lvl, 0, pts[i]);
		zoneaddenv(zn, 0, env);
	}

	zonewrite(stdout, zn);
	zonefree(zn);
	return 0;
}

static _Bool fits(Zone *zn, int z, Point pt)
{
	for (int x = pt.x; x < pt.x + size.x; x++) {
		if (x >= zn->lvl->w)
			return 0;
		int y;
		for (y = pt.y; y < pt.y + size.y; y++) {
			if ((x == Startx && y == Starty)
			    || y >= zn->lvl->h
			    || blk(zn->lvl, x, y, z)->tile != ' ')
				return 0;
		}
		if (y >= zn->lvl->h)
			return 0;
		if (blk(zn->lvl, x, y, z)->tile != '#')
			return 0;
	}
	return 1;
}

static void use(Lvl *l, int z, Point pt)
{
	for (int x = pt.x; x < pt.x + size.x / Twidth; x++) {
		assert(x < l->w);
		for (int y = pt.y; y < pt.y + size.y / Theight; y++) {
			assert(y < l->h);
			blk(l, x, y, z)->tile = '.';
		}
	}
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
