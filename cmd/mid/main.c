// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
#include "../../include/log.h"
#include "../../include/mid.h"
#include "../../include/os.h"
#include "game.h"
#include <stdlib.h>
#include <stdbool.h>

Gfx *gfx;

static void usage(int);

bool init()
{
	if(debugging)
		loginit(0);
	else{
		const char *ad = appdata("mid");
		makedir(ad);
		static char adm[256];
		if(snprintf(adm, sizeof(adm), "%s/debug.log", ad) == -1)
			exit(1);
		if(loginit(adm))
			exit(2);
		if(snprintf(adm, sizeof(adm), "%s/zones", ad) == -1)
			die("Failed to make zoneloc path: %s", miderrstr());
		makedir(adm);
		zoneloc(adm);
	}

	pr("%s", "Let's rock.");

	gfx = gfxinit(Scrnw, Scrnh, "MID");
	if(!gfx)
		return false;

	if(!sndinit())
		return false;

	char vloc[256];
	snprintf(vloc, sizeof(vloc), "%s/vol.txt", appdata("mid"));
	if(sndread(vloc))
		pr("Volume not loaded: %s", miderrstr());

	initresrc();

	return true;
}

void deinit()
{
	freeresrc();
	sndfree();
	gfxfree(gfx);
	logclose();
}

int main(int argc, char *argv[])
{
	char *kmname = NULL;

#	define ARGIS(a) argv[i][0] == '-' && argv[i][1] == a && argv[i][2] == 0

	for(int i = 1; i < argc; i++){	
		if(ARGIS('d')){
			debugging++;
		}else if (ARGIS('h')){
			usage(0);
		}else if(ARGIS('k')){
			if(i + 1 == argc)
				usage(1);
			kmname = argv[i+1];
			i++;
		}else if(ARGIS('m')){
			mute = 1;
		}else if (ARGIS('p')){
			zonestdin();
		}
	}

	if (!init())
		fatal("Failed to initialize: %s", miderrstr());

	if(kmname && keymapread(kmap, kmname))
		die("failed to read %s", kmname);
	else{
		char kloc[256];
		snprintf(kloc, sizeof(kloc), "%s/keys.txt", appdata("mid"));
		if(keymapread(kmap, kloc))
			pr("Keymap not loaded (%s), using defaults.", miderrstr());
	}

	Scrnstk *stk = scrnstknew();
	scrnstkpush(stk, titlescrnnew(gfx));

	scrnrun(stk, gfx);
	pr("Mean frame time: %g ms", meanftime);
	scrnstkfree(stk);

	deinit();
	return 0;
}

static void usage(int s)
{
	puts("Usage: mid [-d] [-h] [-k <file>] [-m] [-p]");
	puts("-d	enable debugging");
	puts("-h	print usage information");
	puts("-k <file>	specify the key map file");
	puts("-m	mute the sound effects");
	puts("-p	accept the pipeline from standard input");
	exit(s);
}