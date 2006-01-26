// -*- c-basic-offset: 2; related-file-name: "timedPullSink.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Element that pulls a new tuple every timed interval.
 * The element becomes inactive if a pull is rejected, waking up when
 * its callback is invoked.
 */


#ifndef __TIMED_PULL_SINK_H__
#define __TIMED_PULL_SINK_H__

#include <element.h>
#include "loop.h"

class TimedPullSink : public Element { 
 public:
  
  /** Initialized with the interval between tuple pull events. */
  TimedPullSink(string name,
                double seconds);

  const char *class_name() const		{ return "TimedPullSink"; }
  const char *flow_code() const			{ return "-/"; }
  const char *processing() const		{ return "l/"; }

  virtual int initialize();

  void runTimer();
  
 private:
  /** The interval in seconds */
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

#endif /* __TIMED_PULL_SINK_H_ */
