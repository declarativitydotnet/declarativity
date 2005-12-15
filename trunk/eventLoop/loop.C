// -*- c-basic-offset: 2; related-file-name: "loop.h" -*-
/*
 * @(#)$Id$
 *
 */

#include "loop.h"

void
b_cbv_null()
{}

void
b_cbi_null(int)
{}

void
b_cbs_null(std::string)
{}

void
b_cbb_null(bool)
{}

timeCBHandle*
delayCB(double secondDelay, b_cbv cb)
{
  return NULL;
}

void
timeCBRemove(timeCBHandle *)
{
}

void
fileCB(int, b_selop, b_cbv)
{
  assert(false);
  // Do nothing in here until the need arises
}

tcpHandle*
tcpconnect(in_addr addr, u_int16_t port, b_cbi cb)
{
  return NULL;
}
