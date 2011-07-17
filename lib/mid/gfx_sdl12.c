// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
#include "../../include/mid.h"
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <stdio.h>
#include <assert.h>
#include <stdarg.h>

struct Gfx{
	SDL_Surface *scrn;
	Point tr;
};

static Gfx gfx;

enum { Bufsize = 256 };

static Img *vtxt2img(Gfx *g, Txt *t, const char *fmt, va_list ap);
static Point vtxtdims(const Txt *t, const char *fmt, va_list ap);
static void prvidinfo(void);

Gfx *gfxinit(int w, int h, const char *title){
	if(TTF_Init() < 0)
		return NULL;

	if (SDL_WasInit(0) == 0) {
		if(SDL_Init(SDL_INIT_VIDEO) < 0)
			return NULL;
	} else {
		if(SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
			return NULL;
	}

	if (debugging)
		prvidinfo();

	gfx.scrn = SDL_SetVideoMode(w, h, 0, SDL_HWSURFACE|SDL_DOUBLEBUF);
	if (!gfx.scrn)
		return NULL;

	SDL_WM_SetCaption(title, title);

	return &gfx;
}

void gfxfree(Gfx *g){
	SDL_FreeSurface(g->scrn);
	TTF_Quit();
	SDL_Quit();
}

Point gfxdims(const Gfx *g){
	return (Point){ g->scrn->w, g->scrn->h };
}

void gfxflip(Gfx *g){
	SDL_Flip(g->scrn);
}

void gfxclear(Gfx *g, Color c){
	Uint32 sc = SDL_MapRGB(g->scrn->format, c.r, c.g, c.b);
	SDL_FillRect(g->scrn, NULL, sc);
}

void gfxfillrect(Gfx *g, Rect r, Color c){
	SDL_Rect sr = { r.a.x, r.a.y, r.b.x - r.a.x, r.b.y - r.a.y };
	Uint32 sc = SDL_MapRGB(g->scrn->format, c.r, c.g, c.b);
	SDL_FillRect(g->scrn, &sr, sc);
}

void gfxdrawrect(Gfx *g, Rect r, Color c){
	Point ul = r.a;
	Point ur = { r.b.x, r.a.y };
	Point ll = { r.a.x, r.b.y };
	Point lr = r.b;
	Rect top = { ul, { ur.x, ur.y + 1 } };
	gfxfillrect(g, top, c);
	Rect left = { ul, { ll.x + 1, ll.y } };
	gfxfillrect(g, left, c);
	Rect bot = { { ll.x, ll.y - 1}, lr };
	gfxfillrect(g, bot, c);
	Rect right = { { ur.x - 1, ur.y }, lr };
	gfxfillrect(g, right, c);
}

struct Img{
	SDL_Surface *tex;
};

Img *imgnew(const char *path){
	SDL_Surface *s = IMG_Load(path);
	if(!s)
		return NULL;

	Img *i = xalloc(1, sizeof(*i));
	i->tex = s;
	return i;
}

void imgfree(Img *img){
	SDL_FreeSurface(img->tex);
	xfree(img);
}

Point imgdims(const Img *img){
	return (Point){ img->tex->w, img->tex->h };
}

void imgdraw(Gfx *g, Img *img, Point p){
	SDL_Rect dst = { p.x, p.y };
	SDL_BlitSurface(img->tex, NULL, g->scrn, &dst);
}

void imgdrawreg(Gfx *g, Img *img, Rect clip, Point p){
	double w = clip.b.x - clip.a.x;
	double h = clip.b.y - clip.a.y;
	SDL_Rect src = { clip.a.x, clip.a.y, w, h };
	SDL_Rect dst = { p.x, p.y, w, h };
	SDL_BlitSurface(img->tex, &src, g->scrn, &dst);
}

struct Txt{
	TTF_Font *font;
	Color color;
};

Txt *txtnew(const char *font, int sz, Color c){
	TTF_Font *f = TTF_OpenFont(font, sz);
	if(!f)
		return NULL;
	Txt *t = xalloc(1, sizeof(*t));
	t->font = f;
	t->color = c;
	return t;
}

void txtfree(Txt *t){
	TTF_CloseFont(t->font);
	xfree(t);
}

Point txtdims(const Txt *t, const char *fmt, ...){
	va_list ap;

	va_start(ap, fmt);
	Point p = vtxtdims(t, fmt, ap);
	va_end(ap);
	return p;
}

static Point vtxtdims(const Txt *t, const char *fmt, va_list ap)
{
	char s[Bufsize];
	vsnprintf(s, Bufsize, fmt, ap);

	int w, h;
	(void)TTF_SizeUTF8(t->font, s, &w, &h);
	return (Point){ w, h };
}

static SDL_Color c2s(Color c){
	return (SDL_Color){ c.r, c.g, c.b };
}

Img *txt2img(Gfx *g, Txt *t, const char *fmt, ...){
	va_list ap;

	va_start(ap, fmt);
	Img *i = vtxt2img(g, t, fmt, ap);
	va_end(ap);
	return i;
}

Point txtdraw(Gfx *g, Txt *t, Point p, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	Img *i = vtxt2img(g, t, fmt, ap);
	va_end(ap);

	imgdraw(g, i, p);
	imgfree(i);

	va_start(ap, fmt);
	Point ret = (Point){ p.x + vtxtdims(t, fmt, ap).x, p.y };
	va_end(ap);

	return ret;
}

static Img *vtxt2img(Gfx *g, Txt *t, const char *fmt, va_list ap)
{
	char s[Bufsize];
	vsnprintf(s, Bufsize, fmt, ap);

	SDL_Surface *tex = TTF_RenderUTF8_Blended(t->font, s, c2s(t->color));
	if (!tex)
		return NULL;

	Img *i = xalloc(1, sizeof(*i));
	i->tex = tex;
	return i;
}

void camreset(Gfx *g){
	g->tr = (Point){0};
}

void cammove(Gfx *g, double dx, double dy){
	g->tr.x += dx;
	g->tr.y += dy;
}

void camdrawrect(Gfx *g, Rect r, Color c){
	r.a = vecadd(r.a, g->tr);
	r.b = vecadd(r.b, g->tr);
	gfxdrawrect(g, r, c);
}

void camfillrect(Gfx *g, Rect r, Color c){
	r.a = vecadd(r.a, g->tr);
	r.b = vecadd(r.b, g->tr);
	gfxfillrect(g, r, c);
}

void camdrawimg(Gfx *g, Img *i, Point p){
	p = vecadd(p, g->tr);
	imgdraw(g, i, p);
}

void camdrawreg(Gfx *g, Img *i, Rect c, Point p){
	p = vecadd(p, g->tr);
	imgdrawreg(g, i, c, p);
}

void camdrawanim(Gfx *g, Anim *a, Point p){
	p = vecadd(p, g->tr);
	animdraw(g, a, p);
}

static void prvidinfo(void)
{
	const SDL_VideoInfo *vi = SDL_GetVideoInfo();

#define PR(str, b) printf(str ":	%s\n", b ? "yes" : "no")
	PR("Hardware surfaces available", vi->hw_available);
	PR("Window manage available", vi->wm_available);
	PR("Hardware blits accelerated", vi->blit_hw);
	PR("Hardware to hardware color key blits accelerated", vi->blit_hw_CC);
	PR("Hardware to hardware alpha blits accelerated", vi->blit_hw_A);
	PR("Software blits accelerated", vi->blit_sw);
	PR("Software to hardware color key blits accelerated", vi->blit_sw_CC);
	PR("Software to hardware alpha blits accelerated", vi->blit_sw_A);
	PR("Color fills accelerated", vi->blit_fill);
	printf("Total video memory (KB):a	%u\n", vi->video_mem);
}