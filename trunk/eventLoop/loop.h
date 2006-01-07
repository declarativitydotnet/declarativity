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

#include <string>
#include <sstream>
#include <iostream>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <set>

// For in_addr
#include <netinet/in.h>

#include <p2Time.h>
#include <time.h>

using std::string;
using std::ostringstream;

#undef warn
#define warn std::cerr
#undef fatal
#define fatal std::cerr

// FIX ME
#define fdcb(a, b, c)
#define inetsocket(type, port, addr) 0
#define make_async(sd) 
#define close_on_exec(sd)
#define suio_uprintf(u3, s)
#define amain()
#define hash_string(p) 0

// Common callback types
typedef boost::function<void (void)>        b_cbv;
typedef boost::function<void (int)>         b_cbi;
typedef boost::function<void (std::string)> b_cbs;
typedef boost::function<void (bool)>        b_cbb;

/** Operation type for selects */
enum
b_selop {
  b_selread = 0,
  b_selwrite = 1
};

/** The global counter of callbacks */
extern long callbackID;

/** A callback record */
struct timeCBHandle {
public:
  /** What was my time target? */
  struct timespec time;
  
  /** What was my callback? */
  const b_cbv callback;
  
  /** My ID */
  long ID;

  /** Construct me */
  timeCBHandle(struct timespec& t, const b_cbv& cb)
    : time(t), callback(cb), ID(callbackID++)
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
fileDescriptorCB(int, b_selop, b_cbv);

tcpHandle*
tcpConnect(in_addr addr, u_int16_t port, b_cbi cb);


/** My ordering predicate for callback handles. Returns true if the
    first time is less than the second, or if they're equal but the
    first ID is less than the second. */
struct timeCBHandleLess
{
  bool operator()(const timeCBHandle* first,
                  const timeCBHandle* second) const
  {
    return ((compare_timespec(first->time, second->time) < 0) ||
            ((compare_timespec(first->time, second->time) == 0) &&
             (first->ID < second->ID)));
  }
};

/** The timed callbacks sorted set */
typedef std::set<timeCBHandle*, timeCBHandleLess> callbackQueueT;
extern callbackQueueT callbacks;

#endif /* __LOOP_H_ */
