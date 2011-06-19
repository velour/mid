#include "../../include/mid.h"
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <math.h>

enum { Blkvis = 1 << 1 };
static const double Grav = 0.5;

static bool tileread(FILE *f, Lvl *l, int x, int y, int z);
static void bkgrnddraw(Gfx *g, int t, Point pt);
static void mgrnddraw(Gfx *g, int t, Point pt);
static void fgrnddraw(Gfx *g, int t, Point pt);
static bool isshaded(Lvl *l, int t, int x, int y);
static bool isvis(Lvl *l, int x, int y);
static void shade(Gfx *g, Point p);
static Rect tilebbox(int x, int y);
static Isect tileisect(int t, int x, int y, Rect r);
static Rect hitzone(Rect a, Point v);
static void visline(Lvl *l, int x0, int y0, int x1, int y1);
static bool edge(Lvl *l, int x, int y);
static bool blkd(Lvl *l, int x, int y);
static Blk *blk(Lvl *l, int x, int y, int z);

static Img *shdimg;

typedef struct Tinfo Tinfo;
struct Tinfo {
	char *bgfile, *mgfile, *fgfile;
	Anim *bganim;
	/* Player and enemies draw on top of bganim but behind mganim
	 * and fganim. */
	Anim *mganim; 		/* mid ground (water) */
	Anim *fganim;		/* foreground (z-- door) */
	unsigned int flags;
};

static Tinfo *tiles[] = {
	[' '] = &(Tinfo){ .bgfile = "anim/blank/anim", .flags = Tilereach },
	['.'] = &(Tinfo){ .bgfile = "anim/blank/anim", },
	['#'] = &(Tinfo){ .bgfile = "anim/land/anim", .flags = Tilecollide },
	['w'] = &(Tinfo){ .bgfile = "anim/blank/anim", .mgfile = "anim/water/anim", .flags = Tilewater },
	['W'] = &(Tinfo){ .bgfile = "anim/blank/anim", .mgfile = "anim/water/anim", .flags = Tilewater | Tilereach },
	['>'] = &(Tinfo){ .bgfile = "anim/bdoor/anim", .flags = Tilebdoor | Tilereach},
	[')'] = &(Tinfo){ .bgfile = "anim/bdoor/anim", .mgfile = "anim/water/anim", .flags = Tilebdoor | Tilewater | Tilereach },
	['<'] = &(Tinfo){ .fgfile = "anim/fdoor/anim", .bgfile = "anim/blank/anim", .flags = Tilefdoor | Tilereach},
	['('] = &(Tinfo){ .fgfile = "anim/fdoor/anim", .mgfile = "anim/water/anim", .bgfile = "anim/blank/anim", .flags = Tilefdoor | Tilewater | Tilereach },
};

int debugging = 0;

enum { Ntiles = sizeof(tiles) / sizeof(tiles[0]) };

bool istile(int t)
{
	return t >= 0 && t < Ntiles && tiles[t];
}

void lvlfree(Lvl *l)
{
	xfree(l);
}

bool lvlinit()
{
	if (!shdimg)
		shdimg = resrcacq(imgs, "img/alph128.png", NULL);
	if (!shdimg)
		return false;

	return true;
}

Lvl *lvlload(const char *path)
{
	FILE *f = fopen(path, "r");
	if (!f)
		return NULL;
	Lvl *l = lvlread(f);
	int err = errno;
	fclose(f);
	errno = err;
	return  l;
}

Lvl *lvlnew(int d, int w, int h)
{
	Lvl *l = xalloc(1, sizeof(*l) + sizeof(Blk[d * w * h]));
	if (!l)
		return NULL;
	l->d = d;
	l->w = w;
	l->h = h;
	return l;
}

