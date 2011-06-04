#include "../../include/mid.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <math.h>

enum { Blkvis = 1 << 1 };

static Lvl *lvlnew(int d, int w, int h);
static Lvl *lvlread(FILE *f);
static bool tileread(FILE *f, Lvl *l, int x, int y, int z);
static void bkgrnddraw(Gfx *g, Rtab *anims, int t, Point pt);
static void fgrnddraw(Gfx *g, Rtab *anims, int t, Point pt);
static Rect tilebbox(int x, int y);
static Isect tileisect(int t, int x, int y, Rect r);
static Rect hitzone(Rect a, Point v);
static Blkinfo blkinfo(Lvl *l, int x, int y);

typedef struct Tinfo Tinfo;
struct Tinfo {
	char *bgfile;
	char *fgfile;
	Anim *bganim;
	Anim *fganim;
	unsigned int flags;
};

static Tinfo *tiles[] = {
	[' '] = &(Tinfo){ .bgfile = "anim/blank/anim", },
	['#'] = &(Tinfo){ .bgfile = "anim/land/anim", .flags = Tilecollide },
	['w'] = &(Tinfo){ .fgfile = "anim/water/anim", .flags = Tilewater },
	['>'] = &(Tinfo){ .bgfile = "anim/bdoor/anim", .flags = Tilebdoor },
	['<'] = &(Tinfo){ .fgfile = "anim/fdoor/anim",
			  .bgfile = "anim/blank/anim",
			  .flags = Tilefdoor },
};

bool lvlgridon = false;

enum { Ntiles = sizeof(tiles) / sizeof(tiles[0]) };

bool istile(int t)
{
	return t >= 0 && t < Ntiles && tiles[t];
}

void lvlfree(Lvl *l)
{
	free(l);
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

static Lvl *lvlnew(int d, int w, int h)
{
	Lvl *l = calloc(1, sizeof(*l) + sizeof(Blk[d * w * h]));
	if (!l)
		return NULL;
	l->d = d;
	l->w = w;
	l->h = h;
	return l;
}

static Lvl *lvlread(FILE *f)
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
	free(l);
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

	l->blks[z * l->w * l->h + y * l->w + x].tile = c;

	return true;
}

static void bkgrnddraw(Gfx *g, Rtab *anims, int t, Point pt)
{
	assert(tiles[t] != NULL);
	if (!tiles[t]->bgfile) {
		Rect r = (Rect){{pt.x, pt.y}, {pt.x + Twidth, pt.y + Theight}};
		gfxfillrect(g, r, (Color){255,255,255,255});
		return;
	}
	if (!tiles[t]->bganim)
		tiles[t]->bganim = resrcacq(anims, tiles[t]->bgfile, NULL);
	if (!tiles[t]->bganim)
		abort();
	animdraw(g, tiles[t]->bganim, pt);
}

static void fgrnddraw(Gfx *g, Rtab *anims, int t, Point pt)
{
	assert(tiles[t] != NULL);
	if (!tiles[t]->fgfile)
		return;
	if (!tiles[t]->fganim)
		tiles[t]->fganim = resrcacq(anims, tiles[t]->fgfile, NULL);
	if (!tiles[t]->fganim)
		abort();
	animdraw(g, tiles[t]->fganim, pt);
}

static Rect tilebbox(int x, int y)
{
	Point a = (Point) {x * Twidth, y * Theight};
	Point b = (Point) {(x + 1) * Twidth, (y + 1) * Theight};
	return (Rect){ .a = a, .b = b };
}

void lvldraw(Gfx *g, Rtab *anims, Lvl *l, bool bkgrnd, Point offs)
{
	int w = l->w, h = l->h;
	int base = l->z * w * h;
	for (int x = 0; x < w; x++){
		int pxx = offs.x + x * Twidth;
		for (int y = 0; y < h; y++) {
			int ind = base + y * w + x;
			if (!(l->blks[ind].flags & Blkvis))
				continue;
			int t = l->blks[ind].tile;
			Point pt = (Point){ pxx, offs.y + y * Theight };

			if (bkgrnd)
				bkgrnddraw(g, anims, t, pt);
			else {
				fgrnddraw(g, anims, t, pt);
				if(lvlgridon){
					Rect r = tilebbox(x, y);
					rectmv(&r, offs.x, offs.y);
					gfxdrawrect(g, r, (Color){0,0,0,255});
				}
			}
		}
	}
}

