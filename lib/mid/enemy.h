// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.

_Bool defaultscan(char *, Enemy *);
_Bool defaultprint(char *, size_t, Enemy *);

extern Sfx *untihit;
extern Img *untiimg;

_Bool untiinit(Enemy *e, int x, int y);
void untifree(Enemy*);
void untiupdate(Enemy*,Player*,Lvl*);
void untidraw(Enemy*,Gfx*);
_Bool untiscan(char *buf, Enemy *e);
_Bool untiprint(char *buf, size_t sz, Enemy *e);

extern Img *nousimg;

_Bool nousinit(Enemy *e, int x, int y);
void nousfree(Enemy*);
void nousupdate(Enemy*,Player*,Lvl*);
void nousdraw(Enemy*,Gfx*);
_Bool nousscan(char *buf, Enemy *e);
_Bool nousprint(char *buf, size_t sz, Enemy *e);
