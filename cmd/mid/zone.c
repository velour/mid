#include <assert.h>
#include "../../include/mid.h"
#include "../../include/log.h"
#include "game.h"

enum { Bufsz = 128 };

static const char *lvlpipe =
	"cmd/lvlgen/lvlgen %d %d %d %d | cmd/itmnear/itmnear 1 1\
 | cmd/itmnear/itmnear 2 5";

Zone *zonegen(int w, int h, int d, int sd)
{
	char cmd[Bufsz];

	int n = snprintf(cmd, Bufsz, lvlpipe, w, h, d, sd);
	assert(n < Bufsz);
	if (debugging)
		pr("lvlgen pipeline: [%s]", cmd);

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
