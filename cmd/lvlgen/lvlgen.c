/* © 2013 the Mid Authors under the MIT license. See AUTHORS for the list of authors.*/

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

static void parseargs(int, char *[]);
static void rng(Rng *);
static void init(Lvl *l);
extern void lvlwrite(FILE *, Lvl *);
static void stairs(Rng *, Lvl *, unsigned int, unsigned int);
static int stairlocs(Lvl *, Loc []);

static char *seedstr = NULL;
static bool addwater = true;
static bool randstart;
Rng r;

int main(int argc, char *argv[])
{
	loginit(NULL);

	if (argc < 4)
		fatal("Expected 3 arguments");

	parseargs(argc, argv);
	rng(&r);

	int w = strtol(argv[1], NULL, 10);
	int h = strtol(argv[2], NULL, 10);
	int d = strtol(argv[3], NULL, 10);
	Lvl *lvl = lvlnew(d, w, h, 0);

	unsigned int x0 = 2, y0 = 2;
	if (randstart) {
		x0 = rnd(1, w-2);
		y0 = rnd(1, h-2);
	}

	mvsinit();

	do{
		init(lvl);
		if (addwater)
			water(lvl);

		Loc loc = (Loc) { x0, y0, 0 };
		Path *p = pathnew(lvl);
		pathbuild(lvl, p, loc);
		pathfree(p);

		morereach(lvl);
		closeunits(lvl);
	}while(closeunreach(lvl) < lvl->w * lvl->h * lvl->d * 0.40);

	stairs(&r, lvl, x0, y0);

	bool foundstart = false;
	for (int x = 0; x < w; x++) {
	for (int y = 0; y < h; y++) {
		if (blk(lvl, x, y, 0)->tile == 'u' || blk(lvl, x, y, 0)->tile == 'U') {
			foundstart = true;
			break;
		}
	}
	}
	assert(foundstart);

	lvlwrite(stdout, lvl);
	lvlfree(lvl);

	return 0;
}

static void parseargs(int argc, char *argv[])
{
	for (unsigned int i = 4; i < argc; i++) {
		if (i < argc - 1 && strcmp("-s", argv[i]) == 0) {
			seedstr = argv[++i];
		} else if (strcmp("-w", argv[i]) == 0) {
			addwater = false;
		} else if (strcmp("-r", argv[i]) == 0) {
			randstart = true;
		}
	}
}

static void rng(Rng *r)
{
	clock_t seed = time(0) ^ getpid() ^ getpid() << 16;

	if (seedstr)
		seed = strtol(seedstr, NULL, 10);

	rnginit(r, seed);
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

static void stairs(Rng *r, Lvl *lvl, unsigned int x0, unsigned int y0)
{
	if (tileinfo(lvl, x0, y0, 0).flags & Twater)
		blk(lvl, x0, y0, 0)->tile = 'U';
	else
		blk(lvl, x0, y0, 0)->tile = 'u';
	setreach(lvl, x0, y0, 0);

	Loc ls[lvl->w * lvl->h * lvl->d];
	int nls = stairlocs(lvl, ls);
	if (nls == 0)
		fatal("No stair locations");

	Loc l = ls[rnd(0, nls - 1)];
	if (tileinfo(lvl, l.x, l.y, l.z).flags & Twater)
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
		if (reachable(lvl, x, y, z) &&  tileinfo(lvl, x, y+1, z).flags & Tcollide
			&& !(tileinfo(lvl, x, y, z).flags & (Tfdoor | Tbdoor | Tup))) {
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