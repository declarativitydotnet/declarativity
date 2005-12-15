// -*- c-basic-offset: 2; related-file-name: "loop.C" -*-
/*
 * @(#)$Id$
 *
 * Event loop, inspired by core.C in libasync by David Mazieres.
 * 
 * Copyright (c) 2005 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software")
 * to deal in the Software without restriction, subject to the conditions
 * listed in the Click LICENSE file. These conditions include: you must
 * preserve this copyright notice, and you cannot mention the copyright
 * holders in advertising related to the Software without their permission.
 * The Software is provided WITHOUT ANY WARRANTY, EXPRESS OR IMPLIED. This
 * notice is a summary of the Click LICENSE file; the license in that file is
 * legally binding.
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Base class for P2 elements
 *
 */

#ifndef __LOOP_H__
#define __LOOP_H__

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

// For in_addr
#include <netinet/in.h>


// Common callback types
typedef boost::function<void (void)>        b_cbv;
typedef boost::function<void (int)>         b_cbi;
typedef boost::function<void (std::string)> b_cbs;
typedef boost::function<void (bool)>        b_cbb;

void
b_cbv_null(void);

void
b_cbi_null(int);

void
b_cbs_null(std::string);

void
b_cbb_null(bool);

/** Operation type for selects */
enum
b_selop {
  b_selread = 0,
  b_selwrite = 1
};


/** A callback record */
struct timeCBHandle {
public:
  /** What was my time target? */
  boost::posix_time::ptime time;
  
  /** What was my callback? */
  const b_cbv callback;
  
  /** Construct me */
  timeCBHandle(const boost::posix_time::ptime& t, const b_cbv& cb)
    : time(t), callback(cb)
  {}
};


/** A TCP control block */
struct tcpHandle {
};


/** Delay a callback for a given time (in seconds). It returns a handle
    that can be used to unschedule the callback. If the delay is
    non-positive, it is interpreted as a delayed callback, to be invoked
    as soon as possible. */
timeCBHandle*
delayCB(double secondDelay, b_cbv cb);

/** Unschedule a scheduled callback */
void
timeCBRemove(timeCBHandle *);


void
fileCB(int, b_selop, b_cbv);

tcpHandle*
tcpconnect(in_addr addr, u_int16_t port, b_cbi cb);


#endif /* __LOOP_H_ */
