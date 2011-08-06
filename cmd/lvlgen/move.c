// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include "../../include/mid.h"
#include "../../include/log.h"
#include "lvlgen.h"

static void cntmoves(void);
static Mvspec *specrev(Mvspec *s);
static int addmv(Mvspec *, Mv moves[]);
static Mv mvmk(Mvspec *);
static int offsets(Mvspec *, const char *accept, Loc l[], int sz);
static void blitdoor(Lvl *, Loc, int);
static Loc indloc(Mvspec *, int);

static const char *strttiles = "s";
static const char *endtiles = "e";
static const char *blkdtiles = "#";
static const char *doortiles = "<>";
static const char *clrtiles = " es<>";

/*
Move specification array: any character in strttiles is the start of
the move, any character in endtiles is the end of the move, anything
in blkdtiles is a collidable block, characters in doortiles are doors,
and any character in clrtiles must be clear and is considered reachable.
Anything else is a wild card (whatever ends up being put there while
building the path).  If .revable is true then both the inital version
and the version with the start and end swapped are considered.
*/
static Mvspec specs[] = {
	{ .blks =
		"####"
		"s >#"
		"####"

		"####"
		"e <#"
		"####",
	  .flgs = Mvrev,
	  .wt = 5, .w = 4, .h = 3 },

	{ .blks =
		"####"
		"#> s"
		"####"

		"####"
		"#< e"
		"####",
	  .flgs = Mvrev,
	  .wt = 5, .w = 4, .h = 3 },

	{ .blks = 
		"######"
		"#    #"
		"#    #"
		"s    e"
		"#    #"
		"######",
	  . flgs = Mvrev,
	  .wt = 2, .w = 6, .h = 6 },

	{ .blks = 
		"######"
		"#    #"
		"#    #"
		"#    s"
		"#    #"
		"##  ##"
		"  e   ",
	  .wt = 2, .w = 6, .h = 7 },

	{ .blks = 
		"######"
		"#    #"
		"#    #"
		"s    #"
		"#    #"
		"##  ##"
		"  e   ",
	  .wt = 2, .w = 6, .h = 7 },

	{ .blks = 
		"##s ##"
		"#    #"
		"#    #"
		"#    e"
		"#    #"
		"######",
	  .wt = 2, .w = 6, .h = 6 },

	{ .blks = 
		"##s ##"
		"#    #"
		"#    #"
		"e    #"
		"#    #"
		"######",
	  .wt = 2, .w = 6, .h = 6 },

	{ .blks = 
		"#####"
		"#   #"
		"#   #"
		"s   e"
		"#####",
	  .flgs = Mvrev,
	  .wt = 2, .w = 5, .h = 5 },

	{ .blks = 
		"se"
		"##",
	  .wt = 5, .w = 2, .h = 2 },

	{ .blks = 
		"es"
		"##",
	  .flgs = Mvrev,
	  .wt = 5, .w = 2, .h = 2 },

	{ .blks = 
		"e s"
		"###",
	  .flgs = Mvrev,
	  .wt = 10, .w = 3, .h = 2 },

	{ .blks = 
		"e  s"
		"####",
	  .flgs = Mvrev,
	  .wt = 10, .w = 4, .h = 2 },

	{ .blks = 
		"    ."
		"     "
		"s   e"
		"#...#",
	  .flgs = Mvrev,
	  .wt = 10, .w = 5, .h = 4, },

	{ .blks = 
		" e"
		"s#"
		"#.",
	  .flgs = Mvrev,
	  .wt = 0, .w = 2, .h = 3 },

	{ .blks = 
		" e"
		"s#"
		"##",
	  .flgs = Mvrev,
	  .wt = 5, .w = 2, .h = 3 },

	{ .blks = 
		"s "
		"#e"
		".#",
	  .flgs = Mvrev,
	  .wt = 1, .w = 2, .h = 3 },

	{ .blks = 
		"s "
		"#e"
		"##",
	  .flgs = Mvrev,
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
	  .flgs = Mvrev,
	  .wt = 5, .w = 3, .h = 4 },

	{ .blks =
		"s   e"
		"#####",
	.flgs = Mvrev | Mvwtr,
	.wt = 10, .w = 5, .h = 2 },

	{ .blks =
		" e"
		" #"
		" *"
		" *"
		"s*"
		"#*",
	.flgs = Mvrev | Mvwtr,
	.wt = 5, .w = 2, .h = 6 },

	{ .blks =
		"e "
		"# "
		"* "
		"* "
		"*s"
		"*#",
	.flgs = Mvrev | Mvwtr,
	.wt = 5, .w = 2, .h = 6 },

	{ .blks =
		"e  "
		"# *"
		"# *"
		"# *"
		"#s "
		"##",
	.flgs = Mvrev | Mvwtr,
	.wt = 5, .w = 3, .h = 6 },

	{ .blks =
		"  e"
		"* #"
		"* #"
		"* #"
		" s#"
		"###",
	.flgs = Mvrev | Mvwtr,
	.wt = 5, .w = 3, .h = 6 },

	{ .blks =
		"   e"
		"   #"
		"   *"
		"  **"
		"s***"
		"#***",
	.flgs = Mvrev | Mvwtr,
	.wt = 5, .w = 4, .h = 6 },

	{ .blks =
		"e   "
		"#   "
		"*   "
		"**  "
		"***s"
		"***#",
	.flgs = Mvrev | Mvwtr,
	.wt = 5, .w = 4, .h = 6 },

	{ .blks =
		"####"
		"#> s"
		"####"

		"####"
		"#< e"
		"####",
	.flgs = Mvrev | Mvwtr,
	.wt = 2, .w = 4, .h = 3 },

	{ .blks =
		"####"
		"s >#"
		"####"

		"####"
		"e <#"
		"####",
	.flgs = Mvrev | Mvwtr,
	.wt = 2, .w = 4, .h = 3 },
};

