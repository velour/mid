#include "../../include/mid.h"
#include <SDL/SDL.h>

typedef struct Maindata Maindata;
struct Maindata{
	Rect rect;
	float dx, dy;
	Color red, white;
	Img *glenda;
	Txt *hi;
};

static Maindata tmpdata;

static void tmpupdate(Scrn *, Scrnstk *);
static void tmpdraw(Scrn *, Gfx *);
static void tmphandle(Scrn *, Scrnstk *, union SDL_Event *);
static void tmpfree(Scrn *);

static Scrnmt tmpmt = {
	tmpupdate,
	tmpdraw,
	tmphandle,
	tmpfree
};

static Scrn mainscrn = {
	&tmpmt,
	0
};

int main(int argc, char *argv[]){
	Gfx *gfx = gfxinit(512, 512);
	if(!gfx)
		return 1;

	tmpdata.rect = (Rect){ (Point){ 0, 0 }, (Point){ 10, 10 } };
	tmpdata.dx = 0;
	tmpdata.dy = 0;
	tmpdata.red = (Color){ 255, 0, 0, 255 };
	tmpdata.white = (Color){ 255, 255, 255, 255 };

	tmpdata.glenda = imgnew(gfx, "resrc/img/9logo.png");
	if (!tmpdata.glenda) {
		fprintf(stderr, "Failed to load 9logo.png\n");
		abort();
	}

	tmpdata.hi = txtnew("resrc/FreeSans.ttf", 32, tmpdata.white);

	mainscrn.data = &tmpdata;

	Scrnstk *stk = scrnstknew();
	scrnstkpush(stk, &mainscrn);

	scrnrun(stk, gfx);

	gfxfree(gfx);
	return 0;
}

static void tmpupdate(Scrn *s, Scrnstk *stk){
	Maindata *md = s->data;
	rectmv(&md->rect, md->dx, md->dy);
}

static void tmpdraw(Scrn *s, Gfx *gfx){
	Maindata *md = s->data;
	gfxclear(gfx, md->red);
	imgdraw(gfx, md->glenda, md->rect.a);
	txtdraw(gfx, md->hi, "hi", (Point){ 100, 100 });
	gfxflip(gfx);
}

static void tmphandle(Scrn *s, Scrnstk *stk, union SDL_Event *e){
	Maindata *md = s->data;
	switch(e->type){
	case SDL_KEYDOWN:
		switch(e->key.keysym.sym){
		case SDLK_LEFT: md->dx--; break;
		case SDLK_RIGHT: md->dx++; break;
		case SDLK_UP: md->dy--; break;
		case SDLK_DOWN: md->dy++; break;
		default:
			scrnstkpop(stk);
		}
		break;
	case SDL_KEYUP:
		switch(e->key.keysym.sym){
		case SDLK_LEFT: md->dx = 0; break;
		case SDLK_RIGHT: md->dx = 0; break;
		case SDLK_UP: md->dy = 0; break;
		case SDLK_DOWN: md->dy = 0; break;
		}
		break;
	}
}

static void tmpfree(Scrn *s){
	Maindata *md = s->data;
	txtfree(md->hi);
	imgfree(md->glenda);
}
