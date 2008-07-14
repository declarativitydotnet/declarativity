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


#ifndef __RAND_PUSH_SOURCE_H__
#define __RAND_PUSH_SOURCE_H__

#include <amisc.h>
#include <element.h>

class RandomPushSource : public Element { 
 public:
  
  /** Initialized with the interval between tuple generation events. */
  RandomPushSource(str name, double seconds, int randSeed, int max);

  const char *class_name() const		{ return "RandomPushSource"; }
  const char *flow_code() const			{ return "/-"; }
  const char *processing() const		{ return "/h"; }

  virtual int initialize();

  void runTimer();
  
 private:
  /** The integer seconds portion of the interval */
  uint _seconds;

  /** The nsec portion of the interval */
  uint _nseconds;

  /** My wakeup callback */
  cbv _wakeupCB;

  /** Callback to my runTimer() */
  cbv _runTimerCB;

  /** My time callback ID. */
  timecb_t * _timeCallback;

  /** My wakeup method */
  void wakeup();

  /** The max value of the random number */
  int _max;
};

#endif /* __RAND_PUSH_SOURCE_H_ */
