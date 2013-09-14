/* © 2013 the Mid Authors under the MIT license. See AUTHORS for the list of authors.*/

#include "../../include/mid.h"
#include "../../include/log.h"
#include "../../include/rng.h"
#include "../../include/os.h"
#include <dirent.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "game.h"

static char savedir[128] = "_save";

static void ldresrc();
static void rmrecur(const char *);
static FILE *opensavefile(const char *file, const char *mode);
static const char *savepath(const char *file);
static _Bool readl(char *buf, int sz, FILE *f);

struct Game {
	Player player;
	_Bool died;
	int znum, zmax;
	Zone *zone;
	Rng rng;
	Msg msg;
	Img *ui;
};

Game *gamenew(void)
{
	static Game gm = {};
	gm = (Game){};

	lvlinit();

	unsigned int seed = time(NULL) ^ getpid();
	rnginit(&gm.rng, seed);
	pr("game seed: %u", seed);

	gm.zone = zonegen(&gm.rng, 0);
	if (!gm.zone)
		fatal("Failed to load zone: %s", miderrstr());

	playerinit(&gm.player, 2, 2);

	ldresrc();
	gm.ui = resrcacq(imgs, "img/ui.png", 0);

	return &gm;
}

void gamefree(Scrn *s)
{
	Game *gm = s->data;
	zonefree(gm->zone);
	zonecleanup(gm->zmax);
	*gm = (Game){};
}

static void trystairs(Scrnstk *stk, Game *gm)
{
	if (gm->zone->updown == Gonone)
		return;

	zoneput(gm->zone, gm->znum);

	if (gm->zone->updown == Goup) {
		gm->znum--;
		if (gm->znum < 0) {
			pr("You just left the dungeon");
			rmsave();
			scrnstkpush(stk, goverscrnnew(&gm->player, gm->znum));
			return;
		}
		gm->zone = zoneget(gm->znum);

		Tileinfo bi = zonedstairs(gm->zone);
		gm->zone->lvl->z = bi.z;
		playersetloc(&gm->player, bi.x, bi.y);

		lvlsetpallet(lvlpallet(gm));
	} else if (gm->zone->updown == Godown) {
		gm->znum++;
		if (gm->znum > gm->zmax) {
			gm->zmax = gm->znum;
			gm->zone = zonegen(&gm->rng, gm->znum);
		} else {
			gm->zone = zoneget(gm->znum);
		}

		playersetloc(&gm->player, 2, 2);

		lvlsetpallet(lvlpallet(gm));
	}
}

int lvlpallet(Game *gm)
{
	int p = gm->znum / 3;
	if (p >= LvlMaxPallets)
		p = LvlMaxPallets - 1;
	return p;
}

void gameupdate(Scrn *s, Scrnstk *stk)
{
	Game *gm = s->data;

	zoneupdate(gm->zone, &gm->player, &gm->msg);

	trystairs(stk, gm);
	if(gm->player.curhp <= 0 && !debugging){
		rmsave();
		scrnstkpush(stk, goverscrnnew(&gm->player, gm->znum));
	}
}

_Bool dropitem(Zone *z, Player *p, Invit *it) {
	int dpos = p->dir == Left ? -1 : 1;

	Point pt = (Point) {
		p->body.bbox.a.x / Twidth + dpos,
		// Player's bbox is not Theight tall, but items are Theight tall, so we must compute
		// the item drop location by subtracting Theight from the player's feet.  Otherwise
		// the item is dropped into the ground.
		(p->body.bbox.b.y - Theight) / Theight
	};

	Item drop = {};
	iteminit(&drop, it->id, pt);
	if (!zoneadditem(z, z->lvl->z, drop)) {
		// Couldn't drop it next to the player, instead drop it directly on the player's square.
		// Since the player was on their square, this must be a free place to drop something.
		pt = (Point){ p->bi.x, p->bi.y };
		iteminit(&drop, it->id, pt);
		if (!zoneadditem(z, z->lvl->z, drop))
			return 0;
	}

	*it = (Invit){};
	return 1;
}

