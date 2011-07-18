// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
#include "../../include/mid.h"

static Img *swordsh;

_Bool swordldresrc(void){
	if(!swordsh)
		swordsh = resrcacq(imgs, "img/swords.png", 0);
	return swordsh != NULL;
}

void sworddraw(Gfx *g, Sword *s){
	int pos;
	Rect *loc;
	if(s->cur == 0){
		pos = 0;
		loc = s->dir == Mvright? s->rightloc : s->leftloc;
	}else if(s->dir == Mvright){
		pos = 1;
		loc = s->rightloc;
	}else{
		pos = 2;
		loc = s->leftloc;
	}

	double y = s->row * 32;
	double x = pos * 32;
	Rect clip = {
		{ x, y },
		{ x + 32, y + 32 }
	};	

	camdrawreg(g, swordsh, clip, loc[s->cur].a);
}

Rect swordbbox(Sword *s){
	Rect *loc;
	if(s->cur == 0)
		loc = s->dir == Mvright? s->rightloc : s->leftloc;
	else if(s->dir == Mvright)
		loc = s->rightloc;
	else
		loc = s->leftloc;
	return loc[s->cur];
}
