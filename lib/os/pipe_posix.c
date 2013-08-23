/* © 2013 the Mid Authors under the MIT license. See AUTHORS for the list of authors.*/

#define _POSIX_SOURCE
#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include "../../include/os.h"

FILE *piperead(const char *cmd){
	return popen(cmd, "r");
}

int pipeclose(FILE *p){
	return pclose(p);
}
