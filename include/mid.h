/* © 2013 the Mid Authors under the MIT license. See AUTHORS for the list of authors.*/

#include <stdio.h> // FILE

// Mean frame time
extern double meanftime;
// Ignore the time for this frame in the mean computation.
void ignframetime(void);

extern int debugging;
extern _Bool mute;

void *xalloc(unsigned long n, unsigned long sz);
void xfree(void*);

const char *miderrstr(void);
void seterrstr(const char *fmt, ...);

typedef struct Point Point;
struct Point{
	double x, y;
};

Point vecadd(Point, Point);
double distsquare(Point, Point);
double dist(Point, Point);

typedef struct Line1d Line1d;
struct Line1d{
	double a, b;
};

/*
Returns the absolute value of the intersection of two lines.
Returns a negative number if the lines don't intersect.
*/
double isection1d(Line1d, Line1d);

typedef struct Rect Rect;
struct Rect{
	Point a, b;
};

Line1d rectprojx(Rect);
Line1d rectprojy(Rect);
void rectmv(Rect *, double dx, double dy);
/* Makes point a the min,min and point b the max,max. */
Rect rectnorm(Rect r);
void ptmv(Point *, double dx, double dy);
double ptsqdist(Point a, Point b);

_Bool rectcontains(Rect, Point);

typedef struct Isect Isect;
struct Isect{
	_Bool is;
	double dx, dy; /* isection1d of x and y projs */
};

Isect isection(Rect, Rect);
int isect(Rect, Rect);
double isectarea(Isect is);

/*
The minimal intersection will be positive, and the maximum intersection will
be negative. If both intersections are equal, both dx and dy will be positive.
*/
Isect minisect(Rect, Rect);


typedef struct Color Color;
struct Color{
	unsigned char r, g, b, a;
};

typedef struct Gfx Gfx;

Gfx *gfxinit(int w, int h, const char *title);
void gfxfree(Gfx *);
Point gfxdims(const Gfx *);
void gfxflip(Gfx *);
void gfxclear(Gfx *, Color);
void gfxdrawpoint(Gfx *, Point, Color);
void gfxfillrect(Gfx *, Rect, Color);
void gfxdrawrect(Gfx *, Rect, Color);
Point projpt(Point);

typedef struct Img Img;

Img *imgnew(const char *path);
void imgfree(Img *);
/* Returns negative dimensions on failure. */
Point imgdims(const Img *);
void imgdraw(Gfx *, Img *, Point);
void imgdrawscale(Gfx *, Img *, Point, float);
void imgdrawreg(Gfx *, Img *, Rect, Point);

typedef struct Txt Txt;

enum{
	TxtSzSmall = 16,
	TxtSzMedium = 32,
	TxtSzLarge = 64,
};

#define TxtStyleMenu "txt/Designosaur-Regular.ttf"

Txt *txtnew(const char *font, int sz, Color);
void txtfree(Txt *);
Point txtdims(const Txt *, const char *fmt, ...);
Img *txt2img(Gfx *, Txt *, const char *fmt, ...);
// Prefer txt2img to this for static text
Point txtdraw(Gfx *, Txt *, Point, const char *fmt, ...);

typedef struct Anim Anim;

void camreset(Gfx*);
void cammove(Gfx *, double dx, double dy);
Point camget(Gfx*);
void camdrawrect(Gfx *, Rect, Color);
void camfillrect(Gfx *, Rect, Color);
void camdrawimg(Gfx *, Img *, Point);
void camdrawreg(Gfx *, Img *, Rect, Point);
void camdrawanim(Gfx *, Anim *, Point);

_Bool sndinit(void);
void sndfree(void);
_Bool sndread(char*);
_Bool sndwrite(char*);

enum{
	SndVolMin = 0,
	SndVolMax = 128,
	SndVolDefault = 64,
};
int sndvol(int);

typedef struct Music Music;

Music *musicnew(const char *);
void musicfree(Music *);
void musicstart(Music *, int fadein);
void musicstop(int fadeout);
void musicpause(void);
void musicresume(void);

typedef struct Sfx Sfx;

Sfx *sfxnew(const char *);
void sfxfree(Sfx *);
void sfxplay(Sfx *);

enum Eventty{
	Quit,
	Keychng,
	Mousemv,
	Mousebt,
};

enum{
	Mouse1 = 1,
	Mouse2 = Mouse1 << 1,
	Mouse3 = Mouse2 << 1,
};

typedef struct Event Event;
struct Event{
	enum Eventty type;

	_Bool down;
	_Bool repeat;
	char key;

	double x, y, dx, dy;
	int butt;
};

