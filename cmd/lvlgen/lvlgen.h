struct Blk;

struct Blk *blk(Lvl *l, int x, int y, int z);

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
void mvblit(Mv *mv, Lvl *l, Loc l0);

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
	_Bool *used;
};

struct Lvl;

Path *pathnew(struct Lvl *l);
void pathfree(Path *p);
_Bool pathadd(struct Lvl *l, Path *p, Seg s);
void pathpr(struct Lvl *l, Path *p);

Loc doorloc(Lvl *lvl, Path *p, Loc loc);
