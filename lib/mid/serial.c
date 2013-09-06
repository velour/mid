/* © 2013 the Mid Authors under the MIT license. See AUTHORS for the list of authors.*/

#include "../../include/mid.h"
#include "../../include/log.h"
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

// Ensure that unsigned long long is at least 64 bits.
enum { assert_keychar_eq = 1/!!(sizeof(unsigned long long) >= sizeof(uint64_t)) };

static void scanint(char **toks, int *d);
static void scanuint64_t(char **toks, uint64_t *d);
static void scandbl(char **toks, double *f);
static void scanbool(char **toks, _Bool *b);
static void scanpt(char **toks, Point *pt);
static void scanrect(char **toks, Rect *r);
static void scanbody(char **toks, Body *b);
static void scaninvit(char **toks, Invit *i);
static void scansword(char **toks, Sword *s);
static void scanplayer(char **toks, Player *p);
static char *nxt(char **toks);
static void printpt(char **bufp, int *szp, Point p);
static void printrect(char **bufp, int *szp, Rect r);
static void printbody(char **bufp, int *szp, Body b);
static void printinvit(char **bufp, int *szp, Invit);
static void printsword(char **bufp, int *szp, Sword s);
static void printplayer(char **bufp, int *szp, Player p);
static void prfield(char **bufp, int *szp, char *fmt, ...);

_Bool scangeom(char *buf, char *fmt, ...)
{
	va_list ap;
	char *f = fmt;
	char * toks = buf;

	va_start(ap, fmt);
	while (*f) {
		switch (*f) {
		case 'd': scanint(&toks, va_arg(ap, int*)); break;
		case 'f': scandbl(&toks, va_arg(ap, double*)); break;
		case 'b': scanbool(&toks, va_arg(ap, _Bool*)); break;
		case 'p': scanpt(&toks, va_arg(ap, Point*)); break;
		case 'r': scanrect(&toks, va_arg(ap, Rect*)); break;
		case 'y': scanbody(&toks, va_arg(ap, Body*)); break;
		case 'l': scanplayer(&toks, va_arg(ap, Player*)); break;
		case 'u': scanuint64_t(&toks, va_arg(ap, uint64_t*)); break;
		}
		f++;
	}
	va_end(ap);

	return *f == '\0';
}

static void scanint(char **toks, int *d)
{
	char *t = nxt(toks);
	long l = strtol(t, NULL, 10);
	if (l == LONG_MAX || l == LONG_MIN)
		fatal("Over/under-flow reading integer");
	*d = l;
}

static void scanuint64_t(char **toks, uint64_t *d)
{
	char *t = nxt(toks);
	unsigned long long l = strtoull(t, NULL, 10);

	if (l == ULLONG_MAX)
		fatal("Over/under-flow reading 64-bit unsigned integer");
	*d = l;
}

static void scandbl(char **toks, double *f)
{
	char *t = nxt(toks);
	double l = strtod(t, NULL);
	*f = l;
}

static void scanbool(char **toks, _Bool *b)
{
	int i;
	scanint(toks, &i);
	*b = i;
}

static void scanpt(char **toks, Point *pt)
{
	scandbl(toks, &pt->x);
	scandbl(toks, &pt->y);
}

static void scanrect(char **toks, Rect *r)
{
	scanpt(toks, &r->a);
	scanpt(toks, &r->b);
}

static void scanbody(char **toks, Body *b)
{
	scanrect(toks, &b->bbox);
	scanpt(toks, &b->vel);
	scanpt(toks, &b->acc);
	scanbool(toks, &b->fall);
}

static void scaninvit(char **toks, Invit *it)
{
	scanint(toks, (int*)&it->id);
	for (int i = 0; i < StatMax; i++)
		scanint(toks, &it->stats[i]);
}

static void scansword(char **toks, Sword *s)
{
	scanrect(toks, &s->rightloc[0]);
	scanrect(toks, &s->rightloc[1]);
	scanrect(toks, &s->leftloc[0]);
	scanrect(toks, &s->leftloc[1]);
	scanint(toks, (int*)&s->dir);
	scanint(toks, &s->cur);
	scanint(toks, &s->row);
}

static void scanplayer(char **toks, Player *p)
{
	scanint(toks, (int*) &p->dir);
	scanint(toks, (int*) &p->act);
	scanbody(toks, &p->body);
	scanbool(toks, &p->acting);
	scanbool(toks, &p->statup);
	scandbl(toks, &p->hitback);
	scanint(toks, &p->jframes);
	scanint(toks, &p->iframes);
	scanint(toks, &p->sframes);
	scanint(toks, &p->mframes);
	for (int i = 0; i < StatMax; i++)
		scanint(toks, &p->stats[i]);
	for (int i = 0; i < StatMax; i++)
		scanint(toks, &p->eqp[i]);
	scanint(toks, &p->curhp);
	scanint(toks, &p->curmp);
	scanint(toks, &p->lives);
	scanint(toks, &p->money);
	for (int i = 0; i < Maxinv; i++)
		scaninvit(toks, &p->inv[i]);
	for (int i = 0; i < EqpMax; i++)
		scaninvit(toks, &p->wear[i]);
	scansword(toks, &p->sw);
}

