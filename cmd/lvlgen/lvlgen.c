#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

/* 2 above brick are reachable via jump. */

static void init(char tiles[], int w, int h);
static void gen(char tiles[], int w, int h);
static void water(char tiles[], int w, int h);
static void floor(char tiles[], int w, int h, int r);
static bool withprob(float p);
static void output(char tiles[], int w, int h);

int main(int argc, char *argv[])
{
	if (argc != 3 && argc != 4)
		return 1;

	int w = strtol(argv[1], NULL, 10);
	int h = strtol(argv[2], NULL, 10);
	int seed = time(NULL);
	if (argc == 4)
		seed = strtol(argv[3], NULL, 10);
	printf("seed: %d\n", seed);
	srand(seed);

	char tiles[w * h];
	init(tiles, w, h);
	gen(tiles, w, h);
	output(tiles, w, h);

	return 0;
}

static void init(char tiles[], int w, int h)
{
	memset(tiles, ' ', w * h);
	for (int r = 0; r < w; r++) {
		tiles[r] = '#';
		tiles[(w - 1) * h + r] = '#';
	}
	for (int c = 0; c < h; c++) {
		tiles[c * h] = '#';
		tiles[c * h + h - 1] = '#';
	}
}

static void gen(char tiles[], int w, int h)
{
	water(tiles, w, h);
	for (int r = 1; r < h - 1; r++)
		floor(tiles, w, h, r);
}

static const float Wmaxfrac = 1.0;
static const float Wprob = 0.1;

static void water(char tiles[], int w, int h)
{
	if (withprob(1.0 - Wprob))
		return;

	int ht = rand() % (int)(Wmaxfrac * (h - 2)) + 1;
	printf("water height: %d\n", ht);

	for (int r = h - 2; r > h - ht - 2; r--) {
		for (int c = 1; c < w - 1; c++) {
			tiles[r * h + c] = 'w';
		}
	}
}

static const float Fprob = 0.4;

static void floor(char tiles[], int w, int h, int r)
{
	if (withprob(1.0 - Fprob))
		return;
	int len = rand() % (w - 3) + 1;
	int slack = w - 2 - len;
	int x0 = rand() % slack + 1;

	printf("row %d, floor length %d, start %d\n", r, len, x0);

	for (int c = x0; c < x0 + len; c++)
		tiles[r * h + c] = '#';
}

static bool withprob(float p)
{
	return (rand() % 100 + 1) < 100 * p;
}

static void output(char tiles[], int w, int h)
{
	for (int r = 0; r < w; r++) {
		for (int c = 0; c < w; c++) {
			fputc(tiles[r * h + c], stdout);
		}
		fputc('\n', stdout);
	}
}
