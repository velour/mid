// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.

_Bool defaultscan(char *, Enemy *);
_Bool defaultprint(char *, size_t, Enemy *);

#define ENEMYDECL(e) \
_Bool e##init(Enemy*,int,int);\
void e##free(Enemy*);\
void e##update(Enemy*,Player*,Lvl*);\
void e##draw(Enemy*,Gfx*);\
_Bool e##scan(char*,Enemy*);\
_Bool e##print(char*,size_t,Enemy*)

extern Sfx *untihit;
extern Img *untiimg;
ENEMYDECL(unti);

extern Img *nousimg;
ENEMYDECL(nous);

extern Img *splatimg;
ENEMYDECL(splat);

#undef ENEMYDECL
