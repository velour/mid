#include "../../include/mid.h"
#include <math.h>

static int between(float min, float max, float n){
	return n >= min && n <= max;
}

float isection1d(Line1d a, Line1d b){
	if(between(b.a, b.b, a.b))
		return a.b - b.a;
	else if(between(a.a, a.b, b.b))
		return b.b - a.a;
	else
		return -1.0;
}

Line1d rectprojx(Rect r){
	if(r.a.x < r.b.x)
		return (Line1d){ .a = r.a.x, .b = r.b.x };
	else
		return (Line1d){ .a = r.b.x, .b = r.a.x };
}

Line1d rectprojy(Rect r){
	if(r.a.y < r.b.y)
		return (Line1d){ .a = r.a.y, .b = r.b.y };
	else
		return (Line1d){ .a = r.b.y, .b = r.a.y };
}

void ptmv(Point *p, float dx, float dy) {
	p->x += dx;
	p->y += dy;
}

void rectmv(Rect *r, float dx, float dy){
	ptmv(&r->a, dx, dy);
	ptmv(&r->b, dx, dy);
}

Isect isection(Rect a, Rect b){
	float ix = isection1d(rectprojx(a), rectprojx(b));
	if(ix >= 0.0){
		float iy = isection1d(rectprojy(a), rectprojy(b));
		if(iy >= 0.0)
			return (Isect){ .is = 1, .dx = ix, .dy = iy };
		else
			return (Isect){ .is = 0 };
	}else
		return (Isect){ .is = 0 };
}

int isect(Rect a, Rect b){
	return isection(a, b).is;
}

Isect minisect(Rect a, Rect b){
	Isect i = isection(a, b);
	if(!i.is)
		return i;
	else if(i.dx < i.dy)
		return (Isect){ .is = 1, .dx = i.dx, .dy = -1.0 };
	else if(i.dy < i.dx)
		return (Isect){ .is = 1, .dx = -1.0, .dy = i.dy };
	else
		return i;
}

/* Makes point a the min,min and point b the max,max. */
Rect rectnorm(Rect r)
{
	if (r.a.x > r.b.x) {
		float t = r.a.x;
		r.a.x = r.b.x;
		r.b.x = t;
	}
	if (r.a.y > r.b.y) {
		float t = r.a.y;
		r.a.y = r.b.y;
		r.b.y = t;
	}
	return r;
}

/* Distance a would have to travel to intersect b. */
Point rectdist(Rect a, Rect b)
{
	a = rectnorm(a);
	b = rectnorm(b);
	float dx = 0.0, dy = 0.0;

	if (a.a.x > b.b.x) {
		dx = b.b.x - a.a.x;
	} else if (a.b.x < b.a.x) {
		dx = b.a.x - a.b.x;
	}
	if (a.a.y > b.b.y) {
		dy = b.b.y - a.a.y;
	} else if (a.b.y < b.a.y) {
		dy = b.a.y - a.b.y;
	}
	return (Point){ dx, dy };
}

Point recttrace1(Rect a, Point v, Rect b)
{
	int steps = fabs(v.x) > fabs(v.y) ? fabs(v.x) : fabs(v.y);
	float xstep = v.x < 0 ? -1.0 : (v.x > 0 ? 1.0 : 0.0);
	float ystep = v.y < 0 ? -1.0 : (v.y > 0 ? 1.0 : 0.0);
	Point r = (Point) { 0, 0 };
	for (int i = 0; i < steps; i++) {
		r.x += xstep;
		r.y += ystep;
		rectmv(&a, xstep, ystep);
		Isect is = minisect(a, b);
		if (is.is) {
			if (is.dx > 0) {
				r.x += xstep > 0 ? -is.dx : is.dx;
				xstep = 0.0;
			}
			if (is.dy > 0) {
				r.y += ystep > 0 ? -is.dy : is.dy;
				ystep = 0.0;
			}
		}
		if (ystep == 0 && xstep == 0)
			break;
	}

	return r;
}
