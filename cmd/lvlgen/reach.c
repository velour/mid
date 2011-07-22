// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
#include "../../include/mid.h"

/* Map a tile to its reachable counter-part.*/
static char mkreach[] = {
	['.'] = ' ',
	['w'] = 'W',
};

/* You can jump up 2. */
enum { Uplim = 2 };

static void expndreach(Lvl *lvl, int x, int y);
static void setreach(Lvl *lvl, int x, int y);
static void reachup(Lvl *lvl, int x, int y);
static void reachdown(Lvl *lvl, int x, int y);
static void reachover(Lvl *lvl, int x, int y);

void morereach(Lvl *lvl)
{
	for (int x = 1; x < lvl->w - 1; x++) {
	for (int y = 1; y < lvl->h - 1; y++) {
		Blkinfo bi = blkinfo(lvl, x, y, lvl->z);
		if (bi.flags & Tilereach)
			expndreach(lvl, x, y);
	}
	}
}

static void expndreach(Lvl *lvl, int x, int y)
{
	Blkinfo bundr = blkinfo(lvl, x, y+1, lvl->z);
	if (bundr.flags & Tilecollide) {
		reachup(lvl, x, y);
		reachover(lvl, x, y);
	} else {
		reachdown(lvl, x, y);
	}
}

static void setreach(Lvl *lvl, int x, int y)
{
	Blkinfo bi = blkinfo(lvl, x, y, lvl->z);
	if (bi.flags & Tilecollide || bi.flags & Tilereach)
		return;

	Blk *b = blk(lvl, x, y, lvl->z);
	char r = mkreach[(int) b->tile];
	if (r)
		b->tile = r;

	expndreach(lvl, x, y);
}

static void reachup(Lvl *lvl, int x, int y)
{
	for (int yy = y; yy >= y - Uplim && yy > 0; yy--) {
		Blkinfo bi = blkinfo(lvl, x, yy, lvl->z);
		if (bi.flags & Tilecollide)
			return;
		setreach(lvl, x, yy);
		setreach(lvl, x-1, yy);
		setreach(lvl, x+1, yy);
	}
}

static void reachdown(Lvl *lvl, int x, int y)
{
	setreach(lvl, x-1, y+1);
	setreach(lvl, x, y+1);
	setreach(lvl, x+1, y+1);
}

static void reachover(Lvl *lvl, int x, int y)
{
	setreach(lvl, x-1, y);
	setreach(lvl, x+1, y);
}

void closeunreach(Lvl *lvl)
{
	for (int x = 1; x < lvl->w - 1; x++) {
	for (int y = 1; y < lvl->h - 1; y++) {
		Blkinfo bi = blkinfo(lvl, x, y, lvl->z);
		if (bi.flags & Tilereach || bi.flags & Tilecollide)
			continue;
		blk(lvl, x, y, lvl->z)->tile = '#';
	}
	}
}