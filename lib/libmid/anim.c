#include "../../include/mid.h"
#include "fs.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct Frame Frame;
struct Frame {
	Img *img;
	int tks;
	int nxt;
};

struct Anim {
	int cur;
	int rem;
	int nframes;
	Frame frames[];
};

static bool readframes(Gfx *g, FILE *f, int n, Anim *a)
{
	int ms, nxt;
	char ipath[PATH_MAX];

	for (int i = 0; i < n; i++) {
		if (fscanf(f, "%d %d %s\n", &ms, &nxt, ipath) != 3) {
			for (i -= 1; i > 0; i--)
				imgfree(a->frames[i].img);
			return false;
		}
		a->frames[i].img = imgnew(g, ipath);
		a->frames[i].tks = ms / Ticktm;
		a->frames[i].nxt = nxt;
	}
	
	return false;
}

Anim *animnew(Gfx *g, const char *path)
{
	int n;
	FILE *f = fopen(path, "r");
	if (!f)
		return NULL;
	if (fscanf(f, "%d", &n) != 1)
		goto err;
	Anim *anim = malloc(sizeof(*anim) + sizeof(Frame[n]));
	if (!anim)
		goto err;
	if (!readframes(g, f, n, anim))
		goto err1;
	anim->nframes = n;
	anim->cur = 0;
	anim->rem = anim->frames[0].tks;
	return anim;
err1:
	free(anim);
err:
	fclose(f);
	return NULL;
}

void animfree(Anim *a)
{
	for (int i = 0; i < a->nframes; i++)
		imgfree(a->frames[i].img);
	free(a);
}

void animupdate(Anim *a, int n)
{
	a->rem -= n;
	if (a->rem > 0 || a->frames[a->cur].nxt < 0)
		return;

	int nxt = a->frames[a->cur].nxt;
	int tks = a->frames[nxt].tks;
	a->cur = nxt;
	a->rem = tks + a->rem; /* a->rem may be negative */
	if (a->rem <= 0) {
		n = -a->rem;
		a->rem = 0;
		animupdate(a, n);
	}
}

void animdraw(Gfx *g, Anim *a, Point p)
{
	imgdraw(g, a->frames[a->cur].img, p);
}