Lvl *lvlread(FILE *f)
{
	int w, h, d;
	fscanf(f, " %d %d %d",&d, &w, &h);
	Lvl *l = lvlnew(d, w, h);
	if (!l)
		return NULL;

	int x, y, z;
	x = y = z = 0;
	if (fgetc(f) != '\n')
		goto errnl;

	for (z = 0; z < d; z++) {
		for (y = 0; y < h; y++) {
			for (x = 0; x < w; x++){
				if (!tileread(f, l, x, y, z))
					goto err;
			}
			if (fgetc(f) != '\n')
				goto errnl;
		}
		if (z < d - 1 && fgetc(f) != '\n')
			goto errnl;
	}
	return l;
errnl:
	seterrstr("Expected newline in level file: z=%d, x=%d, y=%d", z, x, y);
err:
	xfree(l);
	return NULL;
}

static bool tileread(FILE *f, Lvl *l, int x, int y, int z)
{
	int c = fgetc(f);
	if (c == EOF) {
		seterrstr("Unexpected EOF");
		return false;
	}

	if (!istile(c)) {
		seterrstr("Invalid tile: %c at x=%d, y=%d, z=%d\n", c, x, y, z);
		return false;
	}

	if (z == 0 && tiles[c]->flags & Tilefdoor) {
		seterrstr("Front door on x=%d, y=%d, z=0", x, y);
		return false;
	}
	if (z == l->d - 1 && tiles[c]->flags & Tilebdoor) {
		seterrstr("Back door on x=%d, y=%d, z=max", x, y);
		return false;
	}

	blk(l, x, y, z)->tile = c;

	return true;
}

static void bkgrnddraw(Gfx *g, int t, Point pt)
{
	assert(tiles[t] != NULL);
	if (!tiles[t]->bgfile) {
		Rect r = (Rect){{pt.x, pt.y}, {pt.x + Twidth, pt.y + Theight}};
		gfxfillrect(g, r, (Color){255,255,255,255});
		return;
	}
	if (!tiles[t]->bganim)
		tiles[t]->bganim = resrcacq(anims, tiles[t]->bgfile, NULL);
	assert(tiles[t]->bganim);
	animdraw(g, tiles[t]->bganim, pt);
}

static void mgrnddraw(Gfx *g, int t, Point pt)
{
	assert(tiles[t] != NULL);
	if (!tiles[t]->mgfile)
		return;
	if (!tiles[t]->mganim)
		tiles[t]->mganim = resrcacq(anims, tiles[t]->mgfile, NULL);
	assert(tiles[t]->mganim);
	animdraw(g, tiles[t]->mganim, pt);
}
static void fgrnddraw(Gfx *g, int t, Point pt)
{
	assert(tiles[t] != NULL);
	if (!tiles[t]->fgfile)
		return;
	if (!tiles[t]->fganim)
		tiles[t]->fganim = resrcacq(anims, tiles[t]->fgfile, NULL);
	assert(tiles[t]->fganim);
	animdraw(g, tiles[t]->fganim, pt);
}

static Rect tilebbox(int x, int y)
{
	Point a = (Point) {x * Twidth, y * Theight};
	Point b = (Point) {(x + 1) * Twidth, (y + 1) * Theight};
	return (Rect){ .a = a, .b = b };
}

void lvldraw(Gfx *g, Lvl *l, bool bkgrnd, Point offs)
{
	int w = l->w, h = l->h;
	for (int x = 0; x < w; x++){
		int pxx = offs.x + x * Twidth;
		for (int y = 0; y < h; y++) {
			Blk *b = blk(l, x, y, l->z);
			if (!(b->flags & Blkvis) && debugging < 2)
				continue;
			int t = b->tile;
			Point pt = (Point){ pxx, offs.y + y * Theight };

			if (bkgrnd)
				bkgrnddraw(g, t, pt);
			else {
				mgrnddraw(g, t, pt);
				fgrnddraw(g, t, pt);
				if (isshaded(l, t, x, y))
					shade(g, pt);
				if(debugging >= 2){
					Rect r = tilebbox(x, y);
					rectmv(&r, offs.x, offs.y);
					gfxdrawrect(g, r, (Color){0,0,0,255});
				}
			}
		}
	}
}

