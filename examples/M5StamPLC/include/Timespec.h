
/**
 * \file timespec.h
 *
 * Helpers to deal with timespec structures.
 */

#ifndef TIMESPEC_H
#define TIMESPEC_H

#include <stdint.h>
#include <assert.h>
#include <time.h>
#include <stdbool.h>


// nanoseconds
#define NSEC_PER_MICROSEC         1000LL
#define NSEC_PER_MILLISEC         1000000LL
#define NSEC_PER_TENTH_OF_SECOND  100000000LL
#define NSEC_PER_SEC              1000000000LL
#define NSEC_PER_MINUTE           (60 * 1000000000LL)

// microseconds
#define USEC_PER_MILLISEC         1000LL
#define USEC_PER_TENTH_OF_SECOND  100000LL
#define USEC_PER_SEC              1000000LL
#define USEC_PER_MINUTE           (60 * 1000000LL)

// milliseconds
#define MSEC_PER_TENTH_OF_SECOND  100LL
#define MSEC_PER_SEC              1000LL
#define MSEC_PER_MINUTE           (60 * 1000LL)

#define CLOCK_DOMAIN              CLOCK_REALTIME

#define TIMESPEC_STR_LEN          31

typedef char TIMESPEC_BUFFER [TIMESPEC_STR_LEN];

// all timespec
void timespec_now(struct timespec *r);
void timespec_now_mono(struct timespec *r);
int  timespec_to_str(TIMESPEC_BUFFER buf, struct timespec *ts);
void timespec_normalize(struct timespec *ts) ;
void timespec_add(struct timespec *r, const struct timespec *a, const struct timespec *b);
void timespec_sub(struct timespec *r, const struct timespec *a, const struct timespec *b);
bool timespec_eq(const struct timespec *a, const struct timespec *b);
bool timespec_is_after(const struct timespec *a, const struct timespec *b);
void timespec_clone(struct timespec *a, const struct timespec *b);
bool timespec_passed(const struct timespec *deadline);
bool timespec_is_zero(const struct timespec *a);

// nanoseconds
void timespec_from_nsec(struct timespec *r, int64_t b);
void timespec_add_nsec(struct timespec *r, const struct timespec *a, int64_t b);
void timespec_sub_nsec(struct timespec *r, const struct timespec *a, int64_t b);
int64_t timespec_now_to_nsec();
int64_t timespec_to_nsec(const struct timespec *a);
int64_t timespec_add_to_nsec(const struct timespec *a, const struct timespec *b);
int64_t timespec_add_nsec_to_nsec(const struct timespec *a, int64_t b);
int64_t timespec_sub_to_nsec(const struct timespec *a, const struct timespec *b);
int64_t timespec_sub_nsec_to_nsec(const struct timespec *a, int64_t b);

// microseconds
void timespec_from_usec(struct timespec *r, int64_t b);
void timespec_add_usec(struct timespec *r, const struct timespec *a, int64_t b);
void timespec_sub_usec(struct timespec *r, const struct timespec *a, int64_t b);
int64_t timespec_now_to_usec();
int64_t timespec_to_usec(const struct timespec *a);
int64_t timespec_add_to_usec(const struct timespec *a, const struct timespec *b);
int64_t timespec_add_usec_to_usec(const struct timespec *a, int64_t b);
int64_t timespec_sub_to_usec(const struct timespec *a, const struct timespec *b);
int64_t timespec_sub_usec_to_usec(const struct timespec *a, int64_t b);

// milliseconds
void timespec_from_msec(struct timespec *r, int64_t b);
void timespec_add_msec(struct timespec *r, const struct timespec *a, int64_t b);
void timespec_sub_msec(struct timespec *r, const struct timespec *a, int64_t b);
int64_t timespec_now_to_msec();
int64_t timespec_to_msec(const struct timespec *a);
int64_t timespec_add_to_msec(const struct timespec *a, const struct timespec *b);
int64_t timespec_add_msec_to_msec(const struct timespec *a, int64_t b);
int64_t timespec_sub_to_msec(const struct timespec *a, const struct timespec *b);
int64_t timespec_sub_msec_to_msec(const struct timespec *a, int64_t b);

#endif /* TIMESPEC_H */