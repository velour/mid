#include "../../include/log.h"
#include "../../include/mid.h"
#include <stdlib.h>

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

static Gfx *gfx = NULL;
static Rcache *imgs = NULL;

void *imgload(const /* deal with it */ char *path)
{
	return imgnew(gfx, path);
}

void imgunload(void *img)
{
	imgfree(img);
}

static Rcache *txt = NULL;
void *txtload(const char *path)
{
	return txtnew(path, 32, (Color){ 255, 255, 255, 255 });
}

void txtunload(void *txt)
{
	txtfree(txt);
}

int main(int argc, char *argv[]){
	loginit(0);

	pr("%s\n", "Let's rock.");

	gfx = gfxinit(512, 512);
	if(!gfx)
		return EXIT_FAILURE;
	imgs = rcachenew(imgload, imgunload);
	if (!imgs)
		fatal("Failed to allocate img cache: %s", miderrstr());
	txt = rcachenew(txtload, txtunload);
	if (!txt)
		fatal("Failed to allocate txt cache: %s", miderrstr());

	tmpdata.rect = (Rect){ (Point){ 0, 0 }, (Point){ 10, 10 } };
	tmpdata.dx = 0;
	tmpdata.dy = 0;
	tmpdata.red = (Color){ 255, 0, 0, 255 };
	tmpdata.white = (Color){ 255, 255, 255, 255 };

	/* 9logo doesn't load with my combo of SDL/SDL_image... seems
	 * to be an error detecting the pixel format -- EB */
	tmpdata.glenda = resrc(imgs, "ship.png");
	if (!tmpdata.glenda)
		fatal("Failed to load ship.png: %s\n", miderrstr());

	tmpdata.hitxt = resrc(txt, "FreeSans.ttf");
	tmpdata.hi = txt2img(gfx, tmpdata.hitxt, "hi");

	mainscrn.data = &tmpdata;

	Scrnstk *stk = scrnstknew();
	scrnstkpush(stk, &mainscrn);

	scrnrun(stk, gfx);

	rcachefree(txt);
	rcachefree(imgs);
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
	imgfree(md->hi);
	txtfree(md->hitxt);
	imgfree(md->glenda);
}
