#include "../../include/mid.h"
#include "../../include/log.h"
#include "../../include/rng.h"
#include <time.h>
#include <stdlib.h>
#include <limits.h>

enum { Startx = 2, Starty = 2 };

typedef struct Loc {
	Point p;
	int z;
} Loc;

extern _Bool enemyinit(Enemy *, EnemyID id, int x, int y);
static int rng(Rng *, int, char *[]);
static int rmz(Loc [], int nls, int z);
static int idargs(int argc, char *argv[], int **ids);
static int locs(Zone *, Loc []);
static _Bool goodloc(Zone *, int, Point);

int main(int argc, char *argv[])
{
	int *ids;

	loginit(NULL);

	Rng r;
	int usdargs = rng(&r, argc, argv);
	argc -= usdargs;
	argv += usdargs;

	int n = idargs(argc, argv, &ids);

	if (argc < 3)
		fatal("usage: enmgen [-s <seed>] <ID>+ <num>");

	long num = strtol(argv[argc-1], NULL, 10);
	if (num == LLONG_MIN || num == LLONG_MAX)
			fatal("Invalid number: %s", argv[argc-1]);

	Zone *zn = zoneread(stdin);
	if (!zn)
		die("Failed to read the zone: %s", miderrstr());

	Loc ls[zn->lvl->d * zn->lvl->w * zn->lvl->h];
	int nls = locs(zn, ls);

	int i;
	for (i = 0; i < num && nls > 0; i++) {
		int idind = rngintincl(&r, 0, n);
		int lind = rngintincl(&r, 0, nls);
		Loc l = ls[lind];
		if (nls > 1)
			ls[lind] = ls[nls-1];
		nls--;
		Enemy enm;
		if (!enemyinit(&enm, ids[idind], l.p.x, l.p.y))
			fatal("Failed to initialize enemy with ID: %d", ids[idind]);
		if (!zoneaddenemy(zn, l.z, enm)) {
			nls = rmz(ls, nls, l.z);
			num--;
		}
	}

	if (i < num)
		fatal("Failed to place all items");

	zonewrite(stdout, zn);
	zonefree(zn);
	xfree(ids);

	return 0;
}

static int rng(Rng *r, int argc, char *argv[])
{
	int args = 0;
	clock_t seed = 0;

	if (argv[1][0] == '-' && argv[1][1] == 's') {
		seed = strtol(argv[2], NULL, 10);
		args = 2;
	} else {
		seed = time(0);
		pr("enmgen seed = %lu", (unsigned long) seed);
	}
	rnginit(r, seed);

	return args;
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
		for (int i = 0; i < npts; i++) {
			locs[nxt] = (Loc) { pts[i], z };
			nxt++;
		}
	}
	return nxt;
}

static _Bool goodloc(Zone *zn, int z, Point pt)
{
	int doorrad = 2;
	return (pt.x != Startx || pt.y != Starty)
		&& !zonehasflags(zn, z, pt, (Point) { Twidth, Theight }, Tcollide)
		&& !zonehasflags(zn, z, (Point) { pt.x - doorrad, pt.y },
			(Point) { 2 * doorrad * Twidth, Theight },
			Tfdoor | Tbdoor | Tup)
		&& zoneongrnd(zn, z, pt, (Point) { Twidth, Theight })
		&& !zoneoverlap(zn, z, pt, (Point) { Twidth, Theight });
}

static int rmz(Loc ls[], int nls, int z)
{
	for (int i = 0; i < nls; i++) {
		if (ls[i].z != z)
			continue;
		ls[i] = ls[nls-1];
		nls--;
	}

	return nls;
}
