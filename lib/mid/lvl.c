// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
#include "../../include/mid.h"
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <math.h>

enum { Blkvis = 1 << 1 };
static const double Grav = 0.5;

static bool tileread(FILE *f, Lvl *l, int x, int y, int z);
static void tiledraw(Gfx *g, int t, Point pt, int l);
static void tiledrawlyrs(Gfx *g, int t, Point pt, int mn, int mx);
static bool isshaded(Lvl *l, int t, int x, int y);
static bool isvis(Lvl *l, int x, int y);
static void shade(Gfx *g, Point p);
static Rect tilebbox(int x, int y);
static Isect tileisect(int t, int x, int y, Rect r);
static Rect hitzone(Rect a, Point v);
static void visline(Lvl *l, int x0, int y0, int x1, int y1);
static bool edge(Lvl *l, int x, int y);
static bool blkd(Lvl *l, int x, int y);

static Img *shdimg;
static Img *tisht;

enum { Tlayers = 4 };

typedef struct Tinfo Tinfo;
struct Tinfo {
	_Bool ok;
	Anim anims[Tlayers];
	unsigned int flags;
};

static Tinfo tiles[] = {
	[' '] = {
		.ok = true,
		.anims = { [0] = { .row = 0, .len = 4, .delay = 400/Ticktm, .w = 32, .h = 32, .d = 400/Ticktm } },
	},
	['#'] = {
		.ok = true,
		.anims = { [0] = { .row = 1, .len = 1, .delay = 1, .w = 32, .h = 32, .d = 1 } },
		.flags = Tilecollide
	},
	['w'] = {
		.ok = true,
		.anims = {
			[0] = { .row = 0, .len = 1, .delay = 1, .w = 32, .h = 32, .d = 1 },
			[2] = { .row = 2, .len = 11, .delay = 300/Ticktm, .w = 32, .h = 32, .d = 300/Ticktm },
		},
		.flags = Tilewater
	},
	['>'] = {
		.ok = true,
		.anims = {
			[0] = { .row = 0, .len = 4, .delay = 400/Ticktm, .w = 32, .h = 32, .d = 400/Ticktm },
			[1] = { .row = 3, .len = 4, .delay = 400/Ticktm, .w = 32, .h = 32, .d = 400/Ticktm },
		},
		.flags = Tilebdoor
	},
	[')'] = {
		.ok = true,
		.anims = {
			[0] = { .row = 0, .len = 4, .delay = 400/Ticktm, .w = 32, .h = 32, .d = 400/Ticktm },
			[1] = { .row = 3, .len = 4, .delay = 400/Ticktm, .w = 32, .h = 32, .d = 400/Ticktm },
			[2] = { .row = 2, .len = 11, .delay = 300/Ticktm, .w = 32, .h = 32, .d = 300/Ticktm },
		},
		.flags = Tilebdoor | Tilewater
	},
	['<'] = {
		.ok = true,
		.anims = {
			[0] = { .row = 0, .len = 4, .delay = 400/Ticktm, .w = 32, .h = 32, .d = 400/Ticktm },
			[3] = { .row = 4, .len = 4, .delay = 400/Ticktm, .w = 32, .h = 32, .d = 400/Ticktm },
		},
		.flags = Tilefdoor
	},
	['('] = {
		.ok = true,
		.anims = {
			[0] = { .row = 0, .len = 4, .delay = 400/Ticktm, .w = 32, .h = 32, .d = 400/Ticktm },
			[2] = { .row = 4, .len = 4, .delay = 400/Ticktm, .w = 32, .h = 32, .d = 400/Ticktm },
			[3] = { .row = 2, .len = 11, .delay = 300/Ticktm, .w = 32, .h = 32, .d = 300/Ticktm },
		},
		.flags = Tilefdoor | Tilewater
	},
	['d'] = {
		.ok = true,
		.anims = {
			[0] = { .row = 0, .len = 4, .delay = 400/Ticktm, .w = 32, .h = 32, .d = 400/Ticktm },
			[1] = { .row = 5, .len = 1, .delay = 1, .w = 32, .h = 32, .d = 1 },
		},
		.flags = Tiledown
	},
	['D'] = {
		.ok = true,
		.anims = {
			[0] = { .row = 0, .len = 4, .delay = 400/Ticktm, .w = 32, .h = 32, .d = 400/Ticktm },
			[1] = { .row = 5, .len = 1, .delay = 1, .w = 32, .h = 32, .d = 1 },
			[3] = { .row = 2, .len = 11, .delay = 300/Ticktm, .w = 32, .h = 32, .d = 300/Ticktm },
		},
		.flags = Tiledown | Tilewater
	},
	['u'] = {
		.ok = true,
		.anims = {
			[0] = { .row = 0, .len = 4, .delay = 400/Ticktm, .w = 32, .h = 32, .d = 400/Ticktm },
			[1] = { .row = 6, .len = 1, .delay = 1, .w = 32, .h = 32, .d = 1 },
		},
		.flags = Tileup
	},
	['U'] = {
		.ok = true,
		.anims = {
			[0] = { .row = 0, .len = 4, .delay = 400/Ticktm, .w = 32, .h = 32, .d = 400/Ticktm },
			[1] = { .row = 6, .len = 1, .delay = 1, .w = 32, .h = 32, .d = 1 },
			[3] = { .row = 2, .len = 11, .delay = 300/Ticktm, .w = 32, .h = 32, .d = 300/Ticktm },
		},
		.flags = Tileup | Tilewater
	},
};