void gamedraw(Scrn *s, Gfx *g)
{
	Game *gm = s->data;

	gfxclear(g, PureBlack);

	camcenter(g, playerimgloc(&gm->player));

	zonedraw(g, gm->zone, &gm->player);

	int maxhp = playerstat(&gm->player, StatHp);
	Meter lm = {
		.base = (gm->player.curhp*100) / maxhp,
		.extra = 0,
		.max = 100,
		.xscale = 1,
		.h = TxtSzSmall,
		.cbg = { 98, 36, 38 },
		.cbase = { 119, 172, 213 },
	};
	meterdraw(g, &lm, (Point){1,1});

	
	Meter mm = {
		.base = (gm->player.curmp*100) / MaxMP,
		.extra = 0,
		.max = 100,
		.xscale = 1,
		.h = TxtSzSmall,
		.cbg = { 98, 36, 38 },
		.cbase = { 119, 172, 213 },
	};
	meterdraw(g, &mm, (Point){1,2+lm.h});

	msgdraw(&gm->msg, g);

	gfxflip(g);
}

void gamehandle(Scrn *s, Scrnstk *stk, Event *e)
{
	if(e->type != Keychng || e->repeat)
		return;

	Game *gm = s->data;

	if(e->down && e->key == kmap[Mvinv]){
		scrnstkpush(stk, invscrnnew(&gm->player, gm->zone, gm->znum));
		return;
	}

	playerhandle(&gm->player, e);

	if(gm->player.acting){
		int z = gm->zone->lvl->z;
		Env *ev = gm->zone->envs[z];
		for(int i = 0; i < Maxenvs; i++) {
			if (!ev[i].id)
				continue;
			envact(&ev[i], &gm->player, gm->zone);
			if(gm->player.statup){
				scrnstkpush(stk, statscrnnew(gm, &gm->player, &ev[i]));
				gm->player.statup = 0;
				msg(&gm->msg, "%s", "Game Saved");
				return;
			}
		}
	}
}

Scrnmt gamemt = {
	gameupdate,
	gamedraw,
	gamehandle,
	gamefree,
};

void saveloc(const char *l)
{
	strncpy(savedir, l, sizeof(savedir)-1);
}

void gamesave(Game *gm)
{
	ignframetime();
	if (!ensuredir(savedir))
		die("Failed to make the save directory: %s", miderrstr());

	for (int i = 0; i <= gm->zmax; i++) {
		char zfile[128];
		if (snprintf(zfile, sizeof(zfile), "%d.zone", i) > sizeof(zfile))
			die("Buffer is too small for the save's zone file");

		const char *p = savepath(zfile);
		FILE *f = fopen(p, "w");
		if (!f)
			die("Failed to open zone file for writing [%s]: %s", p, miderrstr());
		Zone *z = i == gm->znum ? gm->zone : zoneget(i);
		zonewrite(f, z);
		fclose(f);
	}

	FILE *f = opensavefile("game", "w");
	static char buf[4096];
	if (!printgeom(buf, sizeof(buf), "bdddul", gm->died, gm->znum, gm->zmax, gm->zone->lvl->z, gm->rng.v, gm->player))
		die("Failed to serialize the game information");
	fputs(buf, f);
	fputc('\n', f);
	fclose(f);
}

Game *gameload()
{
	if (!ensuredir(savedir))
		die("Failed to make the save directory: %s", miderrstr());

	static Game gm = {};
	gm = (Game){};
	lvlinit();
	ldresrc();
	playerinit(&gm.player, 2, 2);
	
	static char buf[4096];

	FILE *f = opensavefile("game", "r");
	if (!readl(buf, sizeof(buf), f))
		die("Failed to read the game save file: %s", miderrstr());
	fclose(f);
	int z = 0;
	if (!scangeom(buf, "bdddul", &gm.died, &gm.znum, &gm.zmax, &z, &gm.rng.v, &gm.player))
		die("Failed to deserialize the game information: %s", miderrstr());

	for (int i = 0; i <= gm.zmax; i++) {
		char zfile[128];
		if (snprintf(zfile, sizeof(zfile), "%d.zone", i) > sizeof(zfile))
			die("Buffer is too small for the save's zone file: %s", miderrstr());

		const char *p = savepath(zfile);
		FILE *f = fopen(p, "r");
		if (!f)
			die("Failed to open zone file for reading [%s]: %s", p, miderrstr());
		Zone *z = zoneread(f);
		zoneput(z, i);
		fclose(f);
		zonefree(z);
	}

	gm.zone = zoneget(gm.znum);
	gm.zone->lvl->z = z;
	gm.ui = resrcacq(imgs, "img/ui.png", 0);

	return &gm;
}

