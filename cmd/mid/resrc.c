#include "../../include/mid.h"
#include "../../include/log.h"
#include <stdbool.h>
#include <stdlib.h>

extern Gfx *gfx;

Rcache *imgs = NULL;

void *imgload(const char *path, void *_ignrd)
{
	pr("Loading img %s", path);
	return imgnew(gfx, path);
}

void imgunload(const char *path, void *img, void *_info)
{
	pr("Unloading img %s", path);
	imgfree(img);
}

typedef struct Txtinfo Txtinfo;
struct Txtinfo {
	unsigned int size;
	Color color;
};

Rcache *txt = NULL;

void *txtload(const char *path, void *_info)
{
	Txtinfo *info = _info;
	pr("Loading text %s, %dpt", path, info->size);
	return txtnew(path, info->size, info->color);
}

void txtunload(const char *path, void *txt, void *_info)
{
	Txtinfo *info = _info;
	pr("Unloading txt %s, %dpt", path, info->size);
	txtfree(txt);
}

unsigned int txthash(const char *path, void *_info)
{
	Txtinfo *info = _info;
	return strhash(path) ^ info->size
		^ (info->color.r << 24)
		^ (info->color.g << 16)
		^ (info->color.b << 8)
		^ info->color.a;
}

bool txteq(void *_a, void *_b)
{
	Txtinfo *a = _a, *b = _b;
	return a->size == b->size
		&& a->color.r == b->color.r
		&& a->color.g == b->color.g
		&& a->color.b == b->color.b
		&& a->color.a == b->color.a;
}


Rcache *music = NULL;

void *musicload(const char *path, void *_ignrd)
{
	pr("Loading music %s", path);
	return musicnew(path);
}

void musicunload(const char *path, void *music, void *_info)
{
	pr("Unloading music %s", path);
	musicfree(music);
}

Rcache *sfx = NULL;

void *sfxload(const char *path, void *_ignrd)
{
	pr("Loading sfx %s", path);
	return sfxnew(path);
}

void sfxunload(const char *path, void *s, void *_info)
{
	pr("Unloading sfx %s", path);
	sfxnew(s);
}

void initresrc()
{
	imgs = rcachenew(imgload, imgunload, NULL, NULL);
	if (!imgs)
		fatal("Failed to allocate img cache: %s", miderrstr());
	txt = rcachenew(txtload, txtunload, txthash, txteq);
	if (!txt)
		fatal("Failed to allocate txt cache: %s", miderrstr());
	music = rcachenew(musicload, musicunload, NULL, NULL);
	if (!music)
		fatal("Failed to allocate music cache: %s", miderrstr());
	sfx = rcachenew(sfxload, sfxunload, NULL, NULL);
	if (!sfx)
		fatal("Failed to allocate sfx cache: %s", miderrstr());
}

void freeresrc()
{
	rcachefree(sfx);
	rcachefree(music);
	rcachefree(txt);
	rcachefree(imgs);
}
