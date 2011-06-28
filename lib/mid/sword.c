#include "../../include/mid.h"

void sworddraw(Gfx *g, Sword *s, Point tr){
	imgdraw(g, s->img[s->cur], vecadd(s->loc[s->cur].a, tr));
}
