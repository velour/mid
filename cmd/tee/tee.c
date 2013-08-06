/* © 2013 the Mid Authors under the MIT license. See AUTHORS for the list of authors.*/

#include <stdio.h>

enum { Maxouts = 64 };
FILE *outs[64];
char buf[1024];

int main(int argc, char *argv[]){
	for(int i = 1; i < argc && i < Maxouts; i++){
		outs[i-1] = fopen(argv[i], "w");
		if(!outs[i-1]){
			perror("Failed to open output");
			return 1;
		}
	}

	while(fgets(buf, sizeof(buf), stdin) != NULL){
		fputs(buf, stdout);
		for(FILE **f = outs; *f != NULL; f++)
			fputs(buf, *f);
	}

	for(FILE **f = outs; *f != NULL; f++)
		fclose(*f);

	return 0;
}
