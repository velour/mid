/* All functions automatically print the newline. */
#include <errno.h>

/* Print the message, optionally with the time. */
void pr(const char *fmt, ...);

/* Print the message followed by an with an error string given an
 * errno. */
void prerr(int err, const char *fmt, ...);

/* Prints the message without the time. */
void prraw(const char *fmt, ...);

void abort(void);

#define fail(...)				\
	do {					\
		pr("%s: ", __func__);	\
		prraw(__VA_ARGS__);		\
		abort();			\
	} while (0)

#define failerr(...)					\
	do {						\
		prerr(errno, "%s: ", __func__);	\
		prraw(__VA_ARGS__);			\
		abort();				\
	} while (0)
