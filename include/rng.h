/* © 2013 the Mid Authors under the MIT license. See AUTHORS for the list of authors.*/

#include <stdint.h>

typedef struct Rng Rng;
struct Rng {
	uint64_t v;
};

void rnginit(Rng *r, uint64_t seed);
uint64_t rngint(Rng *r);
uint64_t rngintincl(Rng*, uint64_t min, uint64_t max);
double rngdbl(Rng *r);
