// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include "../../include/mid.h"
#include "../../include/log.h"
#include "lvlgen.h"

static Mv mvmk(Mvspec s, bool rev);
static int blkd(Mvspec s, Loc, Loc l[], int sz);
static int clr(Mvspec s, Loc, Loc l[], int sz);
static Loc mvstart(Mvspec s);
static Loc mvend(Mvspec s);

/* Move specification array: 's' is the start of the move, 'e' is the
 * end of the move, '#' is a block, ' ' is a space that *must* be
 * clear and is considered reachable.  Anything else is a wild card
 * (whatever ends up being put there while building the path).
 */
static Mvspec specs[] = {
	{ .blks = 
		"######"
		"#    #"
		"#    #"
		"s    e"
		"#    #"
		"######",
	  . revable = true,
	  .wt = 2, .w = 6, .h = 6, },

	{ .blks = 
		"######"
		"#    #"
		"#    #"
		"#    s"
		"#    #"
		"##  ##"
		"  e   ",
	  .wt = 2, .w = 6, .h = 7, },

	{ .blks = 
		"######"
		"#    #"
		"#    #"
		"s    #"
		"#    #"
		"##  ##"
		"  e   ",
	  .wt = 2, .w = 6, .h = 7, },

	{ .blks = 
		"##s ##"
		"#    #"
		"#    #"
		"#    e"
		"#    #"
		"######",
	  .wt = 2, .w = 6, .h = 6, },

	{ .blks = 
		"##s ##"
		"#    #"
		"#    #"
		"e    #"
		"#    #"
		"######",
	  .wt = 2, .w = 6, .h = 6, },

	{ .blks = 
		"#####"
		"#   #"
		"#   #"
		"s   e"
		"#####",
	  .revable = true,
	  .wt = 2, .w = 5, .h = 5, },

	{ .blks = 
		"se"
		"##",
	  .wt = 5, .w = 2, .h = 2, },

	{ .blks = 
		"es"
		"##",
	  .revable = true,
	  .wt = 5, .w = 2, .h = 2, },

	{ .blks = 
		"e s"
		"###",
	  .revable = true,
	  .wt = 10, .w = 3, .h = 2 },

	{ .blks = 
		"e  s"
		"####",
	  .revable = true,
	  .wt = 10, .w = 4, .h = 2 },

	{ .blks = 
		"    ."
		"     "
		"s   e"
		"#...#",
	  .revable = true,
	  .wt = 10, .h = 4, .w = 5 },

	{ .blks = 
		" e"
		"s#"
		"#.",
	  .revable = true,
	  .wt = 0, .w = 2, .h = 3 },

	{ .blks = 
		" e"
		"s#"
		"##",
	  .revable = true,
	  .wt = 5, .w = 2, .h = 3 },

	{ .blks = 
		"s "
		"#e"
		".#",
	  .revable = true,
	  .wt = 1, .w = 2, .h = 3 },

	{ .blks = 
		"s "
		"#e"
		"##",
	  .revable = true,
	  .wt = 1, .w = 2, .h = 3 },

	{ .blks = 
		". ."
		"  e"
		"  #"
		"s.."
		"#..",
	  .wt = 10, .w = 3, .h = 5 },

	{ .blks = 
		". ."
		"  e"
		"  #"
		"s##"
		"###",
	  .wt = 5, .w = 3, .h = 5 },

	{ .blks = 
		"  s"
		" .#"
		"e.."
		"#..",
	  .wt = 10, .w = 3, .h = 4 },

	{ .blks = 
		"  s"
		"  #"
		"e##"
		"###",
	  .wt = 5, .w = 3, .h = 4 },

	{ .blks = 
		". ."
		"e  "
		"#  "
		"..s"
		"..#",
	  .wt = 15, .w = 3, .h = 5 },

	{ .blks = 
		"s  "
		"#. "
		"..e"
		"..#",
	  .wt = 15, .w = 3, .h = 4 },

	{ .blks = 
		"e  "
		"#  "
		"##s"
		"###",
	  .revable = true,
	  .wt = 5, .w = 3, .h = 4 },
};

