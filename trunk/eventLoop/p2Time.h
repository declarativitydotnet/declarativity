// -*- c-basic-offset: 2; related-file-name: "p2Time.C" -*-
/*
 * @(#)$Id$
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: P2 time utilities
 *
 */

#ifndef __P2TIME_H__
#define __P2TIME_H__


#include <time.h>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "assert.h"
#include "p2core/value.h"

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
getTime(boost::posix_time::ptime& time,
        clockT clockDescriptor = LOOP_TIME_DEFAULT);

/** Set the default clock facility */
void
setDefaultClock(clockT clockDescriptor);

/** The default clock facility */
extern clockT
defaultClockDescriptor;



#endif // P2TIME
