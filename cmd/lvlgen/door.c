// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "../../include/mid.h"
#include "../../include/log.h"
#include "../../include/rng.h"
#include "lvlgen.h"

static int doorlocs(Lvl *lvl, int, Loc ls[], int sz);
static void sortbydist(Loc ls[], int sz, Loc l0);
static double dist(Loc l0, Loc l1);
static void extdoor(Lvl *lvl, int x, int y, int z);
static int extdoorlocs(Lvl *lvl, int, Loc ls[], int sz);

/* This algorithm is technically not even guaranteed to ever
 * terminate, but its OK for now. */
Loc doorloc(Lvl *lvl, Loc l0)
{
	int sz = lvl->w * lvl->h;
	Loc locs[sz];

	sz = doorlocs(lvl, l0.z, locs, sz);
	assert(sz > 0);
	sortbydist(locs, sz, l0);

	int q = sz / 4;
	int min = 3 * q;
	int max = min + q;
	unsigned int i = max - min == 0 ? rnd(0, sz) : rnd(min, max);
	assert (i < sz);
	return locs[i];
}

static int doorlocs(Lvl *lvl, int z, Loc ls[], int sz)
{
	int i = 0;

	for (int x = 0; x < lvl->w; x++) {
	for (int y = 0; y < lvl->h - 1; y++) {
		Tileinfo bi = tileinfo(lvl, x, y+1, z);
		if (reachable(lvl, x, y, z) && bi.flags & Tilecollide) {
			if (i == sz)
				fatal("Door loc array is too small");
			ls[i] = (Loc) {x, y, z};
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

enum { Nextra = 2 };

void extradoors(Rng *r, Lvl *lvl)
{
	int sz = lvl->w * lvl->h;
	Loc ls[sz];

	for (int z = 0; z < lvl->d - 1; z++) {
		int n = extdoorlocs(lvl, z, ls, sz);
		for (int i = 0; i < Nextra && i < n; i++) {
			int j = rngintincl(r, 0, n);
			extdoor(lvl, ls[j].x, ls[j].y, z);
			ls[j] = ls[n-1];
			n--;
		}
	}
}

static void extdoor(Lvl *lvl, int x, int y, int z)
{
	putdoor(lvl, x, y, z, '>');
	putdoor(lvl, x, y, z+1, '<');
}

static int extdoorlocs(Lvl *lvl, int z, Loc ls[], int sz)
{
	int i = 0;

	for (int x = 1; x < lvl->w; x++) {
	for (int y = 1; y < lvl->h - 1; y++) {
		if (reachable(lvl, x, y, z) && reachable(lvl, x, y, z+1)
			&& tileinfo(lvl, x, y+1, z).flags & Tilecollide
			&& tileinfo(lvl, x, y+1, z+1).flags & Tilecollide) {
			if (i == sz)
				fatal("Door loc array is too small");
			ls[i] = (Loc) {x, y, z};
			i++;
		}
	}
	}

	return i;
}

void putdoor(Lvl *lvl, int x, int y, int z, int door)
{
	if (tileinfo(lvl, x, y, z).flags & Tilewater) {
		if (door == '<')
			door = '(';
		else if (door == '>')
			door = ')';
	}
	blk(lvl, x, y, z)->tile = door;
	setreach(lvl, x, y, z);
}