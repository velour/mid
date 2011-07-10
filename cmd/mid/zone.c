#include <assert.h>
#include "../../include/mid.h"
#include "../../include/log.h"
#include "game.h"

static FILE *inzone = NULL;

static FILE *zfile(int sd);
static FILE *zpipe(int sd);
static void zclose(FILE *f);

void zonestdin()
{
	inzone = stdin;
}

Zone *zonegen(int sd)
{
	FILE *fin = zfile(sd);
	Zone *z = zoneread(fin);
	if (!z)
		fatal("Failed to load the zone: %s", miderrstr());
	zclose(fin);

	return z;
}

enum { Bufsz = 256 };

static FILE *zfile(int sd)
{
	if (inzone)
		return inzone;
	return zpipe(sd);
}


static FILE *zpipe(int sd)
{
	static const char *lvlpipe =
		"cmd/lvlgen/lvlgen 50 50 5 %d"
		" | cmd/itmgen/itmgen 1 1" /* ItemStatup */
		" | cmd/itmgen/itmgen 2 50" /* ItemCopper */
		" | cmd/envgen/envgen 1 1" /* EnvShrempty */
		" | cmd/enmnear/enmnear 1" /* EnemyUnti */
		;
	char cmd[Bufsz];

	int n = snprintf(cmd, Bufsz, lvlpipe, sd, ItemStatup, ItemCopper);
	assert(n < Bufsz);
	if (debugging) {
		n -= snprintf(cmd + n, Bufsz - n, " | tee cur.lvl");
		pr("lvlgen pipeline: [%s]", cmd);
	}

	FILE *fin = popen(cmd, "r");
	if (!fin)
		fatal("Unable to execute zone gen pipeline: %s", miderrstr());

	return fin;
}

static void zclose(FILE *f)
{
	if (inzone) {
		fclose(f);
		inzone = NULL;
		return;
	}

	int ret = pclose(f);
	if (ret == -1)
		fatal("Zone gen pipeline exited with failure: %s", miderrstr());
}