static const int Nspecs = sizeof(specs) / sizeof(specs[0]);

Mv *moves;
int Nmoves;

void mvini(void)
{
	for (int i = 0; i < Nspecs; i++) {
		int n = specs[i].wt;
		if (specs[i].revable)
			n *= 2;
		Nmoves += n;
	}

	moves = xalloc(Nmoves, sizeof(*moves));

	int nxt = 0;
	for (int i = 0; i < Nspecs; i++) {
		Mvspec s = specs[i];
		for (int j = 0; j < s.wt; j++) {
			moves[nxt] = mvmk(s, false);
			nxt++;
		}
		if (!s.revable)
			continue;
		for (int j = 0; j < s.wt; j++) {
			moves[nxt] = mvmk(s, true);
			nxt++;
		}
	}
	assert(nxt == Nmoves);
}

static Mv mvmk(Mvspec spec, bool rev)
{
	Loc s = mvstart(spec), e = mvend(spec);
	if (rev) {
		Loc t = s;
		s = e;
		e = t;
	}

	Mv mv;
	mv.strt = s;
	mv.wt = spec.wt;
	mv.dx = e.x - s.x;
	mv.dy = e.y - s.y;
	mv.dz = 0;
	mv.spec = spec;
	mv.nclr = clr(spec, s, mv.clr, Maxblks);
	mv.nblkd = blkd(spec, s, mv.blkd, Maxblks);

	return mv;
}

static Loc mvstart(Mvspec s)
{
	for (int x = 0; x < s.w; x++) {
	for (int y = 0; y < s.h; y++) {
		char c = s.blks[y * s.w + x];
		if (c == 's')
			return (Loc) {x, y, 0};
	}
	}
	fatal("No start location specified for move");
}

static Loc mvend(Mvspec s)
{
	for (int x = 0; x < s.w; x++) {
	for (int y = 0; y < s.h; y++) {
		char c = s.blks[y * s.w + x];
		if (c == 'e')
			return (Loc) {x, y, 0};
	}
	}
	fatal("No end location specified for move");
}

static int blkd(Mvspec s, Loc l0, Loc l[], int sz)
{

	int i = 0;

	for (int x = 0; x < s.w; x++) {
	for (int y = 0; y < s.h; y++) {
		char c = s.blks[y * s.w + x];
		if (c == '#') {
			if (i >= sz)
				fatal("blkd: array is too small\n");
			l[i] = (Loc){x - l0.x , y - l0.y, 0};
			i++;
		}
	}
	}

	return i;
}

static int clr(Mvspec s, Loc l0, Loc l[], int sz)
{

	int i = 0;

	for (int x = 0; x < s.w; x++) {
	for (int y = 0; y < s.h; y++) {
		char c = s.blks[y * s.w + x];
		if (c == ' ' || c == 's' || c == 'e') {
			if (i >= sz)
				fatal("clr: array is too small\n");
			l[i] = (Loc){x - l0.x , y - l0.y, 0};
			i++;
		}
	}
	}

	return i;
}

void mvblit(Mv *mv, Lvl *l, Loc l0)
{
	for (int x = 0; x < mv->spec.w; x++) {
	for (int y = 0; y < mv->spec.h; y++) {
		int lx = (x - mv->strt.x) + l0.x;
		int ly = (y - mv->strt.y) + l0.y;
		Loc l1 = (Loc) { lx, ly, l0.z };
		int t = mv->spec.blks[y * mv->spec.w + x];
		if (reachable(l, l1.x, l1.y, l1.z))
			continue;
		if (t == '#') {
			blk(l, lx, ly, l1.z)->tile = '#';
		} else if (t == ' ' || t == 's' || t == 'e') {
			blk(l, lx, ly, l1.z)->tile = ' ';
			setreach(l, lx, ly, l1.z);
		}
	}
	}
}

bool startonblk(Mv *mv)
{
	for (int i = 0; i < mv->nblkd; i++)
		if (mv->blkd[i].x == 0 && mv->blkd[i].y == 1)
			return true;
	return false;
}