_Bool pollevent(Event *);

typedef struct Scrn Scrn;
typedef struct Scrnmt Scrnmt;
typedef struct Scrnstk Scrnstk;

struct Scrn{
	Scrnmt *mt;
	Point cam;
	void *data;
};

enum { Ticktm = 20 /* ms */ };

struct Scrnmt{
	void (*update)(Scrn *, Scrnstk *);
	void (*draw)(Scrn *, Gfx *);
	void (*handle)(Scrn *, Scrnstk *, Event *);
	void (*free)(Scrn *);
};

Scrnstk *scrnstknew(Gfx*);
void scrnstkfree(Scrnstk *);
/* Stack now owns Scrn, will call scrn->mt->free(scrn) when popped. */
void scrnstkpush(Scrnstk *, Scrn *);
Scrn *scrnstktop(Scrnstk *);
void scrnstkpop(Scrnstk *);

void scrnrun(Scrnstk *);

typedef struct Rtab Rtab;

unsigned int strhash(const char *);

typedef struct Resrcops Resrcops;
struct Resrcops {
	void*(*load)(const char *path, void *aux);
	void(*unload)(const char *path, void *resrc, void *aux); /* may be NULL */
	unsigned int (*hash)(const char *path, void *aux); /* may be NULL */
	_Bool (*eq)(void *aux0, void *aux1); /* may be NULL */
};

Rtab *rtabnew(Resrcops *);
/* unloads all resources and frees the table. */
void rtabfree(Rtab *);
/* Acquire a reference to a resource (loading it if necessary).  The
 * 3rd param is passed as aux data as the 2nd param of load. */
void *resrcacq(Rtab *, const char *file, void *aux);
/* Release a reference to a resource. */
void resrcrel(Rtab *, const char *file, void *aux);

typedef struct Txtinfo Txtinfo;
struct Txtinfo {
	unsigned int size;
	Color color;
};

extern Rtab *imgs;
extern Rtab *txt;
extern Rtab *music;
extern Rtab *sfx;
void initresrc(void);
void freeresrc(void);

struct Anim{
	Img *sheet;
	int row, len;
	int delay;
	int w, h;
	int f, d;
};

void animupdate(Anim *);
void animdraw(Gfx *, Anim *, Point);
void animreset(Anim *a);

typedef struct Blk Blk;
struct Blk {
	char tile;
	char flags;
};

typedef struct Lvl Lvl;
struct Lvl {
	int d, w, h, z;
	int seenz;
	Blk blks[];
};

Lvl *lvlnew(int, int, int, int);
Lvl *lvlread(FILE *);
void lvlwrite(FILE *, Lvl *);
void lvlfree(Lvl *);
_Bool lvlinit();
void lvlupdate(Lvl *l);
void lvldraw(Gfx *g, Lvl *l, _Bool bkgrnd);
void lvlminidraw(Gfx *g, Lvl *l, Point offs, int scale);
/* Returns the reverse vector that must be added to v in order to
 * respect collisions. */
Isect lvlisect(Lvl *l, Rect r, Point v);

enum{
	LvlMaxPallets = 2
};

void lvlsetpallet(int pallet);

typedef struct Tileinfo Tileinfo;
struct Tileinfo {
	int x, y, z;
	unsigned int flags;
};

enum {
	Tcollide = 1<<0,
	Twater = 1<<1,
	Tfdoor = 1<<2,
	Tbdoor = 1<<3,
	Tdown = 1 << 4,
	Tup = 1 << 5,
	Topaque = 1 << 6,
};

enum { Theight = 32, Twidth = 32 };

Tileinfo tileinfo(Lvl *l, int x, int y, int z);
/* Get the information on the dominant block that r is overlapping. */
Tileinfo lvlmajorblk(Lvl *l, Rect r);

static inline Blk *blk(Lvl *l, int x, int y, int z)
{
	return &l->blks[z * l->w * l->h + y * l->w + x];
}


double blkgrav(int flags);
double blkdrag(int flags);

/* Update the visibility of the level given that the player is viewing
 * the level from location (x, y). */
void lvlvis(Lvl *l, int x, int y);

typedef enum Action Action;
enum Action{
	Mvleft,
	Mvright,
	Mvact,
	Mvjump,
	Mvinv,
	Mvsword,
	Mvmagic,
	Nactions,
};

extern char kmap[Nactions];
_Bool keymapread(char km[Nactions], char *fname);
_Bool keymapwrite(char km[Nactions], char *fname);
_Bool iskeydown(Action);

enum { Scrnw = 512, Scrnh = 288 };

/* Buffer from side of screen at which to begin scrolling. */
enum { Scrlbuf = 192 };

