#include "../../include/mid.h"
#include <string.h>

_Bool keymapread(char km[], char *fname){
	FILE *f = fopen(fname, "r");
	if(!f)
		return 1;

	char act[8];
	char k;
	while(fscanf(f, "%7s %c", act, &k) != EOF){
		if(strcmp(act, "left") == 0)
			km[Mvleft] = k;
		else if(strcmp(act, "right") == 0)
			km[Mvright] = k;
		else if(strcmp(act, "jump") == 0)
			km[Mvjump] = k;
		else if(strcmp(act, "door") == 0)
			km[Mvdoor] = k;
		else if(strcmp(act, "inv") == 0)
			km[Mvinv] = k;
	}

	if(ferror(f)){
		fclose(f);
		return 1;
	}
	fclose(f);
	return 0;
}

char kmap[] = {
	[Mvleft] = 's',
	[Mvright] = 'f',
	[Mvdoor] = 'r',
	[Mvjump] = 'e',
	[Mvinv] = 'n',
};
