// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
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

Point wh;
Rect start;

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
		fatal("%d  usage: envgen <ID>+ <num>", argc);

	long num = strtol(argv[argc-1], NULL, 10);
	if (num == LLONG_MIN || num == LLONG_MAX)
		fatal("Invalid number: %s", argv[argc-1]);

	start = (Rect) { (Point) { Startx * Twidth, Starty * Theight },
		(Point) { (Startx+1) * Twidth, (Starty+1) * Theight } };

	Zone *zn = zoneread(stdin);
	if (!zn)
		die("Failed to read the zone: %s", miderrstr());

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
		if (!zoneaddenv(zn, l.z, env)) {
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
		pr("itmgen seed = %lu", (unsigned long) seed);
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
		&& !zonehasflags(zn, z, pt, wh, Tilecollide | Tilebdoor | Tilefdoor | Tiledown)
		&& zoneongrnd(zn, z, pt, wh)
		&& !zoneoverlap(zn, z, pt, wh);
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
