#include "../../include/mid.h"
#include "../../include/log.h"
#include <stdbool.h>
#include <stddef.h>

enum { Dx = 3, Dy = 8 };

static void loadanim(Anim **a, const char *name, const char *dir, const char *act);
static void chngdir(Player *b);
static void chngact(Player *b);
static Point scroll(Player*, Point delta, Point transl);

typedef enum Act Act;
enum Act {
	Stand,
	Walk,
	Jump,
	Nacts
};

struct Player {
	Anim *leftas[Nacts];
	Anim *rightas[Nacts];
	Anim **anim;
	Act act;
	Point imgloc;

	Body body;
	bool door;
	int jframes;

	/* if changed, update visibility. */
	Blkinfo bi;
};

Player *playernew(int x, int y)
{
	Player *p = xalloc(1, sizeof(*p));

	if(bodyinit(&p->body, x, y)){
		xfree(p);
		return NULL;
	}

	loadanim(&p->leftas[Stand], "knight", "left", "stand");
	loadanim(&p->leftas[Walk], "knight", "left", "walk");
	loadanim(&p->leftas[Jump], "knight", "left", "jump");
	loadanim(&p->rightas[Stand], "knight", "right", "stand");
	loadanim(&p->rightas[Walk], "knight", "right", "walk");
	loadanim(&p->rightas[Jump], "knight", "right", "jump");

	p->anim = p->rightas;
	p->act = Stand;
	p->imgloc = (Point){ x, y - Tall };

	p->bi.x = p->bi.y = p->bi.z = -1;
	return p;
}

void playerfree(Player *p)
{
	xfree(p);
}

void playerupdate(Player *p, Lvl *l, Point *tr)
{
	Point ppos = playerpos(p);

	Blkinfo bi = lvlmajorblk(l, p->body.bbox);

	if (bi.x != p->bi.x || bi.y != p->bi.x || bi.z != p->bi.z)
		lvlvis(l, bi.x, bi.y);
	p->bi = bi;

	double olddx = p->body.vel.x;
	if(olddx)
		p->body.vel.x = (olddx < 0 ? -1 : 1) * blkdrag(bi.flags) * Dx;

	double oldddy = p->body.a.y;
	p->body.a.y = blkgrav(bi.flags);

	if (p->door && bi.flags & Tilebdoor)
		l->z += 1;
	else if (p->door && bi.flags & Tilefdoor)
		l->z -= 1;
	p->door = false;

	bodyupdate(&p->body, l);
	p->body.vel.x = olddx;
	p->body.a.y = oldddy;

	Anim **prevanim = p->anim;
	chngdir(p);
	chngact(p);
	if(p->anim != prevanim)
		animreset(p->anim[p->act]);
	else
		animupdate(p->anim[p->act], 1);

	Point del = { playerpos(p).x - ppos.x, playerpos(p).y - ppos.y };
	*tr = scroll(p, del, *tr);

	if(p->jframes > 0)
		p->jframes--;
}

void playerdraw(Gfx *g, Player *p, Point tr)
{
	if(debugging){
		Rect bbox = p->body.bbox;
		rectmv(&bbox, tr.x, tr.y);
		gfxfillrect(g, bbox, (Color){255,0,0,255});
	}
	Point pt = { p->imgloc.x + tr.x, p->imgloc.y + tr.y };
	animdraw(g, p->anim[p->act], pt);
}

void playerhandle(Player *p, Event *e)
{
	if (e->type != Keychng || e->repeat)
		return;

	char k = e->key;
	if(k == kmap[Mvleft]){
		if(e->down && p->body.vel.x > -Dx)
			p->body.vel.x -= Dx;
		else if(!e->down)
			p->body.vel.x += Dx;
	}else if(k == kmap[Mvright]){
		if(e->down && p->body.vel.x < Dx)
			p->body.vel.x += Dx;
		else if(!e->down)
			p->body.vel.x -= Dx;
	}else if(k == kmap[Mvjump]){
		if(!e->down && p->body.fall){
			if(p->body.vel.y < 0){
				p->body.vel.y += (8 - p->jframes);
				if(p->body.vel.y > 0)
					p->body.vel.y = 0;
			}
			p->jframes = 0;
		}else if(e->down && !p->body.fall){
			p->body.vel.y = -Dy;
			p->body.fall = 1;
			p->jframes = 8;
		}
	}else if(k == kmap[Mvdoor] && e->down){
		p->door = true;
	}
}

Point playerpos(Player *p)
{
	return p->body.bbox.a;
}

Rect playerbox(Player *p)
{
	return p->body.bbox;
}

static void loadanim(Anim **a, const char *name, const char *dir, const char *act)
{
	enum { Buflen = 256 };
	char buf[Buflen];
	snprintf(buf, Buflen, "%s/%s/%s/anim", name, dir, act);
	*a = resrcacq(anims, buf, NULL);
	if (!*a)
		fatal("Failed to load %s: %s", buf, miderrstr());
}

static void chngdir(Player *p)
{
	if (p->body.vel.x < 0)
		p->anim = p->leftas;
	else if (p->body.vel.x > 0)
		p->anim = p->rightas;
}

static void chngact(Player *p)
{
	if (p->body.fall)
		p->act = Jump;
	else if (p->body.vel.x != 0)
		p->act = Walk;
	else
		p->act = Stand;
}

static Point scroll(Player *p, Point delta, Point tr){
	float dx = delta.x;
	float dy = delta.y;

	p->imgloc.x += dx;
	p->imgloc.y += dy;

	double imgx = p->imgloc.x + tr.x;
	double imgy = p->imgloc.y + tr.y;

	if((dx < 0 && imgx < Scrlbuf) || (dx > 0 && imgx > Scrnw - Scrlbuf))
		tr.x -= dx;

	if((dy > 0 && imgy > Scrnh - Scrlbuf) || (dy < 0 && imgy < Scrlbuf))
		tr.y -= dy;

	return tr;
}
