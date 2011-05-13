#include "../../include/mid.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>

typedef enum Tile Tile;
enum Tile { Blank = ' ', Land = 'l' };

enum { Theight = 32, Twidth = 32 };

bool istile(int c)
{
	return c == Blank || c == Land;
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

Lvl *lvlread(FILE *f)
{
	int w, h, d;
	fscanf(f, " %d %d %d",&d, &w, &h);
	Lvl *l = lvlnew(d, w, h);
	if (!l)
		return NULL;
	if (fgetc(f) != '\n')
		goto err;
	for (int z = 0; z < d; z++) {
		int base = z * w * h;
		for (int y = 0; y < h; y++) {
			for (int x = 0; x < w; x++){
				int ind = base + x * h + y;
				int c = fgetc(f);
				if (c == EOF || !istile(c))
					goto err;
				l->tiles[ind] = c;
			}
			if (fgetc(f) != '\n')
				goto err;
		}
		if (z < d - 1 && fgetc(f) != '\n')
			goto err;
	}
	return l;
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

void tiledraw(Gfx *g, Rtab *anims, Tile t, Point pt)
{
	if (t != Land)
		return;
	Anim *a = resrcacq(anims, "anim/land/anim", NULL);
	if (!a)
		abort();
	animdraw(g, a, pt);
	resrcrel(anims, "anim/land/anim", NULL);
}

void lvldraw(Gfx *g, Rtab *anims, Lvl *l, int z, Point offs)
{
	int w = l->w, h = l->h;
	int base = z * w * h;
	for (int x = 0; x < w; x++){
		for (int y = 0; y < h; y++) {
			int ind = base + x * h + y;
			Tile t = l->tiles[ind];
			Point pt = (Point){ offs.x + x * Twidth,
					    offs.y + y * Theight };
			tiledraw(g, anims, t, pt);
		}
	}
}

void lvlupdate(Gfx *g, Rtab *anims, Lvl *l)
{
	/* noop */
}
