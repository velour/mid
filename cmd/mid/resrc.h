typedef struct Txtinfo Txtinfo;
struct Txtinfo {
	unsigned int size;
	Color color;
};

Rtab *imgs;
Rtab *anim;
Rtab *txt;
Rtab *music;
Rtab *sfx;
void initresrc();
void freeresrc();
