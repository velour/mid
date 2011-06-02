enum { Scrnw = 512, Scrnh = 512 };

/* Buffer from side of screen at which to begin scrolling. */
enum { Scrlbuf = 192 };

enum { Tall = 32, Wide = 32 };

enum { Maxdy = 12 };

extern const float Grav;

typedef enum Act Act;
enum Act {
	Stand,
	Walk,
	Jump,
	Nacts
};

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

enum Action{
	Mvleft,
	Mvright,
	Mvbak,
	Mvfwd,
	Mvjump,
	Mvinv,
	Nactions,
};

_Bool keymapread(char km[Nactions], char *fname);
extern char kmap[Nactions];

typedef struct Game Game;
Game *gamenew(void);
void gamefree(Scrn *);
void gameupdate(Scrn *, Scrnstk *);
void gamedraw(Scrn *, Gfx *);
void gamehandle(Scrn *, Scrnstk *, Event *);
extern Scrnmt gamemt;

typedef struct Player Player;
Player *playernew(int x, int y);
void playerfree(Player *);
void playerupdate(Player *, Lvl *l, int *z, Point *tr);
void playerdraw(Gfx *, Player *, Point tr);
void playerhandle(Player *, Event *);
Point playerpos(Player *);
Rect playerbox(Player *);

enum { Invrows = 3, Invcols = 3 };

typedef struct Item Item;
struct Item {
	Anim *icon;
	const char *name;
};

typedef struct Inv Inv;
struct Inv {
	int money;
	Item *items[Invrows * Invcols];
};

Item *itemnew(const char *name, const char *anim);
void itemfree(Item *);

/* Swap the item at the given inventory grid x,y.  Return the old item. */
Item *invmod(Inv *, Item *, int x, int y);

Scrn *invscrnnew(Inv *, Lvl *, Point ppos, int z);
Scrn *titlescrnnew(Gfx *);

typedef struct Enemy Enemy;
typedef struct Enemymt Enemymt;

struct Enemymt{
	void (*free)(Enemy*);
	void (*update)(Enemy*, Player*, Lvl*);
	void (*draw)(Enemy*, Gfx*, Point tr);
};

struct Enemy{
	Enemymt *mt;
	void *data;
};

Enemy *enemynew(unsigned char id, Point loc);
