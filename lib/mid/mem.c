// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
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
