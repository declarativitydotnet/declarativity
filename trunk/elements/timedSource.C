// -*- c-basic-offset: 2; related-file-name: "timedSource.h" -*-
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

#include <timedSource.h>
#include <element.h>
#include <telemental.h>

TimedSource::TimedSource(int millis)
  : Element(0, 1),
    _millis(millis),
    _tuple(0),
    _callBack(0),
    _tElemental(this)
{
  assert(millis > 0);
}
    
int TimedSource::initialize()
{
  // Initialize and schedule my timer
  _tElemental.initialize(this);

  _tElemental.schedule_after_ms(_millis);

  return 0;
}


bool TimedSource::run_timer()
{
  if (!_tuple) {
    // Create a new one
    _tuple = Tuple::mk();

    // Fill it up with the current timeval
    struct timeval t;
    click_gettimeofday(&t);

    _tuple->append(New refcounted< TupleField >((uint64_t) t.tv_sec));
    _tuple->append(New refcounted< TupleField >((uint64_t) t.tv_usec));
    _tuple->freeze();

    // Wake up any sleepers
    if (_callBack) {
      cbv c = _callBack;
      _callBack = 0;
      c();
    }

    _tElemental.reschedule_after_ms(_millis);
    return true;
  } else {
    // Current one hasn't been used. Skip this round
    return false;
  }
}

TuplePtr TimedSource::pull(int port,
                           cbv cb)
{
  // Is this the right port?
  assert(port == 0);

  // Do we have a tuple handy?
  if (_tuple) {
    // Give it
    TupleRef t = _tuple;
    _tuple = 0;
    return t;
  } else {
    // Keep the callback and respond with a null tuple
    _callBack = cb;
    return 0;
  }
}

TimedSource::~TimedSource()
{
  _callBack = 0;
  _tuple = 0;
}
