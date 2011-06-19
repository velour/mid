#include "../../include/mid.h"
#include "../../include/log.h"
#include "game.h"
#include <stdlib.h>

Icon *iconnew(const char *name, const char *resrc)
{
	Icon *item = malloc(sizeof(*item));
	if (!item)
		fatal("malloc failed: %s", miderrstr());

	item->icon = resrcacq(anims, resrc, NULL);
	if (!item->icon)
		fatal("Failed to load icon for item %s: %s", name, miderrstr());
	item->name = name;

	return item;
}

void iconfree(Icon *item)
{
	free(item);
}

Icon *invmod(Inv *inv, Icon *new, int x, int y)
{
	int i = x * Invcols + y;
	Icon *old = inv->items[i];
	inv->items[i] = new;
	return old;
}