int debugging = 0;

enum { Ntiles = sizeof(tiles) / sizeof(tiles[0]) };

static bool istile(int t)
{
	return t >= 0 && t < Ntiles && tiles[t].ok;
}

void lvlfree(Lvl *l)
{
	xfree(l);
}

Lvl *lvlnew(int d, int w, int h)
{
	Lvl *l = xalloc(1, sizeof(*l) + sizeof(Blk[d * w * h]));
	l->d = d;
	l->w = w;
	l->h = h;
	return l;
}

bool lvlinit()
{
	tisht = resrcacq(imgs, "img/tiles.png", NULL);
	assert(tisht != NULL);

	if (!shdimg)
		shdimg = resrcacq(imgs, "img/alph128.png", NULL);
	if (!shdimg)
		return false;

	return true;
}

Lvl *lvlread(FILE *f)
{
	int w, h, d;
	if (fscanf(f, " %d %d %d",&d, &w, &h) != 3) {
		seterrstr("Invalid lvl header");
		return NULL;
	}
	Lvl *l = lvlnew(d, w, h);

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

void lvlwrite(FILE *f, Lvl *l)
{
	fprintf(f, "%d %d %d\n", l->d, l->w, l->h);
	for (int z = 0; z < l->d; z++) {
		for (int y = 0; y < l->h; y++) {
			for (int x = 0; x < l->w; x++) {
				fputc(blk(l, x, y, z)->tile, f);
			}
			fputc('\n', f);
		}
		fputc('\n', f);
	}
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

	if (z == 0 && tiles[c].flags & Tilefdoor) {
		seterrstr("Front door on x=%d, y=%d, z=0", x, y);
		return false;
	}
	if (z == l->d - 1 && tiles[c].flags & Tilebdoor) {
		seterrstr("Back door on x=%d, y=%d, z=max", x, y);
		return false;
	}

	blk(l, x, y, z)->tile = c;

	return true;
}

static Rect tilebbox(int x, int y)
{
	Point a = (Point) {x * Twidth, y * Theight};
	Point b = (Point) {(x + 1) * Twidth, (y + 1) * Theight};
	return (Rect){ .a = a, .b = b };
}

void lvldraw(Gfx *g, Lvl *l, bool bkgrnd)
{
	int w = l->w, h = l->h;
	for (int x = 0; x < w; x++){
		int pxx = x * Twidth;
		for (int y = 0; y < h; y++) {
			Blk *b = blk(l, x, y, l->z);
			int vis = b->flags & Blkvis;
			if (!vis && bkgrnd && !debugging)
				continue;
			int t = b->tile;
			Point pt = {0};

			if (vis || debugging) {
				int mn = bkgrnd ? 0 : (Tlayers-1) / 2 + 1;
				int mx = bkgrnd ? (Tlayers-1) / 2 : Tlayers-1;
				pt = (Point){ pxx, y * Theight };
				tiledrawlyrs(g, t, pt, mn, mx);
			}
			if (!bkgrnd) {
				if(debugging){
					Rect r = tilebbox(x, y);
					camdrawrect(g, r, (Color){0,0,0,255});
					continue;
				}
				if (!vis) {
					Rect r = tilebbox(x, y);
					camfillrect(g, r, (Color){0,0,0,255});
				} else if (vis && isshaded(l, t, x, y)) {
					shade(g, pt);
				}
			}
		}
	}
}

static void tiledraw(Gfx *g, int t, Point pt, int l)
{
	assert(tiles[t].ok);
	assert(tisht != NULL);
	if (!tiles[t].anims[l].sheet)
		tiles[t].anims[l].sheet = tisht;
	assert(tiles[t].anims[l].sheet != NULL);
	camdrawanim(g, &tiles[t].anims[l], pt);
}

static void tiledrawlyrs(Gfx *g, int t, Point pt, int mn, int mx)
{
	for (int l = mn; l <= mx; l++)
		tiledraw(g, t, pt, l);
}

static bool isshaded(Lvl *l, int t, int x, int y)
{
	assert(tiles[t].ok);
	if (tiles[t].flags & Tilecollide)
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
	camdrawimg(g, shdimg, pt);
}

void lvlminidraw(Gfx *g, Lvl *l, Point offs, int scale)
{
	int w = l->w, h = l->h;
	for (int x = 0; x < w; x++){
		int pxx = offs.x + x;
		for (int y = 0; y < h; y++) {
			Blk *b = blk(l, x, y, l->z);
			if (!(b->flags & Blkvis) && !debugging)
				continue;
			int t = b->tile;
			Point pt = (Point){ pxx, offs.y + y };

			Color c = (Color){ 255, 255, 255, 255 };
			unsigned int flags = tiles[t].flags;
			if(flags & Tilecollide)
				c = (Color){ 0, 0, 0 };
			else if(flags & Tilebdoor)
				c = (Color){ 0, 255, 0 };
			else if(flags & Tilefdoor)
				c = (Color){ 64, 255, 0 };
			else if(flags & Tiledown)
				c = (Color){ 230, 255, 0 };
			else if(flags & Tileup)
				c = (Color){ 255, 230, 0 };
			else if(flags & Tilewater)
				c = (Color){ 75, 75, 255 };

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
	for (int l = 0; l < Tlayers; l++) {
		if (!tiles[i].ok)
			continue;
		if (tiles[i].anims[l].sheet)
			animupdate(&tiles[i].anims[l]);
	}
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
	if (!tiles[t].ok)
		printf("t=[%c]", t);
	assert(tiles[t].ok);
	if (!(tiles[t].flags & Tilecollide))
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

Tileinfo lvlmajorblk(Lvl *l, Rect r)
{
	r = rectnorm(r);
	double xmid = (r.b.x + r.a.x) / 2;
	double ymid = (r.b.y + r.a.y) / 2;
	return tileinfo(l, xmid / Twidth, ymid / Theight, l->z);
}

Tileinfo tileinfo(Lvl *l, int x, int y, int z)
{
	int t = blk(l, x, y, z)->tile;
	assert(tiles[t].ok);
	return (Tileinfo) { .x = x, .y = y, .z = z, .flags = tiles[t].flags };
}

static void swap(int *a, int *b)
{
	int t = *a;
	*a = *b;
	*b = t;
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

/*
 * Based on Bresenham's line algorithm: Rasterize a line from the
 * location (x0, y0) to (x1, y1).  Set rasterized blocks to visible,
 * stop when the line is finished or when a collidable block was hit.
 */
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
	return tiles[t].flags & Tilecollide;
}

double blkgrav(int flags)
{
	if(flags & Tilewater)
		return 0.5 * Grav;
	else
		return Grav;
}

double blkdrag(int flags)
{
	if(flags & Tilewater)
		return 0.7;
	else
		return 1.0;
}
