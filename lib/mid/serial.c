// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
#include "../../include/mid.h"
#include "../../include/log.h"
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

static void scanint(char **toks, int *d);
static void scandbl(char **toks, double *f);
static void scanbool(char **toks, _Bool *b);
static void scanpt(char **toks, Point *pt);
static void scanrect(char **toks, Rect *r);
static void scanbody(char **toks, Body *b);
static char *nxt(char **toks);
static void printpt(char **bufp, int *szp, Point p);
static void printrect(char **bufp, int *szp, Rect r);
static void printbody(char **bufp, int *szp, Body b);
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
	scanpt(toks, &b->a);
	scanbool(toks, &b->fall);
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
	printpt(bufp, szp, b.a);
	prfield(bufp, szp, " %d", b.fall);
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