

/**
 * Helpers to deal with timespec structures.
 *
 * We need time calculations for application profiling and serial line timing,
 * not an absolute time (dd.mm.yy hh24:mi:ss) including timezones. For time
 * difference calculations we convert timespec into int64_t instant time as
 * a number of nanosecond ticks since 1.1.1970 (similar to Java Instant class).
 *
 * int64_t is OK, saving time up to 2262-04-12 and makes no issues in the
 *         division and modulo calculations
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "Timespec.h"

/**
* Returns a normalized version of a timespec structure, according to the
* following rules:
*
* 1) If tv_nsec is >1,000,000,00 or <-1,000,000,000, flatten the surplus
*    nanoseconds into the tv_sec field.
*
* 2) If tv_sec is >0 and tv_nsec is <0, decrement tv_sec and roll tv_nsec up
*    to represent the same value on the positive side of the new tv_sec.
*
* 3) If tv_sec is <0 and tv_nsec is >0, increment tv_sec and roll tv_nsec down
*    to represent the same value on the negative side of the new tv_sec.
*
* @param timespec with any property values (negative, overflow, ...)
*/
void timespec_normalize(struct timespec *ts)
{
    while(ts->tv_nsec >= NSEC_PER_SEC)
    {
        ++(ts->tv_sec);
        ts->tv_nsec -= NSEC_PER_SEC;
    }

    while(ts->tv_nsec <= -NSEC_PER_SEC)
    {
        --(ts->tv_sec);
        ts->tv_nsec += NSEC_PER_SEC;
    }

    if(ts->tv_nsec < 0 && ts->tv_sec > 0)
    {
        /* Negative nanoseconds while seconds is positive.
         * Decrement tv_sec and roll tv_nsec over.
        */

        --(ts->tv_sec);
        ts->tv_nsec = NSEC_PER_SEC - (-1 * ts->tv_nsec);
    }
    else if(ts->tv_nsec > 0 && ts->tv_sec < 0)
    {
        /* Positive nanoseconds while seconds is negative.
         * Increment tv_sec and roll tv_nsec over.
        */

        ++(ts->tv_sec);
        ts->tv_nsec = -NSEC_PER_SEC - (-1 * ts->tv_nsec);
    }
}

/**
 * Add two timespec structures into timespec structure
 *
 * @param r[out] result: a + b as timespec structure
 * @param a[in] operand timespec
 * @param b[in] operand timespec
 */
void timespec_add(struct timespec *r, const struct timespec *a, const struct timespec *b)
{
    r->tv_sec = a->tv_sec + b->tv_sec;
    r->tv_nsec = a->tv_nsec + b->tv_nsec;
    if (r->tv_nsec > NSEC_PER_SEC) {
        r->tv_sec++;
        r->tv_nsec -= NSEC_PER_SEC;
    }
}

/**
 * Subtract two timespec structures into timespec structure
 *
 * @param r[out] result: a - b as timespec structure
 * @param a[in] operand timespec
 * @param b[in] operand timespec
 */
void timespec_sub(struct timespec *r, const struct timespec *a, const struct timespec *b)
{
    r->tv_sec = a->tv_sec - b->tv_sec;
    r->tv_nsec = a->tv_nsec - b->tv_nsec;
    if (r->tv_nsec < 0) {
        r->tv_sec--;
        r->tv_nsec += NSEC_PER_SEC;
    }
}

/**
 * Add a nanoseconds to a timespec
 *
 * @param r[out] result: a + b as timespec structure
 * @param a[in] base operand as timespec
 * @param b[in] operand in nanoseconds
 */
void timespec_add_nsec(struct timespec *r, const struct timespec *a, int64_t b)
{
    r->tv_sec = a->tv_sec + (b / NSEC_PER_SEC);
    r->tv_nsec = a->tv_nsec + (b % NSEC_PER_SEC);

    if (r->tv_nsec >= NSEC_PER_SEC) {
        r->tv_sec++;
        r->tv_nsec -= NSEC_PER_SEC;
    } else if (r->tv_nsec < 0) {
        r->tv_sec--;
        r->tv_nsec += NSEC_PER_SEC;
    }
}

