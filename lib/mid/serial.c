// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
#include "../../include/mid.h"
#include "../../include/log.h"
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

typedef struct Toks Toks;
struct Toks {
	char *str;
	char *sv;
};

static void scanint(Toks *toks, int *d);
static void scandbl(Toks *toks, double *f);
static void scanbool(Toks *toks, _Bool *b);
static void scanpt(Toks *toks, Point *pt);
static void scanrect(Toks *toks, Rect *r);
static void scanbody(Toks *toks, Body *b);
static char *nxt(Toks *toks);
static int printpt(char *buf, size_t sz, Point p);
static int printrect(char *buf, size_t sz, Rect r);
static int printbody(char *buf, size_t sz, Body b);

_Bool scangeom(char *buf, char *fmt, ...)
{
	va_list ap;
	char *f = fmt;
	Toks toks = (Toks) { buf };

	va_start(ap, fmt);
	while (*f) {
		switch (*f) {
		case 'd': scanint(&toks, va_arg(ap, int*)); break;
		case 'f': scandbl(&toks, va_arg(ap, double*)); break;
		case 'b': scanbool(&toks, va_arg(ap, _Bool*)); break;
		case 'p': scanpt(&toks, va_arg(ap, Point*)); break;
		case 'r': scanrect(&toks, va_arg(ap, Rect*)); break;
		case 'y': scanbody(&toks, va_arg(ap, Body*)); break;
		}
		f++;
	}
	va_end(ap);

	return *f == '\0';
}

static void scanint(Toks *toks, int *d)
{
	char *t = nxt(toks);
	long l = strtol(t, NULL, 10);
	if (l == LONG_MAX || l == LONG_MIN)
		fatal("Over/under-flow reading integer");
	*d = l;
}

static void scandbl(Toks *toks, double *f)
{
	char *t = nxt(toks);
	double l = strtod(t, NULL);
	*f = l;
}

static void scanbool(Toks *toks, _Bool *b)
{
	int i;
	scanint(toks, &i);
	*b = i;
}

static void scanpt(Toks *toks, Point *pt)
{
	scandbl(toks, &pt->x);
	scandbl(toks, &pt->y);
}

static void scanrect(Toks *toks, Rect *r)
{
	scanpt(toks, &r->a);
	scanpt(toks, &r->b);
}

static void scanbody(Toks *toks, Body *b)
{
	scanrect(toks, &b->bbox);
	scanpt(toks, &b->vel);
	scanpt(toks, &b->a);
	scanbool(toks, &b->fall);
}

/* Get the next white/space delimited token. */
static char *nxt(Toks *toks)
{
	char *t = strtok_r(toks->str, " \t\n\r", &toks->sv);
	if (toks->str)
		toks->str = NULL;
	return t;
}

_Bool printgeom(char *buf, size_t sz, char *fmt, ...)
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
			n += snprintf(buf+n, sz-n, " %d", va_arg(ap, int));
			break;
		case 'f':
			n += snprintf(buf+n, sz-n, " %f", va_arg(ap, double));
			break;
		case 'b':
			n += snprintf(buf+n, sz-n, " %d", va_arg(ap, int));
			break;
		case 'p':
			n += printpt(buf+n, sz-n, va_arg(ap, Point));
			break;
		case 'r':
			n += printrect(buf+n, sz-n, va_arg(ap, Rect));
			break;
		case 'y':
			n += printbody(buf+n, sz-n, va_arg(ap, Body));
			break;
		}
		itms++;
		f++;
	}
	va_end(ap);

	return itms == strlen(fmt) && n < sz;
}

static int printpt(char *buf, size_t sz, Point p)
{
	return snprintf(buf, sz, " %f %f", p.x, p.y);
}

static int printrect(char *buf, size_t sz, Rect r)
{
	int n = printpt(buf, sz, r.a);
	return n + printpt(buf+n, sz-n, r.b);
}

static int printbody(char *buf, size_t sz, Body b)
{
	int n = printrect(buf, sz, b.bbox);
	n += printpt(buf+n, sz-n, b.vel);
	n += printpt(buf+n, sz-n, b.a);
	return n + snprintf(buf+n, sz-n, " %d", b.fall);
}
