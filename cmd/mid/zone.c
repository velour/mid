// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../../include/mid.h"
#include "../../include/log.h"
#include "../../include/rng.h"
#include "game.h"

static const char *zonedir = "_zones";
enum { Bufsz = 1024 };

typedef struct Pipe {
	int n;
	char cmd[Bufsz];
} Pipe;

static FILE *inzone = NULL;

static void ensuredir();
static FILE *zfile(Rng *r);
static FILE *zpipe(Rng *r);
static void zclose(FILE *f);
static void pipeinit(struct Pipe *);
static void pipeadd(struct Pipe *, char *, ...);

void zonestdin()
{
	inzone = stdin;
}

Zone *zonegen(Rng *r)
{
	FILE *fin = zfile(r);
	Zone *z = zoneread(fin);
	if (!z)
		fatal("Failed to read the zone: %s", miderrstr());

	zclose(fin);
	return z;
}

Zone *zoneget(int znum)
{
	char zfile[Bufsz];
	snprintf(zfile, Bufsz, "%s/%d.zone", zonedir, znum);

	FILE *f = fopen(zfile, "r");
	if (!f)
		fatal("Unable to open the zone file [%s]: %s", zfile, miderrstr());

	Zone *zn = zoneread(f);
	if (!zn)
		fatal("Failed to read the zone file [%s]: %s", zfile, miderrstr());

	fclose(f);
	return zn;
}

void zoneput(Zone *zn, int znum)
{
	ensuredir();
	
	char zfile[Bufsz];
	snprintf(zfile, Bufsz, "%s/%d.zone", zonedir, znum);

	FILE *f = fopen(zfile, "w");
	if (!f)
		fatal("Failed to open zone file for writing [%s]: %s", zfile, miderrstr());

	zonewrite(f, zn);
	fclose(f);
}

Blkinfo zonedstairs(Zone *zn)
{
	Blkinfo bi;

	for (int z = 0; z < zn->lvl->d; z++) {
	for (int x = 0; x < zn->lvl->w; x++) {
	for (int y = 0; y < zn->lvl->h; y++) {
		bi = blkinfo(zn->lvl, x, y, z);
		if (bi.flags & Tiledown)
			return bi;
	}
	}
	}
	fatal("No down stairs found in this zone");
}

static void ensuredir()
{
	struct stat sb;
	if (stat(zonedir, &sb) < 0) {
		if (mkdir(zonedir, 0700) < 0)
			fatal("Failed to make the zone directory [%s]: %s", zonedir, miderrstr());
	} else if (!S_ISDIR(sb.st_mode)) {
		fatal("Zone directory [%s] is not a directory", zonedir);
	}
}

static FILE *zfile(Rng *r)
{
	if (inzone)
		return inzone;
	return zpipe(r);
}

static FILE *zpipe(Rng *r)
{
	Pipe p;
	pipeinit(&p);
	pipeadd(&p, "./cmd/lvlgen/lvlgen -s %u 25 25 3", rngint(r));
	pipeadd(&p, " | ./cmd/itmgen/itmgen -s %u 1 1", rngint(r));
	pipeadd(&p, " | ./cmd/itmgen/itmgen -s %u 2 50", rngint(r));
	pipeadd(&p, " | ./cmd/itmgen/itmgen -s %u 3 10", rngint(r));
	pipeadd(&p, " | ./cmd/envgen/envgen -s %u 1 1", rngint(r));
	pipeadd(&p, " | ./cmd/enmgen/enmgen -s %u 1 50", rngint(r));

	pipeadd(&p, "| tee cur.lvl");
	pr("lvlgen pipeline: [%s]", p.cmd);

	FILE *fin = popen(p.cmd, "r");
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

static void pipeinit(Pipe *p)
{	
	memset(p, 0, sizeof(p));
}

static void pipeadd(Pipe *p, char *fmt, ...)
{
	char buf[Bufsz];
	va_list ap;

	va_start(ap, fmt);
	int n = vsnprintf(buf, Bufsz, fmt, ap);
	va_end(ap);

	if (n > Bufsz - p->n)
		fatal("Buffer is too small");

	strncat(p->cmd + p->n, buf, n);
	p->n += n;
}
