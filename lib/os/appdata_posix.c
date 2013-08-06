/* © 2013 the Mid Authors under the MIT license. See AUTHORS for the list of authors.*/

#include <stdlib.h>
#include <stdio.h>
#include "../../include/os.h"
#include <assert.h>

static char path[256];

const char *appdata(const char *prog){
	if(path[0] == 0){
		char *home = getenv("HOME");
		if (!home)
			home = ".";
		int n = snprintf(path, sizeof(path), "%s/.%s", home, prog);
		assert(n > 0);
	}
	return path;
}
