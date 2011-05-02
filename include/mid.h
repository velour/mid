#include <stdbool.h>
#include <limits.h>
/* ick. */
#if !defined(PATH_MAX)
#if !defined(_POSIX_PATH_MAX)
#define PATH_MAX 4096
#else
#define PATH_MAX _POSIX_PATH_MAX
#endif
#endif

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

typedef struct Img Img;

Img *imgnew(Gfx *, const char *path);
void imgfree(Img *);
Point imgdims(const Img *);
void imgdraw(Gfx *, Img *, Point);

typedef struct Txt Txt;

Txt *txtnew(const char *font, int sz, Color);
void txtfree(Txt *);
Point txtdims(const Txt *, const char *);
Point txtdraw(Gfx *, Txt *, const char *, Point);

void sndinit(void);

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

typedef struct Scrn Scrn;
typedef struct Scrnmt Scrnmt;
typedef struct Scrnstk Scrnstk;

struct Scrn{
	Scrnmt *mt;
	void *data;
};

union SDL_Event;
enum { Ticktm = 20 /* ms */ };

struct Scrnmt{
	void (*update)(Scrn *, Scrnstk *);
	void (*draw)(Scrn *, Gfx *);
	void (*handle)(Scrn *, Scrnstk *, union SDL_Event *);
	void (*free)(Scrn *);
};

Scrnstk *scrnstknew(void);
void scrnstkfree(Scrnstk *);
/* Stack now owns Scrn, will call scrn->mt->free(scrn) when popped. */
void scrnstkpush(Scrnstk *, Scrn *);
Scrn *scrnstktop(Scrnstk *);
void scrnstkpop(Scrnstk *);

void scrnrun(Scrnstk *, Gfx *);

/* Concatinate path names.  'cat' must be of size PATH_MAX + 1. */
void fscat(const char *d, const char *f, char cat[]);

bool isdir(const char *p);

/* Find a file in the subtree rooted by 'root'.  'out' is the
 * resulting file, it must be of size PATH_MAX + 1. */
bool fsfind(const char *root, const char *fname, char out[]);

enum {
/* Number of items to allow in the cache. */
	RCACHE_SIZE = 100,
/* Must be larger than CACHE_SIZE and should be prime. */
	RESRC_TBL_SIZE = 257,
};

typedef struct {
	void *data;
	char file[PATH_MAX + 1];
	bool del;
	int seq;
	int ind;
} Resrc;

typedef struct {
	Resrc tbl[RESRC_TBL_SIZE];
	Resrc *heap[RCACHE_SIZE];
	int fill;
	int nxtseq;
	void*(*load)(const char *path);
	void(*free)(void*);
} Rcache;

void *resrc(Rcache *c, const char *file);
void rcache(Rcache *c, void*(*load)(const char *path), void(*free)(void*));
