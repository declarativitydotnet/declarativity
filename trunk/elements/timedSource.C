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
#include <async.h>
#include <iostream>

TimedSource::TimedSource(double seconds)
  : Element(0, 1),
    _tuple(0),
    _callBack(0),
    _runTimerCB(wrap(this, &TimedSource::runTimer))
{
  assert(seconds > 0);
  _seconds = (uint) floor(seconds);
  seconds -= _seconds;
  _nseconds = (uint) (seconds * 1000000000);
}
    
int TimedSource::initialize()
{
  std::cerr << "TIMEDSOURCE/init\n";
  // Schedule my timer
  _timeCallback = delaycb(_seconds,
                          _nseconds, _runTimerCB);

  return 0;
}


void TimedSource::runTimer()
{
  // remove the timer id
  _timeCallback = 0;

  if (!_tuple) {
    std::cerr << "TIMEDSOURCE/timer: Creating new tuple\n";

    // Create a new one
    _tuple = Tuple::mk();

    // Fill it up with the current timeval
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);

    _tuple->append(New refcounted< TupleField >("Time"));
    _tuple->append(New refcounted< TupleField >((uint64_t) t.tv_sec));
    _tuple->append(New refcounted< TupleField >((uint64_t) t.tv_nsec));
    _tuple->append(New refcounted< TupleField >("End of time"));
    _tuple->freeze();

    // Wake up any sleepers
    if (_callBack) {
      (*_callBack)();
      _callBack = 0;
    }

    // Reschedule me
    _timeCallback = delaycb(_seconds,
                            _nseconds,
                            _runTimerCB);
  } else {
    std::cerr << "TIMEDSOURCE/timer: Can't create new tuple. Sleeping\n";
    // Current one hasn't been used. Skip this round and don't
    // reschedule
  }
}

TuplePtr TimedSource::pull(int port,
                           cbv cb)
{
  // Is this the right port?
  assert(port == 0);

  // Do we have a tuple handy?
  if (_tuple) {
    std::cerr << "TIMEDSOURCE/Pull: tuple ready\n";
    // Give it
    TupleRef t = _tuple;
    _tuple = 0;

    // Reschedule the timer if it's not scheduled
    if (_timeCallback == 0) {
      _timeCallback = delaycb(_seconds,
                              _nseconds,
                              _runTimerCB);
    }

    return t;
  } else {
    // If I already have a callback waiting, throw a warning sign and do
    // nothing
    std::cerr << "TIMEDSOURCE/Pull: tuple not ready\n";
    if (_callBack != 0) {
      std::cerr << "TIMEDSOURCE/Pull: Pull received while callback is registered\n";
    } else {
      // Keep the callback and respond with a null tuple
      _callBack = cb;
    }

    return 0;
  }
}

TimedSource::~TimedSource()
{
  _callBack = 0;
  _tuple = 0;
}
