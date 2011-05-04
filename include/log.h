/* Print the message, optionally with the time. */
void pr(_Bool prtime, const char *fmt, ...);

/* Print the message followed by an with an error string given an
 * errno. */
void prerr(_Bool prtime, int err, const char *fmt, ...);

void abort(void);

#define fail(...)				\
	do {					\
		pr(true, "%s: ", __func__);	\
		pr(false, __VA_ARGS);		\
		abort();			\
	} while (0)

extern int errno;

#define failerr(...)					\
	do {						\
		prerr(true, errno, "%s: ", __func__);	\
		pr(false, __VA_ARGS);			\
		abort();				\
	} while (0)
