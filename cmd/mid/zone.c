// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include "../../include/mid.h"
#include "../../include/log.h"
#include "../../include/rng.h"
#include "game.h"

enum { Bufsz = 256 };

typedef struct Pipe {
	int n;
	char cmd[Bufsz];
} Pipe;

static FILE *inzone = NULL;

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
		fatal("Failed to load the zone: %s", miderrstr());
	zclose(fin);

	return z;
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
	pipeadd(&p, "./cmd/lvlgen/lvlgen -s %u 50 50 5", rngint(r));
	pipeadd(&p, " | ./cmd/itmgen/itmgen -s %u 1 1", rngint(r));
	pipeadd(&p, " | ./cmd/itmgen/itmgen -s %u 2 50", rngint(r));
	pipeadd(&p, " | ./cmd/envgen/envgen -s %u 1 1", rngint(r));
	pipeadd(&p, " | ./cmd/enmgen/enmgen -s %u 1 50", rngint(r));

	if (debugging) {
		pipeadd(&p, "| tee cur.lvl");
		pr("lvlgen pipeline: [%s]", p.cmd);
	}

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
