// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../../include/mid.h"
#include "../../include/log.h"
#include "lvlgen.h"

static int extend(Mv [], int, Lvl *, Path *, Loc);
static int tryadd(Lvl *l, Path *p, Seg s);
static Seg segmk(Loc l, Mv *m);
static bool segok(Lvl *l, Path *p, Seg s);
static bool segconfl(Lvl *l, Path *p, Seg s);
static bool segclr(Lvl *l, Seg s);
static bool doorsok(Lvl *l, Path *p, Seg s);
static bool atstart(Path *p, int x, int y, int z);

Path *pathnew(Lvl *l)
{
	Path *p = xalloc(1, sizeof(*p));
	p->maxsegs = l->w * l->h;
	p->segs = xalloc(p->maxsegs, sizeof(p->segs[0]));
	return p;
}

void pathfree(Path *p)
{
	free(p->segs);
	free(p);
}

enum { Minbr = 3, Maxbr = 9 };

void pathbuild(Lvl *lvl, Path *p, Loc loc)
{
	unsigned int br = rnd(Minbr, Maxbr);
	for (int i = 0; i < br; i++) {
		int ind = extend(moves, Nmoves, lvl, p, loc);
		if (ind >= 0)
			pathbuild(lvl, p, p->segs[ind].l1);
	}
}

static int extend(Mv mvs[], int n, Lvl *lvl, Path *p, Loc loc)
{
	Mv *failed = NULL;
	unsigned int base = rnd(0, n);
	for (int i = 0; i < n; i++) {
		Mv *mv = mvs + ((base + i) % n);
		if (mv == failed)
			continue;
		Seg s = segmk(loc, mv);
		int ind = tryadd(lvl, p, s);
		if (ind >= 0)
			return ind;
		failed = mv;
	}
	return -1;
}

static int tryadd(Lvl *l, Path *p, Seg s)
{
	if (p->nsegs == p->maxsegs || !segok(l, p, s))
		return -1;
	mvblit(s.mv, l, s.l0);

	p->segs[p->nsegs] = s;
	p->nsegs++;

	return p->nsegs - 1;
}

static Seg segmk(Loc l, Mv *m)
{
	Seg s;
	s.l0 = l;
	s.l1 = (Loc) { l.x + m->dx, l.y + m->dy, l.z + m->dz };
	s.mv = m;
	return s;
}

static bool segok(Lvl *l, Path *p, Seg s)
{
	return (p->nsegs != 0 || startonblk(s.mv))	// 1st seg must start on a block
		&& s.l1.x > 0 && s.l1.x < l->w - 1
		&& s.l1.y > 0 && s.l1.y < l->h - 1
		&& s.l1.z >= 0 && s.l1.z < l->d
		&& !reachable(l, s.l1.x, s.l1.y, s.l1.z)	// haven't been there yet.
		&& segclr(l, s)
		&& !segconfl(l, p, s)
		&& doorsok(l, p, s);
}

static bool segconfl(Lvl *l, Path *p, Seg s)
{
	for (int i = 0; i < s.mv->nblkd; i++) {
		Loc *bp = &s.mv->blkd[i];
		Loc blk = (Loc) { bp->x + s.l0.x, bp->y + s.l0.y, bp->z + s.l0.z };
		if (blk.x < 0 || blk.x >= l->w
			|| blk.y < 0 || blk.y >= l->h
			|| blk.z < 0 || blk.z >= l->d
			|| reachable(l, blk.x, blk.y, blk.z))
			return true;
	}

	return false;
}

static bool doorsok(Lvl *l, Path *p, Seg s)
{
	static const unsigned long rejflgs = Tilefdoor | Tilebdoor | Tileup | Tiledown;

	for (int i = 0; i < s.mv->ndoor; i++) {
		Loc d = s.mv->door[i];
		int x = s.l0.x + d.x;
		int y = s.l0.y + d.y;
		int z = s.l0.z + d.z;
		if (tileinfo(l, x, y, z).flags & rejflgs || atstart(p, x, y, z))
			return false;
	}

	return true;
}

static bool atstart(Path *p, int x, int y, int z)
{
	if (p->nsegs == 0)
		return 0;
	return p->segs[0].l0.x == x
		&& p->segs[0].l0.y == y
		&& p->segs[0].l0.z == z;
}

static bool segclr(Lvl *l, Seg s)
{
	for (int i = 0; i < s.mv->nclr; i++) {
		Loc *bp = &s.mv->clr[i];
		Loc b = (Loc) { bp->x + s.l0.x, bp->y + s.l0.y, bp->z + s.l0.z };
		if (b.x < 0 || b.x >= l->w
			|| b.y < 0 || b.y >= l->h
			|| b.z < 0 || b.z >= l->d
			|| tileinfo(l, b.x, b.y, b.z).flags & Tilecollide)
			return false;
	}

	return true;
}