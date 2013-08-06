/* © 2013 the Mid Authors under the MIT license. See AUTHORS for the list of authors.*/

struct Rng;

void zoneloc(const char*);
/* Notify zone loader to use stdin for the next zone. */
void zonestdin();
Zone *zoneget(int);
Zone *zonegen(struct Rng *r);
void zoneput(Zone *, int);
void zonecleanup(int zmax);
// Find the down stairs in this zone.
Tileinfo zonedstairs(Zone *zn);

typedef struct Game Game;
Game *gamenew(void);
void gamefree(Scrn *);
void gameupdate(Scrn *, Scrnstk *);
void gamedraw(Scrn *, Gfx *);
void gamehandle(Scrn *, Scrnstk *, Event *);
extern Scrnmt gamemt;
void saveloc(const char *l);
void gamesave(Game *gm);
Game *gameload();

enum { Invrows = 6, Invcols = 3 };

Scrn *invscrnnew(Player *, Zone *, int);
Scrn *titlescrnnew(Gfx *);
Scrn *statscrnnew(Player *, Env *);
Scrn *goverscrnnew(Player *, int);
Scrn *optscrnnew(void);

_Bool ensuredir(const char *d);
