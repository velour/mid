/* © 2013 the Mid Authors under the MIT license. See AUTHORS for the list of authors.*/

#include "../../include/mid.h"
#include <stdlib.h>

void *xalloc(unsigned long n, unsigned long s){
	void *v = calloc(n, s);
	if(!v){
		fputs("out of memory\n", stderr);
		exit(1);
	}
	return v;
}

void xfree(void *v){
	free(v);
}
