#include <assert.h>
#include "../../include/mid.h"
#include "../../include/log.h"
#include "game.h"

static const char *lvlgenbin = "cmd/lvlgen/lvlgen";

enum { Bufsz = 128 };

Lvl *lvlgen(int w, int h, int d, int sd)
{
	char cmd[Bufsz];

	int n = snprintf(cmd, Bufsz, "%s %d %d %d %d", lvlgenbin, w, h, d, sd);
	assert(n < Bufsz);

	FILE *fin = popen(cmd, "r");
	if (!fin)
		fatal("Unable to execute level generator: %s", miderrstr());

	Lvl *l = lvlread(fin);
	if (!l)
		fatal("Failed to load the level: %s", miderrstr());

	int ret = pclose(fin);
	if (ret == -1)
		fatal("Level generator exited with failure: %s", miderrstr());

	return l;
}
