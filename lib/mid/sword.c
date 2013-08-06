/* © 2013 the Mid Authors under the MIT license. See AUTHORS for the list of authors.*/

#include "../../include/mid.h"

static Img *swordsh;
static Img *woosh;

_Bool swordldresrc(void){
	if(!swordsh)
		swordsh = resrcacq(imgs, "img/swords.png", 0);
	if(!woosh)
		woosh = resrcacq(imgs, "img/woosh.png", 0);
	return swordsh != NULL && woosh != NULL;
}

void sworddraw(Gfx *g, Sword *s){
	int pos, wpos;
	Rect *loc;
	if(s->cur == 0){
		pos = 0;
		wpos = -1;
		loc = s->dir == Mvright? s->rightloc : s->leftloc;
	}else if(s->dir == Mvright){
		pos = 1;
		wpos = 0;
		loc = s->rightloc;
	}else{
		pos = 2;
		wpos = 1;
		loc = s->leftloc;
	}

	double y = s->row * 32;
	double x = pos * 32;
	Rect clip = {
		{ x, y },
		{ x + 32, y + 32 }
	};	

	camdrawreg(g, swordsh, clip, loc[s->cur].a);

	if(pos > -1){
		Rect wc = {
			{ wpos * 32, 0 }, //TODO: row should match sword row
			{ wpos *32 + 32, 32 }
		};
		camdrawreg(g, woosh, wc, vecadd(loc[s->cur].a, (Point){0,-16}));
	}
}

Rect swordbbox(Sword *s){
	Rect *loc;
	if(s->cur == 0){
		loc = s->dir == Mvright? s->rightloc : s->leftloc;
		Point ul = vecadd(loc[s->cur].a, (Point){11,0});
		return (Rect){
			ul,
			{ ul.x + 10, ul.y + 32 }
		};
	}

	if(s->dir == Mvright)
		loc = s->rightloc;
	else
		loc = s->leftloc;

	Point ul = vecadd(loc[s->cur].a, (Point){0,-32});
	return (Rect){
		ul,
		{ ul.x + 32, ul.y + 32 + 20 }
	};
}

int swordstr(Sword *s, Player *p){
	return p->stats[StatStr] + p->eqp[StatStr];
}
