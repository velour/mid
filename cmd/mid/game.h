typedef struct Game Game;
Game *gamenew(void);
void gamefree(Scrn *);
void gameupdate(Scrn *, Scrnstk *);
void gamedraw(Scrn *, Gfx *);
void gamehandle(Scrn *, Scrnstk *, Event *);
Scrnmt gamemt;

typedef struct Player Player;
Player *playernew(int x, int y);
void playerfree(Player *);
void playerupdate(Player *);
void playerdraw(Gfx *, Player *, Point tr);
void playerhandle(Player *, Point *tr, Event *);
