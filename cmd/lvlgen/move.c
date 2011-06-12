#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "../../include/mid.h"
#include "lvlgen.h"

static Move movemk(Mvspec s);
static int locs(Mvspec s, Loc l0, bool (*p)(char c), Loc l[], int sz);

static Mvspec specs[] = {
	{ .blks = "se"
                  "##",
	  .wt = 1, .w = 2, .h = 2, },

	{ .blks = "es"
                  "##",
	  .wt = 1, .w = 2, .h = 2, },

	{ .blks = "s e"
                  "###",
	  .wt = 2, .w = 3, .h = 2 },

	{ .blks = "e s"
                  "###",
	  .wt = 2, .w = 3, .h = 2 },

	{ .blks = "s  e"
                  "####",
          .wt = 2, .w = 4, .h = 2 },

	{ .blks = "e  s"
                  "####",
          .wt = 2, .w = 4, .h = 2 },

	{ .blks = "    ."
                  "     "
                  "s   e"
		  "#...#",
          .wt = 1, .h = 4, .w = 5 },

	{ .blks = ".    "
                  "     "
                  "e   s"
		  "#...#",
          .wt = 1, .h = 4, .w = 5 },

	{ .blks = " e"
                  "s#"
		  "#.",
          .wt = 1, .w = 2, .h = 3 },

	{ .blks = " e"
                  "s#"
		  "##",
          .wt = 1, .w = 2, .h = 3 },

	{ .blks = " s"
                  "e#"
		  "#.",
          .wt = 1, .w = 2, .h = 3 },

	{ .blks = " s"
                  "e#"
		  "##",
          .wt = 1, .w = 2, .h = 3 },

	{ .blks = "s "
                  "#e"
		  ".#",
          .wt = 1, .w = 2, .h = 3 },

	{ .blks = "s "
                  "#e"
		  "##",
          .wt = 1, .w = 2, .h = 3 },

	{ .blks = "e "
                  "#s"
		  ".#",
          .wt = 1, .w = 2, .h = 3 },

	{ .blks = "e "
                  "#s"
		  "##",
          .wt = 1, .w = 2, .h = 3 },

	{ .blks = ". ."
                  "  e"
                  "  #"
                  "s.."
		  "#..",
          .wt = 2, .w = 3, .h = 5 },

	{ .blks = ". ."
	          "  e"
                  "  #"
                  "s##"
		  "###",
          .wt = 1, .w = 3, .h = 5 },

	{ .blks = "  s"
                  " .#"
                  "e.."
		  "#..",
          .wt = 2, .w = 3, .h = 4 },

	{ .blks = "  s"
                  " .#"
                  "e##"
		  "###",
          .wt = 1, .w = 3, .h = 4 },

	{ .blks = ". ."
	          "e  "
                  "#  "
                  "..s"
                  "..#",
          .wt = 2, .w = 3, .h = 5 },

	{ .blks = "e  "
                  "#  "
                  "##s"
                  "###",
          .wt = 1, .w = 3, .h = 4 },

	{ .blks = "s  "
                  "#. "
                  "..e"
                  "..#",
          .wt = 2, .w = 3, .h = 4 },

	{ .blks = "s  "
                  "#. "
                  "##e"
                  "###",
          .wt = 1, .w = 3, .h = 4 },

};

static const int Nspecs = sizeof(specs) / sizeof(specs[0]);

Move *moves;
int Nmoves;

void movesini(void)
{
	for (int i = 0; i < Nspecs; i++)
		Nmoves += specs[i].wt;

	moves = xalloc(Nmoves, sizeof(*moves));

	int nxt = 0;
	for (int i = 0; i < Nspecs; i++) {
		Mvspec s = specs[i];
		for (int j = 0; j < s.wt; j++) {
			moves[nxt] = movemk(s);
			nxt++;
		}
	}
	assert(nxt == Nmoves);
}

bool isclr(char c)
{
	return c == ' ' || c == 's' || c == 'e';
}

bool isblkd(char c)
{
	return c == '#';
}

bool isstart(char c)
{
	return c == 's';
}

bool isend(char c)
{
	return c == 'e';
}

static Move movemk(Mvspec s)
{
	Loc start, end;
	locs(s, (Loc){0, 0}, isstart, &start, 1);
	locs(s, (Loc){0, 0}, isend, &end, 1);

	Move mv;
	mv.wt = s.wt;
	mv.dx = end.x - start.x;
	mv.dy = end.y - start.y;
	mv.spec = s;
	mv.nclr = locs(s, start, isclr, mv.clr, Maxblks);
	mv.nblkd = locs(s, start, isblkd, mv.blkd, Maxblks);

	return mv;
}

// Get the locations of the various blocks matching character 'ch'.
static int locs(Mvspec s, Loc l0, bool (*p)(char c), Loc l[], int sz)
{

	int i = 0;

	for (int x = 0; x < s.w; x++) {
	for (int y = 0; y < s.h; y++) {
		if (p(s.blks[y * s.w + x])){
			if (i < sz) {
				l[i] = (Loc){x - l0.x , y - l0.y};
				i++;
			} else {
				fprintf(stderr, "locs: array is too small\n");
			}
		}
	}
	}

	return i;
}
