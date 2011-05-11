/* A resource table finds and tracks resource usage (reference
 * counts).  A finite number of unreferenced resources are kept in a
 * simple cache with a FIFO-ish replacement policy. */
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "fs.h"
#include "../../include/mid.h"

enum { Initsize = 257 };
enum { Fillfact = 3 };
enum { Cachesize = 100 };

static const char *roots[] = { "resrc" };
enum { NROOTS = sizeof(roots) / sizeof(roots[0]) };

typedef struct Resrc Resrc;
struct Resrc {
	void *resrc, *aux;
	char file[PATH_MAX + 1], path[PATH_MAX + 1];
	int refs;
	Resrc *nxt;
	Resrc *unxt;
};

struct Rtab {
	Resrc **tbl;
	int sz, fill;
	Resrc *cache[Cachesize]; /* cache of unused nodes. */
	int cfill;
	Resrcops *ops;
};

/* From K&R 2nd edition. */
unsigned int strhash(const char *s)
{
	unsigned int h;
	for (h = 0; *s != '\0'; s += 1)
		h = *s + 31 * h;
	return h;
}

unsigned int hash(Resrcops *ops, const char *file, void *aux)
{
	if (ops->hash)
		return ops->hash(file, aux);
	return strhash(file);
}

bool eq(Resrcops *ops, Resrc *r, const char *file, void *aux)
{
	if (ops->eq)
		return ops->eq(r->aux, aux) && strcmp(r->file, file) == 0;
	return strcmp(r->file, file) == 0;
}

static Resrc *tblfind(Resrcops *ops, Resrc *tbl[], int sz,
		      const char *file, void *aux)
{
	if (sz == 0)
		return NULL;

	unsigned int i = hash(ops, file, aux) % sz;
	Resrc *p = NULL;
	for (p = tbl[i]; p; p = p->nxt)
		if (eq(ops, p, file, aux))
			break;
	return p;
}

static void tblrem(Resrcops *ops, Resrc *tbl[], int sz, Resrc *rm)
{
	unsigned int i = hash(ops, rm->file, rm->aux) % sz;
	if (tbl[i] == rm) {
		tbl[i] = rm->nxt;
		return;
	}
	Resrc *p;
	for (p = tbl[i]; p->nxt && p->nxt != rm; p = p->nxt)
		;
	if (!p->nxt)
		abort();
	if (p->nxt == rm)
		p->nxt = rm->nxt;
}

static void tblins(Resrcops *ops, Resrc *tbl[], int sz, Resrc *r)
{
	unsigned int i = hash(ops, r->file, r->aux) % sz;
	assert (!r->nxt);
	r->nxt = tbl[i];
	tbl[i] = r;
}

static void rtabgrow(Rtab *t)
{
	int nxtsz = t->sz * 2;
	if (nxtsz == 0)
		nxtsz = Initsize;
	Resrc **nxttbl = calloc(nxtsz, sizeof(*nxttbl));
	for (int i = 0; i < t->sz; i++) {
		for (Resrc *p = t->tbl[i]; p; p = p->nxt)
			tblins(t->ops, nxttbl, nxtsz, p);
	}
	if (t->tbl)
		free(t->tbl);
	t->tbl = nxttbl;
	t->sz = nxtsz;
}

static void cacheresrc(Rtab *t, Resrc *r)
{
	if (t->cfill == Cachesize) {
		Resrc *bump = t->cache[0];
		t->cache[0] = t->cache[t->cfill];
		t->cfill--;
		tblrem(t->ops, t->tbl, t->sz, bump);
		t->ops->unload(bump->path, bump->resrc, bump->aux);
		free(bump);
	}
	assert (t->cfill < Cachesize);
	t->cache[t->fill] = r;
	t->fill++;
}

static Resrc *resrcnew(const char *path, const char *file, void *aux)
{
	Resrc *r = calloc(1, sizeof(*r));
	if (!r)
		return NULL;
	strncpy(r->file, file, PATH_MAX + 1);
	strncpy(r->path, path, PATH_MAX + 1);
	r->aux = aux;

	return r;
}

static Resrc *resrcload(Rtab *t, const char *file, void *aux)
{
	char path[PATH_MAX + 1];
	bool found = false;
	for (int i = 0; !found && i < NROOTS; i += 1) {
		if (fsfind(roots[i], file, path))
			found = true;
	}
	if (!found)
		return NULL;

	Resrc *r = resrcnew(path, file, aux);
	if (!r)
		return NULL;
	r->resrc = t->ops->load(path, aux);
	t->fill++;
	if (t->fill * Fillfact >= t->sz)
		rtabgrow(t);
	tblins(t->ops, t->tbl, t->sz, r);

	return r;
}

void *resrcacq(Rtab *t, const char *file, void *aux)
{
	Resrc *r = tblfind(t->ops, t->tbl, t->sz, file, aux);
	if (!r)
		r = resrcload(t, file, aux);
	r->refs++;
	return r->resrc;
}

void resrcrel(Rtab *t, const char *file, void *aux)
{
	Resrc *r = tblfind(t->ops, t->tbl, t->sz, file, aux);
	if (!r)
		abort();
	r->refs--;
	if (r->refs == 0)
		cacheresrc(t, r);
}

Rtab *rtabnew(Resrcops *ops)
{
	Rtab *t = calloc(1, sizeof(*t));
	if (!t)
		return NULL;

	t->ops = ops;

	return t;
}

void rtabfree(Rtab *t)
{
	for (int i = 0; i < t->sz; i += 1) {
		Resrc *p, *q;
		for (p = q = t->tbl[i]; p; p = q) {
			t->ops->unload(p->path, p->resrc, p->aux);
			q = p->nxt;
			free(p);
		}
	}
	free(t);
}