static bool isshaded(Lvl *l, int t, int x, int y)
{
	assert(tiles[t]);
	if (tiles[t]->flags & Tilecollide)
		return false;

	return !isvis(l, x-1, y) || !isvis(l, x+1, y)
		|| !isvis(l, x, y-1) || !isvis(l, x, y+1);
}

// If the x,y is out of bounds of the array then it is visible.
static bool isvis(Lvl *l, int x, int y)
{
	if (x < 0 || x >= l->w || y < 0 || y >= l->h)
		return true;
	return blk(l, x, y, l->z)->flags & Blkvis;
}

static void shade(Gfx *g, Point pt)
{
	imgdraw(g, shdimg, pt);
}

void lvlminidraw(Gfx *g, Lvl *l, Point offs, int scale)
{
	int w = l->w, h = l->h;
	for (int x = 0; x < w; x++){
		int pxx = offs.x + x;
		for (int y = 0; y < h; y++) {
			Blk *b = blk(l, x, y, l->z);
			if (!(b->flags & Blkvis) && debugging < 2)
				continue;
			int t = b->tile;
			Point pt = (Point){ pxx, offs.y + y };

			Color c = (Color){ 255, 255, 255, 255 };
			unsigned int flags = tiles[t]->flags;
			if(flags & Tilecollide)
				c = (Color){ 0, 0, 0, 255 };
			else if(flags & Tilebdoor)
				c = (Color){ 0, 255, 0, 255 };
			else if(flags & Tilefdoor)
				c = (Color){ 64, 255, 0, 255 };
			else if(flags & Tilewater)
				c = (Color){ 75, 75, 255, 255 };

			Rect r = {
				(Point){ pt.x * scale, pt.y * scale },
				(Point){ pt.x * scale + scale, pt.y * scale + scale }
			};
			gfxfillrect(g, r, c);
		}
	}
}

void lvlupdate(Lvl *l)
{
	for (int i = 0; i < sizeof(tiles) / sizeof(tiles[0]); i++) {
		if (!tiles[i])
			continue;
		if (tiles[i]->fganim)
			animupdate(tiles[i]->fganim, 1);
		if (tiles[i]->mganim)
			animupdate(tiles[i]->mganim, 1);
		if (tiles[i]->bganim)
			animupdate(tiles[i]->bganim, 1);
	}
}

Isect lvlisect(Lvl *l, Rect r, Point v)
{
	Rect test = hitzone(r, v);

	Isect isect = (Isect) { .is = 0, .dx = 0.0, .dy = 0.0 };
	Rect mv = r;
	rectmv(&mv, 0, v.y);
	for (int x = test.a.x; x <= test.b.x; x++) {
		for (int y = test.a.y; y <= test.b.y; y++) {
			Blk *b = blk(l, x, y, l->z);
			Isect is = tileisect(b->tile, x, y, mv);
			if (is.is && is.dy > isect.dy) {
				isect.is = true;
				isect.dy = is.dy;
			}

		}
	}

	if (v.x == 0.0)
		return isect;

	mv = r;
	rectmv(&mv, v.x, v.y + (v.y < 0 ? isect.dy : -isect.dy));
	for (int x = test.a.x; x <= test.b.x; x++) {
		for (int y = test.a.y; y <= test.b.y; y++) {
			Blk *b = blk(l, x, y, l->z);
			Isect is = tileisect(b->tile, x, y, mv);
			if (is.is && is.dx > isect.dx) {
				isect.is = true;
				isect.dx = is.dx;
			}

		}
	}

	return isect;
}

static Isect tileisect(int t, int x, int y, Rect r)
{
	if (!tiles[t])
		printf("t=[%c]", t);
	assert(tiles[t]);
	if (!(tiles[t]->flags & Tilecollide))
		return (Isect){ .is = 0 };
	return isection(r, tilebbox(x, y));
}

