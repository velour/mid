struct Blk;
struct Lvl;
struct Rng;

unsigned int rnd(int min, int max);

typedef struct Loc Loc;
struct Loc {
	int x, y, z;
};

typedef struct Mvspec Mvspec;
struct Mvspec {
	int wt;
	int w, h;
	char flgs;
	char *blks;
};

enum {
	Mvrev = 1 << 0,
	Mvwtr = 1 << 1,
};

enum { Maxblks = 64, Maxdrs = 2 };

typedef struct Mv Mv;
struct Mv {
	int wt;
	int dx, dy, dz;
	Loc strt;

	Loc clr[Maxblks];
	Loc blkd[Maxblks];
	Loc door[Maxdrs];
	int nclr, nblkd, ndoor;

	Mvspec *spec;
};

extern Mv *moves;
extern int nmoves;
extern Mv *wtrmvs;
extern int nwtrmvs;

void mvsinit(void);
void mvblit(Mv *mv, struct Lvl *l, Loc l0);
_Bool startonblk(Mv *mv);

typedef struct Seg Seg;
struct Seg {
	Loc l0, l1;;
	Mv *mv;
};

typedef struct Path Path;
struct Path {
	int maxsegs, nsegs;
	Seg *segs;
};

Path *pathnew(struct Lvl *);
void pathfree(Path *);
void pathbuild(struct Lvl *lvl, Path *, Loc);
void pathpr(struct Lvl *, Path *);

_Bool reachable(struct Lvl *, int, int, int);
void setreach(struct Lvl *, int, int, int);


void water(struct Lvl *);

void morereach(struct Lvl *);
void closeunits(struct Lvl *);
int closeunreach(struct Lvl *);	// returns count of reachable blocks
