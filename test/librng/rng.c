#include <stdio.h>
#include "../../include/rng.h"

int main()
{
	Rng r = rngnew(8008);
	printf("%lu\n", rngint(r));
	printf("%g\n", rngdbl(r));
	return 0;
}
