#include "../../include/mid.h"
#include "../../include/log.h"
#include "../../include/rng.h"
#include <sys/times.h>
#include <stdlib.h>
#include <limits.h>

enum { Startx = 2, Starty = 2 };

static int idargs(int argc, char *argv[], int **ids);
static _Bool goodloc(Zone *, int, Point);

int main(int argc, char *argv[])
{
	int *ids;
	int n = idargs(argc, argv, &ids);

	loginit(NULL);

	if (argc < 3)
		fatal("usage: itmgen <ID>+ <num>");

	long num = strtol(argv[argc-1], NULL, 10);
	if (num == LLONG_MIN || num == LLONG_MAX)
			fatal("Invalid number: %s", argv[argc-1]);

	Rng r;
	struct tms tm;
	clock_t seed = times(&tm);
	pr("itmgen seed = %lu", (unsigned long) seed);
	rngini(&r, seed);

	Zone *zn = zoneread(stdin);

	/* Find all valid locations. */
	int sz = zn->lvl->w * zn->lvl->h;
	Point pts[zn->lvl->d][sz];
	int npts[zn->lvl->d];
	for (int z = 0; z < zn->lvl->d; z++) {
		npts[z] = zonelocs(zn, z, goodloc, pts[z], sz);
		if (!npts[z])
			fatal("No available locations at layer %d", z);
	}

	/* Scramble locations */
	for (int z = 0; z < zn->lvl->d; z++) {
	for (int i = 0; i < npts[z]; i++) {
		if (npts[z] == 0)
			continue;
		int j = rngintincl(&r, 0, npts[z]-1);
		Point t = pts[z][i];
		pts[z][i] = pts[z][j];
		pts[z][j] = t;
	}
	}

	/* Place items */
	for (long i = 0; i < num; i++) {
		int z = rngintincl(&r, 0, zn->lvl->d - 1);
		int idind = rngintincl(&r, 0, n);
		Item it;
		iteminit(&it, ids[idind], pts[z][npts[z]-1]);
		npts[z]--;
		zoneadditem(zn, z, it);
	}

	zonewrite(stdout, zn);
	zonefree(zn);
	xfree(ids);

	return 0;
}

static int idargs(int argc, char *argv[], int *ids[])
{
	int i;

	*ids = xalloc(argc, sizeof(*ids));
	for (i = 1; i < argc - 1; i++) {
		long l = strtol(argv[i], NULL, 10);
		if (l == LLONG_MIN || l == LLONG_MAX)
			fatal("Invalid number: %s", argv[i]);
		(*ids)[i-1] = l;
	}

	return i-1;
}

static _Bool goodloc(Zone *zn, int z, Point pt)
{
	return (pt.x != Startx || pt.y != Starty)
		&& zonefits(zn, z, pt, (Point) { Twidth, Theight })
		&& zoneonground(zn, z, pt, (Point) { Twidth, Theight })
		&& !zoneoverlap(zn, z, pt, (Point) { Twidth, Theight });
}
