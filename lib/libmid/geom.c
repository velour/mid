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

/* Trace the movement of a along the vector v (dx, dy).  If a will
 * intersect with b, then the result is the new vector (dx, dy) that
 * respects the collision. */
Point recttrace1(Rect a, Point v, Rect b)
{
	Point d = rectdist(a, b);
	if (d.x * v.x < 0 || d.y * v.y < 0) /* signs differ, no hit. */
		return v;

	/* OK, we may collide so slide the rect along the path to see
	 * when it hits (if ever). */
	d = (Point) { 0, 0 };
	int max = v.x > v.y ? ceil(v.x) : ceil(v.y);
	for (int i = 0; i < max; i++) {
		if (v.x < 0 && d.x > v.x)
			d.x -= 1.0;
		else if (v.x > 0 && d.x < v.x)
			d.x += 1.0;
		if (v.y < 0 && d.y > v.y)
			d.y -= 1.0;
		else if (v.y > 0 && d.x < v.y)
			d.y += 1.0;
		Rect nxt = a;
		rectmv(&nxt, d.x, d.y);
		if (isect(nxt, b))
			break;
		a = nxt;
	}

	if (fabs(v.x) < fabs(d.x))
		d.x = v.x;
	if (fabs(v.y) < fabs(d.y))
		d.y = v.y;
	return d;
}
