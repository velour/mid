#include "mid.h"
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <stdlib.h>

struct Gfx{
	SDL_Window *win;
	SDL_Renderer *rend;
};

static Gfx gfx;

Gfx *gfxinit(int w, int h){
	if(SDL_Init(SDL_INIT_VIDEO) < 0)
		return 0; //TODO: error messages

	gfx.win = SDL_CreateWindow("TODO: Title...",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		w, h,
		SDL_WINDOW_SHOWN);

	gfx.rend = SDL_CreateRenderer(gfx.win, -1, 0);

	return &gfx;
}

void gfxfree(Gfx *g){
	SDL_DestroyRenderer(g->rend);
	SDL_DestroyWindow(g->win);
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
	if(!s) return 0;

	SDL_Texture *t = SDL_CreateTextureFromSurface(g->rend, s);
	SDL_FreeSurface(s);
	if(!t) return 0;

	Img *i = malloc(sizeof(*i));
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
	SDL_QueryTexture(img->tex, &fmt, &access, &w, &h);
	return (Point){ w, h };
}

void imgdraw(Gfx *g, Img *img, Point p){
	Point wh = imgdims(img);
	SDL_Rect r = { p.x, p.y, wh.x, wh.y };
	SDL_RenderCopy(g->rend, img->tex, 0, &r);
}
