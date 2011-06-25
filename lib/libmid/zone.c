#include <stdlib.h>
#include "../../include/mid.h"

Zone *zoneread(FILE *f)
{
	Zone *z = xalloc(1, sizeof(*z));
	z->lvl = lvlread(f);
	return z;
}

void zonefree(Zone *z)
{
	lvlfree(z->lvl);
	free(z);
}
