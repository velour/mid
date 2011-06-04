#include "../../include/mid.h"
#include "../../include/log.h"
#include "game.h"
#include <stdlib.h>

Item *itemnew(const char *name, const char *resrc)
{
	Item *item = malloc(sizeof(*item));
	if (!item)
		fatal("malloc failed: %s", miderrstr());

	item->icon = resrcacq(anim, resrc, NULL);
	if (!item->icon)
		fatal("Failed to load icon for item %s: %s", name, miderrstr());
	item->name = name;

	return item;
}

void itemfree(Item *item)
{
	free(item);
}

Item *invmod(Inv *inv, Item *new, int x, int y)
{
	int i = x * Invcols + y;
	Item *old = inv->items[i];
	inv->items[i] = new;
	return old;
}
