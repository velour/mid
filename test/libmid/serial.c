#include "../../include/mid.h"
#include "../../include/log.h"

enum { Bufsz = 256 };

static void testint()
{
	char buf[Bufsz];

	pr("printing 5");
	_Bool ok = printbuf(buf, Bufsz, "d", 5);
	if (!ok)
		fatal("printbuf failed");
	pr("buf=[%s]", buf);
	int i;
	ok = scanbuf(buf, "d", &i);
	if (!ok)
		fatal("scanbuf failed");
	pr("i=[%d]", i);
	if (i != 5)
		fatal("Scanned wrong values");

	/*  */

	pr("printing 5 6");
	ok = printbuf(buf, Bufsz, "dd", 5, 6);
	if (!ok)
		fatal("printbuf failed");
	pr("buf=[%s]", buf);
	int j;
	ok = scanbuf(buf, "dd", &i, &j);
	if (!ok)
		fatal("scanbuf failed");
	pr("i=[%d], j=[%d]", i, j);
	if (i != 5 || j != 6)
		fatal("Scanned wrong values");
}

static void testdbl()
{
	char buf[Bufsz];

	pr("printing 5.0");
	_Bool ok = printbuf(buf, Bufsz, "f", 5.0);
	if (!ok)
		fatal("printbuf failed");
	pr("buf=[%s]", buf);
	double f;
	ok = scanbuf(buf, "f", &f);
	if (!ok)
		fatal("scanbuf failed");
	pr("f=[%f]", f);
	if (f != 5.0)
		fatal("Scanned wrong values");

	/*  */

	pr("printing 5.0 6.0");
	ok = printbuf(buf, Bufsz, "ff", 5.0, 6.0);
	if (!ok)
		fatal("printbuf failed");
	pr("buf=[%s]", buf);
	double g;
	ok = scanbuf(buf, "ff", &f, &g);
	if (!ok)
		fatal("scanbuf failed");
	pr("f=[%f], g=[%f]", f, g);
	if (f != 5.0 || g != 6.0)
		fatal("Scanned wrong values");
}

static void testpt()
{
	char buf[Bufsz];

	pr("printing 5.0, 6.0");
	_Bool ok = printbuf(buf, Bufsz, "p", (Point) {5.0, 6.0});
	if (!ok)
		fatal("printbuf failed");
	pr("buf=[%s]", buf);
	Point p;
	ok = scanbuf(buf, "p", &p);
	if (!ok)
		fatal("scanbuf failed");
	pr("p=[x=%g, y=%g]", p.x, p.y);
	if (p.x != 5.0 || p.y != 6.0)
		fatal("Scanned wrong values");

	/*  */

	pr("printing 5.0, 6.0 and 7.0, 8.0");
	ok = printbuf(buf, Bufsz, "pp", (Point) {5.0, 6.0}, (Point) {7.0, 8.0});
	if (!ok)
		fatal("printbuf failed");
	pr("buf=[%s]", buf);
	Point q;
	ok = scanbuf(buf, "pp", &p, &q);
	if (!ok)
		fatal("scanbuf failed");
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
