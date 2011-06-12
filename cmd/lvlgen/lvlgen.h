typedef struct Loc Loc;
struct Loc {
	int x, y;
};

typedef struct Mvspec Mvspec;
struct Mvspec {
	int wt, w, h;
	char *blks;
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
	int nblkd;
	Loc blkd[Maxblks];

	Mvspec spec;
};

Move *moves;
int Nmoves;
void movesini(void);

typedef struct Seg Seg;
struct Seg {
	Loc l0, l1;;
	Move *mv;
};

Seg segmk(Loc l, Move *m);
void segpr(Seg s);

enum { Maxsegs = 1024 };

typedef struct Path Path;
struct Path {
	int n;
	Seg ss[Maxsegs];
	_Bool used[];
};

struct Lvl;

Path *pathnew(struct Lvl *l);
void pathfree(Path *p);
_Bool pathadd(struct Lvl *l, Path *p, Seg s);
void pathpr(struct Lvl *l, Path *p);
