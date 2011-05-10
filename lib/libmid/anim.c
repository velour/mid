#include "../../include/mid.h"
#include "fs.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <ctype.h>

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

/* len should include room for the '\0' */
static int readpath(FILE *f, char buf[], int len)
{
	int i = 0;
	int c = fgetc(f);
	while (!isspace(c)) {
		if (i < len - 2)
			buf[i] = c;
		i++;
		c = fgetc(f);
	}
	if (i < len - 1)
		buf[i] = '\0';
	else
		buf[len - 1] = '\0';
	return i;
}

static bool readframes(Rcache *imgs, FILE *f, int n, Anim *a)
{
	int i, err, ms, nxt;
	char fname[PATH_MAX + 1];

	for (i = 0; i < n; i++) {
		if (fscanf(f, "%d %d\n", &ms, &nxt) != 2)
			goto err;
		if (readpath(f, fname, PATH_MAX + 1) > PATH_MAX)
			goto err;
		a->frames[i].img = resrc(imgs, fname, NULL);
		if (!a->frames[i].img)
			goto err;
		a->frames[i].tks = ms / Ticktm;
		a->frames[i].nxt = nxt;
	}

	return true;
err:
	err = errno;
	for (i -= 1; i > 0; i--)
		imgfree(a->frames[i].img);
	errno = err;
	return false;
}

Anim *animnew(Rcache *imgs, const char *path)
{
	assert (imgs);
	int n;
	FILE *f = fopen(path, "r");
	if (!f)
		return NULL;
	if (fscanf(f, "%d", &n) != 1)
		goto err;
	Anim *anim = calloc(1, sizeof(*anim) + sizeof(Frame[n]));
	if (!anim)
		goto err;
	if (!readframes(imgs, f, n, anim))
		goto err1;
	anim->nframes = n;
	anim->cur = 0;
	anim->rem = anim->frames[0].tks;
	fclose(f);
	return anim;
err1:
	free(anim);
err:
	fclose(f);
	return NULL;
}

void animfree(Anim *a)
{
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
