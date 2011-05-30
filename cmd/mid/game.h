enum { Scrnw = 512, Scrnh = 512 };

/* Buffer from side of screen at which to begin scrolling. */
enum { Scrlbuf = 192 };

enum Action{
	Mvleft,
	Mvright,
	Mvjump,
	Mvinv,
	Nactions,
};

_Bool keymapread(char km[Nactions], char *fname);
extern char kmap[Nactions];

typedef struct Game Game;
Game *gamenew(void);
void gamefree(Scrn *);
void gameupdate(Scrn *, Scrnstk *);
void gamedraw(Scrn *, Gfx *);
void gamehandle(Scrn *, Scrnstk *, Event *);
extern Scrnmt gamemt;

typedef struct Player Player;
Player *playernew(int x, int y);
void playerfree(Player *);
void playerupdate(Player *, Lvl *l, int z, Point *tr);
void playerdraw(Gfx *, Player *, Point tr);
void playerhandle(Player *, Event *);

Scrn *invscrnnew(Player *, Lvl *);
Scrn *titlescrnnew(Gfx *);
