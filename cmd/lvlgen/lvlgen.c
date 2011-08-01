// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <assert.h>
#include <unistd.h>
#include "../../include/mid.h"
#include "../../include/log.h"
#include "../../include/rng.h"
#include "lvlgen.h"

static int rng(Rng *, int, char *[]);
static void init(Lvl *l);
extern void lvlwrite(FILE *, Lvl *);
static void stairs(Rng *, Lvl *);
static int stairlocs(Lvl *, Loc []);

Rng r;
enum { Startx = 2, Starty = 2 };

int main(int argc, char *argv[])
{
	loginit(NULL);

	if (argc != 4 && argc != 6)
		return 1;

	int usdargs = rng(&r, argc, argv);
	argc -= usdargs;
	argv += usdargs;

	int w = strtol(argv[1], NULL, 10);
	int h = strtol(argv[2], NULL, 10);
	int d = strtol(argv[3], NULL, 10);
	Lvl *lvl = lvlnew(d, w, h);

	mvsinit();

restart:
	init(lvl);
	water(lvl);

	Loc loc = (Loc) { Startx, Starty, 0 };
	blk(lvl, loc.x, loc.y, 0)->tile = ' ';
	Path *p = pathnew(lvl);
	pathbuild(lvl, p, loc);
	pathfree(p);

	morereach(lvl);
	closeunits(lvl);
	if (closeunreach(lvl) < lvl->w * lvl->h * lvl->d * 0.40)
		goto restart;

	stairs(&r, lvl);
	lvlwrite(stdout, lvl);
	lvlfree(lvl);

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
		seed = time(0) ^ getpid() ^ getpid() << 16;
		pr("lvlgen seed = %lu", (unsigned long) seed);
	}
	rnginit(r, seed);

	return args;
}

static void init(Lvl *l)
{
	for (int z = 0; z < l->d; z++) {
	for (int y = 0; y < l->h; y++) {
	for (int x = 0; x < l->w; x++) {
		int c = ' ';
		if (x == 0 || x == l->w - 1 || y == 0 || y == l->h - 1)
				c = '#';
		*blk(l, x, y, z) = (Blk) { .tile = c };
	}
	}
	}
}

unsigned int rnd(int min, int max)
{
	return rngintincl(&r, min, max);
}

static void stairs(Rng *r, Lvl *lvl)
{
	if (tileinfo(lvl, Startx, Starty, 0).flags & Tilewater)
		blk(lvl, Startx, Starty, 0)->tile = 'U';
	else
		blk(lvl, Startx, Starty, 0)->tile = 'u';
	setreach(lvl, Startx, Starty, 0);

	Loc ls[lvl->w * lvl->h * lvl->d];
	int nls = stairlocs(lvl, ls);
	if (nls == 0)
		fatal("No stair locations");

	Loc l = ls[rnd(0, nls - 1)];
	if (tileinfo(lvl, l.x, l.y, l.z).flags & Tilewater)
		blk(lvl, l.x, l.y, l.z)->tile = 'D';
	else
		blk(lvl, l.x, l.y, l.z)->tile = 'd';
	setreach(lvl, l.x, l.y, l.z);
}

static int stairlocs(Lvl *lvl, Loc ls[])
{
	int nls = 0;
	for (int z = 0; z < lvl->d; z++)
	for (int x = 1; x < lvl->w-1; x++)
	for (int y = 1; y < lvl->h-2; y++) {
		if (reachable(lvl, x, y, z) &&  tileinfo(lvl, x, y+1, z).flags & Tilecollide
			&& !(tileinfo(lvl, x, y, z).flags & (Tilefdoor | Tilebdoor))) {
			ls[nls] = (Loc){ x, y, z };
			nls++;
		}
	}
	return nls;
}

bool reachable(Lvl *l, int x, int y, int z)
{
	return blk(l, x, y, z)->flags != 0;
}

void setreach(Lvl *l, int x, int y, int z)
{
	blk(l, x, y, z)->flags = 1;
	if (blk(l, x, y, z)->tile == '.')
		blk(l, x, y, z)->tile = ' ';
}