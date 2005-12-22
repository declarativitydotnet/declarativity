// -*- c-basic-offset: 2; related-file-name: "timedPushSource.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Element that creates and pushes a new tuple at every
 * timed interval.  The element becomes inactive if a push is rejected,
 * waking up when its callback is invoked.
 */


#ifndef __TIMED_PUSH_SOURCE_H__
#define __TIMED_PUSH_SOURCE_H__

#include <amisc.h>
#include <element.h>
#include "loop.h"

class TimedPushSource : public Element { 
 public:
  
  /** Initialized with the interval between tuple generation events. */
  TimedPushSource(str name, double seconds);

  const char *class_name() const		{ return "TimedPushSource"; }
  const char *flow_code() const			{ return "/-"; }
  const char *processing() const		{ return "/h"; }

  virtual int initialize();

  void runTimer();
  
 private:
  /** The interval duration in seconds. */
  double _seconds;

  /** My wakeup callback */
  b_cbv _wakeupCB;

  /** Callback to my runTimer() */
  b_cbv _runTimerCB;

  /** My time callback ID. */
  timeCBHandle * _timeCallback;

  /** My wakeup method */
  void wakeup();
};

#endif /* __TIMED_PUSH_SOURCE_H_ */