enum { Tall = 32, Wide = 32 };

enum { Maxdy = 12 };

typedef struct Body Body;
struct Body {
	Rect bbox;
	Point vel;
	Point acc;
	_Bool fall;
};

void bodyinit(Body *, int x, int y, int w, int h);
void bodyupdate(Body *b, Lvl *l);

typedef struct Sword Sword;
typedef enum Act Act;
typedef enum Stat Stat;
typedef struct Item Item;
typedef struct Invit Invit;
typedef enum ItemID ItemID;
typedef enum ItemStatus ItemStatus;
typedef enum EqpLoc EqpLoc;
typedef struct Player Player;
typedef struct Zone Zone;
typedef struct Magic Magic;
typedef struct Msg Msg;

struct Sword{
	Rect rightloc[2];
	Rect leftloc[2];

	Action dir;
	int cur;

	int row;
};

_Bool swordldresrc(void);
void sworddraw(Gfx*, Sword*);
Rect swordbbox(Sword*);
int swordstr(Sword*, Player*);

enum Act {
	Stand,
	Walk,
	Jump,
	Nacts
};

enum Stat{
	StatHp,
	StatDex,
	StatStr,
	StatMag,
	StatLuck,
	StatMax
};

extern int statmax[StatMax];

enum { Maxinv = 18 };

enum EqpLoc{
	EqpNone,
	EqpHead,
	EqpBody,
	EqpArms,
	EqpLegs,
	EqpWep,
	EqpMag,
	EqpMax,
};

enum ItemID{
	ItemNone,
	ItemStatup,
	ItemCopper,
	ItemHealth,
	ItemSilver,
	ItemGold,
	ItemCarrot,
	ItemHamCan,
	ItemTopHat,
	ItemIronHelm,
	ItemIronGlove,
	ItemIronBody,
	ItemIronBoot,
	ItemSilverSwd,
	ItemBroadSwd,
	ItemWindSwd,
	ItemBubble,
	ItemZap,
	ItemMax
};

struct Item{
	ItemID id;
	Body body;
};

enum ItemStatus{
	ItemStatusNormal,
	ItemStatusPicked,
	ItemStatusNoRoom
};

_Bool itemldresrc(void);
_Bool iteminit(Item*, ItemID id, Point p);
void itemupdateanims(void);
ItemStatus itemupdate(Item*, Player*, Zone *z);
void itemdraw(Item*, Gfx*);
char *itemname(ItemID);
EqpLoc itemeqploc(ItemID);
void itemcast(Magic*, ItemID, Player*);

struct Invit{
	ItemID id;
	int stats[StatMax];
};

void invitinit(Invit*, ItemID);
void invitdraw(Invit*, Gfx*, Point p);
_Bool inviteat(Invit*, Player*, Zone*);

typedef enum ArmorLoc ArmorLoc;
enum ArmorLoc{
	ArmorBackArm,
	ArmorBody,
	ArmorHead,
	ArmorFrontArm,
	ArmorLegs,
	ArmorMax
};

typedef enum ArmorSetID ArmorSetID;
enum ArmorSetID{
	ArmorSetNone,
	ArmorSetIron,
	ArmorSetMax
};

extern Img *knightsheet;
void armorinit(void);
ArmorSetID itemarmorset(ItemID);
void applyarmorbonus(Player*, ArmorSetID);

struct Magic{
	ItemID id;
	Body body;
	Anim anim;
};

_Bool magicldresrc(void);
void magicdraw(Gfx*, Magic*);
void magicupdate(Magic*, Zone*);

typedef enum Dir {
	Left,
	Right,
	Ndirs,
} Dir;

struct Player {
	Anim as[Ndirs][Nacts];
	Dir dir;
	Act act;
	Point imgloc;

	Body body;

	_Bool acting;
	_Bool statup;

	double hitback;
	int jframes;
	int iframes; // invulnerability after damage;
	int sframes;
	int mframes;

	/* if changed, update visibility. */
	Tileinfo bi;

	int stats[StatMax];
	int eqp[StatMax];
	int curhp;
	int lives;

	int money;
	Invit inv[Maxinv];
	Invit wear[EqpMax];

	Sword sw;
};

void playerinit(Player *p, int x, int y);
void playersetloc(Player *p, int x, int y);	// tile coords
void playerupdate(Player *, Zone *, Point *tr);
void playerdraw(Gfx *, Player *);
void playerhandle(Player *, Event *);
Point playerpos(Player *);
Rect playerbox(Player *);
void playerdmg(Player *, int, int);
void playerheal(Player *, int);
_Bool playertake(Player *, Item *);
void resetstats(Player*);

