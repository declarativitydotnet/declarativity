// -*- c-basic-offset: 2; related-file-name: "routerthread.h" -*-
/*
 * @(#)$Id$
 *
 * Modified from the Click router thread class by Benjie
 * Chen, Eddie Kohler, Petros Zerfos
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
 * DESCRIPTION: Router thread class
 *
 * The router thread contains a lock for all of its tasks queued up.
 */

#include <router.h>
#include <task.h>
#include <routerthread.h>
#include <master.h>

void RouterThread::driver()
{
  // Counter used for periodic tasks such as timers, OS hooks, etc.
  int iter = 0;
  
  while (_master->running()) {
    iter++;

    // run occasional tasks: timers, select, etc.
    if ((iter % _iters_per_os) == 0) {
      run_os();
    }
    if ((iter % _iters_per_timers) == 0) {
      _master->run_timers();
    }

    // run task requests (1)
    if (_pending) {
      _master->process_pending(this);
    }

    // run a bunch of tasks
    run_tasks(_tasks_per_iter);
  }

  unlock_tasks();
}


REMOVABLE_INLINE void RouterThread::run_os()
{
  unlock_tasks();

  _master->run_selects();
    
  nice_lock_tasks();
}





RouterThread::RouterThread(Master *m, int id)
  : _master(m),
    _id(id),
    _taskList()
{
  // Make the task list contain only the sentinel
  _taskList._next = _taskList._prev = &_taskList;

  
  _task_lock_waiting = 0;
  _pending = 0;

  _tasks_per_iter = 128;
  _iters_per_timers = 32;

  _iters_per_os = 64;           /* iterations per select(), assuming
                                   user-level runs */
}

RouterThread::~RouterThread()
{
  unschedule_all_tasks();
}

REMOVABLE_INLINE void RouterThread::nice_lock_tasks()
{
  lock_tasks();
}


/******************************/
/* Adaptive scheduler         */
/******************************/

#ifdef HAVE_ADAPTIVE_SCHEDULER

void
RouterThread::set_cpu_share(unsigned min_frac, unsigned max_frac)
{
  if (min_frac == 0)
    min_frac = 1;
  if (min_frac > MAX_UTILIZATION - 1)
    min_frac = MAX_UTILIZATION - 1;
  if (max_frac > MAX_UTILIZATION - 1)
    max_frac = MAX_UTILIZATION - 1;
  if (max_frac < min_frac)
    max_frac = min_frac;
  _min_click_share = min_frac;
  _max_click_share = max_frac;
}

void
RouterThread::client_set_tickets(int client, int new_tickets)
{
  Client &c = _clients[client];

  // pin 'tickets' in a reasonable range
  if (new_tickets < 1)
    new_tickets = 1;
  else if (new_tickets > Task::MAX_TICKETS)
    new_tickets = Task::MAX_TICKETS;
  unsigned new_stride = Task::STRIDE1 / new_tickets;
  assert(new_stride < Task::MAX_STRIDE);

  // calculate new pass, based possibly on old pass
  // start with a full stride on initialization (c.tickets == 0)
  if (c.tickets == 0)
    c.pass = _global_pass + new_stride;
  else {
    int delta = (c.pass - _global_pass) * new_stride / c.stride;
    c.pass = _global_pass + delta;
  }

  c.tickets = new_tickets;
  c.stride = new_stride;
}

REMOVABLE_INLINE void
RouterThread::client_update_pass(int client, const struct timeval &t_before, const struct timeval &t_after)
{
  Client &c = _clients[client];
  int elapsed = (1000000 * (t_after.tv_sec - t_before.tv_sec)) + (t_after.tv_usec - t_before.tv_usec);
  if (elapsed > 0)
    c.pass += (c.stride * elapsed) / DRIVER_QUANTUM;
  else
    c.pass += c.stride;
}

