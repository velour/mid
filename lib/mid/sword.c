#include "../../include/mid.h"

void sworddraw(Gfx *g, Sword *s){
	camdrawimg(g, s->img[s->cur], s->loc[s->cur].a);
}
