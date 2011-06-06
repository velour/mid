#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <math.h>
#include "../../include/mid.h"
#include "../../include/log.h"
#include "path.h"

static void init(Lvl *l);
static void output(Lvl *l);
/* Fill in a z-layer and return a Loc for a good door placement to the
 * next zlayer. */
static Loc zlayer(Loc loc, Lvl *lvl);
static void buildpath(Lvl *lvl, Path *p, Loc loc);
static Blk *ind(Lvl *l, int x, int y, int z);
static Loc doorloc(Loc loc, Lvl *lvl, Path *p);
static double dist(Loc l0, Loc l1);

int main(int argc, char *argv[])
{
	loginit(NULL);
	if (argc != 4 && argc != 5)
		return 1;

	int w = strtol(argv[1], NULL, 10);
	int h = strtol(argv[2], NULL, 10);
	int d = strtol(argv[3], NULL, 10);
	Lvl *lvl = lvlnew(d, w, h);
	int seed = time(NULL);
	if (argc == 5)
		seed = strtol(argv[4], NULL, 10);
	pr("seed: %d\n", seed);
	srand(seed);

	mvinit();
	init(lvl);

	Loc loc = (Loc) { 2, 3 };
	for (int z = 0; z < d; z++) {
		loc = zlayer(loc, lvl);
		if (z < d - 1) {
			ind(lvl, loc.x, loc.y, lvl->z)->tile = '>';
			lvl->z++;
			ind(lvl, loc.x, loc.y, lvl->z)->tile = '<';
		}
	}

	output(lvl);
	lvlfree(lvl);

	return 0;
}

static void init(Lvl *l)
{
	for (int z = 0; z < l->d; z++) {
	for (int y = 0; y < l->h; y++) {
	for (int x = 0; x < l->w; x++) {
		int c = ' ';
		if (x == 0 || x == l->w - 1 || y == 0 || y == l->h - 1)
				c = '#';
			ind(l, x, y, z)->tile = c;
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
				fputc(ind(l, x, y, z)->tile, stdout);
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
	Loc nxt = doorloc(loc, lvl, p);
	pathfree(p);
	return nxt;
}

enum { Maxbr = 5 };

static void buildpath(Lvl *lvl, Path *p, Loc loc)
{
	int br = rand() % Maxbr + 1;
	for (int i = 0; i < br; i++) {
		int base = rand() % Nmoves;
		for (int j = 0; j < Nmoves; j++) {
			int mv = (base + j) % Nmoves;
			Seg s = segmk(loc, moves[mv]);
			if (pathadd(lvl, p, s)) {
				buildpath(lvl, p, s.l1);
				break;
			}
		}
	}
}

static Blk *ind(Lvl *l, int x, int y, int z)
{
	int i = z * l->w * l->h + y * l->w + x;
	return &l->blks[i];
}

/* This algorithm is technically not even guaranteed to ever
 * terminate, but its OK for now. */
static Loc doorloc(Loc loc, Lvl *lvl, Path *p)
{
	int mv = lvl->w > lvl->h ? lvl->w : lvl->h;
	int mindist = mv * 2 / 3;

	for ( ; ; ) {
		Loc l1 = (Loc) { rand() % (lvl->w - 1) + 1,
				 rand() % (lvl->h - 1) + 1 };
		bool used = p->used[l1.y * lvl->w + l1.x];
		Blkinfo bi = blkinfo(lvl, l1.x, l1.y + 1);
		if (used && dist(loc, l1) >= mindist && bi.flags & Tilecollide)
			return l1;
	}

	fatal("Can't get here");
	return (Loc){0};
}

static double dist(Loc l0, Loc l1)
{
	int dx = l1.x - l0.x;
	int dy = l1.y - l0.y;
	return sqrt(dx * dx + dy * dy);
}

