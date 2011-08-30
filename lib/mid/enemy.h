// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.

_Bool defaultscan(char *, Enemy *);
_Bool defaultprint(char *, size_t, Enemy *);

enum{
	MaxDrops = 2,
};

typedef struct Drops Drops;
typedef struct Info Info;

struct Drops{
	ItemID item[MaxDrops];
	int prob[MaxDrops]; // MUST SUM TO 100!!!
};

struct Info{
	int stats[StatMax];
	Drops drops;
	Sfx *hit;
	EnemyID death;
};

void enemygenupdate(Enemy*,Player*,Zone*,Info*);

#define ENEMYDECL(e) \
_Bool e##init(Enemy*,int,int);\
void e##free(Enemy*);\
void e##update(Enemy*,Player*,Zone*);\
void e##draw(Enemy*,Gfx*);\
_Bool e##scan(char*,Enemy*);\
_Bool e##print(char*,size_t,Enemy*);\
extern Info e##info

extern Sfx *untihit;
extern Img *untiimg;
ENEMYDECL(unti);

extern Img *nousimg;
ENEMYDECL(nous);

extern Img *splatimg;
ENEMYDECL(splat);

extern Img *daimg;
ENEMYDECL(da);

extern Img *thuimg;
ENEMYDECL(thu);

extern Img *grenduimg;
ENEMYDECL(grendu);

#undef ENEMYDECL
