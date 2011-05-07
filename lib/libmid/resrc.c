#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "fs.h"
#include "../../include/mid.h"

enum {
/* Number of items to allow in the cache. */
	RCACHE_SIZE = 100,
/* Must be larger than CACHE_SIZE and should be prime. */
	RESRC_TBL_SIZE = 257,
};

static const char *roots[] = { "resrc" };
enum { NROOTS = sizeof(roots) / sizeof(roots[0]) };

typedef struct Resrc Resrc;
struct Resrc {
	/* The loaded resource. */
	void *resrc;
	/* Extra info used to key on the resource. */
	void *info;
	char file[PATH_MAX + 1];
	_Bool del;
	int seq;
	int ind;
};

struct Rcache {
	Resrc tbl[RESRC_TBL_SIZE];
	Resrc *heap[RCACHE_SIZE];
	int fill;
	int nxtseq;
	void*(*load)(const char*, void*);
	void(*free)(void*, void*);
	unsigned int (*hash)(const char*, void*);
	bool (*eq)(void*, void*);
};

static void swap(Resrc *heap[], int i, int j)
{
	Resrc *t = heap[i];
	heap[j] = heap[i];
	heap[i] = t;
	heap[j]->ind = j;
	heap[i]->ind = i;
}

static int left(int i)
{
	return 2 * i + 1;
}

static int right(int i)
{
	return 2 * i + 2;
}

static int parent(int i)
{
	return (i - 1) / 2;
}

static int pullup(Resrc *heap[], int i)
{
	if (i > 0) {
		int p = parent(i);
		if (heap[i]->seq < heap[p]->seq) {
			swap(heap, i, p);
			pullup(heap, p);
		}
	}
	return i;
}

static int pushdown(Resrc *heap[], int fill, int i)
{
	int l = left(i), r = right(i);
	int small = i;
	if (l < fill && heap[l]->seq < heap[i]->seq)
		small = l;
	if (r < fill && heap[r]->seq < heap[small]->seq)
		small = r;
	if (i != small) {
		swap(heap, i, small);
		pushdown(heap, fill, small);
	}
	return i;
}

static void heappush(Resrc *heap[], int fill, Resrc *r)
{
	assert(fill < RCACHE_SIZE);

	heap[fill] = r;
	r->ind = fill;
	pullup(heap, fill);
}

static Resrc *heappop(Resrc *heap[], int fill)
{
	assert(fill > 0);

	Resrc *e = heap[0];
	e->ind = -1;
	heap[0] = heap[fill - 1];
	pushdown(heap, fill - 1, 0);
	return e;
}

static void heapupdate(Resrc *heap[], int fill, int i)
{
	i = pullup(heap, i);
	pushdown(heap, fill, i);
}

/* From K&R 2nd edition. */
unsigned int strhash(const char *s)
{
	unsigned int h;
	for (h = 0; *s != '\0'; s += 1)
		h = *s + 31 * h;
	return h;
}

static bool used(Resrc *r)
{
	return r->file[0] != '\0';
}

unsigned int hash(Rcache *c, const char *file, void *info)
{
	if (c->hash)
		return c->hash(file, info);
	return strhash(file);
}

bool eq(Rcache *c, Resrc *r, const char *file, void *info)
{
	if (c->eq)
		return c->eq(r->info, info) && strcmp(r->file, file) == 0;
	return strcmp(r->file, file) == 0;
}

static Resrc *tblfind(Rcache *c, const char *file, void *info)
{
	unsigned int init = hash(c, file, info) % RESRC_TBL_SIZE;
	unsigned int i = init;

	do {
		if (!used(&c->tbl[i]))
			break;
		else if (!c->tbl[i].del && eq(c, &c->tbl[i], file, info))
			return &c->tbl[i];
		i = (i + 1) % RESRC_TBL_SIZE;
	} while (i != init);

	return NULL;
}

static Resrc *tblalloc(Rcache *c, const char *file, void *info)
{
	unsigned int i;
	for (i = hash(c, file, info) % RESRC_TBL_SIZE;
	     used(&c->tbl[i]);
	     i  = (i + 1) % RESRC_TBL_SIZE)
		;
	assert(!used(&c->tbl[i]) || c->tbl[i].del);

	return &c->tbl[i];
}

static int nextseq(Rcache *c)
{
	int prev = c->nxtseq;
	c->nxtseq += 1;
	if (c->nxtseq < prev) {
		if (c->fill == 0) {
			c->nxtseq = 1;
			goto out;
		}
		unsigned int max = 0;
		unsigned int min = c->heap[0]->seq;
		for (int i = 0; i < c->fill; i += 1) {
			c->heap[i]->seq -= min;
			if (c->heap[i]->seq > max)
				max = c->heap[i]->seq;
		}
		c->nxtseq = max + 2;
	}
out:
	return c->nxtseq - 1;
}

static void *load(Rcache *c, const char *file, void *info)
{
	char path[PATH_MAX + 1];
	if (c->fill == RCACHE_SIZE) {
		Resrc *bump = heappop(c->heap, c->fill);
		c->free(bump->resrc, bump->info);
		bump->del = true;
		c->fill -= 1;
	}
	for (int i = 0; i < NROOTS; i += 1) {
		if (fsfind(roots[i], file, path)) {
			Resrc *r = tblalloc(c, file, info);
			r->del = false;
			strncpy(r->file, file, PATH_MAX + 1);
			r->file[PATH_MAX] = '\0';
			r->resrc = c->load(path, info);
			r->info = info;
			r->seq = nextseq(c);
			heappush(c->heap, c->fill, r);
			c->fill += 1;
			return r->resrc;
		}
	}
	return NULL;
}

void *resrc(Rcache *c, const char *file, void *info)
{
	Resrc *r = tblfind(c, file, info);
	if (!r) {
		return load(c, file, info);
	} else {
		assert(!r->del);
		r->seq = nextseq(c);
		heapupdate(c->heap, c->fill, r->ind);
		return r->resrc;
	}
}

Rcache *rcachenew(void*(*load)(const char*, void*),
		  void(*free)(void*, void*),
		  unsigned int (*hash)(const char*, void*),
		  _Bool (*eq)(void*, void*))
{
	Rcache *c = malloc(sizeof(*c));
	if (!c)
		return NULL;

	for (int i = 0; i < RESRC_TBL_SIZE; i += 1) {
		c->tbl[i].del = false;
		c->tbl[i].file[0] = '\0';
	}
	c->fill = 0;
	c->nxtseq = 1;
	c->load = load;
	c->free = free;
	c->hash = hash;
	c->eq = eq;

	return c;
}

void rcachefree(Rcache *c)
{
	for (int i = 0; i < RCACHE_SIZE; i += 1) {
		if (used(&c->tbl[i]))
			c->free(c->tbl[i].resrc, c->tbl[i].info);
	}
	free(c);
}
