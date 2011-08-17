#include <stdio.h>
#include "../../include/os.h"

static char path[256];

const char *appdata(const char *prog){
	if(path[0] == 0){
		int n = snprintf(path, sizeof(path), "~/.%s", prog);
		assert(n > 0);
	}
	return path;
}
