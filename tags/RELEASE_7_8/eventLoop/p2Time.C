// -*- c-basic-offset: 2; related-file-name: "p2Time.h" -*-
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
 */

#include "p2Time.h"
#include "math.h"


/** The default clock facility is the local wall clock unless
    changed. */
clockT defaultClockDescriptor = LOOP_TIME_WALLCLOCK;

/**
   Only accept the realtime descriptor for now, and fail noisly
   otherwise */
void
getTime(boost::posix_time::ptime& t,
        clockT clockDescriptor)
{
  if (clockDescriptor == LOOP_TIME_DEFAULT) {
    clockDescriptor = defaultClockDescriptor;
  }
  
  switch (clockDescriptor) {
  case LOOP_TIME_WALLCLOCK:
    // note that on some platforms (e.g. Win32) this often does not
    // achieve microsecond resolution, or so says the Boost
    // documentation.
    t = boost::posix_time::microsec_clock::local_time();
    break;
  default:
    assert(false);
  }
}

void
setDefaultClock(clockT clockDescriptor)
{
  defaultClockDescriptor = clockDescriptor;
}


