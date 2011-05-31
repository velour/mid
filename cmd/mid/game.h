enum { Scrnw = 512, Scrnh = 512 };

/* Buffer from side of screen at which to begin scrolling. */
enum { Scrlbuf = 192 };

enum Action{
	Mvleft,
	Mvright,
	Mvbak,
	Mvfwd,
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
void playerupdate(Player *, Lvl *l, int *z, Point *tr);
void playerdraw(Gfx *, Player *, Point tr);
void playerhandle(Player *, Event *);

enum { Invrows = 3, Invcols = 3 };

typedef struct Item Item;
struct Item {
	Anim *icon;
	const char *name;
};

typedef struct Inv Inv;
struct Inv {
	int money;
	Item *items[Invrows * Invcols];
};

Item *itemnew(const char *name, const char *anim);
void itemfree(Item *);

/* Caller must call gfxflip(). */
void invdraw(Gfx *, Inv *);
void invdrawcur(Gfx *, Item *);
void invupdate(Inv *);
/* Swap the item at the given inventory grid x,y.  Return the old item. */
Item *invmod(Inv *, Item *, int x, int y);
/* Inventory item at the given screen location */
Item *invat(Inv *inv, int x, int y);

Scrn *invscrnnew(Inv *, Lvl *, int z);
Scrn *titlescrnnew(Gfx *);

