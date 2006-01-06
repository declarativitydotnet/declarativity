// -*- c-basic-offset: 2; related-file-name: "loop.h" -*-
/*
 * @(#)$Id$
 *
 */

#include "loop.h"
#include <time.h>

timeCBHandle*
delayCB(double secondDelay, b_cbv cb)
{
  return NULL;
}

void
timeCBRemove(timeCBHandle * handle)
{
}

void
fileDescriptorCB(int fileDescriptor,
                 b_selop op,
                 b_cbv callback)
{
  assert(false);
  // Do nothing in here until the need arises
}

tcpHandle*
tcpConnect(in_addr addr, u_int16_t port, b_cbi cb)
{
  return NULL;
}

/** The default clock facility is the local wall clock unless
    changed. */
clockT defaultClockDescriptor = LOOP_TIME_WALLCLOCK;

/**
   Only accept the realtime descriptor for now, and fail noisly
   otherwise */
void
getTime(struct timespec& t,
        clockT clockDescriptor)
{
  if (clockDescriptor == LOOP_TIME_DEFAULT) {
    clockDescriptor = defaultClockDescriptor;
  }
  
  switch (clockDescriptor) {
  case LOOP_TIME_WALLCLOCK:
    clock_gettime(CLOCK_REALTIME, &t);
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

void
subtract_timespec(struct timespec& difference,
                  struct timespec& minuend,
                  struct timespec& subtrahend)
{
  difference.tv_sec = minuend.tv_sec - subtrahend.tv_sec;
  difference.tv_nsec = minuend.tv_nsec - subtrahend.tv_nsec;

  // The carry
  if (difference.tv_nsec < 0) {
    difference.tv_nsec += 1000 * 1000 * 1000;
    difference.tv_sec++;
  }
}

int
compare_timespec(struct timespec& first,
                 struct timespec& second)
{
  if (first.tv_sec < second.tv_sec) {
    return -1;
  } else if (first.tv_sec > second.tv_sec) {
    return 1;
  } else if (first.tv_nsec < second.tv_nsec) {
    return -1;
  } else if (first.tv_nsec > second.tv_nsec) {
    return 1;
  } else {
    return 0;
  }
}
