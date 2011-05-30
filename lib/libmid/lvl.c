#include "../../include/mid.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <math.h>

enum { Theight = 32, Twidth = 32 };

enum {
	Collide = 1<<0,
	Water = 1<<1,
};

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
	['#'] = &(Tinfo){ .bgfile = "anim/land/anim", .flags = Collide },
	['w'] = &(Tinfo){ .fgfile = "anim/water/anim", .flags = Water },
};

bool lvlgridon = false;

enum { Ntiles = sizeof(tiles) / sizeof(tiles[0]) };

bool istile(int t)
{
	return t >= 0 && t < Ntiles && tiles[t];
}

struct Lvl {
	int d, w, h;
	char tiles[];
};

Lvl *lvlnew(int d, int w, int h)
{
	Lvl *l = malloc(sizeof(*l) + sizeof(char[d * w * h]));
	if (!l)
		return NULL;
	l->d = d;
	l->w = w;
	l->h = h;
	return l;
}

void lvlfree(Lvl *l)
{
	free(l);
}

int tileread(FILE *f)
{
	int c = fgetc(f);
	if (c == EOF) {
		seterrstr("Unexpected EOF");
		goto err;
	}

	if (!istile(c)) {
		seterrstr("Invalid tile: %c\n", c);
		goto err;
	}
	return c;
err:
	return -1;
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
		int base = z * w * h;
		for (y = 0; y < h; y++) {
			for (x = 0; x < w; x++){
				int ind = base + x * h + y;
				int c = tileread(f);
				if (c < 0)
					goto err;
				l->tiles[ind] = c;
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

void bkgrnddraw(Gfx *g, Rtab *anims, int t, Point pt)
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
void fgrnddraw(Gfx *g, Rtab *anims, int t, Point pt)
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

Rect tilebbox(int x, int y)
{
	Point a = (Point) {x * Twidth, y * Theight};
	Point b = (Point) {(x + 1) * Twidth, (y + 1) * Theight};
	return (Rect){ .a = a, .b = b };
}

void lvldraw(Gfx *g, Rtab *anims, Lvl *l, int z, bool bkgrnd, Point offs)
{
	int w = l->w, h = l->h;
	int base = z * w * h;
	for (int x = 0; x < w; x++){
		int pxx = offs.x + x * Twidth;
		for (int y = 0; y < h; y++) {
			int ind = base + x * h + y;
			int t = l->tiles[ind];
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

void lvlminidraw(Gfx *g, Lvl *l, int z, Point offs)
{
	int w = l->w, h = l->h;
	int base = z * w * h;
	for (int x = 0; x < w; x++){
		int pxx = offs.x + x;
		for (int y = 0; y < h; y++) {
			int ind = base + x * h + y;
			int t = l->tiles[ind];
			Point pt = (Point){ pxx, offs.y + y };

			Color c;
			if(tiles[t]->flags & Collide)
				c = (Color){ 0, 0, 0, 255 };
			else if(!tiles[t]->fgfile)
				c = (Color){ 255, 255, 255, 255 };
			else if(tiles[t]->flags & Water)
				c = (Color){ 75, 75, 255, 255 };
			gfxdrawpoint(g, pt, c);
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

Isect tileisect(int t, int x, int y, Rect r)
{
	assert(tiles[t]);
	if (!(tiles[t]->flags & Collide))
		return (Isect){ .is = 0 };
	return isection(r, tilebbox(x, y));
}

Rect hitzone(Rect a, Point v)
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

Isect lvlisect(Lvl *l, int z, Rect r, Point v)
{
	Rect test = hitzone(r, v);

	Isect isect = (Isect) { .is = 0, .dx = 0.0, .dy = 0.0 };
	Rect mv = r;
	rectmv(&mv, 0, v.y);
	for (int x = test.a.x; x <= test.b.x; x++) {
		for (int y = test.a.y; y <= test.b.y; y++) {
			int i = z * l->h * l->w + x * l->h + y;
			Isect is = tileisect(l->tiles[i], x, y, mv);
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
			int i = z * l->h * l->w + x * l->h + y;
			Isect is = tileisect(l->tiles[i], x, y, mv);
			if (is.is && is.dx > isect.dx) {
				isect.is = true;
				isect.dx = is.dx;
			}

		}
	}

	return isect;
}
