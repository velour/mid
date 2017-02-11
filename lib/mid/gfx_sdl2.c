/* © 2013 the Mid Authors under the MIT license. See AUTHORS for the list of authors.*/

#include "../../include/mid.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <assert.h>
#include <stdarg.h>

struct Gfx{
	SDL_Window *win;
	SDL_Renderer *rend;
	Point tr;
};

static Gfx gfx;

enum { Bufsize = 256 };

static Img *vtxt2img(Gfx *g, Txt *t, const char *fmt, va_list ap);
static Point vtxtdims(const Txt *t, const char *fmt, va_list ap);

Point projpt(Point p){
	p.x /= 2;
	p.y /= 2;
	return p;
}

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

	gfx.win = SDL_CreateWindow(title,
				   SDL_WINDOWPOS_CENTERED,
				   SDL_WINDOWPOS_CENTERED,
				   w, h,
				   SDL_WINDOW_SHOWN);
	if (gfx.win == 0)
		return NULL;

	gfx.rend = SDL_CreateRenderer(gfx.win, -1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
	if (!gfx.rend){
		SDL_DestroyWindow(gfx.win);
		return NULL;
	}

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
	return (Point){ w/2, h/2 };
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

void gfxdrawpoint(Gfx *g, Point p, Color c){
	rendcolor(g, c);
	SDL_RenderDrawPoint(g->rend, p.x*2, p.y*2);
}

void gfxfillrect(Gfx *g, Rect r, Color c){
	SDL_Rect sr = { r.a.x*2, r.a.y*2, (r.b.x - r.a.x)*2, (r.b.y - r.a.y)*2 };
	rendcolor(g, c);
	SDL_RenderFillRect(g->rend, &sr);
}

void gfxdrawrect(Gfx *g, Rect r, Color c){
	SDL_Rect sr = { r.a.x*2, r.a.y*2, (r.b.x - r.a.x)*2, (r.b.y - r.a.y)*2 };
	rendcolor(g, c);
	SDL_RenderDrawRect(g->rend, &sr);
}

struct Img{
	SDL_Texture *tex;
	float posscale, sizescale;
};

Img *imgnew(const char *path){
	SDL_Surface *s = IMG_Load(path);
	if(!s)
		return NULL;

	SDL_Texture *t = SDL_CreateTextureFromSurface(gfx.rend, s);
	SDL_FreeSurface(s);
	if(!t)
		return NULL;

	Img *i = xalloc(1, sizeof(*i));
	i->tex = t;
	i->posscale = 2;
	i->sizescale = 2;
	return i;
}

void imgfree(Img *img){
	SDL_DestroyTexture(img->tex);
	xfree(img);
}

Point imgdims(const Img *img){
	Uint32 fmt;
	int access, w, h;
	if (SDL_QueryTexture(img->tex, &fmt, &access, &w, &h) < 0)
		return (Point) { -1, -1 };
	return (Point){ w, h };
}

void imgdraw(Gfx *g, Img *img, Point p){
	float ps = img->posscale;
	float ss = img->sizescale;
	Point wh = imgdims(img);
	SDL_Rect r = { p.x*ps, p.y*ps, wh.x*ss, wh.y*ss };
	SDL_RenderCopy(g->rend, img->tex, 0, &r);
}

void imgdrawscale(Gfx *g, Img *img, Point p, float s){
	float ps = img->posscale;
	float ss = img->sizescale;
	Point wh = imgdims(img);
	SDL_Rect r = { p.x*ps, p.y*ps, wh.x*ss*s, wh.y*ss*s };
	SDL_RenderCopy(g->rend, img->tex, 0, &r);
}

void imgdrawreg(Gfx *g, Img *img, Rect clip, Point p){
	float ps = img->posscale;
	float ss = img->sizescale;
	double w = clip.b.x - clip.a.x;
	double h = clip.b.y - clip.a.y;
	SDL_Rect src = { clip.a.x, clip.a.y, w, h };
	SDL_Rect dst = { p.x*ps, p.y*ps, w*ss, h*ss };
	SDL_RenderCopy(g->rend, img->tex, &src, &dst);
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
	char s[Bufsize + 1];
	vsnprintf(s, Bufsize + 1, fmt, ap);

	int w, h;
	(void)TTF_SizeUTF8(t->font, s, &w, &h);
	return (Point){ w/2, h/2 };
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
	char s[Bufsize + 1];
	vsnprintf(s, Bufsize + 1, fmt, ap);

	SDL_Surface *srf = TTF_RenderUTF8_Blended(t->font, s, c2s(t->color));
	if (!srf)
		return NULL;

	SDL_Texture *tex = SDL_CreateTextureFromSurface(g->rend, srf);
	SDL_FreeSurface(srf);
	if (!tex)
		return NULL;

	Img *i = xalloc(1, sizeof(*i));
	i->tex = tex;
	i->sizescale = 1;
	i->posscale = 2;
	return i;
}

void camreset(Gfx *g){
	g->tr = (Point){0};
}

void cammove(Gfx *g, double dx, double dy){
	g->tr.x += dx;
	g->tr.y += dy;
}

Point camget(Gfx *g){
	return g->tr;
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

void camcenter(Gfx *g, Point p){
	Point dims = gfxdims(g);
	g->tr.x = -p.x + dims.x/2;
	g->tr.y = -p.y + dims.y/2;
}
