// -*- c-basic-offset: 2; related-file-name: "master.h" -*-
/*
 * @(#)$Id$
 *
 * Modified from the Click event master by Eddie Kohler
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
 * DESCRIPTION: P2 event master
 */

#include <master.h>
#include <router.h>
#include <routerthread.h>


Master::Master() :
  _router(0),
  _task_list(),
  _timer_list()
{
  _thread = New refcounted< RouterThread >(this, 0);
  if (_thread == 0) {
    // No memory left
    throw MasterError();
  }
  
  _task_list.make_list();
  _timer_list.make_list();
}

Master::~Master()
{
  _timer_list.unmake_list();
}

void Master::run()
{
}

REMOVABLE_INLINE RouterThreadPtr Master::thread() const
{
  // return the router thread.
  return _thread;
}

void Master::run_selects()
{
  // Couldn't lock the master?
  if (!_master_lock.attempt()) {
    return;
  }

  // Check for asynchronous events
  // XXXX

  _master_lock.release();
}

void Master::run_timers()
{
  if (_master_lock.attempt()) {
    if (_timer_lock.attempt()) {
      // Get the time
      struct timeval now;
      click_gettimeofday(&now);

      // Linearly search through timers.  As long as timers found have
      // expired, run them and remove them.
      while (_timer_list._next != &_timer_list
             && !timercmp(&_timer_list._next->_expiry, &now, >)
             && running()) {
        Timer *t = _timer_list._next;
        _timer_list._next = t->_next;
        _timer_list._next->_prev = &_timer_list;
        t->_prev = 0;
        t->run();
      }
      _timer_lock.release();
    }
    _master_lock.release();
  }
}



void Master::process_pending(RouterThread *thread)
{
  if (_master_lock.attempt()) {
    // get a copy of the list
    _task_lock.acquire();
    Task *t = _task_list._pending_next;
    _task_list._pending_next = &_task_list;
    thread->_pending = 0;
    _task_lock.release();
    
    // reverse list so pending tasks are processed in the order we
    // added them
    Task *prev = &_task_list;
    while (t != &_task_list) {
      Task *next = t->_pending_next;
      t->_pending_next = prev;
      prev = t;
      t = next;
    }
    
    // process list
    for (t = prev; t != &_task_list; ) {
      Task *next = t->_pending_next;
      t->_pending_next = 0;
      t->process_pending(thread);
      t = next;
    }
    _master_lock.release();
  }
}


// TIMERS

// How long until next timer expires.

int Master::timer_delay(struct timeval *tv)
{
  int retval;
  _timer_lock.acquire();
  if (_timer_list._next == &_timer_list) {
    tv->tv_sec = 1000;
    tv->tv_usec = 0;
    retval = 0;
  } else {
    struct timeval now;
    click_gettimeofday(&now);
    if (timercmp(&_timer_list._next->_expiry, &now, >)) {
      timersub(&_timer_list._next->_expiry, &now, tv);
    } else {
      tv->tv_sec = 0;
      tv->tv_usec = 0;
    }
    retval = 1;
  }
  _timer_lock.release();
  return retval;
}