/* Get the next white/space delimited token. */
static char *nxt(char **toks)
{
	char *t = strtok(*toks, " \t\n\r");
	if (*toks)
		*toks = NULL;
	return t;
}

_Bool printgeom(char *buf, int sz, char *fmt, ...)
{
	va_list ap;
	char *f = fmt;
	int itms = 0;
	int n = 0;

	va_start(ap, fmt);
	while (*f) {
		if (n >= sz)
			break;
		switch (*f) {
		case 'd':
			prfield(&buf, &sz, " %d", va_arg(ap, int));
			break;
		case 'f':
			prfield(&buf, &sz, " %f", va_arg(ap, double));
			break;
		case 'b':
			prfield(&buf, &sz, " %d", va_arg(ap, int));
			break;
		case 'p':
			printpt(&buf, &sz, va_arg(ap, Point));
			break;
		case 'r':
			printrect(&buf, &sz, va_arg(ap, Rect));
			break;
		case 'y':
			printbody(&buf, &sz, va_arg(ap, Body));
			break;
		case 'l':
			printplayer(&buf, &sz, va_arg(ap, Player));
			break;
		case 'u':
			prfield(&buf, &sz, " %ull", (unsigned long long) va_arg(ap, uint64_t));
			break;
		}
		itms++;
		f++;
	}
	va_end(ap);

	return itms == strlen(fmt) && n < sz && sz > 0;
}

static void printpt(char **bufp, int *szp, Point p)
{
	prfield(bufp, szp, " %f %f", p.x, p.y);
}

static void printrect(char **bufp, int *szp, Rect r)
{
	printpt(bufp, szp, r.a);
	printpt(bufp, szp, r.b);
}

static void printbody(char **bufp, int *szp, Body b)
{
	printrect(bufp, szp, b.bbox);
	printpt(bufp, szp, b.vel);
	printpt(bufp, szp, b.acc);
	prfield(bufp, szp, " %d", b.fall);
}

static void printinvit(char **bufp, int *szp, Invit it)
{
	prfield(bufp, szp, " %d", it.id);
	for (int i = 0; i < StatMax; i++)
		prfield(bufp, szp, " %d", it.stats[i]);
}

static void printsword(char **bufp, int *szp, Sword s)
{
	printrect(bufp, szp, s.rightloc[0]);
	printrect(bufp, szp, s.rightloc[1]);
	printrect(bufp, szp, s.leftloc[0]);
	printrect(bufp, szp, s.leftloc[1]);
	prfield(bufp, szp, " %d", s.dir);
	prfield(bufp, szp, " %d", s.cur);
	prfield(bufp, szp, " %d", s.row);
}

static void printplayer(char **bufp, int *szp, Player p)
{
	prfield(bufp, szp, " %d", p.dir);
	prfield(bufp, szp, " %d", p.act);
	printbody(bufp, szp, p.body);
	prfield(bufp, szp, " %d", p.acting);
	prfield(bufp, szp, " %d", p.statup);
	prfield(bufp, szp, " %f", p.hitback);
	prfield(bufp, szp, " %d", p.jframes);
	prfield(bufp, szp, " %d", p.iframes);
	prfield(bufp, szp, " %d", p.sframes);
	prfield(bufp, szp, " %d", p.mframes);
	for (int i = 0; i < StatMax; i++)
		prfield(bufp, szp, " %d", p.stats[i]);
	for (int i = 0; i < StatMax; i++)
		prfield(bufp, szp, " %d", p.eqp[i]);
	prfield(bufp, szp, " %d", p.curhp);
	prfield(bufp, szp, " %d", p.curmp);
	prfield(bufp, szp, " %d", p.lives);
	prfield(bufp, szp, " %d", p.money);
	for (int i = 0; i < Maxinv; i++)
		printinvit(bufp, szp, p.inv[i]);
	for (int i = 0; i < EqpMax; i++)
		printinvit(bufp, szp, p.wear[i]);
	printsword(bufp, szp, p.sw);
}

static void prfield(char **bufp, int *szp, char *fmt, ...)
{
	va_list ap;

	if (*szp < 0)
		return;

	va_start(ap, fmt);
	int n = vsnprintf(*bufp, *szp, fmt, ap);
	va_end(ap);

	if (n < 0) {
		*szp = -1;
		return;
	}
	*bufp += n;
	*szp -= n;
}