static void ldresrc()
{
	if (!itemldresrc())
		fatal("Failed to load item resources: %s", miderrstr());
	if (!envldresrc())
		fatal("Failed to load env resources: %s", miderrstr());
	if(!enemyldresrc())
		fatal("Failed to load enemy resrouces: %s", miderrstr());
	if(!swordldresrc())
		fatal("Failed to load sword resrouces: %s", miderrstr());
	if(!magicldresrc())
		fatal("Failed to load magic resources: %s", miderrstr());
}

static FILE *opensavefile(const char *file, const char *mode)
{
	const char *p = savepath(file);
	FILE *f = fopen(p, mode);
	if (!f)
		die("Failed to open %s file with mode %s [%s]: %s", file, mode, p, miderrstr());
	return f;
}

void rmsave()
{
	if (!saveavailable())
		return;
	rmrecur(savedir);
}

// TODO(eaburns): a candidate function for a more general os lib.
static void rmrecur(const char *path)
{
	struct stat sb;

	if (stat(path, &sb) < 0) {
		pr("Unable to stat: %s: %s", path, strerror(errno));
		return;
	}

	if (!S_ISDIR(sb.st_mode))
		goto rm;

	DIR *d = opendir(path);
	if (!d) {
		pr("Failed to open %s: %s", path, strerror(errno));
		return;
	}

	struct dirent *ent;
	for (;;) {
		errno = 0;
		ent = readdir(d);
		if (!ent && errno != 0) {
			pr("Failed to read %s: %s", path, strerror(errno));
			break;
		}
		if (!ent)
			break;
		if (strcmp(ent->d_name, ".") == 0|| strcmp(ent->d_name, "..") == 0)
			continue;

		// TODO(eaburns): when lib/mid/fs.c is moved into a general os library, we
		// can change this junk to use fscat, which could be renamed to pathjoin.
		char *subpath = calloc(sizeof(path) + sizeof(ent->d_name) + 2, 1);
		if (!subpath)
			die("calloc failed");
		strcpy(subpath, path);
		int l = strlen(subpath);
		subpath[l] = '/';
		strcpy(subpath+l+1, ent->d_name);

		rmrecur(subpath);

		free(subpath);
	}

	closedir(d);

rm:
	pr("Removing %s", path);
	if (remove(path) < 0)
		pr("Failed to remove %s: %s", path, strerror(errno));
}

_Bool saveavailable()
{
	struct stat sb;
	if (stat(savepath("game"), &sb) < 0) {
		return false;
	}
	return true;
}

// Non-reentant
static const char *savepath(const char *file)
{
	static char path[1024];

	if (snprintf(path, sizeof(path), "%s/%s", savedir, file) > sizeof(path))
		die("Buffer is too small for the save's %s path", file);

	return path;
}

// TODO(eaburns): de-duplicate this and the one in ../../lib/mid/zone.c.
static _Bool readl(char *buf, int sz, FILE *f)
{
	extern int isspace(int);
	char *r = fgets(buf, sz, f);
	if (!r) {
		pr("fgets returned NULL");
		return 0;
	}

	int l = strlen(buf);

	for(int i = l-1; i >= 0 && isspace(buf[i]); i--)
		buf[i] = 0;

	return 1;
}

_Bool ensuredir(const char *d)
{
	struct stat sb;
	if (stat(d, &sb) < 0) {
		if (makedir(d) < 0) {
			seterrstr("Failed to make directory %s: %s", d, strerror(errno));
			return false;
		}
	} else if (!S_ISDIR(sb.st_mode)) {
		seterrstr("%s already exists and is not a directory", d);
		return false;
	}
	return true;
}
