/* © 2013 the Mid Authors under the MIT license. See AUTHORS for the list of authors.*/

#include <stdio.h>
#include "../../include/os.h"

FILE *piperead(const char *cmd){
	return popen(cmd, "r");
}

int pipeclose(FILE *p){
	return pclose(p);
}
