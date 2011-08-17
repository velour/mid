#include <stdio.h>
#include "../../include/os.h"
#include <shlobj.h>
#include <stdlib.h>

static char path[256];

const char *appdata(const char *prog){
	if(path[0] != 0)
		return path;

	char ad[MAX_PATH];
	SHGetFolderPathA(0, CSIDL_APPDATA, 0, 0, ad);
	snprintf(path, sizeof(path), "%s\\%s", ad, prog);
	return path;
}
