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

int main()
{
	loginit(NULL);
	test_rectdist();
	pr("Geom tests passed");
	logclose();
	return EXIT_SUCCESS;
}
