#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

/* 2 above brick are reachable via jump. */
enum { Playerx = 2, Playery = 3 };

static void init(char tiles[], int w, int h, int d);
static void gen(char tiles[], int w, int h, int d);
static void water(char tiles[], int w, int h);
static void floor(char tiles[], int w, int h, int r);
static void doors(char tiles[], int w, int h, int d);
static bool withprob(float p);
static void output(char tiles[], int w, int h, int d);

int main(int argc, char *argv[])
{
	if (argc != 4 && argc != 5)
		return 1;

	int w = strtol(argv[1], NULL, 10);
	int h = strtol(argv[2], NULL, 10);
	int d = strtol(argv[3], NULL, 10);
	int seed = time(NULL);
	if (argc == 5)
		seed = strtol(argv[4], NULL, 10);
	fprintf(stderr, "seed: %d\n", seed);
	srand(seed);

	char tiles[w * h * d];
	init(tiles, w, h, d);
	gen(tiles, w, h, d);
	doors(tiles, w, h, d);

	output(tiles, w, h, d);

	return 0;
}

static void init(char tiles[], int w, int h, int d)
{
	memset(tiles, ' ', w * h * d);
	tiles[(Playery + 1) * w + Playerx] = '#';
	for (int z = 0; z < d; z++) {
		for (int x = 0; x < w; x++) {
			tiles[x] = '#';
			tiles[(h - 1) * w + x] = '#';
		}
		for (int y = 0; y < h; y++) {
			tiles[y * w] = '#';
			tiles[y * w + w - 1] = '#';
		}
		tiles += w * h;
	}
}

static void gen(char tiles[], int w, int h, int d)
{
	for (int z = 0; z < d; z++) {
		water(tiles, w, h);
		for (int r = 1; r < h - 1; r++)
			floor(tiles, w, h, r);
		tiles += w * h;
	}
}

static const float Wmaxfrac = 1.0;
static const float Wprob = 0.1;

static void water(char tiles[], int w, int h)
{
	if (withprob(1.0 - Wprob))
		return;

	int ht = rand() % (int)(Wmaxfrac * (h - 2)) + 1;
	fprintf(stderr, "water height: %d\n", ht);

	for (int y = h - 2; y > h - ht - 2; y--) {
		for (int x = 1; x < w - 1; x++) {
			tiles[y * w + x] = 'w';
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

	fprintf(stderr, "row %d, floor length %d, start %d\n", r, len, x0);

	for (int x = x0; x < x0 + len; x++)
		tiles[r * w + x] = '#';
}

static bool withprob(float p)
{
	return (rand() % 100 + 1) < 100 * p;
}

static void doors(char tiles[], int w, int h, int d)
{
	for (int z = 0; z < d - 1; z++) {
		int x, y;
		do {	 /* Don't put two doors in the same place.  */
			x = rand() % (w - 2) + 1;
			y = rand() % (h - 2) + 1;
		} while (tiles[y * w + x] == '<');

		tiles[y * w + x] = '>';
		tiles[(y + 1) * w + x] = '#';
		tiles[w * h + y * w + x] = '<';
		tiles[w * h + (y + 1) * w + x] = '#';
		tiles += w * h;
	}
}

static void output(char tiles[], int w, int h, int d)
{
	printf("%d %d %d\n", d, w, h);
	for (int z = 0; z < d; z++) {
		for (int y = 0; y < h; y++) {
			for (int x = 0; x < w; x++) {
				fputc(tiles[z * h * w + y * w + x], stdout);
			}
			fputc('\n', stdout);
		}
		fputc('\n', stdout);
	}
}
