#include "../../include/log.h"
#include "../../include/mid.h"
#include "game.h"
#include <stdlib.h>
#include <stdbool.h>
#include <sys/times.h>

Gfx *gfx;

bool init()
{
	struct tms tm;
	srand(times(&tm));

	loginit(0);

	pr("%s", "Let's rock.");

	gfx = gfxinit(Scrnw, Scrnh, "MID");
	if(!gfx)
		return false;

	if(!sndinit())
		return false;

	initresrc();

	return true;
}

void deinit()
{
	freeresrc();
	sndfree();
	gfxfree(gfx);
}

int main(int argc, char *argv[])
{
	char *kmname = NULL;

#	define ARGIS(a) argv[i][0] == '-' && argv[i][1] == a

	for(int i = 1; i < argc; i++){
	if(ARGIS('k')){
		if(i + 1 == argc)
			die("k requires a filename", miderrstr());
		kmname = argv[i+1];
		i++;
	}else if(ARGIS('d')){
		debugging++;
	}else if (ARGIS('p')){
		zonestdin();
	}else if(ARGIS('m')){
		mute = 1;
	}
	}

	if (!init())
		fatal("Failed to initialize: %s", miderrstr());


	if(kmname && keymapread(kmap, kmname))
		die("failed to read %s", kmname);

	Scrnstk *stk = scrnstknew();
	scrnstkpush(stk, titlescrnnew(gfx));

	scrnrun(stk, gfx);
	pr("Mean frame time: %g ms", meanftime);

	deinit();
	return 0;
}
