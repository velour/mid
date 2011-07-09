#include "../../include/mid.h"
#include "../../include/log.h"
#include "../../include/rng.h"
#include <sys/times.h>
#include <stdlib.h>
#include <limits.h>

enum { Startx = 2, Starty = 2 };

typedef struct Loc {
	Point p;
	int z;
} Loc;

Point wh;
Rect start;

static Rng rnginit(void);
static int idargs(int argc, char *argv[], int **ids);
static int locs(Zone *, Loc []);
static _Bool goodloc(Zone *, int, Point);

int main(int argc, char *argv[])
{
	int *ids;
	int n = idargs(argc, argv, &ids);

	loginit(NULL);

	if (argc < 3)
		fatal("%d  usage: envgen <ID>+ <num>", argc);

	long num = strtol(argv[argc-1], NULL, 10);
	if (num == LLONG_MIN || num == LLONG_MAX)
		fatal("Invalid number: %s", argv[argc-1]);

	start = (Rect) { (Point) { Startx * Twidth, Starty * Theight },
		(Point) { (Startx+1) * Twidth, (Starty+1) * Theight } };

	Rng r = rnginit();
	Zone *zn = zoneread(stdin);
	Loc ls[zn->lvl->d * zn->lvl->w * zn->lvl->h];

	int i;
	for (i = 0; i < num; i++) {
		int id = ids[rngintincl(&r, 0, n)];
		wh = envsize(id);
		int nls = locs(zn, ls);
		if (nls == 0)
			fatal("No location available to place env ID: %d\n", id);

		int lind = rngintincl(&r, 0, nls);
		Loc l = ls[lind];
		if (nls > 1)
			ls[lind] = ls[nls-1];
		nls--;
		Env env;
		envinit(&env, id, l.p);
		zoneaddenv(zn, l.z, env);
	}

	if (i < num)
		fatal("Failed to place all items");

	zonewrite(stdout, zn);
	zonefree(zn);
	xfree(ids);

	return 0;
}

static Rng rnginit(void)
{
	Rng r;
	struct tms tm;
	clock_t seed = times(&tm);
	pr("envgen seed = %lu", (unsigned long) seed);
	rngini(&r, seed);
	return r;
}

static int idargs(int argc, char *argv[], int *ids[])
{
	int i;

	*ids = xalloc(argc, sizeof(*ids));
	for (i = 1; i < argc - 1; i++) {
		long l = strtol(argv[i], NULL, 10);
		if (l == LLONG_MIN || l == LLONG_MAX)
			fatal("Invalid number: %s", argv[i]);
		(*ids)[i-1] = l;
	}

	return i-1;
}

static int locs(Zone *zn, Loc locs[])
{
	int nxt = 0;
	for (int z = 0; z < zn->lvl->d; z++) {
		int sz = zn->lvl->w * zn->lvl->h;
		Point pts[sz];
		int npts = zonelocs(zn, z, goodloc, pts, sz);
		if (!npts)
			fatal("No available locations at layer %d", z);
		for (int i = 0; i < npts; i++) {
			locs[nxt] = (Loc) { pts[i], z };
			nxt++;
		}
	}
	return nxt;
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