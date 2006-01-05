// -*- c-basic-offset: 2; related-file-name: "loop.h" -*-
/*
 * @(#)$Id$
 *
 */

#include "loop.h"

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

int
clock_gettime(int facilityDescriptor, struct timespec * time)
{
  return 0;
}
