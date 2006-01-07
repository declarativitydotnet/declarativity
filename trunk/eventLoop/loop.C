// -*- c-basic-offset: 2; related-file-name: "loop.h" -*-
/*
 * @(#)$Id$
 *
 */

#include "loop.h"
#include <time.h>
#include "math.h"
#include "assert.h"

callbackQueueT callbacks;
long callbackID = 0;

timeCBHandle*
delayCB(double secondDelay, b_cbv cb)
{
  assert(secondDelay >= 0.0);

  // When will this expire?
  struct timespec expiration;
  getTime(expiration);          // now
  increment_timespec(expiration, secondDelay);
  
  // Create handle for this request
  timeCBHandle* handle = new timeCBHandle(expiration, cb);

  // Place it into the priority queue
  callbacks.insert(handle);

  // Return it
  return handle;
}

void
timeCBRemove(timeCBHandle* handle)
{
  callbacks.erase(handle);
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


/** Go up to current time and empty out the expired elements from the
    callback queue */
void
timeCBCatchup(struct timespec& waitDuration)
{
  struct timespec now;
  getTime(now);

  ////////////////////////////////////////////////////////////
  // Empty the queue prefix that has already expired

  callbackQueueT::iterator iter = callbacks.begin();
  while ((iter != callbacks.end()) &&
         (compare_timespec((*iter)->time, now) <= 0)) {
    // Remove this callback from the queue
    timeCBHandle* theCallback = *iter;
    iter++;
    
    // Run it
    (theCallback->callback)();
    
    // And erase it
    delete theCallback;
  }

  ////////////////////////////////////////////////////////////
  // Set the wait duration to be the time from now till the first
  // scheduled event
  
  // Update current time
  getTime(now);

  // Get first waiting time
  if (callbacks.empty()) {
    // Nothing to worry about. Set it to a minute
    waitDuration.tv_sec = 60;
    waitDuration.tv_nsec = 0;
  } else {
    iter = callbacks.begin();
    assert(iter != callbacks.end()); // since it's not empty

    if (compare_timespec((*iter)->time, now) < 0) {
      // Oops, the first callback has already expired. Don't wait, just
      // poll
      waitDuration.tv_sec = 0;
      waitDuration.tv_nsec = 0;
    } else {
      subtract_timespec(waitDuration, (*iter)->time, now);
    }
  }
}

/** Wait for any pending file descriptor actions for the given time
    period. */
void
fileDescriptorCatchup(struct timespec& waitDuration)
{
  
}


void
eventLoop()
{
  // The wait duration for file descriptor waits. It is set by
  // timeCBCatchup and used by fileDescriptorCatchup.  Equivalent to
  // selwait in libasync
  struct timespec waitDuration;

  while (1) {
    timeCBCatchup(waitDuration);
    fileDescriptorCatchup(waitDuration);
  }
}

