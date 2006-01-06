// -*- c-basic-offset: 2; related-file-name: "loop.h" -*-
/*
 * @(#)$Id$
 *
 */

#include "loop.h"
#include <time.h>
#include "math.h"
#include "assert.h"

std::multiset<timeCBHandle*, timeCBHandleCompare> callbacks;

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


