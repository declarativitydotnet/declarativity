// -*- c-basic-offset: 2; related-file-name: "timer.C" -*-
/*
 * @(#)$Id$
 *
 * Modified from the Click timer class by Eddie Kohler
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
 * DESCRIPTION: A timer class
 */
#ifndef __TIMER_H__
#define __TIMER_H__
#include <inlines.h>
#include <element.h>
#include <sys/time.h>

class Element;
class Router;
class Timer;
class Task;

typedef void (*TimerHook)(Timer *, void *);

class Timer {
 public:
  
  Timer(TimerHook, void *);
  Timer(Element *);			// call element->run_timer()
  Timer(Task *);			// call task->reschedule()
  ~Timer()				{ if (scheduled()) unschedule(); }

  bool initialized() const		{ return _router != 0; }
  bool scheduled() const		{ return _prev != 0; }
  const timeval &expiry() const		{ return _expiry; }
  
  REMOVABLE_INLINE void initialize(Router *);
  REMOVABLE_INLINE void initialize(Element *);
  void cleanup()			{ unschedule(); }

  void schedule_at(const timeval &);
  REMOVABLE_INLINE void reschedule_at(const timeval &); // synonym

  REMOVABLE_INLINE void schedule_now();
  void schedule_after(const timeval &);
  REMOVABLE_INLINE void schedule_after_s(uint32_t);
  REMOVABLE_INLINE void schedule_after_ms(uint32_t);
  REMOVABLE_INLINE void reschedule_after(const timeval &);
  void reschedule_after_s(uint32_t);
  void reschedule_after_ms(uint32_t);

  void unschedule();
  
 private:
  
  Timer *_prev;
  Timer *_next;
  timeval _expiry;
  TimerHook _hook;
  void *_thunk;
  Router *_router;

  Timer(const Timer &);
  Timer &operator=(const Timer &);

  // list functions
  void make_list();
  void unmake_list();

  friend class Master;
  
};

#endif
