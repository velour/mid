enum { Scrnw = 512, Scrnh = 512 };

/* Buffer from side of screen at which to begin scrolling. */
enum { Scrlbuf = 100 };

enum Action{
	Mvleft,
	Mvright,
	Mvjump,
	Nactions,
};

_Bool keymapread(char km[Nactions], char *fname);

typedef struct Game Game;
Game *gamenew(char keymap[]);
void gamefree(Scrn *);
void gameupdate(Scrn *, Scrnstk *);
void gamedraw(Scrn *, Gfx *);
void gamehandle(Scrn *, Scrnstk *, Event *);
Scrnmt gamemt;

typedef struct Player Player;
Player *playernew(int x, int y, char keymap[]);
void playerfree(Player *);
void playerupdate(Player *, Lvl *l, int z, Point *tr);
void playerdraw(Gfx *, Player *, Point tr);
void playerhandle(Player *, Event *);
