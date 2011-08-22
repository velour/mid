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
#include <stdio.h>
#include "../../include/os.h"

extern int cmdpath(char *buf, int sz, char *cmd);

static const char *zonedir = "_zones";
enum { Bufsz = 1024 };

typedef struct Pipe {
	int n;
	char cmd[Bufsz];
} Pipe;

static FILE *inzone = NULL;

static void ensuredir();
static char *zonefile(int);
static FILE *zpipe(Rng *r);
static void pipeadd(struct Pipe *, char *, char *, ...);

void zoneloc(const char *p)
{
	zonedir = p;
}

void zonestdin()
{
	inzone = stdin;
}

Zone *zonegen(Rng *r)
{
	ignframetime();
	FILE *fin = inzone;

	if (!fin)
		fin = zpipe(r);
	Zone *z = zoneread(fin);
	if (!z)
		die("Failed to read the zone: %s", miderrstr());

	if (inzone) {
		fclose(fin);
		inzone = NULL;
	} else {
		int ret = pipeclose(fin);
		if (ret == -1)
			die("Zone gen pipeline exited with failure: %s", miderrstr());
	}

	return z;
}

Zone *zoneget(int znum)
{
	ignframetime();

	char *zfile = zonefile(znum);
	FILE *f = fopen(zfile, "r");
	if (!f)
		die("Unable to open the zone file [%s]: %s", zfile, miderrstr());

	Zone *zn = zoneread(f);
	if (!zn)
		die("Failed to read the zone file [%s]: %s", zfile, miderrstr());

	fclose(f);
	return zn;
}

void zoneput(Zone *zn, int znum)
{
	ignframetime();
	ensuredir();
	
	char *zfile = zonefile(znum);
	FILE *f = fopen(zfile, "w");
	if (!f)
		die("Failed to open zone file for writing [%s]: %s", zfile, miderrstr());

	zonewrite(f, zn);
	fclose(f);
}

Tileinfo zonedstairs(Zone *zn)
{
	Tileinfo bi;

	for (int z = 0; z < zn->lvl->d; z++) {
	for (int x = 0; x < zn->lvl->w; x++) {
	for (int y = 0; y < zn->lvl->h; y++) {
		bi = tileinfo(zn->lvl, x, y, z);
		if (bi.flags & Tdown)
			return bi;
	}
	}
	}
	fatal("No down stairs found in this zone");
}

void zonecleanup(int zmax)
{
	for (int i = 0; i <= zmax; i++) {
		char *zfile = zonefile(i);
		if (!fsexists(zfile))
			continue;
		if (unlink(zfile) < 0)
			pr("Failed to remove zone file [%s]: %s", zfile, miderrstr());
	}
}

// Non re-entrant
static char *zonefile(int znum)
{
	static char zfile[Bufsz];
	snprintf(zfile, Bufsz, "%s/%d.zone", zonedir, znum);
	return zfile;
}

static void ensuredir()
{
	struct stat sb;
	if (stat(zonedir, &sb) < 0) {
		if (makedir(zonedir) < 0)
			die("Failed to make the zone directory [%s]: %s", zonedir, miderrstr());
	} else if (!S_ISDIR(sb.st_mode)) {
		die("Zone directory [%s] is not a directory", zonedir);
	}
}

static FILE *zpipe(Rng *r)
{
	Pipe p = {};
	pipeadd(&p, "lvlgen", "-s %u 25 25 3", rngint(r));
	pipeadd(&p, "itmgen", "-s %u 1 1", rngint(r));
	pipeadd(&p, "itmgen", "-s %u 2 2 2 2 2 4 4 5 6 7 7 7 7 7 50", rngint(r));
	pipeadd(&p, "itmgen", "-s %u 3 3 3 3 6 10", rngint(r));
	pipeadd(&p, "envgen", "-s %u 1 1", rngint(r));
	pipeadd(&p, "enmgen", "-s %u 1 1 1 2 2 2 2 3 3 4 5 50", rngint(r));

	char adc[256];
	if(snprintf(adc, sizeof(adc), "\"%s/cur.lvl\"", zonedir) == -1)
		die("Failed to create cur.lvl path: %s", miderrstr());
	pipeadd(&p, "tee", adc);

	p.cmd[p.n - 3] = 0;

	pr("lvlgen pipeline: [%s]", p.cmd);

	FILE *fin = piperead(p.cmd);
	if (!fin)
		die("Unable to execute zone gen pipeline: %s", miderrstr());

	return fin;
}

static void pipeadd(Pipe *p, char *cmd, char *fmt, ...)
{
	char buf[Bufsz];
	va_list ap;

	va_start(ap, fmt);
	int n = vsnprintf(buf, Bufsz, fmt, ap);
	va_end(ap);

	char cmdbuf[Bufsz];
	int cmdsz = cmdpath(cmdbuf, Bufsz, cmd);

	if ((n+4+cmdsz) > Bufsz - p->n)
		fatal("Buffer is too small");

	strcat(p->cmd + p->n, cmdbuf);
	strcat(p->cmd + p->n + cmdsz, " ");
	strcat(p->cmd + p->n + cmdsz + 1, buf);
	strcat(p->cmd + p->n + cmdsz + 1 + n, " | ");
	p->n += n + cmdsz + 4;
}
