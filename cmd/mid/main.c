#include "../../include/mid.h"
#include <stdio.h>
#include <stdlib.h>

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
		exit(1);
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

static void tmphandle(Scrn *s, Scrnstk *stk, Event *e){
	Maindata *md = s->data;
	switch(e->type){
	case Quit:
		scrnstkpop(stk);
		break;
	case Keychng:
		switch(e->key){
		case 's': md->dx = (e->down? md->dx-1 : 0); break;
		case 'f': md->dx = (e->down? md->dx+1 : 0); break;
		case 'e': md->dy = (e->down? md->dy-1 : 0); break;
		case 'd': md->dy = (e->down? md->dy+1 : 0); break;
		default:
			scrnstkpop(stk);
		}
		break;
	}
}

static void tmpfree(Scrn *s){
	Maindata *md = s->data;
	txtfree(md->hi);
	imgfree(md->glenda);
}
