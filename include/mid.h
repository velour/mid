const char *miderrstr(void);
void seterrstr(const char *fmt, ...);

typedef struct Point Point;
struct Point{
	float x, y;
};

typedef struct Line1d Line1d;
struct Line1d{
	float a, b;
};

/*
Returns the absolute value of the intersection of two lines.
Returns a negative number if the lines don't intersect.
*/
float isection1d(Line1d, Line1d);

typedef struct Rect Rect;
struct Rect{
	Point a, b;
};

Line1d rectprojx(Rect);
Line1d rectprojy(Rect);
void rectmv(Rect *, float dx, float dy);
/* Makes point a the min,min and point b the max,max. */
Rect rectnorm(Rect r);
/* Distance a would have to travel to intersect b. */
Point rectdist(Rect a, Rect b);
/* Trace the movement of a along the vector v (dx, dy).  If a will
 * intersect with b, then the result is the new vector (dx, dy) that
 * respects the collision. */
Point recttrace1(Rect a, Point v, Rect b);
void ptmv(Point *, float dx, float dy);

typedef struct Isect Isect;
struct Isect{
	_Bool is;
	float dx, dy; /* isection1d of x and y projs */
};

Isect isection(Rect, Rect);
int isect(Rect, Rect);

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

Gfx *gfxinit(int w, int h);
void gfxfree(Gfx *);
Point gfxdims(const Gfx *);
void gfxflip(Gfx *);
void gfxclear(Gfx *, Color);
void gfxfillrect(Gfx *, Rect, Color);
void gfxdrawrect(Gfx *, Rect, Color);

typedef struct Img Img;

Img *imgnew(Gfx *, const char *path);
void imgfree(Img *);
/* Returns negative dimensions on failure. */
Point imgdims(const Img *);
void imgdraw(Gfx *, Img *, Point);

typedef struct Txt Txt;

Txt *txtnew(const char *font, int sz, Color);
void txtfree(Txt *);
Point txtdims(const Txt *, const char *);
Img *txt2img(Gfx *, Txt *, const char *fmt, ...);
// Prefer txt2img to this!
Point txtdraw(Gfx *, Txt *, const char *, Point);

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

	float x, y, dx, dy;
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

typedef struct Anim Anim;
Anim *animnew(Rtab *imgs, const char *);
void animfree(Rtab *imgs, Anim *);
void animupdate(Anim *, int);
void animdraw(Gfx *, Anim *, Point);

typedef struct Lvl Lvl;
Lvl *lvlload(const char *path);
void lvlfree(Lvl *l);
void lvlupdate(Rtab *anims, Lvl *l);
void lvldraw(Gfx *g, Rtab *anims, Lvl *l, int z, _Bool bkgrnd, Point offs);
/* Returns a rectangle that is moved out of intersection. */
Rect lvltrace(Lvl *l, int z, Rect r, float dx, float dy);
