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
#include <telemental.h>

class TimedSource : public Element { 
 public:
  
  /** Initialized with the interval between tuple generation events. */
  TimedSource(int millis);

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

  virtual bool run_timer();
  
 private:
  /** The interval (in millis) between tuple generations. Must be
      positive. */
  int _millis;

  /** My tuple */
  TuplePtr _tuple;

  /** My waiter */
  callback< void >::ptr _callBack;

  /** The generation timer event. */
  TElemental _tElemental;
};

#endif /* __TIMED_SOURCE_H_ */
