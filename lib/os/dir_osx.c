/* © 2013 the Mid Authors under the MIT license. See AUTHORS for the list of authors.*/

#include <stdio.h>
#include "../../include/os.h"
#include <sys/stat.h>

int makedir(const char *dir){
	return mkdir(dir, 0700);
}
