#include "../../include/log.h"
#include "../../include/mid.h"
#include "resrc.h"
#include <stdlib.h>
#include <stdbool.h>

typedef struct Maindata Maindata;
struct Maindata{
	Rect rect;
	float dx, dy;
	Color red, white;
	Img *glenda, *hi;
	Txt *hitxt;
};

static Maindata tmpdata;

static void tmpupdate(Scrn *, Scrnstk *);
static void tmpdraw(Scrn *, Gfx *);
static void tmphandle(Scrn *, Scrnstk *, Event *);
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

Gfx *gfx;

Txtinfo txtmain = {
	.size = 32,
	.color = (Color){ 255, 255, 255, 255 }
};

int main(int argc, char *argv[]){
	loginit(0);

	pr("%s", "Let's rock.");

	gfx = gfxinit(512, 512);
	if(!gfx)
		return EXIT_FAILURE;

	initresrc();
	if (!sndinit())
		fatal("Failed to initialze sound: %s\n", miderrstr());

	tmpdata.rect = (Rect){ (Point){ 0, 0 }, (Point){ 10, 10 } };
	tmpdata.dx = 0;
	tmpdata.dy = 0;
	tmpdata.red = (Color){ 255, 0, 0, 255 };
	tmpdata.white = (Color){ 255, 255, 255, 255 };

	/* 9logo doesn't load with my combo of SDL/SDL_image... seems
	 * to be an error detecting the pixel format -- EB */
	tmpdata.glenda = resrc(imgs, "ship.png", NULL);
	if (!tmpdata.glenda)
		fatal("Failed to load ship.png: %s\n", miderrstr());

	tmpdata.hitxt = resrc(txt, "FreeSans.ttf", &txtmain);
	tmpdata.hi = txt2img(gfx, tmpdata.hitxt, "hi %s", "there");

	mainscrn.data = &tmpdata;

	Music *m = resrc(music, "bgm_placeholder.ogg", NULL);
	if (!m)
		fatal("Failed to load bgm_placeholder.ogg");
	musicstart(m, 0);

	Scrnstk *stk = scrnstknew();
	scrnstkpush(stk, &mainscrn);

	scrnrun(stk, gfx);

	freeresrc();
	sndfree();
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
//	txtdraw(gfx, md->hi, "hi", (Point){ 100, 100 });
	imgdraw(gfx, md->hi, (Point){ 100, 100 });
	gfxflip(gfx);
}

static void tmphandle(Scrn *s, Scrnstk *stk, Event *e){
	Maindata *md = s->data;
	Sfx *pew;
	switch(e->type){
	case Quit:
		scrnstkpop(stk);
		break;
	case Keychng:
		if (e->repeat)
			break;
		switch(e->key){
		case 's': md->dx = (e->down? md->dx-1 : 0); break;
		case 'f': md->dx = (e->down? md->dx+1 : 0); break;
		case 'e': md->dy = (e->down? md->dy-1 : 0); break;
		case 'd': md->dy = (e->down? md->dy+1 : 0); break;
		case 'p':
			pew = resrc(sfx, "pew.wav", NULL);
			if (!pew)
				fatal("Failed to load pew.wav: %s",
				      miderrstr());
			sfxplay(pew);
			break;
		default:
			scrnstkpop(stk);
		}
		break;
	}
}

static void tmpfree(Scrn *s){
	/* everything is freed via the resrc cache. */
}
