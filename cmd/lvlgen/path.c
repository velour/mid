#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "../../include/mid.h"
#include "../../include/log.h"
#include "path.h"

static bool segok(Lvl *l, Path *p, Seg s);
static bool segconfl(Lvl *l, Path *p, Seg s);
static bool segclr(Lvl *l, Seg s);
static bool beenthere(Path *p, Seg s);
static int segarea(Seg s, Loc l[], int sz);
static int segblks(Seg s, Loc l[], int sz);

/* Moves that are pieced together to build a path throughout the
 * level.  The comment above the move describes it: s is the start
 * loc, # is a block and c is a location that must be cleared to make
 * the move. */
const Move moves[] = {
	/* sc
           ## */
	{ .dx = 1, .dy = 0,
	  .nclr = 2,
	  .clr = { {0, 0}, {1, 0} },
	  .nblks = 2,
	  .blks = { {0, 1}, {1, 1} } },

	/* cs
           ## */
	{ .dx = -1, .dy = 0,
	  .nclr = 2,
	  .clr = { {0, 0}, {-1, 0} },
	  .nblks = 2,
	  .blks = { {0, 1}, {-1, 1} } },

	/* scc
           ### */
	{ .dx = 2, .dy = 0,
	  .nclr = 3,
	  .clr = { {0, 0}, {1, 0}, {2, 0} },
	  .nblks = 3,
	  .blks = { {0, 1}, {1, 1}, {2, 1} } },

	/* ccs
           ### */
	{ .dx = -2, .dy = 0,
	  .nclr = 3,
	  .clr = { {0, 0}, {-1, 0}, {-2, 0} },
	  .nblks = 3,
	  .blks = { {0, 1}, {-1, 1}, {-2, 1} } },

	/* ccccc
           scccc
           #   # */
	{ .dx = 4, .dy = 0,
	  .nclr = 10,
	  .clr = { {0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0},
		   {0, -1}, {1, -1}, {2, -1}, {3, -1}, {4, -1}, },
	  .nblks = 2,
	  .blks = { {0, 1}, {4, 1} } },

	/* ccccc
           ccccs
           #   # */
	{ .dx = -4, .dy = 0,
	  .nclr = 10,
	  .clr = { {0, 0}, {-1, 0}, {-2, 0}, {-3, 0}, {-4, 0},
		   {0, -1}, {-1, -1}, {-2, -1}, {-3, -1}, {-4, -1}, },
	  .nblks = 2, .blks = { {0, 1}, {-4, 1} } },

	/* cc
	   s#
	   # */
	{ .dx = 1, .dy = -1,
	  .nclr = 3, .clr = { {0, 0}, {0, -1}, {1, -1} },
	  .nblks = 2, .blks = { {0, 1}, {1, 0} } },

	/* cs
	   c#
	   # */
	{ .dx = -1, .dy = 1,
	  .nclr = 3,
	  .clr = { {0, 0}, {-1, 0}, {-1, 1} },
	  .nblks = 2,
	  .blks = { {0, 1}, {-1, 2} } },

	/* sc
	   #c
	    # */
	{ .dx = 1, .dy = 1,
	  .nclr = 3,
	  .clr = { {0, 0}, {1, 0}, {1, 1} },
	  .nblks = 2,
	  .blks = { {0, 1}, {1, 2} } },

	/* cc
	   #s
	    # */
	{ .dx = -1, .dy = -1,
	  .nclr = 3,
	  .clr = { {0, 0}, {0, -1}, {-1, -1} },
	  .nblks = 2,
	  .blks = { {0, 1}, {-1, 0} } },

	/* ccc
           cc#
           s
           # */
	{ .dx = 2, .dy = -2,
	  .nclr = 6,
	  .clr = { {0, 0}, {0, -1}, {0, -2}, {1, -1}, {1, -2}, {2, -2} },
	  .nblks = 2, .blks = { {0, 1}, {2, -1} } },

	/* ccs
           c #
           c
           # */
	{ .dx = -2, .dy = 2,
	  .nclr = 5,
	  .clr = { {0, 0}, {-1, 0}, {-2, 0}, {-2, 1}, {-2, 2} },
	  .nblks = 2,
	  .blks = { {0, 1}, {-2, 3} } },

	/* ccc
           #cc
             s
             # */
	{ .dx = -2, .dy = -2,
	  .nclr = 6,
	  .clr = { {0, 0}, {0, -1}, {0, -2}, {-1, -1}, {-1, -2}, {-2, -2} },
	  .nblks = 2,
	  .blks = { {0, 1}, {-2, -1} } },

	/* scc
           # c
             c
             # */
	{ .dx = 2, .dy = 2,
	  .nclr = 5,
	  .clr = { {0, 0}, {1, 0}, {2, 0}, {2, 1}, {2, 2} },
	  .nblks = 2,
	  .blks = { {0, 1}, {2, 3} } },

	/* scc
           # c
             c
             c
             # */
	{ .dx = 2, .dy = 3,
	  .nclr = 6,
	  .clr = { {0, 0}, {1, 0}, {2, 0}, {2, 1}, {2, 2}, {2, 3} },
	  .nblks = 2,
	  .blks = { {0, 1}, {2, 4} } },
	/* ccs
           c #
           c
           c
           # */
	{ .dx = -2, .dy = 3,
	  .nclr = 6,
	  .clr = { {0, 0}, {-1, 0}, {-2, 0}, {-2, 1}, {-2, 2}, {-2, 3} },
	  .nblks = 2,
	  .blks = { {0, 1}, {-2, 4} } },
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
	if (p->n == Maxsegs - 1 || !segok(l, p, s))
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

	return inrange && segclr(l, s) && !segconfl(l, p, s)
		&& !beenthere(p, s);
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

/* Test if the segment area is clr. */
static bool segclr(Lvl *l, Seg s)
{
	Loc blks[Bufsz];
	int n = segarea(s, blks, Bufsz);

	for (int i = 0; i < n; i++) {
		Blkinfo b = blkinfo(l, blks[i].x, blks[i].y);
		if (b.flags & Tilecollide)
			return false;
	}

	return true;
}

static bool beenthere(Path *p, Seg s)
{
	for (int i = 0; i < p->n; i++) {
		if (p->ss[i].l0.x == s.l1.x && p->ss[i].l0.y == s.l1.y)
			return true;
	}
	return false;
}

/* Fill 'l' with the location of level blocks that must be clr for
 * this segment. */
static int segarea(Seg s, Loc l[], int sz)
{
	const Move *m = s.mv;
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
	const Move *m = s.mv;
	if (sz < m->nblks)
		fatal("Blk buffer is too small");
	for (int i = 0; i < m->nblks; i++)
		l[i] = (Loc) {s.l0.x + m->blks[i].x, s.l0.y + m->blks[i].y};
	return m->nblks;
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
			fputc(c, stdout);
		}
		fputc('\n', stdout);
	}
	fputc('\n', stdout);
}