typedef enum EnemyID EnemyID;
enum EnemyID{
	EnemyNone,
	EnemyUnti,
	EnemyNous,
	EnemyDa,
	EnemyThu,
	EnemyGrendu,
	EnemySplat,
	EnemyMax
};

typedef struct Enemy Enemy;
typedef struct Ai Ai;
struct Ai{
	void (*update)(Enemy*,Player*,Zone*);
	Point mv;
	Point lastp;
	double awdst; // awareness distance
};

struct Enemy{
	EnemyID id;
	Body body;
	int hp;
	double hitback;
	int iframes;
	void *data;
	Ai ai;
};

_Bool enemyldresrc(void);
_Bool enemyinit(Enemy *e, EnemyID id, int x, int y);
void enemyfree(Enemy*);
void enemyupdate(Enemy*, Player*, Zone*);
void enemydraw(Enemy*, Gfx*);

void aijumper(Ai*, double jv);
void aiwalker(Ai*, double wv);
void aipatroller(Ai*, double wv);
void aichaser(Ai*, double wv, double awdst);
void aihunter(Ai*, double jv, double wv, double awdst);

typedef enum EnvID EnvID;
enum EnvID{
	EnvNone,
	EnvShrempty,
	EnvShrused,
	EnvSwdStoneHp,
	EnvSwdStoneDex,
	EnvSwdStoneStr,
	EnvMax
};

typedef struct Env Env;
struct Env{
	EnvID id;
	Body body;
	_Bool gotit;
	int min;
};

_Bool envldresrc(void);
_Bool envinit(Env*, EnvID, Point);
void envupdateanims(void);
void envupdate(Env*, Zone*);
void envdraw(Env*,  Gfx*);
void envact(Env*, Player*, Zone*);
Point envsize(EnvID);

enum {
	Maxenms = 32,
	Maxitms = 32,
	Maxenvs = 16,
	Maxmagics = 32,
	Maxz = 5,
};

enum { Gonone, Goup, Godown };

struct Zone {
	Lvl *lvl;
	int updown;

	Item itms[Maxz][Maxitms];
	Env envs[Maxz][Maxenvs];
	Enemy enms[Maxz][Maxenms];
	Magic mags[Maxz][Maxmagics];
};

Zone *zoneread(FILE *);
void zonewrite(FILE *, Zone *z);
void zonefree(Zone *);
// Zoneadditem returns true if the item was successfully added to the zone.
// It returns false if either there wasn't a spot for the item or if the item was
// placed in a wall.
_Bool zoneadditem(Zone *zn, int z, Item it);
_Bool zoneaddenv(Zone *zn, int z, Env env);
_Bool zoneaddenemy(Zone *zn, int z, Enemy enm);
_Bool zoneaddmagic(Zone *zn, int z, Magic);
void zonedraw(Gfx *g, Zone *zn, Player *p);
void zoneupdate(Zone *zn, Player *p, Point *tr, Msg *);

/* Fills the array with locations that pass the given predicate. */
int zonelocs(Zone *, int z, _Bool (*)(Zone *, int, Point), Point [], int);
_Bool zonehasflags(Zone *zn, int z, Point loc, Point wh, unsigned int f);
_Bool zoneongrnd(Zone *zn, int z, Point loc, Point wh);
_Bool zoneoverlap(Zone *zn, int z, Point loc, Point wh);

/* Scan a set of fields from a string with the given format.  The
 * format is specified as a string of characters with the following
 * meanings:
 *
 * d - int
 * f - double
 * b - _Bool
 * p - Point
 * r - Rect
 * y - Body
 * l - Player
 * u - uint64_t
 *
 * The return value is true if all items in the format were scanned
 * and false if not.
 */
_Bool scangeom(char *buf, char *fmt, ...);

/* Prints a structure to a string buffer using the same type of format
 * specified as is used by scangeom.  The return value is true if the
 * output was not truncated and false if the output was truncated. */
_Bool printgeom(char *buf, int sz, char *fmt, ...);

_Bool fsexists(const char *path);

typedef struct Meter Meter;
struct Meter{
	int base;
	int extra;
	int preview;
	int max;
	int xscale;
	int h;
	Color cbg;
	Color cbase;
	Color cextra;
	Color cpreview;
	Color cborder;
};

Rect meterdraw(Gfx*, Meter*, Point);
Rect meterarea(Meter*, Point);

enum{ MsgMax = 5 };

struct Msg{
	char *txt[MsgMax];
	int left[MsgMax];
	int top;
};

void msgdraw(Msg*, Gfx*);
void msg(Msg*, const char*, ...);
