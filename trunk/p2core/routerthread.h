// -*- c-basic-offset: 2; related-file-name: "routerthread.C" -*-
/*
 * @(#)$Id$
 * Loosely inspired from the Click router thread, by Eddie Kohler
 * 
 * Copyright (c) 1999-2000 Massachusetts Institute of Technology
 * Copyright (c) 2000 Mazu Networks, Inc.
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
 * The router thread contains single-thread scheduling state, including
 * all tasks running on a particular thread.  The _prev/_next pointers
 * in tasks is used to maintain this thread-specific doubly-linked-list
 * of tasks.  Before run, a task is "unscheduled," i.e., removed from
 * the task list in which it belongs.
 */
#ifndef __ROUTERTHREAD_H__
#define __ROUTERTHREAD_H__
#include <inlines.h>
#include <sync.h>
#include <vec.h>
#include <master.h>

class RouterThread {
 public:
  
  /** The main driver method running at the bottom of the thread stack
      frame.  When the thread is destroyed, the method exits. */
  void driver();

  
  int thread_id() const			{ return _id; }
  Master *master() const		{ return _master; }

  void driver_once();

  // Task list functions
  bool empty() const;

  void lock_tasks();
  bool attempt_lock_tasks();
  void unlock_tasks();

  void unschedule_all_tasks();

  REMOVABLE_INLINE void unsleep();

  /** What's my next scheduled task? */
  Task *scheduled_next() const		{ return _taskList._next; }

  unsigned _tasks_per_iter;
  unsigned _iters_per_timers;
  unsigned _iters_per_os;

 private:
    
  Master *_master;
  int _id;

  Spinlock _lock;
  uint32_t _task_lock_waiting;

  /** My task list sentinel */
  Task _taskList;

  /** How many of my tasks are currently pending? */
  uint32_t _pending;

  // called by Master
  RouterThread(Master *, int);
  ~RouterThread();

  // task requests
  REMOVABLE_INLINE void add_pending();

  // task running functions
  REMOVABLE_INLINE void nice_lock_tasks();
  REMOVABLE_INLINE void run_tasks(int ntasks);
  REMOVABLE_INLINE void run_os();
    
  friend class Task;
  friend class Master;
  friend class refcounted< RouterThread >;
};

typedef ptr< RouterThread > RouterThreadPtr;
typedef ref< RouterThread > RouterThreadRef;


#endif
