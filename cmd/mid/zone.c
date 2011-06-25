#include <assert.h>
#include "../../include/mid.h"
#include "../../include/log.h"
#include "game.h"

static const char *lvlgenbin = "cmd/lvlgen/lvlgen";

enum { Bufsz = 128 };

Zone *zonegen(int w, int h, int d, int sd)
{
	char cmd[Bufsz];

	int n = snprintf(cmd, Bufsz, "%s %d %d %d %d", lvlgenbin, w, h, d, sd);
	assert(n < Bufsz);

	FILE *fin = popen(cmd, "r");
	if (!fin)
		fatal("Unable to execute zone gen pipeline: %s", miderrstr());

	Zone *z = zoneread(fin);
	if (!z)
		fatal("Failed to load the zone: %s", miderrstr());

	int ret = pclose(fin);
	if (ret == -1)
		fatal("Zone gen pipeline exited with failure: %s", miderrstr());

	return z;
}
