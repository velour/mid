#include "../../include/log.h"
#include "../../include/mid.h"
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

static Gfx *gfx = NULL;
static Rcache *imgs = NULL;

void *imgload(const char *path, void *_ignrd)
{
	pr("Loading img %s", path);
	return imgnew(gfx, path);
}

void imgunload(void *img, void *_info)
{
	imgfree(img);
}

typedef struct {
	unsigned int size;
	Color color;
} Txtinfo;

static Rcache *txt = NULL;
void *txtload(const char *path, void *_info)
{
	Txtinfo *info = _info;
	pr("Loading text %s, %dpt", path, info->size);
	return txtnew(path, info->size, info->color);
}

void txtunload(void *txt, void *_info)
{
	txtfree(txt);
}

unsigned int txthash(const char *path, void *_info)
{
	Txtinfo *info = _info;
	return strhash(path) ^ info->size
		^ (info->color.r << 24)
		^ (info->color.g << 16)
		^ (info->color.b << 8)
		^ info->color.a;
}

bool txteq(void *_a, void *_b)
{
	Txtinfo *a = _a, *b = _b;
	return a->size == b->size
		&& a->color.r == b->color.r
		&& a->color.g == b->color.g
		&& a->color.b == b->color.b
		&& a->color.a == b->color.a;
}


static Txtinfo txtmain = {
	.size = 32,
	.color = (Color){ 255, 255, 255, 255 }
};

static Rcache *music = NULL;

void *musicload(const char *path, void *_ignrd)
{
	pr("Loading music %s", path);
	return musicnew(path);
}

void musicunload(void *music, void *_info)
{
	musicfree(music);
}

static Rcache *sfx = NULL;

void *sfxload(const char *path, void *_ignrd)
{
	pr("Loading sfx %s", path);
	return sfxnew(path);
}

void sfxunload(void *s, void *_info)
{
	sfxnew(s);
}

Sfx *pew = NULL;

int main(int argc, char *argv[]){
	loginit(0);

	pr("%s", "Let's rock.");

	gfx = gfxinit(512, 512);
	if(!gfx)
		return EXIT_FAILURE;
	imgs = rcachenew(imgload, imgunload, NULL, NULL);
	if (!imgs)
		fatal("Failed to allocate img cache: %s", miderrstr());
	txt = rcachenew(txtload, txtunload, txthash, txteq);
	if (!txt)
		fatal("Failed to allocate txt cache: %s", miderrstr());

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
	tmpdata.hi = txt2img(gfx, tmpdata.hitxt, "hi");

	mainscrn.data = &tmpdata;

	if (!sndinit())
		fatal("Failed to initialze sound: %s\n", miderrstr());
	music = rcachenew(musicload, musicunload, NULL, NULL);
	if (!music)
		fatal("Failed to allocate music cache: %s", miderrstr());
	Music *m = resrc(music, "bgm_placeholder.ogg", NULL);
	musicstart(m, 0);

	sfx = rcachenew(sfxload, sfxunload, NULL, NULL);
	if (!sfx)
		fatal("Failed to allocate sfx cache: %s", miderrstr());
	pew = resrc(sfx, "pew.wav", NULL);
	if (!pew)
		fatal("Failed to load pew.wav: %s", miderrstr());

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
		case 'p': sfxplay(pew); break;
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