/**
 * Subtract a nanoseconds from a timespec
 *
 * @param r[out] result: a - b as timespec structure
 * @param a[in] base operand as timespec (maybe same as r)
 * @param b[in] operand in nanoseconds
 */
void timespec_sub_nsec(struct timespec *r, const struct timespec *a, int64_t b)
{
    r->tv_sec  = a->tv_sec  - (b / NSEC_PER_SEC);
    r->tv_nsec = a->tv_nsec - (b % NSEC_PER_SEC);

    if (r->tv_nsec >= NSEC_PER_SEC) {
        r->tv_sec++;
        r->tv_nsec -= NSEC_PER_SEC;
    } else if (r->tv_nsec < 0) {
        r->tv_sec--;
        r->tv_nsec += NSEC_PER_SEC;
    }
}

/**
 * Add milliseconds to a timespec
 *
 * @param r[out] result: a + b as timespec structure
 * @param a[in] base operand as timespec
 * @param b[in] operand in milliseconds
 */
void timespec_add_msec(struct timespec *r, const struct timespec *a, int64_t b)
{
    timespec_add_nsec(r, a, b * NSEC_PER_MILLISEC);
}

/**
 * Subtract milliseconds from a timespec
 *
 * @param r[out] result: a - b as timespec structure
 * @param a[in] base operand as timespec
 * @param b[in] operand in milliseconds
 */
void timespec_sub_msec(struct timespec *r, const struct timespec *a, int64_t b)
{
    timespec_sub_nsec(r, a, b * NSEC_PER_MILLISEC);
}

/**
 * Add microseconds to a timespec
 *
 * @param r[out] result: a + b as timespec structure
 * @param a[in] base operand as timespec
 * @param b[in] operand in microseconds
 */
void timespec_add_usec(struct timespec *r, const struct timespec *a, int64_t b)
{
    timespec_add_nsec(r, a, b * NSEC_PER_MICROSEC);
}

/**
 * Subtract microseconds from a timespec
 *
 * @param r[out] result: a - b as timespec structure
 * @param a[in] base operand as timespec
 * @param b[in] operand in microseconds
 */
void timespec_sub_usec(struct timespec *r, const struct timespec *a, int64_t b)
{
    timespec_sub_nsec(r, a, b * NSEC_PER_MICROSEC);
}

/**
 * Add two timespec and return result in nanoseconds
 *
 * @param a[in] base operand as timespec
 * @param b[in] operand as timespec
 * @return to_nanoseconds(a + b)
 */
int64_t timespec_add_to_nsec(const struct timespec *a, const struct timespec *b)
{
    struct timespec r;
    timespec_add(&r, a, b);
    return timespec_to_nsec(&r);
}

/**
 * Add two timespec and return result in microseconds
 *
 * @param a[in] base operand as timespec
 * @param b[in] operand as timespec
 * @return to_microseconds(a + b)
 */
int64_t timespec_add_to_usec(const struct timespec *a, const struct timespec *b)
{
    struct timespec r;
    timespec_add(&r, a, b);
    return timespec_to_usec(&r);
}

/**
 * Add two timespec and return result in milliseconds
 *
 * @param a[in] base operand as timespec
 * @param b[in] operand as timespec
 * @return to_milliseconds(a + b)
 */
int64_t timespec_add_to_msec(const struct timespec *a, const struct timespec *b)
{
    struct timespec r;
    timespec_add(&r, a, b);
    return timespec_to_msec(&r);
}

/**
 * Add microseconds to a timespec and return result in microseconds
 *
 * @param a[in] operand as timespec
 * @param b[in] operand in microseconds
 * @return to_microseconds(a + b)
 */
int64_t timespec_add_usec_to_usec(const struct timespec *a, int64_t b)
{
    uint64_t usec = timespec_to_usec(a);
    return usec + b ;
}


/**
 * Subtract microseconds from a timespec and return result in microseconds
 *
 * @param a[in] operand as timespec
 * @param b[in] operand in microseconds
 * @return to_microseconds(a - b)
 */
int64_t timespec_sub_usec_to_usec(const struct timespec *a, int64_t b)
{
    uint64_t usec = timespec_to_usec(a);
    return usec - b ;
}

