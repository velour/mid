#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>
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

int main(int argc, char *argv[])
{
	loginit(NULL);
	if (argc != 4 && argc != 5)
		return 1;

	int w = strtol(argv[1], NULL, 10);
	int h = strtol(argv[2], NULL, 10);
	int d = strtol(argv[3], NULL, 10);
	if (d != 1)
		fatal("d != 1 is unimplemented");
	Lvl *l = lvlnew(d, w, h);
	int seed = time(NULL);
	if (argc == 5)
		seed = strtol(argv[4], NULL, 10);
	pr("seed: %d\n", seed);
	srand(seed);

	mvinit();
	init(l);
	zlayer((Loc) { 2, 3 }, l);
	output(l);

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
	pr("path length: %d", p->n);
	pathpr(lvl, p);
	pathfree(p);
	return loc;
}

enum { Maxbr = 5 };

static void buildpath(Lvl *lvl, Path *p, Loc loc)
{
	int br = rand() % Maxbr + 1;
	pr("branching: %d", br);
	for (int i = 0; i < br; i++) {
		int base = rand() % Nmoves;
		for (int j = 0; j < Nmoves; j++) {
			int mv = (base + j) % Nmoves;
			Seg s = segmk(loc, moves[mv]);
			if (pathadd(lvl, p, s)) {
				fprintf(stderr,
					"loc=%d,%d, mv=%d, loc'=%d,%d\n",
				       loc.x, loc.y, mv, s.l1.x, s.l1.y);
				pathpr(lvl, p);
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
