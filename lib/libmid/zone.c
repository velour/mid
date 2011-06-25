#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stdbool.h>
#include "../../include/mid.h"
#include "../../include/log.h"

_Bool itemscan(char *buf, Item *it);
_Bool itemprint(char *buf, size_t sz, Item *it);

static void readitem(char *buf, Zone *zn);
static char *getzlayer(char *buf, int *z);
static _Bool readl(char *buf, int sz, FILE *f);

enum { Bufsz = 256 };

Zone *zoneread(FILE *f)
{
	char buf[Bufsz];
	int itms = 0;

	Zone *zn = xalloc(1, sizeof(*zn));
	zn->lvl = lvlread(f);
	if (!zn->lvl)
		fatal("Failed to read the level: %s", miderrstr());

	while (readl(buf, Bufsz, f)) {
		if (buf[0] == '\0')
			continue;
		switch (buf[0]) {
		case 'i':
			readitem(buf+1, zn);
			itms++;
			break;
		case 'e':
			fatal("Reading envs is not yet implemented");
		case 'n':
			fatal("Reading enemies is not yet implemented");
		default:
			fatal("Unexpected input line: [%s]", buf);
		}
	}

	if (debugging)
		pr("Loaded %d items", itms);

	return zn;
}

static void readitem(char *buf, Zone *zn)
{
	int z, n;
	if (sscanf(buf, "%d%n", &z, &n) != 1)
		fatal("Failed to scan item's z layer");

	Item it;
	_Bool ok = itemscan(buf+n, &it);
	if (!ok)
		fatal("Failed to scan item [%s]", buf);
	zoneadditem(zn, z, it);
}

static _Bool readl(char *buf, int sz, FILE *f)
{
	char *r = fgets(buf, sz, f);
	if (!r)
		return 0;

	int l = strlen(buf);
	if (buf[l-1] != '\n')
		fatal("Line buffer is too small");

	buf[l-1] = '\0';

	return 1;
}

void zonewrite(FILE *f, Zone *zn)
{
	lvlwrite(f, zn->lvl);

	for (int z = 0; z < Maxz; z++) {
		Item *itms = zn->itms[z];
		for (int i = 0; i < Maxitms; i++) {
			if (!itms[i].id)
				continue;
			char buf[Bufsz];
			itemprint(buf, Bufsz, &itms[i]);
			fprintf(f, "i %d %s\n", z, buf);
		}
	}
}

void zoneadditem(Zone *zn, int z, Item it)
{
	int i;
	Item *itms = zn->itms[z];

	for (i = 0; i < Maxitms && itms[i].id; i++)
		;
	if (i == Maxitms)
		fatal("Too many items");

	zn->itms[z][i] = it;
}

void zonefree(Zone *z)
{
	lvlfree(z->lvl);
	free(z);
}

int zonelocs(Zone *zn, int z, _Bool (*p)(Zone *, int, Point), Point pts[], int sz)
{
	int n = 0;
	Point pt;

	for (pt.x = 0; pt.x < zn->lvl->w; pt.x++) {
	for (pt.y = 0; pt.y < zn->lvl->h; pt.y++) {
		if (n >= sz || !p(zn, z, pt))
			continue;
		pts[n] = pt;
		n++;
	}
	}

	return n;
}

void zoneupdate(Zone *zn, Player *p, Point *tr)
{
	int z = zn->lvl->z;

	lvlupdate(zn->lvl);
	playerupdate(p, zn->lvl, tr);

	itemupdateanims();

	Item *itms = zn->itms[z];
	for(size_t i = 0; i < Maxitms; i++)
		if (itms[i].id) itemupdate(&itms[i], p, zn->lvl);

	envupdateanims();

	Env *en = zn->envs[z];
	for(size_t i = 0; i < Maxenvs; i++)
		if (en[i].id) envupdate(&en[i], zn->lvl);

	Enemy *e = zn->enms[z];
	for(size_t i = 0; i < Maxenms; i++) {
		if (!e[i].mt)
			continue;
		e[i].mt->update(&e[i], p, zn->lvl);
		if(e[i].hp <= 0){
			e[i].mt->free(&e[i]);
			e[i].mt = 0;
		}
	}
}

void zonedraw(Gfx *g, Zone *zn, Player *p, Point tr)
{
	int z = zn->lvl->z;

	lvldraw(g, zn->lvl, true, tr);

	Env *en = zn->envs[z];
	for(size_t i = 0; i < Maxenvs; i++)
		if (en[i].id) envdraw(&en[i], g, tr);

	playerdraw(g, p, tr);

	Item *itms = zn->itms[z];
	for(size_t i = 0; i < Maxitms; i++)
		if (itms[i].id) itemdraw(&itms[i], g, tr);

	Enemy *e = zn->enms[z];
	for(size_t i = 0; i < Maxenms; i++)
		if (e[i].mt) e[i].mt->draw(&e[i], g, tr);

	lvldraw(g, zn->lvl, false, tr);

}

