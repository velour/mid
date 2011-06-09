/* Mean frame time.  May be useful for comparing computation effort. */
extern double meanftime;
extern int debugging;

const char *miderrstr(void);
void seterrstr(const char *fmt, ...);

typedef struct Point Point;
struct Point{
	double x, y;
};

typedef struct Line1d Line1d;
struct Line1d{
	double a, b;
};

/*
Returns the absolute value of the intersection of two lines.
Returns a negative number if the lines don't intersect.
*/
double isection1d(Line1d, Line1d);

typedef struct Rect Rect;
struct Rect{
	Point a, b;
};

Line1d rectprojx(Rect);
Line1d rectprojy(Rect);
void rectmv(Rect *, double dx, double dy);
/* Makes point a the min,min and point b the max,max. */
Rect rectnorm(Rect r);
void ptmv(Point *, double dx, double dy);

typedef struct Isect Isect;
struct Isect{
	_Bool is;
	double dx, dy; /* isection1d of x and y projs */
};

Isect isection(Rect, Rect);
int isect(Rect, Rect);
double isectarea(Isect is);

/*
The minimal intersection will be positive, and the maximum intersection will
be negative. If both intersections are equal, both dx and dy will be positive.
*/
Isect minisect(Rect, Rect);


typedef struct Color Color;
struct Color{
	unsigned char r, g, b, a;
};

typedef struct Gfx Gfx;

Gfx *gfxinit(int w, int h, const char *title);
void gfxfree(Gfx *);
Point gfxdims(const Gfx *);
void gfxflip(Gfx *);
void gfxclear(Gfx *, Color);
void gfxdrawpoint(Gfx *, Point, Color);
void gfxfillrect(Gfx *, Rect, Color);
void gfxdrawrect(Gfx *, Rect, Color);

typedef struct Img Img;

Img *imgnew(const char *path);
void imgfree(Img *);
/* Returns negative dimensions on failure. */
Point imgdims(const Img *);
void imgdraw(Gfx *, Img *, Point);

typedef struct Txt Txt;

Txt *txtnew(const char *font, int sz, Color);
void txtfree(Txt *);
Point txtdims(const Txt *, const char *fmt, ...);
Img *txt2img(Gfx *, Txt *, const char *fmt, ...);
// Prefer txt2img to this for static text
Point txtdraw(Gfx *, Txt *, Point, const char *fmt, ...);

_Bool sndinit(void);
void sndfree(void);

typedef struct Music Music;

Music *musicnew(const char *);
void musicfree(Music *);
void musicstart(Music *, int fadein);
void musicstop(int fadeout);
void musicpause(void);
void musicresume(void);

typedef struct Sfx Sfx;

Sfx *sfxnew(const char *);
void sfxfree(Sfx *);
void sfxplay(Sfx *);

enum Eventty{
	Quit,
	Keychng,
	Mousemv,
	Mousebt,
};

enum{
	Mouse1 = 1,
	Mouse2 = Mouse1 << 1,
	Mouse3 = Mouse2 << 1,
};

typedef struct Event Event;
struct Event{
	enum Eventty type;

	_Bool down;
	_Bool repeat;
	char key;

	double x, y, dx, dy;
	int butt;
};

_Bool pollevent(Event *);

typedef struct Scrn Scrn;
typedef struct Scrnmt Scrnmt;
typedef struct Scrnstk Scrnstk;

struct Scrn{
	Scrnmt *mt;
	void *data;
};

enum { Ticktm = 20 /* ms */ };

struct Scrnmt{
	void (*update)(Scrn *, Scrnstk *);
	void (*draw)(Scrn *, Gfx *);
	void (*handle)(Scrn *, Scrnstk *, Event *);
	void (*free)(Scrn *);
};

Scrnstk *scrnstknew(void);
void scrnstkfree(Scrnstk *);
/* Stack now owns Scrn, will call scrn->mt->free(scrn) when popped. */
void scrnstkpush(Scrnstk *, Scrn *);
Scrn *scrnstktop(Scrnstk *);
void scrnstkpop(Scrnstk *);

