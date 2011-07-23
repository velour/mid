// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
#include <stdbool.h>
#include "lvlgen.h"
#include "../../include/mid.h"

static bool withprob(double pr);

static const double Prob = 0.33;

void water(Lvl *lvl)
{
	if (!withprob(Prob))
		return;

	unsigned int ht = rnd(1, lvl->h - 2);

	for (int x = 0; x < lvl->w - 1; x++) {
	for (int y = lvl->h - 2; y > lvl->h - 2 - ht; y--) {
		Blk *b = blk(lvl, x, y, lvl->z);
		int t = b->tile;
		switch (t) {
		case ' ':
			b->tile = 'w'; break;
		case '>':
			b->tile = ')'; break;
		case '<':
			b->tile = '('; break;
		}
	}
	}
}

enum { Mult = 1000 };

static bool withprob(double pr)
{
	return rnd(0, Mult) < pr * Mult;
}
