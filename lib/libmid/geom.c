#include "mid.h"

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
	return (Line1d){ .a = r.a.x, .b = r.b.x };
}

Line1d rectprojy(Rect r){
	return (Line1d){ .a = r.a.y, .b = r.b.y };
}

void rectmv(Rect *r, float dx, float dy){
	r->a = (Point){ .x = r->a.x + dx, .y = r->a.y + dy };
	r->b = (Point){ .x = r->b.x + dx, .y = r->b.y + dy };
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
