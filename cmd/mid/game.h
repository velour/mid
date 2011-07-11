struct Rng;

/* Notify zone loader to use stdin for the next zone. */
void zonestdin();
Zone *zonegen(struct Rng *);

typedef struct Game Game;
Game *gamenew(void);
void gamefree(Scrn *);
void gameupdate(Scrn *, Scrnstk *);
void gamedraw(Scrn *, Gfx *);
void gamehandle(Scrn *, Scrnstk *, Event *);
extern Scrnmt gamemt;

enum { Invrows = 5, Invcols = 3 };

Scrn *invscrnnew(Player *, Lvl *);
Scrn *titlescrnnew(Gfx *);
Scrn *statscrnnew(Player *, Env *);
Scrn *goverscrnnew(Player *);
