

typedef struct Game Game;
Game *gamenew(void);
void gamefree(Scrn *);
void gameupdate(Scrn *, Scrnstk *);
void gamedraw(Scrn *, Gfx *);
void gamehandle(Scrn *, Scrnstk *, Event *);
extern Scrnmt gamemt;

enum { Invrows = 3, Invcols = 3 };

typedef struct Icon Icon;
struct Icon {
	Anim *icon;
	const char *name;
};

typedef struct Inv Inv;
struct Inv {
	int money;
	Icon *items[Invrows * Invcols];
};

Icon *iconnew(const char *name, const char *anim);
void iconfree(Icon *);

/* Swap the item at the given inventory grid x,y.  Return the old item. */
Icon *invmod(Inv *, Icon *, int x, int y);

Scrn *invscrnnew(Inv *, Lvl *, Point ppos);
Scrn *titlescrnnew(Gfx *);

