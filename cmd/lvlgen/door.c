#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "../../include/mid.h"
#include "../../include/log.h"
#include "lvlgen.h"

static int doorlocs(Lvl *lvl, Path *p, Loc ls[], int sz);
static void sortbydist(Loc ls[], int sz, Loc l0);
static double dist(Loc l0, Loc l1);

/* This algorithm is technically not even guaranteed to ever
 * terminate, but its OK for now. */
Loc doorloc(Lvl *lvl, Path *p, Loc l0)
{
	int sz = lvl->w * lvl->h;
	Loc locs[sz];

	sz = doorlocs(lvl, p, locs, sz);
	assert(sz > 0);
	sortbydist(locs, sz, l0);

	int i = (rand() % (sz / 4)) + (3 * sz / 4);
	assert (i > 0);
	assert (i < sz);
	return locs[i];
}

static int doorlocs(Lvl *lvl, Path *p, Loc ls[], int sz)
{
	int i = 0;

	for (int x = 0; x < lvl->w; x++) {
	for (int y = 0; y < lvl->h - 1; y++) {
		Blkinfo bi = blkinfo(lvl, x, y+1);
		if (p->used[y * lvl->w + x] && bi.flags & Tilecollide) {
			if (i == sz)
				fatal("Door loc array is too small");
			ls[i] = (Loc) {x, y};
			i++;
		}
	}
	}

	return i;
}

static Loc cmploc;

static int doorcmp(const void *a, const void *b)
{
	double da = dist(cmploc, *(Loc*) a);
	double db = dist(cmploc, *(Loc*) b);
	if (da > db)
		return 1;
	else if (da < db)
		return -1;
	return 0;
}

static void sortbydist(Loc ls[], int sz, Loc l0)
{
	cmploc = l0;
	qsort(ls, sz, sizeof(ls[0]), doorcmp);
}

static double dist(Loc l0, Loc l1)
{
	int dx = l1.x - l0.x;
	int dy = l1.y - l0.y;
	return sqrt(dx * dx + dy * dy);
}