REMOVABLE_INLINE void
RouterThread::check_restride(struct timeval &t_before, const struct timeval &t_now, int &restride_iter)
{
  int elapsed = (1000000 * (t_now.tv_sec - t_before.tv_sec)) + (t_now.tv_usec - t_before.tv_usec);
  if (elapsed > DRIVER_RESTRIDE_INTERVAL || elapsed < 0) {
    // mark new measurement period
    t_before = t_now;
	
    // reset passes every 10 intervals, or when time moves backwards
    if (++restride_iter == 10 || elapsed < 0) {
      _global_pass = _clients[C_CLICK].tickets = _clients[C_KERNEL].tickets = 0;
      restride_iter = 0;
    } else
      _global_pass += (DRIVER_GLOBAL_STRIDE * elapsed) / DRIVER_QUANTUM;

    // find out the maximum amount of work any task performed
    int click_utilization = 0;
    for (Task *t = scheduled_next(); t != this; t = t->scheduled_next()) {
      int u = t->utilization();
      t->clear_runs();
      if (u > click_utilization)
        click_utilization = u;
    }

    // constrain to bounds
    if (click_utilization < _min_click_share)
      click_utilization = _min_click_share;
    if (click_utilization > _max_click_share)
      click_utilization = _max_click_share;

    // set tickets
    int click_tix = (DRIVER_TOTAL_TICKETS * click_utilization) / Task::MAX_UTILIZATION;
    if (click_tix < 1)
      click_tix = 1;
    client_set_tickets(C_CLICK, click_tix);
    client_set_tickets(C_KERNEL, DRIVER_TOTAL_TICKETS - _clients[C_CLICK].tickets);
    _cur_click_share = click_utilization;
  }
}

#endif

/******************************/
/* Debugging                  */
/******************************/

#if CLICK_DEBUG_SCHEDULING
timeval
RouterThread::task_epoch_time(uint32_t epoch) const
{
  if (epoch >= _task_epoch_first && epoch <= _driver_task_epoch)
    return _task_epoch_time[epoch - _task_epoch_first];
  else if (epoch > _driver_task_epoch - TASK_EPOCH_BUFSIZ && epoch <= _task_epoch_first - 1)
    // "-1" makes this code work even if _task_epoch overflows
    return _task_epoch_time[epoch - (_task_epoch_first - TASK_EPOCH_BUFSIZ)];
  else
    return make_timeval(0, 0);
}
#endif


/******************************/
/* The driver loop            */
/******************************/

/* Run at most 'ntasks' tasks. */
REMOVABLE_INLINE void RouterThread::run_tasks(int ntasks)
{
  // never run more than 32768 tasks
  if (ntasks > 32768)
    ntasks = 32768;
  
  Task *t;
  while ((t = scheduled_next()),
         t != &_taskList && ntasks >= 0) {
    
    t->fast_unschedule();
    
    t->run();
    
    ntasks--;
  }
}

/******************************/
/* Secondary driver functions */
/******************************/

void
RouterThread::driver_once()
{
  lock_tasks();
  Task *t = scheduled_next();
  if (t != &_taskList) {
    t->fast_unschedule();
    t->run();
  }
  unlock_tasks();
}

void
RouterThread::unschedule_all_tasks()
{
  lock_tasks();
  Task *t;
  while ((t = scheduled_next()),
         t != &_taskList) {
    t->fast_unschedule();
  }
  unlock_tasks();
}


REMOVABLE_INLINE bool RouterThread::empty() const
{
  return (_taskList._next == &_taskList) && !_pending;
}

REMOVABLE_INLINE void RouterThread::lock_tasks()
{
  _task_lock_waiting++;
  _lock.acquire();
  _task_lock_waiting--;
}

REMOVABLE_INLINE bool RouterThread::attempt_lock_tasks()
{
  return _lock.attempt();
}

REMOVABLE_INLINE void RouterThread::unlock_tasks()
{
  _lock.release();
}

REMOVABLE_INLINE void RouterThread::unsleep()
{
}

REMOVABLE_INLINE void RouterThread::add_pending()
{
  _pending++;
  unsleep();
}
