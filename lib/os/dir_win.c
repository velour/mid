#include <stdio.h>
#include "../../include/os.h"
#include <dirent.h>

int makedir(const char *dir){
	return _mkdir(dir);
}
