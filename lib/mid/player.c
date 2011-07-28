// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
#include "../../include/mid.h"
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

static Img *plsh;
static Sfx *ow;

static void loadanim(Anim *a, int, int, int);
static void chngdir(Player *b);
static void chngact(Player *b);
static Point scroll(Player*, Point delta);
static double run(Player *);
static double jmp(Player *);
static void mvsw(Player *);

void playerinit(Player *p, int x, int y)
{
	p->lives = 3;

	bodyinit(&p->body, x * Twidth, y * Theight);
	p->hitback = 0;

	plsh = resrcacq(imgs, "img/knight.png", NULL);
	assert(plsh != NULL);

	ow = resrcacq(sfx, "sfx/ow.wav", NULL);
	assert(ow != NULL);

	loadanim(&p->leftas[Stand], 0, 1, 1);
	loadanim(&p->leftas[Walk], 1, 4, 100);
	loadanim(&p->leftas[Jump], 2, 1, 1);
	loadanim(&p->rightas[Stand], 3, 1, 1);
	loadanim(&p->rightas[Walk], 4, 4, 100);
	loadanim(&p->rightas[Jump], 5, 1, 1);

	p->anim = p->rightas;
	p->act = Stand;
	p->imgloc = (Point){ x * Twidth, y * Theight };

	p->bi.x = p->bi.y = p->bi.z = -1;
	p->stats[StatHp] = 10;
	p->stats[StatDex] = 8;
	p->stats[StatStr] = 5;
	p->eqp[StatHp] = p->stats[StatHp];

	p->sw.row = 0;
	p->sw.dir = Mvright;
	p->sw.cur = -1;
	p->sw.pow = 1;
	mvsw(p);
}

static void trydoorstairs(Player *p, Zone *zn, Tileinfo bi)
{
	Lvl *l = zn->lvl;
	if (!p->acting)
		return;

	int oldz = l->z;
	if (p->acting && bi.flags & Tilebdoor)
		l->z += 1;
	else if (p->acting && bi.flags & Tilefdoor)
		l->z -= 1;
	else if (p->acting && bi.flags & Tileup)
		zn->updown = Goup;
	else if (p->acting && bi.flags & Tiledown)
		zn->updown = Godown;

	p->acting = false;

	if (oldz == l->z)
		return;

	/* center the player on the door */
	playersetloc(p, bi.x, bi.y);
}

void playersetloc(Player *p, int x, int y)
{
	Point dst = (Point) { x * Twidth, y * Theight };
	Point src = rectnorm(p->body.bbox).a;
	double dx = dst.x - src.x, dy = dst.y - src.y;
	rectmv(&p->body.bbox, dx, dy);
}

void playerupdate(Player *p, Zone *zn, Point *tr)
{
	Lvl *l = zn->lvl;
	Point ppos = playerpos(p);

	Tileinfo bi = lvlmajorblk(l, p->body.bbox);

	if (bi.x != p->bi.x || bi.y != p->bi.x || bi.z != p->bi.z)
		lvlvis(l, bi.x, bi.y);
	p->bi = bi;

	double olddx = p->body.vel.x;
	if(olddx && p->hitback == 0)
		p->body.vel.x = (olddx < 0 ? -1 : 1) * blkdrag(bi.flags) * run(p);

	if(p->hitback != 0)
		p->body.vel.x = p->hitback;

	double oldddy = p->body.a.y;
	p->body.a.y = blkgrav(bi.flags);

	trydoorstairs(p, zn, bi);

	bodyupdate(&p->body, l);
	p->body.vel.x = olddx;
	p->body.a.y = oldddy;

	mvsw(p);

	Anim *prevanim = p->anim;
	chngdir(p);
	chngact(p);
	if(p->anim != prevanim)
		animreset(&p->anim[p->act]);
	else
		animupdate(&p->anim[p->act]);

	Point del = { playerpos(p).x - ppos.x, playerpos(p).y - ppos.y };
	*tr = scroll(p, del);

	if(p->jframes > 0)
		p->jframes--;
	if(p->iframes > 0)
		p->iframes--;

	if(p->iframes < 750.0/Ticktm)
		p->hitback = 0;

	if(p->sframes > 8){
		p->sframes--;
		p->sw.cur = 0;
	}else if(p->sframes > 0){
		p->sframes--;
		p->sw.cur = 1;
	}else
		p->sw.cur = -1;
}

void playerdraw(Gfx *g, Player *p)
{
	if(debugging)
		camfillrect(g, p->body.bbox, (Color){255,0,0,255});

	if(p->iframes % 4 == 0)
		animdraw(g, &p->anim[p->act], p->imgloc);

	if(p->sw.cur >= 0)
		sworddraw(g, &p->sw);
}