void scrnrun(Scrnstk *, Gfx *);

typedef struct Rtab Rtab;

unsigned int strhash(const char *);

typedef struct Resrcops Resrcops;
struct Resrcops {
	void*(*load)(const char *path, void *aux);
	void(*unload)(const char *path, void *resrc, void *aux); /* may be NULL */
	unsigned int (*hash)(const char *path, void *aux); /* may be NULL */
	_Bool (*eq)(void *aux0, void *aux1); /* may be NULL */
};

Rtab *rtabnew(Resrcops *);
/* unloads all resources and frees the table. */
void rtabfree(Rtab *);
/* Acquire a reference to a resource (loading it if necessary).  The
 * 3rd param is passed as aux data as the 2nd param of load. */
void *resrcacq(Rtab *, const char *file, void *aux);
/* Release a reference to a resource. */
void resrcrel(Rtab *, const char *file, void *aux);

typedef struct Txtinfo Txtinfo;
struct Txtinfo {
	unsigned int size;
	Color color;
};

extern Rtab *imgs;
extern Rtab *anims;
extern Rtab *lvls;
extern Rtab *txt;
extern Rtab *music;
extern Rtab *sfx;
void initresrc(void);
void freeresrc(void);

typedef struct Anim Anim;
Anim *animnew(const char *);
void animfree(Anim *);
void animupdate(Anim *, int);
void animdraw(Gfx *, Anim *, Point);
void animreset(Anim *a);

typedef struct Blk Blk;
struct Blk {
	char tile;
	char flags;
};

typedef struct Lvl Lvl;
struct Lvl {
	int d, w, h, z;
	Blk blks[];
};

Lvl *lvlnew(int d, int w, int h);
Lvl *lvlload(const char *path);
void lvlfree(Lvl *l);
void lvlupdate(Lvl *l);
void lvldraw(Gfx *g, Lvl *l, _Bool bkgrnd, Point offs);
void lvlminidraw(Gfx *, Lvl *, Point);
/* Returns the reverse vector that must be added to v in order to
 * respect collisions. */
Isect lvlisect(Lvl *l, Rect r, Point v);

typedef struct Blkinfo Blkinfo;
struct Blkinfo {
	int x, y, z;
	unsigned int flags;
};

enum {
	Tilecollide = 1<<0,
	Tilewater = 1<<1,
	Tilefdoor = 1<<2,
	Tilebdoor = 1<<3,
};

enum { Theight = 32, Twidth = 32 };

Blkinfo blkinfo(Lvl *l, int x, int y);
/* Get the information on the dominant block that r is overlapping. */
Blkinfo lvlmajorblk(Lvl *l, Rect r);

float blkgrav(int flags);
float blkdrag(int flags);

/* Update the visibility of the level given that the player is viewing
 * the level from location (x, y). */
void lvlvis(Lvl *l, int x, int y);

enum Action{
	Mvleft,
	Mvright,
	Mvdoor,
	Mvjump,
	Mvinv,
	Nactions,
};

_Bool keymapread(char km[Nactions], char *fname);
extern char kmap[Nactions];

enum { Scrnw = 512, Scrnh = 512 };

/* Buffer from side of screen at which to begin scrolling. */
enum { Scrlbuf = 192 };

enum { Tall = 32, Wide = 32 };

enum { Maxdy = 12 };

extern const double Grav;

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
	double ddy;
	_Bool fall;
};

_Bool bodyinit(Body *, const char *name, int x, int y, int z);
void bodyfree(Body *b);
void bodydraw(Gfx *g, Body *b, Point tr);

/* If transl is non-NULL then this body will scroll the screen. */
void bodyupdate(Body *b, Lvl *l, Point *transl);

typedef struct Player Player;
Player *playernew(int x, int y);
void playerfree(Player *);
void playerupdate(Player *, Lvl *l, Point *tr);
void playerdraw(Gfx *, Player *, Point tr);
void playerhandle(Player *, Event *);
Point playerpos(Player *);
Rect playerbox(Player *);

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

_Bool enemyinit(Enemy *, unsigned char id, Point loc);
