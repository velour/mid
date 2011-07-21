#include <stdio.h>
#include "../../include/os.h"
#include <sys/stat.h>

int makedir(const char *dir){
	return mkdir(dir, 0700);
}
