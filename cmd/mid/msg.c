#include "../../include/mid.h"
#include "../../include/log.h"
#include "game.h"

enum {
	MsgTicks = 2*50,	// 2 second with 20ms frames.
};

static Txt *gettxt(void);

void msgdraw(Msg *m, Gfx *g)
{
	if (!m->txt)
		return;

	Txt *txt = gettxt();
	Point sz = txtdims(txt, m->txt);
	Point em = txtdims(txt, "M");
	Point loc = (Point) { Scrnw - sz.x - em.x, Scrnh - sz.y };
	txtdraw(g, txt, loc, m->txt);

	m->left--;
	if (m->left <= 0)
		m->txt = NULL;
}

void msg(Msg *m, const char *t)
{
	if (t && t[0] == '\0')
		t = NULL;
	m->txt = t;
	m->left = MsgTicks;
}

static Txt *gettxt(void)
{
	static Txt *msgtxt;
	static Txtinfo txtinfo = { TxtSzSmall };
	txtinfo.color = (Color){255, 255, 255, 255};
	if (!msgtxt) {
		msgtxt = resrcacq(txt, TxtStyleMenu, &txtinfo);
		if (!msgtxt)
			fatal("Failed to load inventory text");
	}
	return msgtxt;
}