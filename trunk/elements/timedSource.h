// -*- c-basic-offset: 2; related-file-name: "timedSource.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Element that makes available tuples at timed intervals.
 * If an available tuple has not been pulled, the element skips
 * generating a new one.
 * 
 */


#ifndef __TIMED_SOURCE_H__
#define __TIMED_SOURCE_H__

#include <amisc.h>
#include <element.h>

class TimedSource : public Element { 
 public:
  
  /** Initialized with the interval between tuple generation events. */
  TimedSource(double seconds);

  virtual ~TimedSource();
  
  /** An error. To be replaced by more general exception machinery. */
  struct TimedSourceError {};
  
  const char *class_name() const		{ return "TimedSource"; }
  const char *flow_code() const			{ return "/-"; }
  const char *processing() const		{ return "/l"; }

  /** Overridden because we have no input ports, whereas the default
      processes inputs before returning them.  */
  TuplePtr pull(int port, cbv cb);

  virtual int initialize();

  void runTimer();
  
 private:
  /** The integer seconds portion of the interval */
  uint _seconds;

  /** The nsec portion of the interval */
  uint _nseconds;

  /** My tuple */
  TuplePtr _tuple;

  /** My waiter */
  callback< void >::ptr _callBack;

  /** Callback to my runTimer() */
  cbv _runTimerCB;

  /** My time callback ID. */
  timecb_t * _timeCallback;
};

#endif /* __TIMED_SOURCE_H_ */