/**
 * Add milliseconds to a timespec and return result in milliseconds
 *
 * @param a[in] operand as timespec
 * @param b[in] operand in milliseconds
 * @return to_milliseconds(a + b)
 */
int64_t timespec_add_msec_to_msec(const struct timespec *a, int64_t b)
{
    uint64_t msec = timespec_to_msec(a);
    return msec + b ;
}

/**
 * Subtract milliseconds from a timespec and return result in milliseconds
 *
 * @param a[in] operand as timespec
 * @param b[in] operand in milliseconds
 * @return to_milliseconds(a + b)
 */
int64_t timespec_sub_msec_to_msec(const struct timespec *a, int64_t b)
{
    uint64_t msec = timespec_to_msec(a);
    return msec - b ;
}



/**
 * Add nanoseconds to a timespec and return result in nanoseconds
 *
 * @param a[in] operand as timespec
 * @param b[in] operand in nanoseconds
 * @return to_nanoseconds(a + b)
 */
int64_t timespec_add_nsec_to_nsec(const struct timespec *a, int64_t b)
{
    int64_t nsec = timespec_to_nsec(a);
    return nsec + b ;
}


/**
 * Subtract nanoseconds from a timespec and return result in nanoseconds
 *
 * @param a[in] operand as timespec
 * @param b[in] operand in nanoseconds
 * @return to_nanoseconds(a - b)
 */
int64_t timespec_sub_nsec_to_nsec(const struct timespec *a, int64_t b)
{
    int64_t nsec = timespec_to_nsec(a);
    return nsec - b ;
}

/**
 * Convert timespec to nanoseconds
 *
 * @param a timespec
 * @return nanoseconds
 */
int64_t timespec_to_nsec(const struct timespec *a)
{
    return (int64_t)a->tv_sec * NSEC_PER_SEC + a->tv_nsec;
}

/**
* Get current time as timespec for domain CLOCK_DOMAIN
*
* @param r timespec structure
*/
void timespec_now(struct timespec *r)
{
    clock_gettime(CLOCK_DOMAIN, r);
}

/**
* Get current time as timespec for domain CLOCK_MONOTONIC
*
* @param r timespec structure
*/
void timespec_now_mono(struct timespec *r)
{
    clock_gettime(CLOCK_MONOTONIC, r);
}


/**
 * Returns current time in nanoseconds
 *
 * @return nanoseconds
 */
int64_t timespec_now_to_nsec()
{
    struct timespec now;
    clock_gettime(CLOCK_DOMAIN, &now);
    return timespec_to_nsec(&now);
}

/**
 * Returns current time in microseconds
 *
 * @return microseconds
 */
int64_t timespec_now_to_usec()
{
    struct timespec now;
    clock_gettime(CLOCK_DOMAIN, &now);
    return timespec_to_usec(&now);
}

/**
 * Returns current time in milliseconds
 *
 * @return milliseconds
 */
int64_t timespec_now_to_msec()
{
    struct timespec now;
    clock_gettime(CLOCK_DOMAIN, &now);
    return timespec_to_msec(&now);
}

/**
 * Subtract two timespec and return result in nanoseconds
 *
 * @param a[in] operand as timespec
 * @param b[in] operand as timespec
 * @return to_nanoseconds(a - b)
 */
int64_t timespec_sub_to_nsec(const struct timespec *a, const struct timespec *b)
{
    struct timespec r;
    timespec_sub(&r, a, b);
    return timespec_to_nsec(&r);
}

/**
 * Convert timespec to milliseconds
 *
 * @param a timespec
 * @return milliseconds
 *
 * Rounding to integer milliseconds happens always down (floor()).
 */
int64_t timespec_to_msec(const struct timespec *a)
{
    return (int64_t)a->tv_sec * MSEC_PER_SEC + a->tv_nsec / NSEC_PER_MILLISEC;
}

/**
 * Subtract two timespec and return result in milliseconds
 *
 * @param a[in] operand as timespec
 * @param b[in] operand as timespec
 * @return to_milliseconds(a - b)
 */
