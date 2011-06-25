#include <stdlib.h>
#include "../../include/mid.h"

Item *itemread(FILE *);
Item *itemwrite(FILE *, Item *);
Item *envread(FILE *);
Item *envwrite(FILE *, Env *);
Item *enemyread(FILE *);
Item *enemywrite(FILE *, Enemy *);

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
