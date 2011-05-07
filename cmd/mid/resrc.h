typedef struct Txtinfo Txtinfo;
struct Txtinfo {
	unsigned int size;
	Color color;
};

Rcache *imgs;
Rcache *txt;
Rcache *music;
Rcache *sfx;
void initresrc();
void freeresrc();
