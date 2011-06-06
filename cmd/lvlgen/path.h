typedef struct Loc Loc;
struct Loc {
	int x, y;
};

enum { Maxblks = 15 };

typedef struct Move Move;
struct Move {
	int wt;

	int dx, dy;

	/* Array of locs that must be 'clear' */
	int nclr;
	Loc clr[Maxblks];

	/* Array of blocks. */
	int nblks;
	Loc blks[Maxblks];
};

extern Move **moves;
extern int Nmoves;

typedef struct Seg Seg;
struct Seg {
	Loc l0, l1;;
	Move *mv;
};

enum { Maxsegs = 1024 };

typedef struct Path Path;
struct Path {
	int n;
	Seg ss[Maxsegs];
	bool used[];
};

void mvinit(void);
Path *pathnew(Lvl *l);
void pathfree(Path *p);
_Bool pathadd(Lvl *l, Path *p, Seg s);
Seg segmk(Loc l, Move *m);
void segpr(Seg s);
void pathpr(Lvl *l, Path *p);
