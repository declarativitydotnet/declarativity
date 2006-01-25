// -*- c-basic-offset: 2; related-file-name: "p2Time.C" -*-
/*
 * @(#)$Id$
 *
 * Copyright (c) 2005 Intel Corporation
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: P2 time utilities
 *
 */

#ifndef __P2TIME_H__
#define __P2TIME_H__


#include <time.h>
#include "assert.h"

/** Clock facilities */
enum
clockT {
  // The real, system-wide, local-time-zone clock
  LOOP_TIME_WALLCLOCK,
  // The default clock, whatever that might be
  LOOP_TIME_DEFAULT
};


/** Return the current time, according to the suggested timing facility,
    or the default if no facility is specified.
    */
void
getTime(struct timespec& time,
        clockT clockDescriptor = LOOP_TIME_DEFAULT);

/** Subtract two timespecs and store the difference */
void
subtract_timespec(struct timespec& difference,
                  struct timespec& minuend,
                  struct timespec& subtrahend);


/** Compare two timespecs, and return -1 if the first is less, 0 if they
    are equal, +1 if the first is greater. They are assumed well formed,
    that is, their tv_nsec fields contain fewer than 10^9
    nanoseconds. */
int
compare_timespec(const struct timespec& first,
                 const struct timespec& second);


/** Increment (in place) a timespec by a given number (floating-point)
    of seconds
    */
void
increment_timespec(struct timespec& ts,
                   double seconds);



/** Set the default clock facility */
void
setDefaultClock(clockT clockDescriptor);

/** The default clock facility */
extern clockT
defaultClockDescriptor;



#endif // P2TIME
