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
#include <element.h>
#include <router.h>
#include <master.h>
#include <routerthread.h>
#include <task.h>

/*
 * element_hook is a callback that gets called when a Timer,
 * constructed with just an Element instance, expires. 
 * 
 * When used in userlevel or kernel polling mode, timer is maintained by
 * Click, so element_hook is called within Click.
 */
static void
element_hook(Timer *, void *thunk)
{
    Element *e = (Element *)thunk;
    e->run_timer();
}

static void
task_hook(Timer *, void *thunk)
{
    Task *task = (Task *)thunk;
    task->reschedule();
}

static void
list_hook(Timer *, void *)
{
    assert(0);
}


Timer::Timer(TimerHook hook, void *thunk)
    : _prev(0), _next(0), _hook(hook), _thunk(thunk), _router(0)
{
}

Timer::Timer(Element *e)
    : _prev(0), _next(0), _hook(element_hook), _thunk(e), _router(0)
{
}

Timer::Timer(Task *t)
    : _prev(0), _next(0), _hook(task_hook), _thunk(t), _router(0)
{
}

void
Timer::make_list()
{
    assert(!scheduled());
    _hook = list_hook;
    _prev = _next = this;
}

void
Timer::unmake_list()
{
    assert(_hook == list_hook);
    _prev = _next = 0;
}

void
Timer::schedule_at(const timeval &when)
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
    if (head->_next == this)
	master->_threads[1]->unsleep();

    // done
    master->_timer_lock.release();
}

void
Timer::schedule_after(const timeval &delta)
{
    timeval t;
    click_gettimeofday(&t);
    schedule_at(t + delta);
}

void
Timer::reschedule_after_s(uint32_t s)
{
    timeval t = _expiry;
    t.tv_sec += s;
    schedule_at(t);
}

void
Timer::reschedule_after_ms(uint32_t ms)
{
    timeval t = _expiry;
    timeval interval;
    interval.tv_sec = ms / 1000;
    interval.tv_usec = (ms % 1000) * 1000;
    timeradd(&t, &interval, &t);
    schedule_at(t);
}

void
Timer::unschedule()
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

REMOVABLE_INLINE void
Timer::initialize(Router *router)
{
  assert(!initialized());
  _router = router;
}

REMOVABLE_INLINE void
Timer::initialize(Element *element)
{
  initialize(element->router());
}

REMOVABLE_INLINE void
Timer::schedule_now()
{
  schedule_after_ms(0);
}

REMOVABLE_INLINE void
Timer::schedule_after_s(uint32_t s)
{
  schedule_after(make_timeval(s, 0));
}

REMOVABLE_INLINE void
Timer::schedule_after_ms(uint32_t ms)
{
  schedule_after(make_timeval(ms / 1000, (ms % 1000) * 1000));
}

REMOVABLE_INLINE void
Timer::reschedule_after(const timeval &delta)
{
  schedule_at(_expiry + delta);
}

REMOVABLE_INLINE void
Timer::reschedule_at(const timeval &tv)
{
  schedule_at(tv);
}
