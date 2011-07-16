// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
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
