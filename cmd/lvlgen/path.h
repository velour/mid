typedef struct Loc Loc;
struct Loc {
	int x, y;
};

enum { Maxarea = 10, Maxblks = 10 };

typedef struct Move Move;
struct Move {
	int dx, dy;

	/* Array of locs that must be 'clear' */
	int nclr;
	Loc clr[Maxarea];

	/* Array of blocks. */
	int nblks;
	Loc blks[Maxblks];
};

extern const Move moves[];
extern const int Nmoves;

typedef struct Seg Seg;
struct Seg {
	Loc l0, l1;;
	const Move *mv;
};

enum { Maxsegs = 50 };

typedef struct Path Path;
struct Path {
	int n;
	Seg ss[Maxsegs];
	bool used[];
};

Path *pathnew(Lvl *l);
void pathfree(Path *p);
_Bool pathadd(Lvl *l, Path *p, Seg s);
Seg segmk(Loc l, const Move *m);
void segpr(Seg s);
void pathpr(Lvl *l, Path *p);
