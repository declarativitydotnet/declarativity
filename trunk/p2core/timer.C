// -*- c-basic-offset: 2; related-file-name: "timer.h" -*-
/*
 * @(#)$Id$
 * Modified from the Click portable timers by Eddie Kohler
 * 
 * Copyright (c) 1999-2000 Massachusetts Institute of Technology
 * Copyright (c) 2004 Regents of the University of California
 * Copyright (c) 2004 Intel Corporation
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
 * DESCRIPTION: Portable timers for P2 elements
 *
 * Note: removed the dependency on glue.hh from click for simplicity
 * until the generality is clearly needed.
 */

#include <timer.h>
#include <router.h>
#include <master.h>
#include <routerthread.h>


REMOVABLE_INLINE struct timeval make_timeval(int sec, int usec)
{
  struct timeval tv;
  tv.tv_sec = sec;
  tv.tv_usec = usec;
  return tv;
}

REMOVABLE_INLINE struct timeval & operator+=(struct timeval &a, const struct timeval &b)
{
  a.tv_sec += b.tv_sec;
  a.tv_usec += b.tv_usec;
  if (a.tv_usec >= 1000000) {
    a.tv_sec++;
    a.tv_usec -= 1000000;
  }
  return a;
}

REMOVABLE_INLINE struct timeval operator+(struct timeval a, const struct timeval &b)
{
  a += b;
  return a;
}

REMOVABLE_INLINE void click_gettimeofday(struct timeval * tvp)
{
  gettimeofday(tvp, (struct timezone *)0);
}

















Timer::Timer()
  : _prev(0),
    _next(0),
    _router(0)
{
}

Timer::~Timer()
{
  if (scheduled()) {
    unschedule();
  }
}

void Timer::run()
{
  assert(false);
}

REMOVABLE_INLINE void Timer::initialize(RouterRef router)
{
  assert(!initialized());
  _router = router;
}

void Timer::unschedule()
{
  if (scheduled()) {
    Master *master = _router->master();
    master->_timer_lock.acquire();
    _prev->_next = _next;
    _next->_prev = _prev;
    _prev = _next = 0;
    master->_timer_lock.release();
  }
}


void Timer::schedule_at(const timeval &when)
{
  // acquire lock, unschedule
  assert(_router && initialized());
  Master *master = _router->master();
  master->_timer_lock.acquire();
  if (scheduled())
    unschedule();

  // set expiration timer
  _expiry = when;

  // manipulate list
  Timer *head = &master->_timer_list;
  Timer *prev = head;
  Timer *trav = prev->_next;
  while (trav != head && timercmp(&_expiry, &trav->_expiry, >)) {
    prev = trav;
    trav = trav->_next;
  }
  _prev = prev;
  _next = trav;
  _prev->_next = this;
  trav->_prev = this;

  // if we changed the timeout, wake up the first thread
  if (head->_next == this) {
    master->_thread->unsleep();
  }

  // done
  master->_timer_lock.release();
}

REMOVABLE_INLINE void Timer::reschedule_at(const timeval &tv)
{
  schedule_at(tv);
}

REMOVABLE_INLINE void Timer::schedule_now()
{
  schedule_after_ms(0);
}

void Timer::schedule_after(const timeval &delta)
{
  timeval t;
  click_gettimeofday(&t);
  schedule_at(t + delta);
}

REMOVABLE_INLINE void Timer::schedule_after_s(uint32_t s)
{
  schedule_after(make_timeval(s, 0));
}

REMOVABLE_INLINE void Timer::schedule_after_ms(uint32_t ms)
{
  schedule_after(make_timeval(ms / 1000, (ms % 1000) * 1000));
}

REMOVABLE_INLINE void Timer::reschedule_after(const timeval &delta)
{
  schedule_at(_expiry + delta);
}

void Timer::reschedule_after_s(uint32_t s)
{
  timeval t = _expiry;
  t.tv_sec += s;
  schedule_at(t);
}

void Timer::reschedule_after_ms(uint32_t ms)
{
  timeval t = _expiry;
  timeval interval;
  interval.tv_sec = ms / 1000;
  interval.tv_usec = (ms % 1000) * 1000;
  timeradd(&t, &interval, &t);
  schedule_at(t);
}

void Timer::make_list()
{
    _prev = _next = this;
}

void Timer::unmake_list()
{
    _prev = _next = 0;
}
