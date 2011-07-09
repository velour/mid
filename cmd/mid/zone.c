#include <assert.h>
#include "../../include/mid.h"
#include "../../include/log.h"
#include "game.h"

enum { Bufsz = 256 };

static const char *lvlpipe =
"cmd/lvlgen/lvlgen %d %d %d %d\
 | cmd/itmgen/itmgen %d 1\
 | cmd/itmgen/itmgen %d 50\
 | cmd/envgen/envgen 1 1\
 | cmd/enmnear/enmnear 1";

Zone *zonegen(int w, int h, int d, int sd)
{
	char cmd[Bufsz];

	int n = snprintf(cmd, Bufsz, lvlpipe, w, h, d, sd, ItemStatup, ItemCopper);
	assert(n < Bufsz);
	if (debugging) {
		n -= snprintf(cmd + n, Bufsz - n, " | tee cur.lvl");
		pr("lvlgen pipeline: [%s]", cmd);
	}

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
