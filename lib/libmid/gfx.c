#include "../../include/mid.h"
#include "../../include/log.h"
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

struct Gfx{
	SDL_Window *win;
	SDL_Renderer *rend;
};

static Gfx gfx;

Gfx *gfxinit(int w, int h){
	if(TTF_Init() < 0)
		return NULL;

	if(SDL_Init(SDL_INIT_VIDEO) < 0)
		return NULL;

	gfx.win = SDL_CreateWindow("TODO: Title...",
				   SDL_WINDOWPOS_CENTERED,
				   SDL_WINDOWPOS_CENTERED,
				   w, h,
				   SDL_WINDOW_SHOWN);
	if (gfx.win == 0)
		return NULL;

	gfx.rend = SDL_CreateRenderer(gfx.win, -1, 0);
	if (!gfx.rend)
		return NULL;

	return &gfx;
}

void gfxfree(Gfx *g){
	SDL_DestroyRenderer(g->rend);
	SDL_DestroyWindow(g->win);
	TTF_Quit();
	SDL_Quit();
}

Point gfxdims(const Gfx *g){
	int w, h;
	SDL_GetWindowSize(g->win, &w, &h);
	return (Point){ w, h };
}

void gfxflip(Gfx *g){
	SDL_RenderPresent(g->rend);
}

static void rendcolor(Gfx *g, Color c){
	SDL_SetRenderDrawColor(g->rend, c.r, c.g, c.b, c.a);
}

void gfxclear(Gfx *g, Color c){
	rendcolor(g, c);
	SDL_RenderClear(g->rend);
}

void gfxfillrect(Gfx *g, Rect r, Color c){
	SDL_Rect sr = { r.a.x, r.a.y, r.b.x - r.a.x, r.b.y - r.a.y };
	rendcolor(g, c);
	SDL_RenderFillRect(g->rend, &sr);
}

struct Img{
	SDL_Texture *tex;
};

Img *imgnew(Gfx *g, const char *path){
	SDL_Surface *s = IMG_Load(path);
	if(!s)
		return NULL;

	SDL_Texture *t = SDL_CreateTextureFromSurface(g->rend, s);
	SDL_FreeSurface(s);
	if(!t)
		return NULL;

	Img *i = malloc(sizeof(*i));
	if (!i)
		return NULL;
	i->tex = t;
	return i;
}

void imgfree(Img *img){
	SDL_DestroyTexture(img->tex);
	free(img);
}

Point imgdims(const Img *img){
	Uint32 fmt;
	int access, w, h;
	if (SDL_QueryTexture(img->tex, &fmt, &access, &w, &h) < 0)
		return (Point) { -1, -1 };
	return (Point){ w, h };
}

void imgdraw(Gfx *g, Img *img, Point p){
	Point wh = imgdims(img);
	SDL_Rect r = { p.x, p.y, wh.x, wh.y };
	SDL_RenderCopy(g->rend, img->tex, 0, &r);
}

struct Txt{
	TTF_Font *font;
	Color color;
};

Txt *txtnew(const char *font, int sz, Color c){
	TTF_Font *f = TTF_OpenFont(font, sz);
	if(!f)
		return NULL;
	Txt *t = malloc(sizeof(*t));
	if(!t)
		return NULL;
	t->font = f;
	t->color = c;
	return t;
}

void txtfree(Txt *t){
	TTF_CloseFont(t->font);
	free(t);
}

Point txtdims(const Txt *t, const char *s){
	int w, h;
	(void)TTF_SizeUTF8(t->font, s, &w, &h);
	return (Point){ w, h };
}

static SDL_Color c2s(Color c){
	return (SDL_Color){ c.r, c.g, c.b };
}

Img *txt2img(Gfx *g, Txt *t, const char *s){
	SDL_Surface *srf = TTF_RenderUTF8_Blended(t->font, s, c2s(t->color));
	if (!srf)
		return NULL;

	SDL_Texture *tex = SDL_CreateTextureFromSurface(g->rend, srf);
	SDL_FreeSurface(srf);
	if (!tex)
		return NULL;

	Img *i = malloc(sizeof(*i));
	if (!i)
		return NULL;
	i->tex = tex;
	return i;
}

//TODO: this is sub-optimal
Point txtdraw(Gfx *g, Txt *t, const char *s, Point p){
	Img *i = txt2img(g,t,s) ;
	/* test for !i here. */
	imgdraw(g, i, p);
	imgfree(i);

	return (Point){ p.x + txtdims(t,s).x, p.y };
}
