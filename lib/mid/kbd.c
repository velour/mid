#include "../../include/mid.h"
#include <SDL/SDL.h>
#include <assert.h>
#include <string.h>

static char *names[] = {
	[Mvleft] = "left",
	[Mvright] = "right",
	[Mvjump] = "jump",
	[Mvact] = "act",
	[Mvinv] = "inv",
	[Mvsword] = "sword",
};

_Bool keymapread(char km[], char *fname){
	FILE *f = fopen(fname, "r");
	if(!f)
		return 1;

	char act[8];
	char k;
	while(fscanf(f, "%7s %c", act, &k) != EOF){
		for(int i = Mvleft; i < Nactions; i++)
			if(strcmp(act, names[i]) == 0)
				km[i] = k;
	}

	if(ferror(f)){
		fclose(f);
		return 1;
	}
	fclose(f);
	return 0;
}

_Bool keymapwrite(char km[], char *fname){
	FILE *f = fopen(fname, "w");
	if(!f)
		return 1;

	for(int i = Mvleft; i < Nactions; i++)
		fprintf(f, "%s %c\n", names[i], km[i]);

	return fclose(f);
}

char kmap[] = {
	[Mvleft] = 's',
	[Mvright] = 'f',
	[Mvact] = 'u',
	[Mvjump] = 'e',
	[Mvinv] = 'n',
	[Mvsword] = 'j',
};