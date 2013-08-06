/* © 2013 the Mid Authors under the MIT license. See AUTHORS for the list of authors.*/

#include "../../include/mid.h"

Rect meterdraw(Gfx *g, Meter *m, Point p){
	Rect base = {
		{ p.x, p.y },
		{ p.x + m->xscale * m->base, p.y + m->h }
	};
	Rect extra = {
		{ base.b.x, base.a.y },
		{ base.b.x + m->xscale * m->extra, base.b.y }
	};
	extra = rectnorm(extra);
	Rect both = {
		base.a,
		{ base.a.x + m->xscale * m->max, extra.b.y }
	};

	gfxfillrect(g, both, m->cbg);
	gfxfillrect(g, base, m->cbase);
	gfxfillrect(g, extra, m->cextra);
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
