/* © 2013 the Mid Authors under the MIT license. See AUTHORS for the list of authors.*/

#include "../../include/mid.h"
#include "../../include/log.h"
#include "../../include/rng.h"
#include "../../include/os.h"
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
static FILE *opensavefile(const char *file, const char *mode);
static const char *savepath(const char *file);
static _Bool readl(char *buf, int sz, FILE *f);

struct Game {
	Player player;
	Point transl;
	_Bool died;
	int znum, zmax;
	Zone *zone;
	Rng rng;
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

	gm.zone = zonegen(&gm.rng);
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

	Point loc0 = gm->player.body.bbox.a;
	zoneput(gm->zone, gm->znum);

	if (gm->zone->updown == Goup) {
		gm->znum--;
		if (gm->znum < 0) {
			pr("You just left the dungeon");
			scrnstkpush(stk, goverscrnnew(&gm->player, gm->znum));
			return;
		}
		gm->zone = zoneget(gm->znum);

		Tileinfo bi = zonedstairs(gm->zone);
		gm->zone->lvl->z = bi.z;
		playersetloc(&gm->player, bi.x, bi.y);

		int p = gm->znum / 3;
		lvlsetpallet(p);
	} else if (gm->zone->updown == Godown) {
		gm->znum++;
		if (gm->znum > gm->zmax) {
			gm->zmax = gm->znum;
			gm->zone = zonegen(&gm->rng);
		} else {
			gm->zone = zoneget(gm->znum);
		}

		playersetloc(&gm->player, 2, 2);

		int p = gm->znum / 3;
		if(p >= LvlMaxPallets) p = LvlMaxPallets - 1;
		lvlsetpallet(p);
	}

	Point loc1 = gm->player.body.bbox.a;
	gm->transl = (Point) {
		gm->transl.x + loc0.x - loc1.x,
		gm->transl.y + loc0.y - loc1.y
	};
}

void gameupdate(Scrn *s, Scrnstk *stk)
{
	Game *gm = s->data;

	Point tr;
	zoneupdate(gm->zone, &gm->player, &tr);
	gm->transl.x += tr.x;
	gm->transl.y += tr.y;

	trystairs(stk, gm);
	if(gm->player.curhp <= 0 && !debugging){
		if(gm->player.lives == 0)
			scrnstkpush(stk, goverscrnnew(&gm->player, gm->znum));
		else{
			gm->died = 1;
			gm->player.curhp = gm->player.eqp[StatHp] + gm->player.stats[StatHp];
			Player p = gm->player;
			playerinit(&p, 2, 2);
			gm->player.body = p.body;
			gm->player.imgloc = p.imgloc;
			gm->player.hitback = 0;
			gm->player.sframes = 0;
			gm->player.lives--;
			gm->transl.x = 0;
			gm->transl.y = 0;
			gm->zone->lvl->z = 0;

			int lose = rngintincl(&gm->rng, 0, Maxinv-1);
			gm->player.inv[lose] = (Invit){};
		}
	}
}

void gamedraw(Scrn *s, Gfx *g)
{
	Game *gm = s->data;

	gfxclear(g, (Color){ 0, 0, 0, 0 });

	if(gm->died){
		gm->died = 0;
		camreset(g);
	}else{
		cammove(g, gm->transl.x, gm->transl.y);
		gm->transl = (Point){};
	}

	zonedraw(g, gm->zone, &gm->player);

	int maxhp = gm->player.stats[StatHp] + gm->player.eqp[StatHp];
	Meter lm = {
		.base = gm->player.curhp,
		.extra = 0,
		.max = maxhp,
		.xscale = 5,
		.h = TxtSzSmall,
		.cbg = { 200 },
		.cbase = { 0, 200, 200 },
	};
	meterdraw(g, &lm, (Point){1,1});

	for(int i = 0; i < gm->player.lives; i++){
		Point life = { 1 + i*16, 16 + 2 };
		Rect clip = { { 0, 0 }, { 16, 16 } };
		imgdrawreg(g, gm->ui, clip, life);
	}

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

	Point ploc = gm->player.body.bbox.a;
	Point c = (Point){ Scrnw/2 - Wide, Scrnh/2 - Tall };
	Point tr = (Point){
		gm->transl.x - ploc.x + c.x,
		gm->transl.y - ploc.y + c.y,
	};
	Player player = gm->player;
	player.imgloc = c;

	FILE *f = opensavefile("game", "w");
	static char buf[4096];
	if (!printgeom(buf, sizeof(buf), "pbdddul", tr, gm->died, gm->znum, gm->zmax, gm->zone->lvl->z, gm->rng.v, player))
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
	if (!scangeom(buf, "pbdddul", &gm.transl, &gm.died, &gm.znum, &gm.zmax, &z, &gm.rng.v, &gm.player))
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
}

static FILE *opensavefile(const char *file, const char *mode)
{
	const char *p = savepath(file);
	FILE *f = fopen(p, mode);
	if (!f)
		die("Failed to open %s file with mode %s [%s]: %s", file, mode, p, miderrstr());
	return f;
}

_Bool saveavailable(){
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
