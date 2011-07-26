// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include "../../include/mid.h"
#include "../../include/log.h"
#include "lvlgen.h"

static Mvspec *specrev(Mvspec *s);
static Mvspec *swapdoor(Mvspec *s);
static int addmv(Mvspec *, Mv moves[]);
static Mv mvmk(Mvspec *);
static int offsets(Mvspec *, const char *accept, Loc l[], int sz);
static void doormv(Mv *mv);
static void blittile(Lvl *lvl, Loc l0, Loc loc, char t);
static char otherdoor(char c);
static Loc indloc(Mvspec *, int);

static const char *strttiles = "s";
static const char *endtiles = "e<>";
static const char *doortiles = "<>";
static const char *clrtiles = " es";
static const char *blkdtiles = "#";

/* Move specification array: any character in strttiles is the start of
 the move, any character in endtiles is the end of the move, anything in
 blkdtiles is a collidable block, any character in clrtiles must be clear
 and is considered reachable.  Anything else is a wild card (whatever
 ends up being put there while building the path).  If .revable is true
 then both the inital version and the version with the start and end
 swapped are considered.

 If the end tile is a door ('<' or '>') then moves are created for both
 front and back doors. */
static Mvspec specs[] = {
	{ .blks =
		"####"
		"s >#"
		"####",
	  .wt = 5, .w = 4, .h = 3, },

	{ .blks =
		"####"
		"#< s"
		"####",
	  .wt = 5, .w = 4, .h = 3, },

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
		int mul = 1;
		if (specs[i].revable)
			mul *= 2;
		if (hasdoor(specs+i))
			mul *= 2;
		Nmoves += specs[i].wt * mul;
	}

	Mv *m;
	moves = m = xalloc(Nmoves, sizeof(*moves));

	for (int i = 0; i < Nspecs; i++) {
		m += addmv(specs+i, m);
		if (specs[i].revable)
			m += addmv(specrev(specs+i), m);

		if (!hasdoor(specs+i))
			continue;

		Mvspec *s = swapdoor(specs+i);
		m += addmv(s, m);
		if (s->revable)
			m += addmv(specrev(s), m);
	}

	assert(m - moves == Nmoves);
}

bool hasdoor(Mvspec *s)
{
	return strcspn(s->blks, doortiles) != strlen(s->blks);
}

Loc dooroffs(Mvspec *spec)
{
	Loc d = indloc(spec, strcspn(spec->blks, doortiles));
	Loc s = indloc(spec, strcspn(spec->blks, strttiles));
	return (Loc) { d.x - s.x, d.y - s.y, 0 };
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

static Mvspec *swapdoor(Mvspec *s)
{
	Mvspec *swp = xalloc(1, sizeof(*swp));
	*swp = *s;
	swp->blks = xalloc(strlen(s->blks)+1, sizeof(*s->blks));
	strcpy(swp->blks, s->blks);

	int ei = strcspn(swp->blks, endtiles);
	swp->blks[ei] = otherdoor(swp->blks[ei]);

	return swp;
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
		.dz = 0,
		.spec = spec,
	};
	mv.nclr = offsets(spec, clrtiles, mv.clr, Maxblks);
	mv.nblkd = offsets(spec, blkdtiles, mv.blkd, Maxblks);

	if (hasdoor(spec))
		doormv(&mv);

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
			l[n] = (Loc) { cur.x - l0.x, cur.y - l0.y, 0 };
			n++;
		}
	}

	return n;
}

static void doormv(Mv *mv)
{
	if (mv->nclr > Maxblks/2)
		fatal("doormv: clr array is too small");
	if (mv->nblkd > Maxblks/2)
		fatal("doormv: blkd array is too small");

	int ei = strcspn(mv->spec->blks, endtiles);
	mv->dz += mv->spec->blks[ei] == '<' ? -1 : 1;
	mv->dy = mv->dx = 0;

	for (int i = 0; i < mv->nclr; i++) {
		mv->clr[mv->nclr+i] = mv->clr[i];
		mv->clr[mv->nclr+i].z += mv->dz;
	}
	mv->nclr *= 2;

	for (int i = 0; i < mv->nblkd; i++) {
		mv->blkd[mv->nblkd+i] = mv->blkd[i];
		mv->blkd[mv->nblkd+i].z += mv->dz;
	}
	mv->nblkd *= 2;
}

void mvblit(Mv *mv, Lvl *lvl, Loc l0)
{
	char *blks = mv->spec->blks;
	Loc s = mv->strt;

	for (int i = 0; i < strlen(blks); i++) {
		char t = blks[i];
		Loc cur = indloc(mv->spec, i);
		Loc loc = (Loc) { cur.x - s.x + l0.x, cur.y - s.y + l0.y, l0.z };

		if (reachable(lvl, loc.x, loc.y, loc.z) && strchr(doortiles, t) == NULL)
			continue;

		blittile(lvl, l0, loc, t);
		if (hasdoor(mv->spec)) {
			loc.z += mv->dz;
			blittile(lvl, l0, loc, t);
		}
	}
}

static void blittile(Lvl *lvl, Loc l0, Loc loc, char t)
{
	if (strchr(blkdtiles, t) != NULL) {
		blk(lvl, loc.x, loc.y, loc.z)->tile = t;
	} else if (strchr(doortiles, t) != NULL) {
		if (l0.z != loc.z)
			t = otherdoor(t);
		assert(blk(lvl, loc.x, loc.y, loc.z)->tile == ' ');
		blk(lvl, loc.x, loc.y, loc.z)->tile = t;
		setreach(lvl, loc.x, loc.y, loc.z);
	} else if (strchr(clrtiles, t) != NULL) {
		blk(lvl, loc.x, loc.y, loc.z)->tile = ' ';
		setreach(lvl, loc.x, loc.y, loc.z);
	}
}

static char otherdoor(char c)
{
	assert (strchr(doortiles, c) != NULL);
	return c == '<' ? '>' : '<';
}

static Loc indloc(Mvspec *s, int i)
{
	return (Loc) { i % s->w, i / s->w, 0 };
}

bool startonblk(Mv *mv)
{
	for (int i = 0; i < mv->nblkd; i++)
		if (mv->blkd[i].x == 0 && mv->blkd[i].y == 1)
			return true;
	return false;
}
