

typedef struct Game Game;
Game *gamenew(void);
void gamefree(Scrn *);
void gameupdate(Scrn *, Scrnstk *);
void gamedraw(Scrn *, Gfx *);
void gamehandle(Scrn *, Scrnstk *, Event *);
extern Scrnmt gamemt;

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

/* Swap the item at the given inventory grid x,y.  Return the old item. */
Item *invmod(Inv *, Item *, int x, int y);

Scrn *invscrnnew(Inv *, Lvl *, Point ppos);
Scrn *titlescrnnew(Gfx *);

