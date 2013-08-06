/* © 2013 the Mid Authors under the MIT license. See AUTHORS for the list of authors.*/

#undef __STRICT_ANSI__
#include <stdio.h>
#include "../../include/os.h"

FILE *piperead(const char *cmd){
	return _popen(cmd, "r");
}

int pipeclose(FILE *p){
	return _pclose(p);
}
