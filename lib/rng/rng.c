/* Ranq1, from "Numerical Recipes 3rd edition," Press, Tenkolsky,
   Vetterling and Flannery, 2007.  Technically, the algorithm is
   defined on *unsigned* 64-bit integers. */

#include "../../include/rng.h"
#include <assert.h>

static const uint64_t Mul = UINT64_C(2685821657736338717);
static const uint64_t Vini = UINT64_C(4101842887655102017);
static const double Fl = 5.42101086242752217e-20;

void rngini(Rng *r, uint64_t seed)
{
	r->v = Vini ^ seed;
	r->v = rngint(r);
}

uint64_t rngint(Rng *r)
{
	r->v ^= r->v >> 21;
	r->v ^= r->v << 35;
	r->v ^= r->v >> 4;
	r->v *= Mul;
	return r->v;
}

uint64_t rngintincl(Rng *r, uint64_t min, uint64_t max)
{
	if (min == max)
		return min;

	assert(min >= 0);
	assert(min <= max);

	return rngint(r) % (max - min) + min;
}

double rngdbl(Rng *r)
{
	return (double) rngint(r) * Fl;
}
