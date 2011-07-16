// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
#include "../../include/mid.h"

void sworddraw(Gfx *g, Sword *s){
	camdrawimg(g, s->img[s->cur], s->loc[s->cur].a);
}
