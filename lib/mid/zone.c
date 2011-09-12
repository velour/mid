// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stdbool.h>
#include "../../include/mid.h"

_Bool itemscan(char *, Item *);
_Bool itemprint(char *, size_t, Item *);
_Bool envscan(char *, Env *);
_Bool envprint(char *, size_t, Env *);
_Bool enemyscan(char *, Enemy *);
_Bool enemyprint(char *, size_t, Enemy *);

static _Bool readitem(char *buf, Zone *zn);
static _Bool readenv(char *buf, Zone *zn);
static _Bool readenemy(char *buf, Zone *zn);
static _Bool readl(char *buf, int sz, FILE *f);
static _Bool readblkflgs(char *, Lvl *);
static _Bool blkflgszero(Lvl *lvl, int y, int z);
static void writeblkflgs(FILE *, Lvl *);

enum { Bufsz = 256 };

Zone *zoneread(FILE *f)
{
	char buf[Bufsz];
	int itms = 0, envs = 0, enms = 0;

	Zone *zn = xalloc(1, sizeof(*zn));
	zn->lvl = lvlread(f);
	if (!zn->lvl) {
		seterrstr("Failed to read the level: %s", miderrstr());
		return false;
	}

	while (readl(buf, Bufsz, f)) {
		if (buf[0] == '\0')
			continue;
		switch (buf[0]) {
		case 'i':
			if (!readitem(buf+1, zn))
				return NULL;
			itms++;
			break;
		case 'e':
			if (!readenv(buf+1, zn))
				return NULL;
			envs++;
			break;
		case 'n':
			if (!readenemy(buf+1, zn))
				return NULL;
			enms++;
			break;
		case 'f':
			if (!readblkflgs(buf+1, zn->lvl))
				return NULL;
			break;
		default:
			seterrstr("Unexpected input line: [%s]", buf);
			return NULL;
		}
	}

	return zn;
}

static _Bool readitem(char *buf, Zone *zn)
{
	int z, n;
	if (sscanf(buf, "%d%n", &z, &n) != 1) {
		seterrstr("Failed to scan item's z layer");
		return false;
	}

	Item it;
	_Bool ok = itemscan(buf+n, &it);
	if (!ok) {
		seterrstr("Failed to scan item [%s]", buf);
		return false;
	}
	if (!zoneadditem(zn, z, it)) {
		seterrstr("Failed to add item [%s]: too many items", buf);
		return false;
	}
	return true;
}

static _Bool readenv(char *buf, Zone *zn)
{
	int z, n;
	if (sscanf(buf, "%d%n", &z, &n) != 1) {
		seterrstr("Failed to scan env's z layer");
		return false;
	}

	Env env;
	_Bool ok = envscan(buf+n, &env);
	if (!ok) {
		seterrstr("Failed to scan env [%s]", buf);
		return false;
	}
	if (!zoneaddenv(zn, z, env)) {
		seterrstr("Failed to add env [%s]: too many envs", buf);
		return false;
	}
	return true;
}

static _Bool readenemy(char *buf, Zone *zn)
{
	int z, n;
	if (sscanf(buf, "%d%n", &z, &n) != 1) {
		seterrstr("Failed to scan enemy's z layer");
		return false;
	}

	Enemy en;
	_Bool ok = enemyscan(buf+n, &en);
	if (!ok) {
		seterrstr("Failed to scan item [%s]", buf);
		return false;
	}
	if (!zoneaddenemy(zn, z, en)) {
		seterrstr("Failed to add enemy [%s]: too many enemies", buf);
		return false;
	}
	return true;
}

static _Bool readl(char *buf, int sz, FILE *f)
{
	char *r = fgets(buf, sz, f);
	if (!r)
		return 0;

	int l = strlen(buf);

	for(int i = l-1; i >= 0 && isspace(buf[i]); i--)
		buf[i] = 0;

	return 1;
}

void zonewrite(FILE *f, Zone *zn)
{
	lvlwrite(f, zn->lvl);
	writeblkflgs(f, zn->lvl);

	for (int z = 0; z < Maxz; z++) {
		Item *itms = zn->itms[z];
		for (int i = 0; i < Maxitms; i++) {
			if (!itms[i].id)
				continue;
			char buf[Bufsz];
			itemprint(buf, Bufsz, &itms[i]);
			fprintf(f, "i %d %s\n", z, buf);
		}
		Env *envs = zn->envs[z];
		for (int i = 0; i < Maxenvs; i++) {
			if (!envs[i].id)
				continue;
			char buf[Bufsz];
			envprint(buf, Bufsz, &envs[i]);
			fprintf(f, "e %d %s\n", z, buf);
		}
		Enemy *enms = zn->enms[z];
		for (int i = 0; i < Maxenms; i++) {
			if (!enms[i].id)
				continue;
			char buf[Bufsz];
			enemyprint(buf, Bufsz, &enms[i]);
			fprintf(f, "n %d %s\n", z, buf);
		}
	}
}

_Bool zoneadditem(Zone *zn, int z, Item it)
{
	int i;
	Item *itms = zn->itms[z];

	for (i = 0; i < Maxitms && itms[i].id; i++)
		;
	if (i == Maxitms)
		return false;

	zn->itms[z][i] = it;
	return true;
}

_Bool zoneaddenv(Zone *zn, int z, Env env)
{
	int i;
	Env *envs = zn->envs[z];

	for (i = 0; i < Maxenvs && envs[i].id; i++)
		;
	if (i == Maxenvs)
		return false;

	zn->envs[z][i] = env;
	return true;
}

