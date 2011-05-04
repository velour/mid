#include "../../include/mid.h"
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
		return 0;

	if(SDL_Init(SDL_INIT_VIDEO) < 0)
		return 0; //TODO: error messages

	gfx.win = SDL_CreateWindow("TODO: Title...",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		w, h,
		SDL_WINDOW_SHOWN);

	gfx.rend = SDL_CreateRenderer(gfx.win, -1, 0);
	if (!gfx.rend)
		return 0;

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
	if(!s) {
		fprintf(stderr, "%s: IMG_Load failed: %s\n", __func__,
			IMG_GetError());
		return NULL;
	}

	SDL_Texture *t = SDL_CreateTextureFromSurface(g->rend, s);
	SDL_FreeSurface(s);
	if(!t) {
		fprintf(stderr, "%s: SDL_CreateTextureFromSurface failed\n", __func__);
		return NULL;
	}

	Img *i = malloc(sizeof(*i));
	if (!i) {
		perror("malloc");
		fprintf(stderr, "%s: malloc failed\n", __func__);
		return NULL;
	}
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
	if (SDL_QueryTexture(img->tex, &fmt, &access, &w, &h) < 0) {
		fprintf(stderr, "SDL_QueryTexturer: query failed\n");
		abort();
	}
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
		return 0;
	Txt *t = malloc(sizeof(*t));
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
	assert(srf != 0);

	SDL_Texture *tex = SDL_CreateTextureFromSurface(g->rend, srf);
	SDL_FreeSurface(srf);
	assert(tex != 0);

	Img *i = malloc(sizeof(*i));
	i->tex = tex;
	return i;
}

//TODO: this is sub-optimal
Point txtdraw(Gfx *g, Txt *t, const char *s, Point p){
	Img *i = txt2img(g,t,s) ;
	imgdraw(g, i, p);
	imgfree(i);

	return (Point){ p.x + txtdims(t,s).x, p.y };
}