static const int Nspecs = sizeof(specs) / sizeof(specs[0]);

Mv *moves;
int nmoves;
Mv *wtrmvs;
int nwtrmvs;

void mvsinit(void)
{
	cntmoves();

	Mv *mv = xalloc(nmoves, sizeof(moves[0]));
	Mv *wmv = xalloc(nwtrmvs, sizeof(wtrmvs[0]));
	moves = mv;
	wtrmvs = wmv;

	for (int i = 0; i < Nspecs; i++) {
		Mvspec *s = specs + i;
		Mv **m = s->flgs & Mvwtr ? &wmv : &mv;
		*m += addmv(s, *m);
		if (s->flgs & Mvrev)
			*m += addmv(specrev(s), *m);
	}

	assert(mv - moves == nmoves);
	assert(wmv - wtrmvs == nwtrmvs);
}

static void cntmoves(void)
{
	for (int i = 0; i < Nspecs; i++) {
		Mvspec *s = specs + i;
		int n = s->wt;
		if (s->flgs & Mvrev)
			n += s->wt;

		if (s->flgs & Mvwtr)
			nwtrmvs += n;
		else
			nmoves += n;
	}
}

static Mvspec *specrev(Mvspec *s)
{
	Mvspec *rev = xalloc(1, sizeof(*rev));
	*rev = *s;
	rev->blks = xalloc(strlen(s->blks)+1, sizeof(*s->blks));
	strcpy(rev->blks, s->blks);

	int si = strcspn(rev->blks, strttiles);
	int ei = strcspn(rev->blks, endtiles);
	char tmp = rev->blks[si];
	rev->blks[si] = rev->blks[ei];
	rev->blks[ei] = tmp;

	return rev;
}

static int addmv(Mvspec *s, Mv moves[])
{
	for (int i = 0; i < s->wt; i++)
		moves[i] = mvmk(s);
	return s->wt;
}

static Mv mvmk(Mvspec *spec)
{
	Loc s = indloc(spec, strcspn(spec->blks, strttiles));
	Loc e = indloc(spec, strcspn(spec->blks, endtiles));

	Mv mv = (Mv) {
		.strt = s,
		.wt = spec->wt,
		.dx = e.x - s.x,
		.dy = e.y - s.y,
		.dz = e.z - s.z,
		.spec = spec,
	};
	mv.nclr = offsets(spec, clrtiles, mv.clr, Maxblks);
	mv.nblkd = offsets(spec, blkdtiles, mv.blkd, Maxblks);
	mv.ndoor = offsets(spec, doortiles, mv.door, Maxdrs);

	return mv;
}

static int offsets(Mvspec *s, const char *accept, Loc l[], int sz)
{
	int n = 0;
	Loc l0 = indloc(s, strcspn(s->blks, strttiles));

	for (int i = 0; i < strlen(s->blks); i++) {
		if (strchr(accept, s->blks[i]) != NULL) {
			if (n >= sz)
				fatal("offsets: array is too small\n");
			Loc cur =  indloc(s, i);
			l[n] = (Loc) { cur.x - l0.x, cur.y - l0.y, cur.z - l0.z };
			n++;
		}
	}

	return n;
}

void mvblit(Mv *mv, Lvl *lvl, Loc l0)
{
	char *blks = mv->spec->blks;
	Loc s = mv->strt;

	for (int i = 0; i < strlen(blks); i++) {
		char t = blks[i];
		Loc cur = indloc(mv->spec, i);
		Loc loc = (Loc) {
			cur.x - s.x + l0.x,
			cur.y - s.y + l0.y,
			cur.z - s.z + l0.z,
		};
		if (reachable(lvl, loc.x, loc.y, loc.z) && strchr(doortiles, t) == NULL)
			continue;

		bool clr = strchr(clrtiles, t) != NULL;
		if (clr)
			setreach(lvl, loc.x, loc.y, loc.z);

		Blk *b = blk(lvl, loc.x, loc.y, loc.z);
		if (strchr(blkdtiles, t) != NULL)
			b->tile = t;
		else if (strchr(doortiles, t) != NULL)
			blitdoor(lvl, loc, t);
	}
}

static void blitdoor(Lvl *lvl, Loc l, int door)
{
	if (tileinfo(lvl, l.x, l.y, l.z).flags & Twater) {
		if (door == '<')
			door = '(';
		else if (door == '>')
			door = ')';
	}
	blk(lvl, l.x, l.y, l.z)->tile = door;
}

static Loc indloc(Mvspec *s, int i)
{
	int z = i / (s->w * s->h);
	i %= s->w * s->h;
	int x = i % s->w;
	int y = i / s->w;
	return (Loc) { x, y, z };
}

bool startonblk(Mv *mv)
{
	for (int i = 0; i < mv->nblkd; i++)
		if (mv->blkd[i].x == 0 && mv->blkd[i].y == 1)
			return true;
	return false;
}
