// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
#include "../../include/mid.h"
#include "lvlgen.h"

/* You can jump up 2. */
enum { Uplim = 2 };

static void expndreach(Lvl *lvl, int x, int y);
static void reach(Lvl *lvl, int x, int y);
static void reachup(Lvl *lvl, int x, int y);
static void reachdown(Lvl *lvl, int x, int y);
static void reachover(Lvl *lvl, int x, int y);

void morereach(Lvl *lvl)
{
	for (int x = 1; x < lvl->w - 1; x++) {
	for (int y = 1; y < lvl->h - 1; y++) {
		if (reachable(lvl, x, y, lvl->z))
			expndreach(lvl, x, y);
	}
	}
}

static void expndreach(Lvl *lvl, int x, int y)
{
	Tileinfo bundr = tileinfo(lvl, x, y+1, lvl->z);
	if (bundr.flags & Tilecollide) {
		reachup(lvl, x, y);
		reachover(lvl, x, y);
	} else {
		reachdown(lvl, x, y);
	}
}

static void reach(Lvl *lvl, int x, int y)
{
	Tileinfo bi = tileinfo(lvl, x, y, lvl->z);
	if (bi.flags & Tilecollide || reachable(lvl, x, y, lvl->z))
		return;
	setreach(lvl, x, y, lvl->z);
	expndreach(lvl, x, y);
}

static void reachup(Lvl *lvl, int x, int y)
{
	for (int yy = y; yy >= y - Uplim && yy > 0; yy--) {
		Tileinfo bi = tileinfo(lvl, x, yy, lvl->z);
		if (bi.flags & Tilecollide)
			return;
		reach(lvl, x, yy);
		reach(lvl, x-1, yy);
		reach(lvl, x+1, yy);
	}
}

static void reachdown(Lvl *lvl, int x, int y)
{
	reach(lvl, x-1, y+1);
	reach(lvl, x, y+1);
	reach(lvl, x+1, y+1);
}

static void reachover(Lvl *lvl, int x, int y)
{
	reach(lvl, x-1, y);
	reach(lvl, x+1, y);
}

void closeunreach(Lvl *lvl)
{
	for (int x = 1; x < lvl->w - 1; x++) {
	for (int y = 1; y < lvl->h - 1; y++) {
		Tileinfo bi = tileinfo(lvl, x, y, lvl->z);
		if (reachable(lvl, x, y, lvl->z) || bi.flags & Tilecollide)
			continue;
		blk(lvl, x, y, lvl->z)->tile = '#';
	}
	}
}