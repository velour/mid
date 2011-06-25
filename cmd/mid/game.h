Lvl *lvlgen(int w, int h, int d, int sd);

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
