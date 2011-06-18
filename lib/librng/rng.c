/* Ranq1, from "Numerical Recipes 3rd edition," Press, Tenkolsky,
   Vetterling and Flannery, 2007.  Technically, the algorithm is
   defined on *unsigned* 64-bit integers. */

#include "../../include/rng.h"

static const uint64_t Mul = UINT64_C(2685821657736338717);
static const uint64_t Vini = UINT64_C(4101842887655102017);
static const double Fl = 5.42101086242752217e-20;

Rng rngnew(uint64_t seed)
{
	return (Rng) { .v = rngint((Rng) { .v = Vini ^ seed }) };
}

uint64_t rngint(Rng r)
{
	r.v ^= r.v >> 21;
	r.v ^= r.v << 35;
	r.v ^= r.v >> 4;
	r.v *= Mul;
	return r.v;
}

double rngdbl(Rng r)
{
	return (double) rngint(r) * Fl;
}
