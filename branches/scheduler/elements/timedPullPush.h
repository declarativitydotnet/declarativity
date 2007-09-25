// -*- c-basic-offset: 2; related-file-name: "timedPullPush.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: Element that at timed intervals pulls a tuple from its
 * input and pushes it down its output's throat.  A fixed number of
 * tuples can be pulled.  If the fixed number is 0, then no limit in the
 * number of pulled tuples is imposed. When either its output or its
 * input are blocked, it sleeps until both are awake.
 */


#ifndef __TIMED_PULL_PUSH_H__
#define __TIMED_PULL_PUSH_H__

#include "element.h"
#include "elementRegistry.h"

#include "eventLoop.h"

class TimedPullPush : public Element { 
 public:
  
  /** Initialized with the interval between forwards. */
  TimedPullPush(string name, double seconds, int tuples);
  TimedPullPush(TuplePtr args);

  const char *class_name() const		{ return "TimedPullPush"; }
  const char *flow_code() const			{ return "-/-"; }
  const char *processing() const		{ return "l/h"; }

  virtual int initialize();

  void runTimer();

  TuplePtr pull(int port, b_cbv cb);
  
  DECLARE_PUBLIC_ELEMENT_INITS

 private:
  /** The interval in seconds */
  double _seconds;

  /** The number of tuples to pull */
  int _tuples;

  /** The number of tuples pulled so far */
  int _counter;

  /** My pull wakeup callback */
  b_cbv _unblockPull;
  bool _pendingPull;

  /** My push wakeup callback */
  b_cbv _unblockPush;
  bool _pendingPush;

  /** Callback to my runTimer() */
  b_cbv _runTimerCB;

  /** My time callback ID. */
  EventLoop::TimerID _timeCallback;

  /** My pull wakeup method */
  void pullWakeup();

  /** My push wakeup method */
  void pushWakeup();

  /** Reschedule me to run in the future. */
  void reschedule();

  DECLARE_PRIVATE_ELEMENT_INITS
};

#endif /* __TIMED_PULL_PUSH_H_ */
