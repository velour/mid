// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
struct Blk;
struct Lvl;
struct Rng;

unsigned int rnd(int min, int max);

typedef struct Loc Loc;
struct Loc {
	int x, y;
};

typedef struct Mvspec Mvspec;
struct Mvspec {
	int wt, w, h;
	_Bool revable;
	char *blks;
};

enum { Maxblks = 64 };

typedef struct Mv Mv;
struct Mv {
	int wt;
	int dx, dy;

	/* Array of locs that must be 'clear' */
	int nclr;
	Loc clr[Maxblks];

	/* Array of blocks. */
	int nblkd;
	Loc blkd[Maxblks];

	Mvspec spec;
};

Mv *moves;
int Nmoves;
void mvini(void);
void mvblit(Mv *mv, struct Lvl *l, Loc l0);
_Bool startonblk(Mv *mv);

typedef struct Seg Seg;
struct Seg {
	Loc l0, l1;;
	Mv *mv;
};

Seg segmk(Loc l, Mv *m);
void segpr(Seg s);

typedef struct Path Path;
struct Path {
	int maxsegs, nsegs;
	Seg *segs;
};

Path *pathnew(struct Lvl *);
void pathfree(Path *);
_Bool pathadd(struct Lvl *, Path *, Seg);
void pathpr(struct Lvl *, Path *);

_Bool reachable(struct Lvl *, int, int, int);
void setreach(struct Lvl *, int, int, int);

void putdoor(struct Lvl *lvl, int x, int y, int z, int door);
Loc doorloc(struct Lvl *, Path *, Loc);
void extradoors(struct Rng *, struct Lvl *);

void water(struct Lvl *);

void morereach(struct Lvl *);
void closeunreach(struct Lvl *lvl);
