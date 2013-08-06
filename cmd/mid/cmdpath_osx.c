/* © 2013 the Mid Authors under the MIT license. See AUTHORS for the list of authors.*/

#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int cmdpath(char *out, int sz, char *cmd){
	int n = snprintf(out, sz, "./cmd/%s/%s", cmd, cmd);
	assert(n > 0);

	struct stat s;
	if(stat(out, &s) != -1)
		return n;

	n = snprintf(out, sz, "./%s", cmd);
	assert(n > 0);

	if(stat(out, &s) != -1)
		return n;

	n = strlen(cmd);
	strcpy(out, cmd);
	return n;
}
