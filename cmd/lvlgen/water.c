/* © 2013 the Mid Authors under the MIT license. See AUTHORS for the list of authors.*/

#include <stdbool.h>
#include "lvlgen.h"
#include "../../include/mid.h"

static bool withprob(double pr);

static const double Prob = 0.33;

void water(Lvl *lvl)
{
	for (int z = 0; z < lvl->d; z++) {
		if (!withprob(Prob))
			continue;

		unsigned int ht = rnd(1, lvl->h - 2);

		for (int x = 0; x < lvl->w - 1; x++) {
		for (int y = lvl->h - 2; y > lvl->h - 2 - ht; y--) {
			Blk *b = blk(lvl, x, y, z);
			if (b->tile == ' ')
				b->tile = 'w';
		}
		}
	}
}

enum { Mult = 1000 };

static bool withprob(double pr)
{
	return rnd(0, Mult) < pr * Mult;
}
