#include "../../include/mid.h"
#include <stdio.h>

static Gfx *gfx;
static Img *img;

typedef struct Box Box;
struct Box{
	_Bool used;
	Rect r;
};
enum { Nboxes = 6 };
static const Color Trans = { 255, 255, 255, 255 };
static Box boxes[Nboxes];
static Color colors[Nboxes];
static unsigned char curbox = 0;
static _Bool dragging = 0;

static void fillcolors(void);
static void drawboxes(void);
static void printboxes(void);
static void *imgload(const char*,void*);
static void update(Scrn*,Scrnstk*);
static void draw(Scrn*,Gfx*);
static void handle(Scrn*,Scrnstk*,Event*);
static void rvfree(Scrn*);

Scrnmt mt = {
	update,
	draw,
	handle,
	rvfree
};

int main(int argc, const char *argv[]){
	if(argc != 2){
		fputs("I need an image name.\n", stderr);
		return 1;
	}
	gfx = gfxinit(640, 480);
	if(!gfx){
		fprintf(stderr, "Failed to start gfx: %s\n", miderrstr());
		return 1;
	}

	fillcolors();

	Resrcops rop = { imgload, NULL, NULL, NULL };
	Rtab *rc = rtabnew(&rop);
	if(!rc){
		fprintf(stderr, "Failed to create rtab: %s\n", miderrstr());
		return 1;
	}
	img = resrcacq(rc, argv[1], 0);
	if(!img){
		fprintf(stderr, "Failed to load img: %s\n", miderrstr());
		return 1;
	}

	Scrn s = { &mt, 0 };
	Scrnstk *stk = scrnstknew();
	if(!stk){
		fprintf(stderr, "Failed to create stk: %s\n", miderrstr());
		return 1;
	}
	scrnstkpush(stk, &s);

	scrnrun(stk, gfx);

	resrcrel(rc, argv[1], 0);
	scrnstkfree(stk);
	rtabfree(rc);
	gfxfree(gfx);
	printboxes();
	return 0;
}

static void fillcolors(void){
	colors[0] = (Color){ 255, 0, 0, 255 };
	colors[1] = (Color){ 0, 255, 0, 255 };
	colors[2] = (Color){ 0, 0, 255, 255 };
	colors[3] = (Color){ 255, 255, 0, 255 };
	colors[4] = (Color){ 255, 0, 255, 255 };
	colors[5] = (Color){ 0, 255, 255, 255 };
}

static void drawboxes(void){
	Box *b = boxes;
	Color *c = colors;
	for(; b->used && b != &boxes[curbox]; b++, c++)
		gfxfillrect(gfx, b->r, *c);
	if(b->used && b == &boxes[curbox])
		gfxfillrect(gfx, b->r, Trans);
}

static void printboxes(void){
	for(Box *b = boxes; b->used && b != &boxes[curbox]; b++)
		printf("%.0f %.0f %.0f %.0f\n",
			b->r.a.x, b->r.a.y, b->r.b.x, b->r.b.y);
}

static void *imgload(const char *p, void *ignore){
	return imgnew(gfx, p);
}

static void update(Scrn *s, Scrnstk *stk){
}
int frame = 0;
static void draw(Scrn *s, Gfx *g){
	gfxclear(gfx, (Color){0,0,0,255});
	imgdraw(gfx, img, (Point){0,0});
	drawboxes();
	gfxflip(gfx);
}

static void handle(Scrn *s, Scrnstk *stk, Event *e){
	switch(e->type){
	case Quit:
	case Keychng:
		if(e->key == 'q')
			scrnstkpop(stk);
		else if(e->key == 'u' && curbox != 0){
			curbox--;
			boxes[curbox].used = 0;
		}
		return;
	case Mousemv:
		if(dragging) boxes[curbox].r.b = (Point){ e->x, e->y };
		break;
	case Mousebt:
		if(e->butt == Mouse1 && e->down && curbox != Nboxes){
			dragging = 1;
			boxes[curbox].used = 1;
			boxes[curbox].r.a = (Point){ e->x, e->y };
		}else if(e->butt == Mouse1 && !e->down && curbox != Nboxes){
			dragging = 0;
			curbox++;
		}
		break;
	default:
		break;
	}
}

static void rvfree(Scrn *s){
}
