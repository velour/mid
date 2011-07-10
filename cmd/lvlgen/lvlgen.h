struct Blk;
struct Lvl;

unsigned int rnd(int min, int max);

typedef struct Loc Loc;
struct Loc {
	int x, y;
};

typedef struct Mvspec Mvspec;
struct Mvspec {
	int wt, w, h;
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

Path *pathnew(struct Lvl *l);
void pathfree(Path *p);
_Bool pathadd(struct Lvl *l, Path *p, Seg s);
void pathpr(struct Lvl *l, Path *p);

_Bool used(struct Lvl *l, Loc loc);

Loc doorloc(struct Lvl *lvl, Path *p, Loc loc);

void water(struct Lvl *lvl);

void morereach(struct Lvl *lvl);