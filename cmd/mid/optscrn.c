// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
#include "../../include/mid.h"
#include "../../include/log.h"
#include "game.h"
#include <string.h>
#include <ctype.h>

typedef struct Opts Opts;
struct Opts{
	char kmap[Nactions];
	int curkey;
	Txt *txt;
};

static void update(Scrn *s, Scrnstk *stk);
static void draw(Scrn *s, Gfx *g);
static void handle(Scrn *s, Scrnstk *stk, Event *e);
static void optsfree(Scrn *s);

static Scrnmt optsmt = {
	update,
	draw,
	handle,
	optsfree
};

Scrn *optscrnnew(void){
	static Opts opts = {};
	static Scrn s = {};

	memcpy(opts.kmap, kmap, Nactions);
	opts.curkey = Mvleft;
	Txtinfo ti = { TxtSzMedium };
	opts.txt = resrcacq(txt, TxtStyleMenu, &ti);
	if(!opts.txt)
		return NULL;

	s.mt = &optsmt;
	s.data = &opts;
	return &s;
}

static void update(Scrn *s, Scrnstk *stk){
	// nothing
}

static void draw(Scrn *s, Gfx *g){
	static char *names[] = {
		[Mvleft] = "Left",
		[Mvright] = "Right",
		[Mvact] = "Action",
		[Mvjump] = "Jump",
		[Mvinv] = "Menu",
		[Mvsword] = "Sword",
	};

	Opts *opt = s->data;

	gfxclear(g, (Color){ 240, 240, 240 });

	int pad = TxtSzMedium/2;
	Point loc = { pad, pad };
	Color hilit = { 255, 219, 0 };

	for(int i = Mvleft; i < Nactions; i++){
		Point d = txtdims(opt->txt, "%-7s%2c", names[i], opt->kmap[i]);
		if(i == opt->curkey){
			Rect hover = { vecadd(loc, (Point){-4,-4}), vecadd(vecadd(loc, d), (Point){4,4}) };
			gfxfillrect(g, hover, hilit);
		}
		Point p = txtdraw(g, opt->txt, loc, "%-7s%2c", names[i], opt->kmap[i]);
		loc.y = p.y + d.y + pad;
	}

	loc.y += pad*2;
	txtdraw(g, opt->txt, loc, "Volume: %d", (int)(sndvol(-1) / (float)SndVolMax * 100));

	gfxflip(g);
}

static void handle(Scrn *s, Scrnstk *stk, Event *e){
	if(e->type != Keychng || e->repeat || !e->down)
		return;

	if(!isalpha(e->key))
		return;

	Opts *opt = s->data;

	if(opt->curkey == Nactions){
		scrnstkpop(stk);
		return;
	}

	int ck = opt->kmap[opt->curkey];
	for(int i = Mvleft; i < Nactions; i++)
		if(opt->kmap[i] == e->key){
			opt->kmap[i] = ck;
			break;
		}
	opt->kmap[opt->curkey] = e->key;
	opt->curkey++;
}

static void optsfree(Scrn *s){
	Opts *opt = s->data;
	memcpy(kmap, opt->kmap, Nactions);
	//TODO: save to a file in appdata, also load at game start iff exists
}
