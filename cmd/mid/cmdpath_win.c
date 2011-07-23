// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int cmdpath(char *out, int sz, char *cmd){
	int n = snprintf(out, sz, "cmd\\%s\\%s.exe", cmd, cmd);
	assert(n > 0);

	struct _stat s;
	if(_stat(out, &s) != -1)
		return n;

	n = strlen(cmd);
	strcpy(out, cmd);
	return n;
}
