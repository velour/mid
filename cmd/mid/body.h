enum { Tall = 32, Wide = 32 };

enum { Maxdy = 12 };

extern const float Grav;

typedef enum Act Act;
enum Act { Stand, Walk, Jump,
	   Nacts};

typedef struct Dir Dir;
struct Dir {
	Anim *anim[Nacts];
	Rect bbox[Nacts];
};

typedef struct Body Body;
struct Body {
	Dir left, right;
	Dir *curdir;
	Act curact;
	Point vel, imgloc;
	int z;
	float ddy;
	_Bool fall;
};

_Bool bodyinit(Body *, const char *name, int x, int y, int z);
void bodyfree(Body *b);
void bodydraw(Gfx *g, Body *b, int z, Point tr);

/* If transl is non-NULL then this body will scroll the screen. */
void bodyupdate(Body *b, Lvl *l, int z, Point *transl);
