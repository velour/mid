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
static int offsets(Mvspec s, const char *accept, Loc l0, Loc l[], int sz);
static Loc indloc(Mvspec, int);

static const char *clrtiles = " es";
static const char *blkdtiles = "#";

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
	int si = strcspn(spec.blks, "s");
	int ei = strcspn(spec.blks, "e");
	if (rev) {
		int t = si;
		si = ei;
		ei = t;
	}

	Loc s = indloc(spec, si), e = indloc(spec, ei);

	Mv mv = (Mv) {
		.strt = s,
		.wt = spec.wt,
		.dx = e.x - s.x,
		.dy = e.y - s.y,
		.dz = 0,
		.spec = spec,
	};
	mv.nclr = offsets(spec, clrtiles, s, mv.clr, Maxblks);
	mv.nblkd = offsets(spec, blkdtiles, s, mv.blkd, Maxblks);

	return mv;
}

static int offsets(Mvspec s, const char *accept, Loc l0, Loc l[], int sz)
{

	int n = 0;

	for (int i = 0; i < strlen(s.blks); i++) {
		if (strchr(accept, s.blks[i]) != NULL) {
			if (n >= sz)
				fatal("offsets: array is too small\n");
			Loc cur =  indloc(s, i);
			l[n] = (Loc) { cur.x - l0.x, cur.y - l0.y, 0 };
			n++;
		}
	}

	return n;
}

static Loc indloc(Mvspec s, int i)
{
	return (Loc) { i % s.w, i / s.w, 0 };
}

void mvblit(Mv *mv, Lvl *lvl, Loc l0)
{
	char *blks = mv->spec.blks;
	Loc s = mv->strt;

	for (int i = 0; i < strlen(blks); i++) {
		char t = blks[i];
		Loc cur = indloc(mv->spec, i);
		Loc l1 = (Loc) { cur.x - s.x + l0.x, cur.y - s.y + l0.y, l0.z };

		if (reachable(lvl, l1.x, l1.y, l1.z))
			continue;

		if (strchr(blkdtiles, t) != NULL) {
			blk(lvl, l1.x, l1.y, l1.z)->tile = t;
		} else if (strchr(clrtiles, t) != NULL) {
			blk(lvl, l1.x, l1.y, l1.z)->tile = ' ';
			setreach(lvl, l1.x, l1.y, l1.z);
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
