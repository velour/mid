#include "../../include/log.h"
#include "../../include/mid.h"
#include "resrc.h"
#include <stdlib.h>
#include <stdbool.h>

typedef struct Maindata Maindata;
struct Maindata{
	Rect rect0, rect1;
	float dx, dy;
	Color red, white;
	Img *hi;
	Anim *wlk;
	Txt *hitxt;
	Lvl *lvl;
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
	.color = { 255, 255, 255, 255 },
};

int main(int argc, char *argv[]){
	loginit(0);

	pr("%s", "Let's rock.");

	gfx = gfxinit(512, 512);
	if(!gfx)
		return EXIT_FAILURE;

	if (!sndinit())
		fatal("Failed to initialze sound: %s\n", miderrstr());

	initresrc();

	tmpdata.rect0 = (Rect){ .a = (Point){ 0, 0 }, .b = (Point){ 10, 10 } };
	tmpdata.rect1 = (Rect){ .a = (Point){ 100, 0 }, .b = (Point){ 0, 0 } };
	tmpdata.dx = 0;
	tmpdata.dy = 0;
	tmpdata.red = (Color){ 255, 0, 0, 255 };
	tmpdata.white = (Color){ 255, 255, 255, 255 };

	tmpdata.wlk = resrcacq(anim, "anim/wlk/anim", NULL);
	if (!tmpdata.wlk)
		fatal("Failed to load wlk.anim: %s\n", miderrstr());

	tmpdata.lvl = resrcacq(lvls, "lvl/0.lvl", NULL);
	if (!tmpdata.lvl)
		fatal("Failed to load 0.lvl: %s\n", miderrstr());

	tmpdata.hitxt = resrcacq(txt, "txt/FreeSans.ttf", &txtmain);
	tmpdata.hi = txt2img(gfx, tmpdata.hitxt, "hi %s", "there");

	mainscrn.data = &tmpdata;

	Music *m = resrcacq(music, "music/bgm_placeholder.ogg", NULL);
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
	animupdate(md->wlk, 1);
	rectmv(&md->rect0, md->dx, md->dy);
	rectmv(&md->rect1, md->dx, md->dy);
}

static void tmpdraw(Scrn *s, Gfx *gfx){
	Maindata *md = s->data;
	gfxclear(gfx, md->red);
	lvldraw(gfx, imgs, tmpdata.lvl, 0, (Point){-md->rect0.a.x, -md->rect0.a.y});
	animdraw(gfx, md->wlk, md->rect1.a);
	imgdraw(gfx, md->hi, (Point){ 100, 100 });
	gfxflip(gfx);
}

static void tmphandle(Scrn *s, Scrnstk *stk, Event *e){
	Maindata *md = s->data;
	static Sfx *pew;
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
//			if (!pew) {
				pew = resrcacq(sfx, "sfx/pew.wav", NULL);
				if (!pew)
					fatal("Failed to load pew.wav: %s",
					      miderrstr());
	//		}
			sfxplay(pew);
			resrcrel(sfx, "sfx/pew.wav", NULL);
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
