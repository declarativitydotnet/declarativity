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



/** Glue functions for timers */
REMOVABLE_INLINE struct timeval make_timeval(int sec, int usec);
REMOVABLE_INLINE struct timeval & operator+=(struct timeval &a, const struct timeval &b);
REMOVABLE_INLINE struct timeval operator+(struct timeval a, const struct timeval &b);
REMOVABLE_INLINE void click_gettimeofday(struct timeval * tvp);


class Router;

class Timer {
 public:
  
  Timer();
  ~Timer()				{ if (scheduled()) unschedule(); }

  /** Call whatever it is this task is doing.  Instances of this class
      should never be run.  They are only used as sentinel elements of
      task lists. */
  REMOVABLE_INLINE virtual void run()	 { assert(false); };

  bool initialized() const		{ return _router != 0; }
  bool scheduled() const		{ return _prev != 0; }
  const timeval &expiry() const		{ return _expiry; }
  
  REMOVABLE_INLINE void initialize(Router *);
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
  
  /** Pointer to previous timer in the scheduler */
  Timer *_prev;

  /** Pointer to next timer in the scheduler */
  Timer *_next;

  timeval _expiry;
  Router *_router;

  Timer(const Timer &);
  Timer &operator=(const Timer &);

  // list functions
  void make_list();
  void unmake_list();

  friend class Master;
};

#endif
