/* © 2013 the Mid Authors under the MIT license. See AUTHORS for the list of authors.*/

/* All functions automatically print the newline. */

int loginit(const char *fname);
void logclose(void);

void pr(const char *fmt, ...);
void prerr(int err, const char *fmt, ...);
void prfn(const char *func, const char *fmt, ...);
void prfnerr(const char *func, int err, const char *fmt, ...);
void flushlog(void);

void exit(int);
void abort(void);

#define die(...) \
	do {\
		prfn(__func__, __VA_ARGS__);\
		exit(1);\
	} while(0)

#define fatal(...)				\
	do {					\
		prfn(__func__, __VA_ARGS__);	\
		flushlog();			\
		abort();			\
	} while (0)

#define fatalerr(err, ...)				\
	do {						\
		prfnerr(__func__, err, __VA_ARGS__);	\
		flushlog();				\
		abort();				\
	} while (0)