static Rect hitzone(Rect a, Point v)
{
	Rect b = a;
	rectmv(&b, v.x, v.y);

	a = rectnorm(a);
	b = rectnorm(b);

	int xmin = a.a.x < b.a.x ? a.a.x : b.a.x;
	int ymin = a.a.y < b.b.y ? a.a.y : b.a.y;
	int xmax = a.b.x > b.b.x ? ceil(a.b.x) : ceil(b.b.x);
	int ymax = a.b.y > b.b.y ? ceil(a.b.y) : ceil(b.b.y);
	xmin /= Twidth;
	xmax /= Twidth;
	ymin /= Theight;
	ymax /= Theight;
	if (ymin > 0)
		ymin--;

	return (Rect) { .a = {xmin, ymin}, .b = {xmax, ymax} };
}

Blkinfo lvlmajorblk(Lvl *l, Rect r)
{
	r = rectnorm(r);
	double xmid = (r.b.x + r.a.x) / 2;
	double ymid = (r.b.y + r.a.y) / 2;
	return blkinfo(l, xmid / Twidth, ymid / Theight);
}

Blkinfo blkinfo(Lvl *l, int x, int y)
{
	int t = blk(l, x, y, l->z)->tile;
	assert (tiles[t]);
	return (Blkinfo) { .x = x, .y = y, .z = l->z, .flags = tiles[t]->flags };
}

static void swap(int *a, int *b)
{
	int t = *a;
	*a = *b;
	*b = t;
}

/*
 * Based on Bresenham's line algorithm: Rasterize a line from the
 * location (x0, y0) to (x1, y1).  Set rasterized blocks to visible,
 * stop when the line is finished or when a collidable block was hit.
 */
/* Update the visibility of the level given that the player is viewing
 * the level from location (x, y).  This works by rasterizing
 * 'visibility lines' from (x,y) to each block on the convex hull of
 * the map.
 *
 * This seems pretty fast, it hardly changes the frame rendering time
 * at all. */
void lvlvis(Lvl *l, int x, int y)
{
	for (int i = 0; i < l->w; i++) {
		visline(l, x, y, i, 0);
		visline(l, x, y, i, l->h - 1);
	}
	for (int i = 0; i < l->h; i++) {
		visline(l, x, y, 0, i);
		visline(l, x, y, l->w - 1, i);
	}
}

static void visline(Lvl *l, int x0, int y0, int x1, int y1)
{
	bool steep = abs(y1 - y0) > abs(x1 - x0);
	if (steep) {
		swap(&x0, &y0);
		swap(&x1, &y1);
	}
	int dx = abs(x1 - x0);
	int dy = abs(y1 - y0);
	double err = 0.0;
	double derr = (double)dy / (double)dx;
	int ystep = y0 < y1 ? 1 : -1;
	int y = y0;
	int xstep = x0 < x1 ? 1 : -1;
	for (int x = x0; ; x += xstep) {
		int px = steep ? y : x;
		int py = steep ? x : y;
		blk(l, px, py, l->z)->flags |= Blkvis;
		if (blkd(l, px, py) || x == x1)
			break;
		err += derr;
		if (err >= 0.5) {
			/* Test if diagonal movement is blocked on sides. */
			int x1 = steep ? y + ystep : x + xstep;
			int y1 = steep ? x + xstep : y + ystep;
			if (blkd(l, px, y1) && blkd(l, x1, py)) {
				if (edge(l, x1, y1))
					blk(l, x1, y1, l->z)->flags |= Blkvis;
				break;
			}
			y += ystep;
			err -= 1.0;
		}
	}
}

static bool edge(Lvl *l, int x, int y)
{
	return x <= 0 || y <= 0 || x >= l->w - 1 || y >= l->h - 1;
}

static bool blkd(Lvl *l, int x, int y)
{
	int t = blk(l, x, y, l->z)->tile;
	return tiles[t]->flags & Tilecollide;
}

float blkgrav(int flags)
{
	if(flags & Tilewater)
		return 0.5f * Grav;
	else
		return Grav;
}

float blkdrag(int flags)
{
	if(flags & Tilewater)
		return 0.7f;
	else
		return 1.0f;
}

static Blk *blk(Lvl *l, int x, int y, int z)
{
	return &l->blks[z * l->w * l->h + y * l->w + x];
}

