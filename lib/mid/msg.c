#include "../../include/mid.h"
#include "../../include/log.h"
#include <string.h>
#include <stdarg.h>

enum {
	MsgTicks = 2*50,	// 2 second with 20ms frames.
};

static Txt *gettxt(void);

void msgdraw(Msg *m, Gfx *g)
{
	Txt *txt = gettxt();
	int y = Scrnh;
	for(int i = 0; i < m->top; i++){
		if (!m->txt[i])
			continue;

		Point sz = txtdims(txt, m->txt[i]);
		Point em = txtdims(txt, "M");
		Point loc = (Point) { Scrnw - sz.x - em.x, y - sz.y };
		txtdraw(g, txt, loc, m->txt[i]);
		y -= sz.y + 2;

		if(m->left[i] > 0)
			m->left[i]--;
		if (m->left[i] == 0){
			xfree(m->txt[i]);
			m->txt[i] = NULL;
		}
	}
}

void msg(Msg *m, const char *t, ...)
{
	// compact list
	int j = 0;
	for(int i = 0; i < m->top; i++){
		if(m->txt[i]){
			m->txt[j] = m->txt[i];
			m->left[j] = m->left[i];
			j++;
		}
	}
	m->top = j;
	if(m->top == MsgMax){
		// trash oldest to make room
		xfree(m->txt[0]);
		memmove(m->txt, &m->txt[1], sizeof(char*)*(MsgMax-1));
		memmove(m->left, &m->left[1], sizeof(int)*(MsgMax-1));
		m->top = MsgMax - 1;
	}

	va_list ap;
	va_start(ap, t);
	size_t len = vsnprintf(0, 0, t, ap);
	va_end(ap);
	m->txt[m->top] = xalloc(len+1, 1);
	va_start(ap, t);
	vsnprintf(m->txt[m->top], len+1, t, ap);
	va_end(ap);
	m->left[m->top] = MsgTicks;
	m->top++;
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