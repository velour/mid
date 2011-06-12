#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "../../include/mid.h"
#include "../../include/log.h"
#include "lvlgen.h"

static bool segok(Lvl *l, Path *p, Seg s);
static bool segconfl(Lvl *l, Path *p, Seg s);
static bool segclr(Lvl *l, Seg s);
static bool beenthere(Lvl *l, Path *p, Seg s);
static int segarea(Seg s, Loc l[], int sz);
static int segblks(Seg s, Loc l[], int sz);

Seg segmk(Loc l, Mv *m)
{
	Seg s;
	s.l0 = l;
	s.l1 = (Loc) { l.x + m->dx, l.y + m->dy };
	s.mv = m;
	return s;
}

void segpr(Seg s)
{
	pr("seg: %d,%d -> %d,%d", s.l0.x, s.l0.y, s.l1.x, s.l1.y);
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
	if (p->n == Maxsegs || !segok(l, p, s))
		return false;

	mvblit(s.mv, l, s.l0);

	Loc blks[Maxblks];
	int n = segarea(s, blks, Maxblks);
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

	return inrange && segclr(l, s) && !segconfl(l, p, s)
		&& !beenthere(l, p, s);
}

/* Conflict with other path segments? */
static bool segconfl(Lvl *l, Path *p, Seg s)
{
	Loc blks[Maxblks];
	int n = segblks(s, blks, Maxblks);

	for (int i = 0; i < n; i++) {
		if (p->used[blks[i].y * l->w + blks[i].x])
			return true;
	}

	return false;
}

/* Test if the segment area is clr. */
static bool segclr(Lvl *l, Seg s)
{
	Loc blks[Maxblks];
	int n = segarea(s, blks, Maxblks);

	for (int i = 0; i < n; i++) {
		if (blks[i].x < 0 || blks[i].x >= l->w
		    || blks[i].y < 0 || blks[i].y >= l->h)
			return false;
		Blkinfo b = blkinfo(l, blks[i].x, blks[i].y);
		if (b.flags & Tilecollide)
			return false;
	}

	return true;
}

static bool beenthere(Lvl *l, Path *p, Seg s)
{
	for (int i = 0; i < p->n; i++) {
		if (p->used[s.l1.y * l->w + s.l1.x])
			return true;
	}
	return false;
}

/* Fill 'l' with the location of level blocks that must be clr for
 * this segment. */
static int segarea(Seg s, Loc l[], int sz)
{
	const Mv *m = s.mv;
	if (sz < m->nclr)
		fatal("Area buffer is too small");
	for (int i = 0; i < m->nclr; i++)
		l[i] = (Loc) {s.l0.x + m->clr[i].x, s.l0.y + m->clr[i].y};
	return m->nclr;
}


/* Fill in 'l' (which has size 'sz') with the location of the blocks
 * for the given segment.  The return value is the number of blocks
 * filled in. */
static int segblks(Seg s, Loc l[], int sz)
{
	const Mv *m = s.mv;
	if (sz < m->nblkd)
		fatal("Blk buffer is too small");
	for (int i = 0; i < m->nblkd; i++)
		l[i] = (Loc) {s.l0.x + m->blkd[i].x, s.l0.y + m->blkd[i].y};
	return m->nblkd;
}

void pathpr(Lvl *l, Path *p)
{
	for (int y = 0; y < l->h; y++) {
		for (int x = 0; x < l->w; x++) {
			char c = ' ';
			Blkinfo bi = blkinfo(l, x, y);
			if (bi.flags & Tilecollide)
				c = '#';
			if (p->used[y * l->w + x]) {
				if (c == '#')
					c = '$';
				else
					c = 'O';
			}
			fputc(c, stderr);
		}
		fputc('\n', stderr);
	}
	fputc('\n', stderr);
}
