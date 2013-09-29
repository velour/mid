/* © 2013 the Mid Authors under the MIT license. See AUTHORS for the list of authors.*/

#include "../../include/mid.h"

Rect meterdraw(Gfx *g, Meter *m, Point p){
	Rect base = {
		{ p.x, p.y },
		{ p.x + m->xscale * m->base, p.y + m->h }
	};
	Rect both = {
		base.a,
		{ base.a.x + m->xscale * m->max, base.b.y }
	};

	Rect extra = {
		{ base.b.x, base.a.y },
		{ base.b.x + m->xscale * m->extra, base.b.y }
	};
	extra = rectnorm(extra);
	if(extra.b.x > both.b.x)
		extra.b.x = both.b.x;

	Rect preview = {
		{ extra.b.x, extra.a.y },
		{ extra.b.x + m->xscale * m->preview, base.b.y }
	};
	preview = rectnorm(preview);
	if(preview.b.x > both.b.x)
		preview.b.x = both.b.x;

	gfxfillrect(g, both, m->cbg);
	gfxfillrect(g, base, m->cbase);
	gfxfillrect(g, extra, m->cextra);
	gfxfillrect(g, preview, m->cpreview);
	gfxdrawrect(g, both, m->cborder);

	return both;
}

Rect meterarea(Meter *m, Point p){
	Rect base = {
		{ p.x, p.y },
		{ p.x + m->xscale * m->base, p.y + m->h }
	};
	Rect extra = {
		{ base.b.x, base.a.y },
		{ base.b.x + m->xscale * m->extra, base.b.y }
	};
	Rect both = {
		base.a,
		{ base.a.x + m->xscale * m->max, extra.b.y }
	};
	return both;
}
