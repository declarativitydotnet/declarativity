// -*- c-basic-offset: 2; related-file-name: "task.h" -*-
/*
 * @(#)$Id$
 *
 * Modified from the Click Task class by Eddie Kohler and Benjie Chen
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
 * DESCRIPTION: A linked list of schedulable entities
 */

#include <master.h>
#include <router.h>
#include <routerthread.h>
#include <task.h>

// - Changes to _thread are protected by _thread->lock.
// - Changes to _thread_preference are protected by
//   _router->master()->task_lock.
// - If _pending is nonzero, then _pending_next is nonnull.
// - Either _thread_preference == _thread->thread_id(), or
//   _thread->thread_id() == -1.


REMOVABLE_INLINE Task::Task()
  : _prev(0),
    _next(0),
    _thread(0),
    _router(0),
    _pending(0),
    _pending_next(0)
{
}

Master * Task::master() const
{
  assert(_thread != 0);
  return _thread->master();
}

void Task::initialize(RouterRef router,
                      bool join)
{
  assert(!initialized() && !scheduled());

  _router = router;
    
  _thread = router->master()->thread();
  
  if (join)
    add_pending(RESCHEDULE);
}

void Task::add_pending(int p)
{
  Master *m = _router->master();
  m->_task_lock.acquire();
  _pending |= p;
  if (!_pending_next && _pending) {
    _pending_next = m->_task_list._pending_next;
    m->_task_list._pending_next = this;
  }
  if (_pending) {
    _thread->add_pending();
  }
  m->_task_lock.release();
}















bool Task::error_hook(Task *, void *)
{
  assert(0);
  return false;
}

void Task::make_list()
{
  _pending_next = this;
}

Task::~Task()
{
  if (scheduled() || _pending) {
    cleanup();
  }
}

void Task::cleanup()
{
  if (initialized()) {
    strong_unschedule();

    if (_pending) {
      Master *m = _router->master();
      m->_task_lock.acquire();
      Task *prev = &m->_task_list;
      for (Task *t = prev->_pending_next; t != &m->_task_list; prev = t, t = t->_pending_next)
        if (t == this) {
          prev->_pending_next = t->_pending_next;
          break;
        }
      _pending = 0;
      _pending_next = 0;
      m->_task_lock.release();
    }
	
    _router = 0;
    _thread = 0;
  }
}

inline void
Task::lock_tasks()
{
  while (1) {
    RouterThreadRef t = _thread;
    t->lock_tasks();
    if (t == _thread)
      return;
    t->unlock_tasks();
  }
}

inline bool
Task::attempt_lock_tasks()
{
  RouterThreadRef t = _thread;
  if (t->attempt_lock_tasks()) {
    if (t == _thread)
      return true;
    t->unlock_tasks();
  }
  return false;
}

void
Task::unschedule()
{
  // Thanksgiving 2001: unschedule() will always unschedule the task. This
  // seems more reliable, since some people depend on unschedule() ensuring
  // that the task is not scheduled any more, no way, no how. Possible
  // problem: calling unschedule() from run_task() will hang!
  if (_thread != 0) {
    lock_tasks();
    fast_unschedule();
    _pending &= ~RESCHEDULE;
    _thread->unlock_tasks();
  }
}

void Task::true_reschedule()
{
  assert(_thread != 0);
  bool done = false;
  // If I can lock the queue
  if (attempt_lock_tasks()) {
    if (!scheduled()) {
      // I've already locked, so call the fast_ version
      fast_schedule();
      _thread->unsleep();
    }
    done = true;
    _thread->unlock_tasks();
  }

  // Wasn't able to lock the queue.  Try later
  if (!done)
    add_pending(RESCHEDULE);
}

void Task::strong_unschedule()
{
  // unschedule() and move to the quiescent thread, so that subsequent
  // reschedule()s won't have any effect
  if (_thread != 0) {
    lock_tasks();
    fast_unschedule();
    RouterThreadRef old_thread = _thread;
    _pending &= ~RESCHEDULE;
    _thread = _router->master()->thread();
    old_thread->unlock_tasks();
  }
}

void Task::strong_reschedule()
{
  assert(_thread != 0);
  lock_tasks();
  fast_unschedule();
  RouterThreadRef old_thread = _thread;
  _thread = _router->master()->thread();
  add_pending(RESCHEDULE);
  old_thread->unlock_tasks();
}

void Task::process_pending(RouterThread *thread)
{
  // must be called with thread->lock held
  if (_thread == thread) {
    if (_pending & RESCHEDULE) {
      _pending &= ~RESCHEDULE;
      if (!scheduled())
        fast_schedule();
    }
  }
  
  if (_pending)
    add_pending(0);
}


REMOVABLE_INLINE int Task::fast_unschedule()
{
  if (_prev) {
    _next->_prev = _prev;
    _prev->_next = _next;
    _next = _prev = 0;
  }
  return 0;
}

REMOVABLE_INLINE void Task::fast_reschedule()
{
  assert(_thread != 0);
  if (!scheduled()) {
    _prev = _thread->_taskList._prev;
    _next = &_thread->_taskList;
    _thread->_taskList._prev = this;
    _thread->_taskList._next = this;
  }
}

REMOVABLE_INLINE void Task::reschedule()
{
  assert(_thread != 0);
  if (!scheduled())
    true_reschedule();
}

REMOVABLE_INLINE void Task::fast_schedule()
{
    fast_reschedule();
}

