#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fs.h"

/* Number of items to allow in the cache. */
#define CACHE_SIZE 100
/* Must be larger than CACHE_SIZE and should be prime. */
#define TBL_SIZE 257

static const char *roots[] = { ".", "resrc" };
static const int nroots = sizeof(roots) / sizeof(roots[0]);

struct resrc {
	void *data;
	char file[PATH_MAX + 1];
	bool del;
	int seq;
	int ind;
};

struct rcache {
	struct resrc tbl[TBL_SIZE];
	struct resrc *heap[CACHE_SIZE];
	int fill;
	int nxtseq;
	void*(*load)(const char *path);
	void(*free)(void*);
};

static void swap(struct resrc *heap[], int i, int j)
{
	struct resrc *t = heap[i];
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

static int pullup(struct resrc *heap[], int i)
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

static int pushdown(struct resrc *heap[], int fill, int i)
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

static void heappush(struct resrc *heap[], int fill, struct resrc *r)
{
	if (fill >= CACHE_SIZE) {
		fprintf(stderr, "%s: Heap is full", __func__);
		abort();
	}
	heap[fill] = r;
	r->ind = fill;
	pullup(heap, fill);
}

static struct resrc *heappop(struct resrc *heap[], int fill)
{
	if (fill <= 0) {
		fprintf(stderr, "%s: Heap is empty\n", __func__);
		abort();
	}
	struct resrc *e = heap[0];
	e->ind = -1;
	heap[0] = heap[fill - 1];
	pushdown(heap, fill - 1, 0);
	return e;
}

static void heapupdate(struct resrc *heap[], int fill, int i)
{
	i = pullup(heap, i);
	pushdown(heap, fill, i);
}

/* Change this to use the K&R string hash. */
unsigned int hash(const char *str)
{
	unsigned int h = 0;
	unsigned int base = 1;

	for (const char *p = str; p < str + strlen(str); p += 1) {
		h += *p * base;
		base *= base;
	}

	return h;
}

static bool used(struct resrc *r)
{
	return r->file[0] != '\0';
}

static struct resrc *tblfind(struct resrc tbl[], const char *file)
{
	unsigned int init = hash(file) % TBL_SIZE;
	unsigned int i = init;

	do {
		if (!used(&tbl[i]))
			break;
		else if (!tbl[i].del && strcmp(tbl[i].file, file) == 0)
			return &tbl[i];
		i = (i + 1) % TBL_SIZE;
	} while (i != init);

	return NULL;
}

static struct resrc *tblalloc(struct resrc tbl[], const char *file)
{
	unsigned int i;
	for (i = hash(file) % TBL_SIZE; used(&tbl[i]); i  = (i + 1) % TBL_SIZE)
		;
	assert(!used(&tbl[i]) || tbl[i].del);

	return &tbl[i];
}

static int nextseq(struct rcache *c)
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

static void *load(struct rcache *c, const char *file)
{
	char path[PATH_MAX + 1];
	if (c->fill == CACHE_SIZE) {
		struct resrc *bump = heappop(c->heap, c->fill);
		c->free(bump->data);
		bump->del = true;
		c->fill -= 1;
	}
	for (int i = 0; i < nroots; i += 1) {
		if (fsfind(roots[i], file, path)) {
			struct resrc *r = tblalloc(c->tbl, file);
			r->del = false;
			strncpy(r->file, path, PATH_MAX + 1);
			r->file[PATH_MAX] = '\0';
			r->data = c->load(path);
			r->seq = nextseq(c);
			heappush(c->heap, c->fill, r);
			return r->data;
		}
	}
	return NULL;
}

void *resrc(struct rcache *c, const char *file)
{
	struct resrc *r = tblfind(c->tbl, file);
	assert(!r->del);
	if (!r) {
		return load(c, file);
	} else {
		r->seq = nextseq(c);
		heapupdate(c->heap, c->fill, r->ind);
		return r->data;
	}
}

void rcache(struct rcache *c, void*(*load)(const char *path),
	    void(*free)(void*))
{
	for (int i = 0; i < TBL_SIZE; i += 1) {
		c->tbl[i].del = false;
		c->tbl[i].file[0] = '\0';
	}
	c->fill = 0;
	c->nxtseq = 1;
	c->load = load;
	c->free = free;
}


