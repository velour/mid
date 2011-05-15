#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../../include/mid.h"
#include "../../include/log.h"

/* A sloppy equal for comparing floating point values. */
bool fleq(float a, float b)
{
	return a - b < 0.000001 && a - b > -0.000001;
}

void test_rectdist()
{
	Rect a = (Rect){ {0, 0}, {32, 32} };
	Rect b = (Rect){ {0, 64}, {32, 96} };
	Point d = rectdist(a, b);
	if (!fleq(d.x, 0))
		fatal("d.x == %g, expected 0", d.x);
	if (!fleq(d.y, 32))
		fatal("d.y == %g, expected 32", d.y);
	d = rectdist(b, a);
	if (!fleq(d.x, 0))
		fatal("d.x == %g, expected 0", d.x);
	if (!fleq(d.y, -32))
		fatal("d.y == %g, expected -32", d.y);

	b = (Rect){ {64, 0}, {96, 0} };
	d = rectdist(a, b);
	if (!fleq(d.x, 32))
		fatal("d.x == %g, expected 32", d.x);
	if (!fleq(d.y, 0))
		fatal("d.y == %g, expected 0", d.y);
	d = rectdist(b, a);
	if (!fleq(d.x, -32))
		fatal("d.x == %g, expected -32", d.x);
	if (!fleq(d.y, 0))
		fatal("d.y == %g, expected 0", d.y);

	b = (Rect){ {64, 64}, {96, 96} };
	d = rectdist(a, b);
	if (!fleq(d.x, 32))
		fatal("d.x == %g, expected 32", d.x);
	if (!fleq(d.y, 32))
		fatal("d.y == %g, expected 32", d.y);
	d = rectdist(b, a);
	if (!fleq(d.x, -32))
		fatal("d.x == %g, expected -32", d.x);
	if (!fleq(d.y, -32))
		fatal("d.y == %g, expected -32", d.y);

	b = (Rect){ {65, 64}, {97, 96} };
	d = rectdist(a, b);
	if (!fleq(d.x, 33))
		fatal("d.x == %g, expected 33", d.x);
	if (!fleq(d.y, 32))
		fatal("d.y == %g, expected 32", d.y);
	d = rectdist(b, a);
	if (!fleq(d.x, -33))
		fatal("d.x == %g, expected -33", d.x);
	if (!fleq(d.y, -32))
		fatal("d.y == %g, expected -32", d.y);
}

void test_recttrace1()
{
	Rect a = (Rect){ {0, 0}, {32, 32} };
	Rect b = (Rect){ {33, 0}, {65, 32} };

	Point v = (Point) { 1, 0 };
	v = recttrace1(a, v, b);
	if (!fleq(v.y, 0))
		fatal("v.y == %g, expected 0", v.y);
	if (!fleq(v.x, 1))
		fatal("v.x == %g, expected 1", v.x);

	v = (Point) { 2, 0 };
	v = recttrace1(a, v, b);
	if (!fleq(v.y, 0))
		fatal("v.y == %g, expected 0", v.y);
	if (!fleq(v.x, 1))
		fatal("v.x == %g, expected 1", v.x);

	v = (Point) { -1, 0 };
	v = recttrace1(a, v, b);
	if (!fleq(v.y, 0))
		fatal("v.y == %g, expected 0", v.y);
	if (!fleq(v.x, -1))
		fatal("v.x == %g, expected -1", v.x);

	v = (Point) { 5, 2 };
	v = recttrace1(a, v, b);
	if (!fleq(v.y, 1))
		fatal("v.y == %g, expected 1", v.y);
	if (!fleq(v.x, 1))
		fatal("v.x == %g, expected 1", v.x);

	a = (Rect){ {0, 0}, {32, 32} };
	b = (Rect){ {0, 33}, {32, 65} };

	v = (Point) { 0, 1 };
	v = recttrace1(a, v, b);
	if (!fleq(v.y, 1))
		fatal("v.y == %g, expected 1", v.y);
	if (!fleq(v.x, 0))
		fatal("v.x == %g, expected 0", v.x);

	v = (Point) { 0, 2 };
	v = recttrace1(a, v, b);
	if (!fleq(v.y, 1))
		fatal("v.y == %g, expected 1", v.y);
	if (!fleq(v.x, 0))
		fatal("v.x == %g, expected 0", v.x);

	v = (Point) { 0, -1 };
	v = recttrace1(a, v, b);
	if (!fleq(v.y, -1))
		fatal("v.y == %g, expected -1", v.y);
	if (!fleq(v.x, 0))
		fatal("v.x == %g, expected 0", v.x);

	v = (Point) { 2, 5 };
	v = recttrace1(a, v, b);
	if (!fleq(v.y, 1))
		fatal("v.y == %g, expected 1", v.y);
	if (!fleq(v.x, 1))
		fatal("v.x == %g, expected 1", v.x);
}

int main()
{
	loginit(NULL);
	test_rectdist();
	test_recttrace1();
	pr("Geom tests passed");
	logclose();
	return EXIT_SUCCESS;
}
