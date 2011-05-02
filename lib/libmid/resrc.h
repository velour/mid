#if !defined(_RESRC_H_)
#define _RESRC_H_
/* Eventually goes in the library's header file. */

#include "fs.h"

/* Number of items to allow in the cache. */
#define RCACHE_SIZE 100
/* Must be larger than CACHE_SIZE and should be prime. */
#define RESRC_TBL_SIZE 257

struct resrc {
	void *data;
	char file[PATH_MAX + 1];
	bool del;
	int seq;
	int ind;
};

struct rcache {
	struct resrc tbl[RESRC_TBL_SIZE];
	struct resrc *heap[RCACHE_SIZE];
	int fill;
	int nxtseq;
	void*(*load)(const char *path);
	void(*free)(void*);
};

void *resrc(struct rcache *c, const char *file);
void rcache(struct rcache *c, void*(*load)(const char *path),
	    void(*free)(void*));

#endif
