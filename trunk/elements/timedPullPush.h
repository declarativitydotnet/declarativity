// -*- c-basic-offset: 2; related-file-name: "timedPullPush.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Element that at timed intervals pulls a tuple from its
 * input and pushes it down its output's throat.  When either its output
 * or its input are blocked, it sleeps until both are awake.
 */


#ifndef __TIMED_PULL_PUSH_H__
#define __TIMED_PULL_PUSH_H__

#include <amisc.h>
#include <element.h>

class TimedPullPush : public Element { 
 public:
  
  /** Initialized with the interval between forwards. */
  TimedPullPush(str name, double seconds);

  const char *class_name() const		{ return "TimedPullPush"; }
  const char *flow_code() const			{ return "-/-"; }
  const char *processing() const		{ return "l/h"; }

  virtual int initialize();

  void runTimer();
  
 private:
  /** The integer seconds portion of the interval */
  uint _seconds;

  /** The nsec portion of the interval */
  uint _nseconds;

  /** My pull wakeup callback */
  cbv _unblockPull;

  /** My push wakeup callback */
  cbv _unblockPush;

  /** Callback to my runTimer() */
  cbv _runTimerCB;

  /** My time callback ID. */
  timecb_t * _timeCallback;

  /** My pull wakeup method */
  void pullWakeup();

  /** My push wakeup method */
  void pushWakeup();
};

#endif /* __TIMED_PULL_PUSH_H_ */
