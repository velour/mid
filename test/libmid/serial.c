/* © 2013 the Mid Authors under the MIT license. See AUTHORS for the list of authors.*/

#include "../../include/mid.h"
#include "../../include/log.h"

enum { Bufsz = 256 };

static void testint()
{
	char buf[Bufsz];

	pr("printing 5");
	_Bool ok = printgeom(buf, Bufsz, "d", 5);
	if (!ok)
		fatal("printgeom failed");
	pr("buf=[%s]", buf);
	int i;
	ok = scangeom(buf, "d", &i);
	if (!ok)
		fatal("scangeom failed");
	pr("i=[%d]", i);
	if (i != 5)
		fatal("Scanned wrong values");

	/*  */

	pr("printing 5 6");
	ok = printgeom(buf, Bufsz, "dd", 5, 6);
	if (!ok)
		fatal("printgeom failed");
	pr("buf=[%s]", buf);
	int j;
	ok = scangeom(buf, "dd", &i, &j);
	if (!ok)
		fatal("scangeom failed");
	pr("i=[%d], j=[%d]", i, j);
	if (i != 5 || j != 6)
		fatal("Scanned wrong values");
}

static void testdbl()
{
	char buf[Bufsz];

	pr("printing 5.0");
	_Bool ok = printgeom(buf, Bufsz, "f", 5.0);
	if (!ok)
		fatal("printgeom failed");
	pr("buf=[%s]", buf);
	double f;
	ok = scangeom(buf, "f", &f);
	if (!ok)
		fatal("scangeom failed");
	pr("f=[%f]", f);
	if (f != 5.0)
		fatal("Scanned wrong values");

	/*  */

	pr("printing 5.0 6.0");
	ok = printgeom(buf, Bufsz, "ff", 5.0, 6.0);
	if (!ok)
		fatal("printgeom failed");
	pr("buf=[%s]", buf);
	double g;
	ok = scangeom(buf, "ff", &f, &g);
	if (!ok)
		fatal("scangeom failed");
	pr("f=[%f], g=[%f]", f, g);
	if (f != 5.0 || g != 6.0)
		fatal("Scanned wrong values");
}

static void testpt()
{
	char buf[Bufsz];

	pr("printing 5.0, 6.0");
	_Bool ok = printgeom(buf, Bufsz, "p", (Point) {5.0, 6.0});
	if (!ok)
		fatal("printgeom failed");
	pr("buf=[%s]", buf);
	Point p;
	ok = scangeom(buf, "p", &p);
	if (!ok)
		fatal("scangeom failed");
	pr("p=[x=%g, y=%g]", p.x, p.y);
	if (p.x != 5.0 || p.y != 6.0)
		fatal("Scanned wrong values");

	/*  */

	pr("printing 5.0, 6.0 and 7.0, 8.0");
	ok = printgeom(buf, Bufsz, "pp", (Point) {5.0, 6.0}, (Point) {7.0, 8.0});
	if (!ok)
		fatal("printgeom failed");
	pr("buf=[%s]", buf);
	Point q;
	ok = scangeom(buf, "pp", &p, &q);
	if (!ok)
		fatal("scangeom failed");
	pr("p=[x=%g, y=%g], q=[x=%g, y=%g]", p.x, p.y, q.x, q.y);
	if (p.x != 5.0 || p.y != 6.0 || q.x != 7.0 || q.y != 8.0)
		fatal("Scanned wrong values");
}

int main()
{
	loginit(NULL);
	testint();
	testdbl();
	testpt();
}
