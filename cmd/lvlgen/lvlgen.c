#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include "../../include/mid.h"
#include "../../include/log.h"

static void init(Lvl *l);
static void output(Lvl *l);
static Blk *ind(Lvl *l, int x, int y, int z);
//static bool withprob(float p);

int main(int argc, char *argv[])
{
	loginit(NULL);
	if (argc != 4 && argc != 5)
		return 1;

	int w = strtol(argv[1], NULL, 10);
	int h = strtol(argv[2], NULL, 10);
	int d = strtol(argv[3], NULL, 10);
	Lvl *l = lvlnew(d, w, h);
	int seed = time(NULL);
	if (argc == 5)
		seed = strtol(argv[4], NULL, 10);
	pr("seed: %d\n", seed);
	srand(seed);

	init(l);
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

static Blk *ind(Lvl *l, int x, int y, int z)
{
	int i = z * l->w * l->h + y * l->w + x;
	return &l->blks[i];
}

/*
static bool withprob(float p)
{
	return (rand() % 100 + 1) < 100 * p;
}
*/