_Bool zoneaddenemy(Zone *zn, int z, Enemy enm)
{
	int i;
	Enemy *enms = zn->enms[z];

	for (i = 0; i < Maxenms && enms[i].id; i++)
		;
	if (i == Maxenms)
		return false;

	zn->enms[z][i] = enm;
	return true;
}

_Bool zoneaddmagic(Zone *zn, int z, Magic mag)
{
	int i;
	Magic *mags = zn->mags[z];

	for (i = 0; i < Maxmagics && mags[i].id; i++)
		;
	if (i == Maxmagics)
		return false;

	mags[i] = mag;
	return true;
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

// Is there a block contained in the area from loc with width-height
// wh that has any of the given flags?
_Bool zonehasflags(Zone *zn, int z, Point loc, Point wh, unsigned int f)
{
	wh.x /= Twidth;
	wh.y /= Theight;
	for (int x = loc.x; x < (int) (loc.x + wh.x + 0.5); x++) {
		if (x < 0 || x >= zn->lvl->w)
			continue;
		for (int y = loc.y; y < (int) (loc.y + wh.y + 0.5); y++) {
			if (y < 0 || y >= zn->lvl->h)
				continue;
			Tileinfo bi = tileinfo(zn->lvl, x, y, z);
			if (bi.flags & f)
				return true;
		}
	}
	return false;
}

_Bool zoneongrnd(Zone *zn, int z, Point loc, Point wh)
{
	wh.x /= Twidth;
	int y = loc.y + wh.y / Theight;
	if (y < 0 || y >= zn->lvl->h)
		return false;

	for (int x = loc.x; x < (int) (loc.x + wh.x + 0.5); x++) {
		if (x < 0 || x >= zn->lvl->w)
			return false;
		if (tileinfo(zn->lvl, x, y, z).flags & Tcollide)
			return true;
	}
	return false;
}

_Bool zoneoverlap(Zone *zn, int z, Point loc, Point wh)
{
	loc.x *= Twidth;
	loc.y *= Theight;
	Rect r = (Rect) { loc, (Point) { loc.x + wh.x, loc.y + wh.y } };

	for (int i = 0; i < Maxitms; i++) {
		Item *it = &zn->itms[z][i];
		if (it->id && isect(r, it->body.bbox))
			return true;
	}
	for (int i = 0; i < Maxenvs; i++) {
		Env *en = &zn->envs[z][i];
		if (en->id && isect(r, en->body.bbox))
			return true;
	}
	for (int i = 0; i < Maxenms; i++) {
		Enemy *en = &zn->enms[z][i];
		if (en->id && isect(r, en->body.bbox))
			return true;
	}

	return false;
}

void zoneupdate(Zone *zn, Player *p, Point *tr)
{
	lvlupdate(zn->lvl);
	playerupdate(p, zn, tr);

	itemupdateanims();

	int z = zn->lvl->z;

	Item *itms = zn->itms[z];
	for(size_t i = 0; i < Maxitms; i++)
		if (itms[i].id) itemupdate(&itms[i], p, zn);

	envupdateanims();

	Env *en = zn->envs[z];
	for(size_t i = 0; i < Maxenvs; i++)
		if (en[i].id) envupdate(&en[i], zn);

	Magic *m = zn->mags[z];
	for(size_t i = 0; i < Maxmagics; i++)
		if(m[i].id > 0) magicupdate(&m[i], zn);

	Enemy *e = zn->enms[z];
	for(size_t i = 0; i < Maxenms; i++) {
		if (!e[i].id)
			continue;
		enemyupdate(&e[i], p, zn);
		if(e[i].hp <= 0)
			enemyfree(&e[i]);
	}
}

void zonedraw(Gfx *g, Zone *zn, Player *p)
{
	int z = zn->lvl->z;

	lvldraw(g, zn->lvl, true);

	Env *en = zn->envs[z];
	for(size_t i = 0; i < Maxenvs; i++)
		if (en[i].id) envdraw(&en[i], g);

	playerdraw(g, p);

	Item *itms = zn->itms[z];
	for(size_t i = 0; i < Maxitms; i++)
		if (itms[i].id) itemdraw(&itms[i], g);

	Magic *m = zn->mags[z];
	for(size_t i = 0; i < Maxmagics; i++)
		if(m[i].id > 0) magicdraw(g, &m[i]);

	Enemy *e = zn->enms[z];
	for(size_t i = 0; i < Maxenms; i++)
		enemydraw(&e[i], g);

	lvldraw(g, zn->lvl, false);

}

static void writeblkflgs(FILE *f, Lvl *lvl)
{
	for (int z = 0; z < lvl->d; z++) {
	for (int y = 0; y < lvl->h; y++) {
		if (blkflgszero(lvl, y, z))
			continue;

		fprintf(f, "f %u %u", z, y);
		for (int x = 0; x < lvl->w; x++)
			fprintf(f, " %u", blk(lvl, x, y, z)->flags);
		fputc('\n', f);
	}
	}
}

static _Bool readblkflgs(char *buf, Lvl *lvl)
{
	int z, y, n;

	if (sscanf(buf, " %u %u%n", &z, &y, &n) != 2) {
		seterrstr("Failed to read flag row");
		return false;
	}
	buf += n;

	for (int x = 0; x < lvl->w; x++) {
		unsigned int flgs = 0;
		if (sscanf(buf, " %u%n", &flgs, &n) != 1) {
			seterrstr("Failed to read flags for block %u, %u, %u", x, y, z);
			return false;
		}
		blk(lvl, x, y, z)->flags = flgs;
		buf += n;
	}
	return true;
}

static _Bool blkflgszero(Lvl *lvl, int y, int z)
{
	for (int x = 0; x < lvl->w; x++) {
		if (blk(lvl, x, y, z)->flags != 0)
			return false;
	}

	return true;
}