void playerhandle(Player *p, Event *e)
{
	if (e->type != Keychng || e->repeat)
		return;

	char k = e->key;
	if(k == kmap[Mvleft]){
		if(e->down && p->body.vel.x > -run(p))
			p->body.vel.x -= run(p);
		else if(!e->down)
			p->body.vel.x += run(p);
	}else if(k == kmap[Mvright]){
		if(e->down && p->body.vel.x < run(p))
			p->body.vel.x += run(p);
		else if(!e->down)
			p->body.vel.x -= run(p);
	}else if(k == kmap[Mvjump]){
		if(!e->down && p->body.fall){
			if(p->body.vel.y < 0){
				p->body.vel.y += (8 - p->jframes);
				if(p->body.vel.y > 0)
					p->body.vel.y = 0;
			}
			p->jframes = 0;
		}else if(e->down && !p->body.fall){
			p->body.vel.y = -jmp(p);
			p->body.fall = 1;
			p->jframes = 8;
		}
	}else if(k == kmap[Mvact] && e->down){
		p->acting = true;
	}else if(k == kmap[Mvsword] && e->down && p->sw.cur < 0){
		p->sframes = 16;
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

void playerdmg(Player *p, int x, int dir){
	if(p->iframes > 0)
		return;

	sfxplay(ow);

	p->iframes = 1000.0 / Ticktm; // 1s
	p->hitback = 0;
	if (dir > 0)
		p->hitback =  5;
	else if (dir < 0)
		p->hitback = -5;
	p->eqp[StatHp] -= x;
	if(p->eqp[StatHp] <= 0)
		p->eqp[StatHp] = 0;
}

void playerheal(Player *p, int x)
{
	p->eqp[StatHp] += x;
	if (p->eqp[StatHp] > p->stats[StatHp])
		p->eqp[StatHp] = p->stats[StatHp];
}

_Bool playertake(Player *p, Item *i){
	for(size_t j = 0; j < Maxinv; j++){
		if(p->inv[j].id > 0)
			continue;
		invitinit(&p->inv[j], i->id);
		return 1;
	}
	return 0;
}

static void loadanim(Anim *a, int row, int len, int delay)
{
	*a = (Anim){
		.sheet = plsh,
		.row = row,
		.len = len,
		.delay = delay/Ticktm,
		.w = Twidth,
		.h = Theight,
		.d = delay/Ticktm
	};
}

static void chngdir(Player *p)
{
	if (p->body.vel.x < 0){
		p->anim = p->leftas;
		p->sw.dir = Mvleft;
	}else if (p->body.vel.x > 0){
		p->anim = p->rightas;
		p->sw.dir = Mvright;
	}
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

static Point scroll(Player *p, Point delta){
	Point ntr = {0};
	double dx = delta.x;
	double dy = delta.y;

	p->imgloc.x += dx;
	p->imgloc.y += dy;

	double imgx = p->imgloc.x;
	double imgy = p->imgloc.y;

	if((dx < 0 && imgx < Scrlbuf) || (dx > 0 && imgx > Scrnw - Scrlbuf)) {
		p->imgloc.x -= dx;
		ntr.x = -dx;
	}

	if((dy > 0 && imgy > Scrnh - Scrlbuf) || (dy < 0 && imgy < Scrlbuf)) {
		p->imgloc.y -= dy;
		ntr.y = -dy;
	}

	return ntr;
}

static double run(Player *p){
	return (p->stats[StatDex] + p->eqp[StatDex]) / 2 - 1;
}

static double jmp(Player *p){
	return (p->stats[StatDex] + p->eqp[StatDex]);
}

static void mvsw(Player *p){
	p->sw.rightloc[0] = (Rect){
		{ p->body.bbox.a.x - 11, p->body.bbox.a.y - 32 },
		{ p->body.bbox.b.x - 11, p->body.bbox.b.y - 32 }
	};
	p->sw.rightloc[1] = (Rect){
		{ p->body.bbox.a.x + 20, p->body.bbox.a.y },
		{ p->body.bbox.b.x + 20, p->body.bbox.b.y }
	};
	p->sw.leftloc[0] = (Rect){
		{ p->body.bbox.a.x + 11, p->body.bbox.a.y - 32 },
		{ p->body.bbox.b.x + 11, p->body.bbox.b.y - 32 }
	};
	p->sw.leftloc[1] = (Rect){
		{ p->body.bbox.a.x - 20, p->body.bbox.a.y },
		{ p->body.bbox.b.x - 20, p->body.bbox.b.y }
	};
}
