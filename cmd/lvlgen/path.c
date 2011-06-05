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
static bool beenthere(Lvl *l, Path *p, Seg s);
static int segarea(Seg s, Loc l[], int sz);
static int segblks(Seg s, Loc l[], int sz);

/* Moves that are pieced together to build a path throughout the
 * level.  The comment above the move describes it: s is the start
 * loc, # is a block and c is a location that must be cleared to make
 * the move. */
static Move allmoves[] = {
	/* sc
           ## */
	{ .wt = 1,
	  .dx = 1, .dy = 0,
	  .nclr = 2,
	  .clr = { {0, 0}, {1, 0} },
	  .nblks = 2,
	  .blks = { {0, 1}, {1, 1} } },

	/* cs
           ## */
	{ .wt = 1,
	  .dx = -1, .dy = 0,
	  .nclr = 2,
	  .clr = { {0, 0}, {-1, 0} },
	  .nblks = 2,
	  .blks = { {0, 1}, {-1, 1} } },

	/* scc
           ### */
	{ .wt = 2,
	  .dx = 2, .dy = 0,
	  .nclr = 3,
	  .clr = { {0, 0}, {1, 0}, {2, 0} },
	  .nblks = 3,
	  .blks = { {0, 1}, {1, 1}, {2, 1} } },

	/* ccs
           ### */
	{ .wt = 2,
	  .dx = -2, .dy = 0,
	  .nclr = 3,
	  .clr = { {0, 0}, {-1, 0}, {-2, 0} },
	  .nblks = 3,
	  .blks = { {0, 1}, {-1, 1}, {-2, 1} } },

	/* sccc
           #### */
	{ .wt = 2,
	  .dx = 3, .dy = 0,
	  .nclr = 4,
	  .clr = { {0, 0}, {1, 0}, {2, 0}, {3, 0} },
	  .nblks = 4,
	  .blks = { {0, 1}, {1, 1}, {2, 1}, {3, 1} } },

	/* cccs
           #### */
	{ .wt = 2,
	  .dx = -3, .dy = 0,
	  .nclr = 3,
	  .clr = { {0, 0}, {-1, 0}, {-2, 0}, {-3, 0} },
	  .nblks = 3,
	  .blks = { {0, 1}, {-1, 1}, {-2, 1}, {-3, 1} } },

	/* ccccc
           scccc
           #   # */
	{ .wt = 1,
	  .dx = 4, .dy = 0,
	  .nclr = 10,
	  .clr = { {0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0},
		   {0, -1}, {1, -1}, {2, -1}, {3, -1}, {4, -1}, },
	  .nblks = 2,
	  .blks = { {0, 1}, {4, 1} } },

	/* ccccc
           ccccs
           #   # */
	{ .wt = 1,
	  .dx = -4, .dy = 0,
	  .nclr = 10,
	  .clr = { {0, 0}, {-1, 0}, {-2, 0}, {-3, 0}, {-4, 0},
		   {0, -1}, {-1, -1}, {-2, -1}, {-3, -1}, {-4, -1}, },
	  .nblks = 2, .blks = { {0, 1}, {-4, 1} } },

	/* cc
	   s#
	   # */
	{ .wt = 1,
	  .dx = 1, .dy = -1,
	  .nclr = 3, .clr = { {0, 0}, {0, -1}, {1, -1} },
	  .nblks = 2, .blks = { {0, 1}, {1, 0} } },

	/* cc
	   s#
	   ##*/
	{ .wt = 1,
	  .dx = 1, .dy = -1,
	  .nclr = 3, .clr = { {0, 0}, {0, -1}, {1, -1} },
	  .nblks = 3, .blks = { {0, 1}, {1, 1}, {1, 0} } },

	/* cs
	   c#
	   # */
	{ .wt = 1,
	  .dx = -1, .dy = 1,
	  .nclr = 3,
	  .clr = { {0, 0}, {-1, 0}, {-1, 1} },
	  .nblks = 2,
	  .blks = { {0, 1}, {-1, 2} } },

	/* cs
	   c#
	   ##*/
	{ .wt = 2,
	  .dx = -1, .dy = 1,
	  .nclr = 3,
	  .clr = { {0, 0}, {-1, 0}, {-1, 1} },
	  .nblks = 3,
	  .blks = { {0, 1}, {0, 2}, {-1, 2} } },

	/* sc
	   #c
	    # */
	{ .wt = 1,
	  .dx = 1, .dy = 1,
	  .nclr = 3,
	  .clr = { {0, 0}, {1, 0}, {1, 1} },
	  .nblks = 2,
	  .blks = { {0, 1}, {1, 2} } },

	/* sc
	   #c
	   ## */
	{ .wt = 1,
	  .dx = 1, .dy = 1,
	  .nclr = 3,
	  .clr = { {0, 0}, {1, 0}, {1, 1} },
	  .nblks = 3,
	  .blks = { {0, 1}, {0, 2}, {1, 2} } },

	/* cc
	   #s
	    # */
	{ .wt = 1,
	  .dx = -1, .dy = -1,
	  .nclr = 3,
	  .clr = { {0, 0}, {0, -1}, {-1, -1} },
	  .nblks = 2,
	  .blks = { {0, 1}, {-1, 0} } },

	/* cc
	   #s
	   ## */
	{ .wt = 2,
	  .dx = -1, .dy = -1,
	  .nclr = 3,
	  .clr = { {0, 0}, {0, -1}, {-1, -1} },
	  .nblks = 3,
	  .blks = { {0, 1}, {-1, 1}, {-1, 0} } },

	/* ccc
           cc#
           s
           # */
	{ .wt = 1,
	  .dx = 2, .dy = -2,
	  .nclr = 6,
	  .clr = { {0, 0}, {0, -1}, {0, -2}, {1, -1}, {1, -2}, {2, -2} },
	  .nblks = 2, .blks = { {0, 1}, {2, -1} } },

	/* ccc
           cc#
           s##
           ### */
	{ .wt = 2,
	  .dx = 2, .dy = -2,
	  .nclr = 6,
	  .clr = { {0, 0}, {0, -1}, {0, -2}, {1, -1}, {1, -2}, {2, -2} },
	  .nblks = 6,
	  .blks = { {0, 1}, {1, 1}, {2, 1}, {1, 0}, {2, 0}, {2, -1} } },

	/* ccs
           c #
           c
           # */
	{ .wt = 1,
	  .dx = -2, .dy = 2,
	  .nclr = 5,
	  .clr = { {0, 0}, {-1, 0}, {-2, 0}, {-2, 1}, {-2, 2} },
	  .nblks = 2,
	  .blks = { {0, 1}, {-2, 3} } },

	/* ccs
           c #
           c##
           ### */
	{ .wt = 2,
	  .dx = -2, .dy = 2,
	  .nclr = 5,
	  .clr = { {0, 0}, {-1, 0}, {-2, 0}, {-2, 1}, {-2, 2} },
	  .nblks = 6,
	  .blks = { {0, 1}, {0, 2}, {0, 3}, {-1, 2}, {-1, 3}, {-2, 3} } },

	/* ccc
           #cc
             s
             # */
	{ .wt = 1,
	  .dx = -2, .dy = -2,
	  .nclr = 6,
	  .clr = { {0, 0}, {0, -1}, {0, -2}, {-1, -1}, {-1, -2}, {-2, -2} },
	  .nblks = 2,
	  .blks = { {0, 1}, {-2, -1} } },

	/* ccc
           #cc
           ##s
           ### */
	{ .wt = 2,
	  .dx = -2, .dy = -2,
	  .nclr = 6,
	  .clr = { {0, 0}, {0, -1}, {0, -2}, {-1, -1}, {-1, -2}, {-2, -2} },
	  .nblks = 6,
	  .blks = { {0, 1}, {-1, 1}, {-2, 1}, {-1, 0}, {-2, 0}, {-2, -1} } },

	/* scc
           # c
             c
             # */
	{ .wt = 1,
	  .dx = 2, .dy = 2,
	  .nclr = 5,
	  .clr = { {0, 0}, {1, 0}, {2, 0}, {2, 1}, {2, 2} },
	  .nblks = 2,
	  .blks = { {0, 1}, {2, 3} } },

	/* scc
           # c
           ##c
           ### */
	{ .wt = 2,
	  .dx = 2, .dy = 2,
	  .nclr = 5,
	  .clr = { {0, 0}, {1, 0}, {2, 0}, {2, 1}, {2, 2} },
	  .nblks = 6,
	  .blks = { {0, 1}, {0, 2}, {0, 3}, {1, 2}, {1, 3}, {2, 3} } },

	/* scc
           # c
             c
             c
             # */
	{ .wt = 1,
	  .dx = 2, .dy = 3,
	  .nclr = 6,
	  .clr = { {0, 0}, {1, 0}, {2, 0}, {2, 1}, {2, 2}, {2, 3} },
	  .nblks = 2,
	  .blks = { {0, 1}, {2, 4} } },

	/* ccs
           c #
           c
           c
           # */
	{ .wt = 1,
	  .dx = -2, .dy = 3,
	  .nclr = 6,
	  .clr = { {0, 0}, {-1, 0}, {-2, 0}, {-2, 1}, {-2, 2}, {-2, 3} },
	  .nblks = 2,
	  .blks = { {0, 1}, {-2, 4} } },

	/* sc
           #c
            c
            c
            # */
	{ .wt = 1,
	  .dx = 1, .dy = 3,
	  .nclr = 5,
	  .clr = { {0, 0}, {1, 0}, {1, 1}, {1, 2}, {1, 3} },
	  .nblks = 2,
	  .blks = { {0, 1}, {1, 4} } },

	/* cs
           c#
           c
           c
           # */
	{ .wt = 1,
	  .dx = -1, .dy = 3,
	  .nclr = 5,
	  .clr = { {0, 0}, {-1, 0}, {-1, 1}, {-1, 2}, {-1, 3} },
	  .nblks = 2,
	  .blks = { {0, 1}, {-1, 4} } },

	/* sc
           #c
           #c
           #c
           ## */
	{ .wt = 2,
	  .dx = 1, .dy = 3,
	  .nclr = 5,
	  .clr = { {0, 0}, {1, 0}, {1, 1}, {1, 2}, {1, 3} },
	  .nblks = 3,
	  .blks = { {0, 1}, {0, 2}, {0, 3}, {0, 4}, {1, 4} } },

	/* cs
           c#
           c#
           c#
           ## */
	{ .wt = 2,
	  .dx = -1, .dy = 3,
	  .nclr = 5,
	  .clr = { {0, 0}, {-1, 0}, {-1, 1}, {-1, 2}, {-1, 3} },
	  .nblks = 5,
	  .blks = { {0, 1}, {0, 2}, {0, 3}, {0, 4}, {-1, 4} } },
};

static const int Nallmoves = sizeof(allmoves) / sizeof(allmoves[0]);

Move **moves;
int Nmoves;

void mvinit(void)
{
	for (int i = 0; i < Nallmoves; i++)
		Nmoves += allmoves[i].wt;
	moves = calloc(Nmoves, sizeof(*moves));
	if (!moves)
		fatal("Failed to allocate move array");
	int nxt = 0;
	for (int i = 0; i < Nallmoves; i++) {
		for (int j = 0; j < allmoves[i].wt; j++) {
			moves[nxt] = &allmoves[i];
			nxt++;
		}
	}
	assert(nxt == Nmoves);
}

enum { Bufsz = 10 };

Seg segmk(Loc l, Move *m)
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
		&& !beenthere(l, p, s);
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
