#include <stdio.h>
#include "../../include/os.h"

FILE *piperead(const char *cmd){
	return popen(cmd, "r");
}

int pipeclose(FILE *p){
	return pclose(p);
}
