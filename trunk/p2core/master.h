// -*- c-basic-offset: 2; related-file-name: "master.C" -*-
/*
 * @(#)$Id$
 * Loosely inspired from the Click master event scheduler class, by Eddie Kohler
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
 * The master hold a single-linked list of pending tasks.
 */
#ifndef __MASTER_H__
#define __MASTER_H__
#include <inlines.h>
#include <refcnt.h>
#include <sync.h>
#include <task.h>
#include <routerthread.h>
#include <router.h>
#include <timer.h>

// Put here to resolve cyclical dependencies
class Router;
typedef ptr< Router > RouterPtr;

class Master {
public:

  Master();
  ~Master();

  /** Start up the master */
  void run();
  
  void run_timers();

  /** Deal with asynchronous IPC */
  void run_selects();

  /** Process pending tasks that have not been officially scheduled
      (e.g., because the lock could not be acquired) */
  void process_pending(RouterThread *);

  REMOVABLE_INLINE RouterThread * thread() const;

  /** Are we still running? */
  bool running() const {return true;}

  /** When should I check for timers next? */
  int timer_delay(struct timeval *);

private:
  
  /** The main router thread */
  RouterThread * _thread;

  /** The main synchronization lock.  Currently a no-op (as per
      user-level Click). */
  Spinlock _master_lock;
  
  
  /** The router */
  RouterPtr _router;

  /** The pending tasks list (single-linked only). Sentinel node is a
      dummy task.  Follow _pending_next pointers forward. */
  Task _task_list;
  
  /** The lock over the task list */
  Spinlock _task_lock;
    
  /** The timers list.  Sentinel node is a dummy timer. */
  Timer _timer_list;

  /** The lock over the timer list */
  Spinlock _timer_lock;

  friend class Task;
  friend class Timer;
};

// Handy dandy shorthand refcounted types
typedef ref< Master > MasterRef;
typedef ptr< Master > MasterPtr;

/** The master error structure */
struct MasterError {};

#endif
