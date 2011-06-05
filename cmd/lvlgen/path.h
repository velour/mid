typedef struct Move Move;
struct Move {
	int dx, dy;
};

extern const Move moves[];
extern const int Nmoves;

typedef struct Loc Loc;
struct Loc {
	int x, y;
};

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
