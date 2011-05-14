#include "../../include/mid.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>

enum { Theight = 32, Twidth = 32 };

typedef enum Tile Tile;
enum Tile { Blank = ' ', Land = 'l', Water = 'w' };

bool istile(int c)
{
	return c == Blank || c == Land || c == Water;
}

struct Lvl {
	int d, w, h;
	Tile tiles[];
};

Lvl *lvlnew(int d, int w, int h)
{
	Lvl *l = malloc(sizeof(*l) + sizeof(Tile[d * w * h]));
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

static char *pallet[] = {
	['l'] = "anim/land/anim",
	['w'] = "anim/water/anim",
};

static Anim *tanims[sizeof(pallet) / sizeof(pallet[0])];

static Color bkgrnd = { 96, 96, 96, 255 };

void tiledraw(Gfx *g, Rtab *anims, Tile t, Point pt)
{
	if (t == Blank) {
		Rect r = (Rect){{pt.x, pt.y + Theight}, {pt.x + Twidth, pt.y}};
		gfxfillrect(g, r, bkgrnd);
		return;
	}
	if (pallet[t] == NULL)
		return;
	if (tanims[t] == NULL)
		tanims[t] = resrcacq(anims, pallet[t], NULL);
	animdraw(g, tanims[t], pt);
}

void lvldraw(Gfx *g, Rtab *anims, Lvl *l, int z, Point offs)
{
	int w = l->w, h = l->h;
	int base = z * w * h;
	for (int x = 0; x < w; x++){
		int pxx = offs.x + x * Twidth;
		for (int y = 0; y < h; y++) {
			int ind = base + x * h + y;
			Tile t = l->tiles[ind];
			Point pt = (Point){ pxx, offs.y + y * Theight };
			tiledraw(g, anims, t, pt);
		}
	}
}

void lvlupdate(Rtab *anims, Lvl *l)
{
	for (int i = 0; i < sizeof(tanims) / sizeof(tanims[0]); i++)
		if (tanims[i])
			animupdate(tanims[i], 1);
}
