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

#include <task.h>
#include <router.h>
#include <routerthread.h>
#include <master.h>

// - Changes to _thread are protected by _thread->lock.
// - Changes to _thread_preference are protected by
//   _router->master()->task_lock.
// - If _pending is nonzero, then _pending_next is nonnull.
// - Either _thread_preference == _thread->thread_id(), or
//   _thread->thread_id() == -1.

bool Task::error_hook(Task *, void *)
{
  assert(0);
  return false;
}

void Task::make_list()
{
  _hook = error_hook;
  _pending_next = this;
}

Task::~Task()
{
  if ((scheduled() || _pending) && _thread != this)
    cleanup();
}

Master *
Task::master() const
{
  assert(_thread);
  return _thread->master();
}

void
Task::initialize(Router *router, bool join)
{
  assert(!initialized() && !scheduled());

  _router = router;
    
  _thread_preference = router->initial_thread_preference(this, join);
  if (_thread_preference == ThreadSched::THREAD_PREFERENCE_UNKNOWN)
    _thread_preference = 0;
  // Master::thread() returns the quiescent thread if its argument is out of
  // range
  _thread = router->master()->thread(_thread_preference);
    
  if (join)
    add_pending(RESCHEDULE);
}

void
Task::initialize(Element *e, bool join)
{
  initialize(e->router(), join);
}

void
Task::cleanup()
{
  if (initialized()) {
    strong_unschedule();

    if (_pending) {
      Master *m = _router->master();
      SpinlockIRQ::flags_t flags = m->_task_lock.acquire();
      Task *prev = &m->_task_list;
      for (Task *t = prev->_pending_next; t != &m->_task_list; prev = t, t = t->_pending_next)
        if (t == this) {
          prev->_pending_next = t->_pending_next;
          break;
        }
      _pending = 0;
      _pending_next = 0;
      m->_task_lock.release(flags);
    }
	
    _router = 0;
    _thread = 0;
  }
}

inline void
Task::lock_tasks()
{
  while (1) {
    RouterThread *t = _thread;
    t->lock_tasks();
    if (t == _thread)
      return;
    t->unlock_tasks();
  }
}

inline bool
Task::attempt_lock_tasks()
{
  RouterThread *t = _thread;
  if (t->attempt_lock_tasks()) {
    if (t == _thread)
      return true;
    t->unlock_tasks();
  }
  return false;
}

void
Task::add_pending(int p)
{
  Master *m = _router->master();
  SpinlockIRQ::flags_t flags = m->_task_lock.acquire();
  if (_router->_running >= Router::RUNNING_PAUSED) {
    _pending |= p;
    if (!_pending_next && _pending) {
      _pending_next = m->_task_list._pending_next;
      m->_task_list._pending_next = this;
    }
    if (_pending)
      _thread->add_pending();
  }
  m->_task_lock.release(flags);
}

void
Task::unschedule()
{
  // Thanksgiving 2001: unschedule() will always unschedule the task. This
  // seems more reliable, since some people depend on unschedule() ensuring
  // that the task is not scheduled any more, no way, no how. Possible
  // problem: calling unschedule() from run_task() will hang!
  if (_thread) {
    lock_tasks();
    fast_unschedule();
    _pending &= ~RESCHEDULE;
    _thread->unlock_tasks();
  }
}

void
Task::true_reschedule()
{
  assert(_thread);
  bool done = false;
  if (attempt_lock_tasks()) {
    if (_router->_running >= Router::RUNNING_BACKGROUND) {
      if (!scheduled()) {
        fast_schedule();
        _thread->unsleep();
      }
      done = true;
    }
    _thread->unlock_tasks();
  }
  if (!done)
    add_pending(RESCHEDULE);
}

void Task::strong_unschedule()
{
  // unschedule() and move to the quiescent thread, so that subsequent
  // reschedule()s won't have any effect
  if (_thread) {
    lock_tasks();
    fast_unschedule();
    RouterThread *old_thread = _thread;
    _pending &= ~RESCHEDULE;
    _thread = _router->master()->thread(-1);
    old_thread->unlock_tasks();
  }
}

void Task::strong_reschedule()
{
  assert(_thread);
  lock_tasks();
  fast_unschedule();
  RouterThread *old_thread = _thread;
  _thread = _router->master()->thread(_thread_preference);
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


REMOVABLE_INLINE Task::Task(TaskHook hook, void *thunk)
  : _prev(0),
    _next(0),
    _hook(hook),
    _thunk(thunk),
    _thread(0),
    _thread_preference(-1),
    _router(0),
    _pending(0),
    _pending_next(0)
{
}

REMOVABLE_INLINE Task::Task(Element *e)
  : _prev(0),
    _next(0),
    _hook(0),
    _thunk(e),
    _thread(0),
    _thread_preference(-1),
    _router(0),
    _pending(0),
    _pending_next(0)
{
}

REMOVABLE_INLINE Element * Task::element() const
{ 
  return _hook ? 0 : reinterpret_cast<Element*>(_thunk); 
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
  assert(_thread);
  if (!scheduled()) {
    _prev = _thread->_prev;
    _next = _thread;
    _thread->_prev = this;
    _thread->_next = this;
  }
}

REMOVABLE_INLINE void Task::fast_schedule()
{
  fast_reschedule();
}

REMOVABLE_INLINE void Task::reschedule()
{
  assert(_thread);
  if (!scheduled())
    true_reschedule();
}

REMOVABLE_INLINE void Task::call_hook()
{
  if (!_hook)
    (void) ((Element *)_thunk)->run_task();
  else
    (void) _hook(this, _thunk);
}
