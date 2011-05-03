#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
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

static const char *roots[] = { ".", "resrc" };
enum { NROOTS = sizeof(roots) / sizeof(roots[0]) };

typedef struct Resrc Resrc;
struct Resrc {
	void *data;
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
	void*(*load)(const char *path);
	void(*free)(void*);
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
	if (fill >= RCACHE_SIZE) {
		fprintf(stderr, "%s: Heap is full", __func__);
		abort();
	}
	heap[fill] = r;
	r->ind = fill;
	pullup(heap, fill);
}

static Resrc *heappop(Resrc *heap[], int fill)
{
	if (fill <= 0) {
		fprintf(stderr, "%s: Heap is empty\n", __func__);
		abort();
	}
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
unsigned int hash(const char *s)
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

static Resrc *tblfind(Resrc tbl[], const char *file)
{
	unsigned int init = hash(file) % RESRC_TBL_SIZE;
	unsigned int i = init;

	do {
		if (!used(&tbl[i]))
			break;
		else if (!tbl[i].del && strcmp(tbl[i].file, file) == 0)
			return &tbl[i];
		i = (i + 1) % RESRC_TBL_SIZE;
	} while (i != init);

	return NULL;
}

static Resrc *tblalloc(Resrc tbl[], const char *file)
{
	unsigned int i;
	for (i = hash(file) % RESRC_TBL_SIZE; used(&tbl[i]); i  = (i + 1) % RESRC_TBL_SIZE)
		;
	assert(!used(&tbl[i]) || tbl[i].del);

	return &tbl[i];
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

static void *load(Rcache *c, const char *file)
{
	char path[PATH_MAX + 1];
	if (c->fill == RCACHE_SIZE) {
		Resrc *bump = heappop(c->heap, c->fill);
		c->free(bump->data);
		bump->del = true;
		c->fill -= 1;
	}
	for (int i = 0; i < NROOTS; i += 1) {
		if (fsfind(roots[i], file, path)) {
			Resrc *r = tblalloc(c->tbl, file);
			r->del = false;
			strncpy(r->file, file, PATH_MAX + 1);
			r->file[PATH_MAX] = '\0';
			r->data = c->load(path);
			r->seq = nextseq(c);
			heappush(c->heap, c->fill, r);
			c->fill += 1;
			return r->data;
		}
	}
	return NULL;
}

void *resrc(Rcache *c, const char *file)
{
	Resrc *r = tblfind(c->tbl, file);
	if (!r) {
		return load(c, file);
	} else {
		assert(!r->del);
		r->seq = nextseq(c);
		heapupdate(c->heap, c->fill, r->ind);
		return r->data;
	}
}

Rcache *rcachenew(void*(*load)(const char *path), void(*free)(void*))
{
	Rcache *c = malloc(sizeof(*c));
	if (!c) {
		perror("malloc");
		fprintf(stderr, "%s: failed to allocate cache\n", __func__);
		return NULL;
	}
	for (int i = 0; i < RESRC_TBL_SIZE; i += 1) {
		c->tbl[i].del = false;
		c->tbl[i].file[0] = '\0';
	}
	c->fill = 0;
	c->nxtseq = 1;
	c->load = load;
	c->free = free;

	return c;
}

void rcachefree(Rcache *c)
{
	free(c);
}


