#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../../include/mid.h"
#include "../../include/log.h"
#include "path.h"

static bool segok(Lvl *l, Path *p, Seg s);
static bool segconfl(Lvl *l, Path *p, Seg s);
static bool segclear(Lvl *l, Seg s);
static int segarea(Seg s, Loc l[], int sz);
static int segblks(Seg s, Loc l[], int sz);

const Move moves[] = {
	{ 1, 0 }, { -1, 0 },
	{ 2, 0 }, { -2, 0 },
	{ 1, 2 }, { 1, -2 }, { -1, 2 }, { -1, -2 },
	{ 2, 1 }, { 2, -1 }, { -2, 1 }, { -2, -1 },
	{ 1, 1 }, { 1, -1 }, { -1, 1 }, { -1, -1 },
	{ 2, 2 }, { 2, -2 }, { -2, 2 }, { -2, -2 },
};

const int Nmoves = sizeof(moves) / sizeof(moves[0]);

enum { Bufsz = 10 };

Seg segmk(Loc l, const Move *m)
{
	Seg s;
	s.l0 = l;
	s.l1 = (Loc) { l.x + m->dx, l.y + m->dy };
	s.mv = m;
	return s;
}

Path *pathnew(Lvl *l)
{
	Path *p = calloc(1, sizeof(*p) + sizeof(bool[l->w * l->h]));
	return p;
}

void pathfree(Path *p)
{
	free(p);
}

bool pathadd(Lvl *l, Path *p, Seg s)
{
	if (p->n == Maxsegs - 1)
		fatal("Path is full");

	if (!segok(l, p, s))
		return false;

	Loc blks[Bufsz];
	int n = segblks(s, blks, Bufsz);
	int base = l->z * l->w * l->h;
	for (int i = 0; i < n; i++)
		l->blks[base + blks[i].y * l->w + blks[i].x].tile = '#';

	n = segarea(s, blks, Bufsz);
	for (int i = 0; i < n; i++)
		p->used[blks[i].y * l->w + blks[i].x] = true;

	p->ss[p->n] = s;
	p->n++;

	return true;
}

static bool segok(Lvl *l, Path *p, Seg s)
{
	assert (s.l0.x > 0 && s.l0.x < l->w - 1);
	assert (s.l0.y > 0 && s.l0.y < l->h - 1);
	bool inrange = s.l1.x > 0 && s.l1.x < l->w - 1
		&& s.l1.y > 0 && s.l1.y < l->h - 1;

	return inrange && segclear(l, s) && !segconfl(l, p, s);
}

/* Conflict with other path segments? */
static bool segconfl(Lvl *l, Path *p, Seg s)
{
	Loc blks[Bufsz];
	int n = segblks(s, blks, Bufsz);

	for (int i = 0; i < n; i++) {
		if (p->used[blks[i].y * l->w + blks[i].x])
			return true;
	}

	return false;
}

/* Test if the segment area is clear. */
static bool segclear(Lvl *l, Seg s)
{
	Loc blks[Bufsz];
	int n = segblks(s, blks, Bufsz);

	for (int i = 0; i < n; i++) {
		Blkinfo b = blkinfo(l, blks[i].x, blks[i].y);
		if (b.flags & Tilecollide)
			return false;
	}

	return true;
}

/* Fill 'l' with the location of level blocks that must be clear for
 * this segment. */
static int segarea(Seg s, Loc l[], int sz)
{
	int dx = s.l0.x > s.l1.x ? -1 : 1;
	int dy = s.l0.y > s.l1.y ? -1 : 1;
	int i = 0;

	for (int x = s.l0.x; x != s.l1.x; x += dx) {
	for (int y = s.l0.y; y != s.l1.y; y += dy) {
		if (i == sz)
			fatal("Too many segment clear blocks");
		l[i] = (Loc) { x, y };
		i++;
	}
	}
	return i;
}


/* Fill in 'l' (which has size 'sz') with the location of the blocks
 * for the given segment.  The return value is the number of blocks
 * filled in. */
static int segblks(Seg s, Loc l[], int sz)
{
	if (s.mv->dy == 0) {
		int i = 0;;
		int dx = s.l0.x > s.l1.x ? -1 : 1;
		for (int x = s.l0.x; x != s.l1.x; x += dx) {
			if (i == sz)
				fatal("Too many segment blocks");
			l[i] = (Loc) { x, s.l0.y + 1 };
			i++;
		}
		return i;
	}

	if (sz < 2)
		fatal("Too many segment blocks");
	l[0] = (Loc) { s.l0.x, s.l0.y + 1 };
	l[1] = (Loc) { s.l1.x, s.l1.y + 1 };
	return 2;
}
