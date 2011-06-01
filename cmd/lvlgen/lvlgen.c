#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>

/* 2 above brick are reachable via jump. */
enum { Playerx = 2, Playery = 3 };

typedef struct Pair Pair;
struct Pair {
	int x, y;
};

typedef struct Path Path;
struct Path {
	Pair start;
	Pair end;
};

typedef enum Dir Dir;
enum Dir { Left, Right };

static void init(char tiles[], int w, int h, int d);
static void gen(char tiles[], int w, int h, int d);
static void water(char tiles[], int w, int h);
static void doors(char tiles[], int w, int h, int d);
static void platforms(char tiles[], int w, int h);
static bool blocking(Path plat, Path path[], int plen);
static bool overlap(Path a, Path b);
static void fillpath(char tiles[], int w, Path plat, char c);
static bool clear(char tiles[], int w, int h, int x0, int y0, int x1, int y1);
static Path platform(char tiles[], int w, int h, Dir dir, int x, int y);
static Pair jump(char tiles[], int w, int h, int x, int y);
static bool land(char t);

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
		platforms(tiles, w, h);
		water(tiles, w, h);
		tiles += w * h;
	}
}

static const Pair jumps[] = {
	{ 2, -2 },
	{ 1, -2 },
};

enum { Njumps = sizeof(jumps) / sizeof(jumps[0]) };

static void platforms(char tiles[], int w, int h)
{
	int plen = 0;
	Path path[w * h];

	for (int y = h - 2; y > 0; y--) {
		for (int x = 1; x < w - 1; x++) {
			if (!land(tiles[(y + 1) * w + x]))
				continue;
			Pair j = jump(tiles, w, h, x, y);
			if (j.x == x && j.y == y)
				continue;
			Dir dir = Left;
			if (j.x > x)
				dir = Right;
			j.y += 1;
			Path p = platform(tiles, w, h, dir, j.x, j.y);
			if (!blocking(p, path, plen)) {
				fillpath(tiles, w, p, '#');
				path[plen] = (Path) {{x, y}, j};
				plen++;
			}
		}
	}

/*
	for (int i = 0; i < plen; i++) {
		fprintf(stderr, "Jump: %d,%d -> %d,%d\n", path[i].start.x,
			path[i].start.y, path[i].end.x, path[i].end.y);
		assert (clear(tiles, w, h, path[i].start.x, path[i].start.y,
			      path[i].end.x, path[i].end.y));
	}
*/
}

static void fillpath(char tiles[], int w, Path plat, char c)
{
	for (int x = plat.start.x; x <= plat.end.x; x++)
		for (int y = plat.start.y; y <= plat.end.y; y++)
			tiles[y * w + x] = c;
}

static bool blocking(Path plat, Path path[], int plen)
{
	for (int i = 0; i < plen; i++)
		if (overlap(plat, path[i]))
		    return true;
	return false;
}

static bool overlap(Path a, Path b)
{
	bool xhit = a.start.x <= b.end.x && a.start.x >= b.start.x;
	xhit = xhit || b.start.x <= a.end.x && b.start.x >= a.start.x;

	bool yhit = a.start.y <= b.end.y && a.start.y >= b.start.y;
	yhit = yhit || b.start.y <= a.end.y && b.start.y >= a.start.y;

	return xhit && yhit;
}

static const float Platpr = 0.5;

static Path platform(char tiles[], int w, int h, Dir dir, int x, int y)
{
	Pair start, end;
	if (dir == Right) {
		int space = w - x;
		int len = rand() % space + 1;
		end.y = start.y = y;
		start.x = x;
		end.x = x + len;
	} else {
		int space = w;
		int len = rand() % space;
		end.y = start.y = y;
		start.x = x - len;
		end.x = x;
	}

	return (Path) {start, end};
}

static Pair jump(char tiles[], int w, int h, int x, int y)
{
	if (withprob(1.0 - Platpr))
		return (Pair) {x, y};

	int i = rand() % Njumps;
	int right = rand() % 2;
	int x1 = x + (right ? 1 : -1) * jumps[i].x;
	int y1 = y + jumps[i].y;
	fprintf(stderr, "considering jump: %d,%d -> %d,%d\n", x, y, x1, y1);
	if (!clear(tiles, w, h, x, y, x1, y1)) {
		fprintf(stderr, "jump bad\n");
		return (Pair) {x, y};
	}
	fprintf(stderr, "jump good\n");

	return (Pair) {x1, y1};
}

static bool land(char t)
{
	return t == '#';
}

static bool clear(char tiles[], int w, int h, int x0, int y0, int x1, int y1)
{

	y1 -= 1;
	if (y1 <= 0 || x1 <= 0 || x1 >= h - 1)
		return false;

	if (x0 > x1) {
		int t = x0;
		x0 = x1;
		x1 = t;
	}
	for (int i = x0; i <= x1; i++) {
		for (int j = y0; j >= y1; j--) {
			if (tiles[j * w + i] == '#')
				return false;
		}
	}
	return true;
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
			int i = y * w + x;
			if (tiles[i] == ' ')
				tiles[i] = 'w';
		}
	}
}

static const float Fprob = 0.4;

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
			printf("%02d ", y);
			for (int x = 0; x < w; x++) {
				fputc(tiles[z * h * w + y * w + x], stdout);
			}
			fputc('\n', stdout);
		}
		fputc('\n', stdout);
	}
}