int64_t timespec_sub_to_msec(const struct timespec *a, const struct timespec *b)
{
    struct timespec r;
    timespec_sub(&r, a, b);
    return timespec_to_msec(&r);
}

/**
 * Subtract two timespec and return result in microseconds
 *
 * @param a[in] operand as timespec
 * @param b[in] operand as timespec
 * @return to_microseconds(a - b)
 */
int64_t timespec_sub_to_usec(const struct timespec *a, const struct timespec *b)
{
    struct timespec r;
    timespec_sub(&r, a, b);
    return timespec_to_usec(&r);
}

/**
 * Convert timespec to microseconds
 *
 * @param a timespec
 * @return microseconds
 *
 * Rounding to integer microseconds happens always down (floor()).
 */
int64_t timespec_to_usec(const struct timespec *a)
{
    return (int64_t)a->tv_sec * USEC_PER_SEC + a->tv_nsec / NSEC_PER_MICROSEC;
}

/**
 * Convert nanoseconds to timespec
 *
 * @param[out] r timespec
 * @param      b nanoseconds
 */
void timespec_from_nsec(struct timespec *r, int64_t b)
{
    r->tv_sec  = b / NSEC_PER_SEC;
    r->tv_nsec = b % NSEC_PER_SEC;
}

/**
 * Convert microseconds to timespec
 *
 * @param [out] r timespec
 * @param       b microseconds
 */
void timespec_from_usec(struct timespec *r, int64_t b)
{
    timespec_from_nsec(r, b * NSEC_PER_MICROSEC);
}

/**
 * Convert milliseconds to timespec
 *
 * @param[out] r timespec
 * @param      b milliseconds
 */
void timespec_from_msec(struct timespec *r, int64_t b)
{
    timespec_from_nsec(r, b * NSEC_PER_MILLISEC);
}

/**
 * Check if a timespec is zero
 *
 * @param a timespec
 * @return true if timespec is zero
 */
bool timespec_is_zero(const struct timespec *a)
{
    return a->tv_sec == 0 && a->tv_nsec == 0;
}

/**
 * Check if two timespec are equal
 *
 * @param a[in] timespec to check
 * @param b[in] timespec to check
 * @return true if timespec a and b are equal
 */
bool timespec_eq(const struct timespec *a, const struct timespec *b)
{
    return a->tv_sec == b->tv_sec && a->tv_nsec == b->tv_nsec;
}

/**
 * Checks whether a timespec value is after another
 *
 * @param a[in] timespec to compare
 * @param b[in] timespec to compare
 * @return true if a is after b
 */
bool timespec_is_after(const struct timespec *a, const struct timespec *b)
{
    return (a->tv_sec == b->tv_sec) ?
           (a->tv_nsec > b->tv_nsec) :
           (a->tv_sec > b->tv_sec);
}

/**
 * Create a timespec clone
 *
 * @param r[out] result: clone
 * @param a[in] operand original
 */
void timespec_clone(struct timespec *r, const struct timespec *a) {
    r->tv_sec  = a->tv_sec;
    r->tv_nsec = a->tv_nsec;
}


/**
 * Checks whether a timespec value is after the current time
 *
 * @param deadline[in] timespec to compare
 * @return true if deadline is after the current time
 */
bool timespec_passed(const struct timespec *deadline)
{
    struct timespec current_time;
    clock_gettime(CLOCK_DOMAIN, &current_time);
    return timespec_is_after(&current_time, deadline);
}

/**
 * Convert timespec to human readable string
 *
 * @param buf output string buffer
 * @param ts  timespec to convert
 * @return    0 if conversion ok
 *            > 0 error
 */
int timespec_to_str(TIMESPEC_BUFFER buf, struct timespec *ts) {

    int len = TIMESPEC_STR_LEN ;
    int ret;
    struct tm t;

    tzset();
    if (localtime_r(&(ts->tv_sec), &t) == NULL) {
        return 1;
    }

    ret = strftime(buf, len, "%F %T", &t);

    if (ret == 0) {
        return 2;
    }

    len -= ret - 1;

    ret = snprintf(&buf[strlen(buf)], len, ".%09ld", ts->tv_nsec);
    if (ret >= len) {
        return 3;
    }

    return 0;
}

