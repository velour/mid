#include "../../include/mid.h"
#include "game.h"
#include <stdio.h>
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
		else if(strcmp(act, "forward") == 0)
			km[Mvfwd] = k;
		else if(strcmp(act, "back") == 0)
			km[Mvbak] = k;
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
	[Mvbak] = 'r',
	[Mvfwd] = 'w',
	[Mvjump] = 'e',
	[Mvinv] = 'n',
};
