/* © 2013 the Mid Authors under the MIT license. See AUTHORS for the list of authors.*/

#include <stdio.h>
#include "../../include/rng.h"

int main()
{
	Rng r;
	rngini(&r, 8008);
	printf("%lu\n", rngint(&r));
	printf("%g\n", rngdbl(&r));
	return 0;
}
