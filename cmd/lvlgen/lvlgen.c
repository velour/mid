#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/times.h>
#include <assert.h>
#include "../../include/mid.h"
#include "../../include/log.h"
#include "../../include/rng.h"
#include "lvlgen.h"

static void doseed(int argc, char *argv[]);
static void init(Lvl *l);
static void output(Lvl *l);
/* Fill in a z-layer and return a Loc for a good door placement to the
 * next zlayer. */
static Loc zlayer(Loc loc, Lvl *lvl);
static void buildpath(Lvl *lvl, Path *p, Loc loc);

Rng rng;

int main(int argc, char *argv[])
{
	loginit(NULL);
	if (argc != 4 && argc != 5)
		return 1;

	int w = strtol(argv[1], NULL, 10);
	int h = strtol(argv[2], NULL, 10);
	int d = strtol(argv[3], NULL, 10);
	Lvl *lvl = lvlnew(d, w, h);
	doseed(argc, argv);

	mvini();
	init(lvl);

	Loc loc = (Loc) { 2, 3 };
	for (int z = 0; z < d; z++) {
		loc = zlayer(loc, lvl);
		if (z < d - 1) {
			blk(lvl, loc.x, loc.y, lvl->z)->tile = '>';
			lvl->z++;
			blk(lvl, loc.x, loc.y, lvl->z)->tile = '<';
		}
	}

	output(lvl);
	lvlfree(lvl);

	return 0;
}

static void doseed(int argc, char *argv[])
{
	int seed;
	struct tms tm;

	if (argc == 5)
		seed = strtol(argv[4], NULL, 10);
	else
		seed =  times(&tm);
	pr("Level seed: %d", seed);
	rngini(&rng, seed);
}

static void init(Lvl *l)
{
	for (int z = 0; z < l->d; z++) {
	for (int y = 0; y < l->h; y++) {
	for (int x = 0; x < l->w; x++) {
		int c = ' ';
		if (x == 0 || x == l->w - 1 || y == 0 || y == l->h - 1)
				c = '#';
			blk(l, x, y, z)->tile = c;
	}
	}
	}
}

static void output(Lvl *l)
{
	printf("%d %d %d\n", l->d, l->w, l->h);
	for (int z = 0; z < l->d; z++) {
		for (int y = 0; y < l->h; y++) {
			for (int x = 0; x < l->w; x++) {
				fputc(blk(l, x, y, z)->tile, stdout);
			}
			fputc('\n', stdout);
		}
		fputc('\n', stdout);
	}
}

static Loc zlayer(Loc loc, Lvl *lvl)
{
	Path *p = pathnew(lvl);
	buildpath(lvl, p, loc);
	Loc nxt = doorloc(lvl, p, loc);
	pathfree(p);
	return nxt;
}

enum { Minbr = 3, Maxbr = 9 };

static void buildpath(Lvl *lvl, Path *p, Loc loc)
{
	unsigned int br = rnd(Minbr, Maxbr);
	for (int i = 0; i < br; i++) {
		unsigned int base = rnd(0, Nmoves);
		for (int j = 0; j < Nmoves; j++) {
			unsigned int mv = (base + j) % Nmoves;
			Seg s = segmk(loc, &moves[mv]);
			if (pathadd(lvl, p, s)) {
				buildpath(lvl, p, s.l1);
				break;
			}
		}
	}
}

Blk *blk(Lvl *l, int x, int y, int z)
{
	int i = z * l->w * l->h + y * l->w + x;
	return &l->blks[i];
}

unsigned int rnd(int min, int max)
{
	assert (min >= 0);
	assert (max > min);
	unsigned int r = rngint(&rng);

	if (min == 0)
		return r % max;

	return r % (max - min) + min;
}