void lvlminidraw(Gfx *g, Lvl *l, Point offs)
{
	int w = l->w, h = l->h;
	int base = l->z * w * h;
	for (int x = 0; x < w; x++){
		int pxx = offs.x + x;
		for (int y = 0; y < h; y++) {
			int ind = base + y * w + x;
			if (!(l->blks[ind].flags & Blkvis))
				continue;
			int t = l->blks[ind].tile;
			Point pt = (Point){ pxx, offs.y + y };

			Color c;
			if(tiles[t]->flags & Tilecollide)
				c = (Color){ 0, 0, 0, 255 };
			else if(tiles[t]->flags & Tilebdoor)
				c = (Color){ 0, 255, 0, 255 };
			else if(tiles[t]->flags & Tilefdoor)
				c = (Color){ 64, 255, 0, 255 };
			else if(!tiles[t]->fgfile)
				c = (Color){ 255, 255, 255, 255 };
			else if(tiles[t]->flags & Tilewater)
				c = (Color){ 75, 75, 255, 255 };

			Rect r = {
				(Point){ pt.x * 2, pt.y *2 },
				(Point){ pt.x*2 + 2, pt.y*2 + 2 }
			};
			gfxdrawrect(g, r, c);
		}
	}
}

void lvlupdate(Rtab *anims, Lvl *l)
{
	for (int i = 0; i < sizeof(tiles) / sizeof(tiles[0]); i++) {
		if (!tiles[i])
			continue;
		if (tiles[i]->fganim)
			animupdate(tiles[i]->fganim, 1);
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
			int i = l->z * l->h * l->w + y * l->w + x;
			Isect is = tileisect(l->blks[i].tile, x, y, mv);
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
			int i = l->z * l->h * l->w + y * l->w + x;
			Isect is = tileisect(l->blks[i].tile, x, y, mv);
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
	Rect zone = hitzone(r, (Point){0, 0});
	Blkinfo bi = blkinfo(l, zone.a.x, zone.a.y);
	float area = 0.0;
	Isect is = isection(r, tilebbox(bi.x, bi.y));
	if (is.is)
		area = isectarea(is);

	for (int x = zone.a.x; x <= zone.b.x; x++) {
		for (int y = zone.a.y; y <= zone.b.y; y++) {
			is = isection(r, tilebbox(x, y));
			if (is.is) {
				float a = isectarea(is);
				if (a > area) {
					bi = blkinfo(l, x, y);
					area = a;
				}
			}
		}
	}

	return bi;
}

static Blkinfo blkinfo(Lvl *l, int x, int y)
{
	int i = l->z * l->w * l->h + y * l->w + x;
	int t = l->blks[i].tile;
	assert (tiles[t]);
	return (Blkinfo) { .x = x, .y = y, .z = l->z, .flags = tiles[t]->flags };
}

void swap(int *a, int *b)
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
void visline(Lvl *l, int x0, int y0, int x1, int y1)
{
	bool steep = abs(y1 - y0) > abs(x1 - x0);
	if (steep) {
		swap(&x0, &y0);
		swap(&x1, &y1);
	}
	int dx = abs(x1 - x0);
	int dy = abs(y1 - y0);
	float err = 0.0;
	float derr = (float)dy / (float)dx;
	int ystep = y0 < y1 ? 1 : -1;
	int y = y0;
	int base = l->z * l->w * l->h;
	int xstep = x0 < x1 ? 1 : -1;
	for (int x = x0; ; x += xstep) {
		int px = steep ? y : x;
		int py = steep ? x : y;
		int ind = base + py * l->w + px;
		int t = l->blks[ind].tile;
		l->blks[ind].flags |= Blkvis;
		if (tiles[t]->flags & Tilecollide || x == x1)
			break;
		err += derr;
		if (err >= 0.5) {
			y += ystep;
				err -= 1.0;
		}
	}
}

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
