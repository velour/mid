// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
#include "../../include/mid.h"
#include "lvlgen.h"

/* You can jump up 2. */
enum { Uplim = 2 };

static void expndreach(Lvl *lvl, int x, int y, int z);
static void reach(Lvl *lvl, int x, int y, int z);
static void reachup(Lvl *lvl, int x, int y, int z);
static void reachdown(Lvl *lvl, int x, int y, int z);
static void reachover(Lvl *lvl, int x, int y, int z);

void morereach(Lvl *lvl)
{
	for (int z = 0; z < lvl->d; z++) {
	for (int x = 1; x < lvl->w - 1; x++) {
	for (int y = 1; y < lvl->h - 1; y++) {
		if (reachable(lvl, x, y, z))
			expndreach(lvl, x, y, z);
	}
	}
	}
}

static void expndreach(Lvl *lvl, int x, int y, int z)
{
	Tileinfo tundr = tileinfo(lvl, x, y+1, z);
	if (tundr.flags & Tilecollide) {
		reachup(lvl, x, y, z);
		reachover(lvl, x, y, z);
	} else {
		reachdown(lvl, x, y, z);
	}
}

static void reach(Lvl *lvl, int x, int y, int z)
{
	if (tileinfo(lvl, x, y, z).flags & Tilecollide || reachable(lvl, x, y, z))
		return;
	setreach(lvl, x, y, z);
	expndreach(lvl, x, y, z);
}

static void reachup(Lvl *lvl, int x, int y, int z)
{
	for (int yy = y; yy >= y - Uplim && yy > 0; yy--) {
		Tileinfo ti = tileinfo(lvl, x, yy, z);
		if (ti.flags & Tilecollide)
			return;
		reach(lvl, x, yy, z);
		reach(lvl, x-1, yy, z);
		reach(lvl, x+1, yy, z);
	}
}

static void reachdown(Lvl *lvl, int x, int y, int z)
{
	reach(lvl, x-1, y+1, z);
	reach(lvl, x, y+1, z);
	reach(lvl, x+1, y+1, z);
}

static void reachover(Lvl *lvl, int x, int y, int z)
{
	reach(lvl, x-1, y, z);
	reach(lvl, x+1, y, z);
}

/* Fill in single block dips in the ground.  This cannot hurt
 * reachability and gets rid of a bunch of unpleasentness
 * in the levels. */
void closeunits(Lvl *lvl)
{
	for (int z = 0; z < lvl->d; z++) {
	for (int x = 1; x < lvl->w - 1; x++) {
	for (int y = 1; y < lvl->h - 1; y++) {
		if (tileinfo(lvl, x-1, y, z).flags & Tilecollide
			&& tileinfo(lvl, x+1, y, z).flags & Tilecollide
			&& tileinfo(lvl, x, y+1, z).flags & Tilecollide) {
			*blk(lvl, x, y, z) = (Blk) { .tile = '#' };
		}
	}
	}
	}
}

int closeunreach(Lvl *lvl)
{
	int nreach = 0;

	for (int z = 0; z < lvl->d; z++) {
	for (int x = 1; x < lvl->w - 1; x++) {
	for (int y = 1; y < lvl->h - 1; y++) {
		if (reachable(lvl, x, y, z)) {
			nreach++;
			continue;
		}
		if (tileinfo(lvl, x, y, z).flags & Tilecollide)
			continue;
		blk(lvl, x, y, z)->tile = '#';
	}
	}
	}

	return nreach;
}