// -*- c-basic-offset: 2; related-file-name: "timedPullSink.h" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include <timedPullSink.h>
#include <element.h>
#include <async.h>
#include <math.h>

TimedPullSink::TimedPullSink(double seconds)
  : Element(1, 0),
    _wakeupCB(wrap(this, &TimedPullSink::wakeup)),
    _runTimerCB(wrap(this, &TimedPullSink::runTimer))
{
  _seconds = (uint) floor(seconds);
  seconds -= _seconds;
  _nseconds = (uint) (seconds * 1000000000);
}
    
int TimedPullSink::initialize()
{
  log(LoggerI::INFO, 0, "initialize: TIMEDPULLSINK/init");
  // Schedule my timer
  _timeCallback = delaycb(_seconds,
                          _nseconds, _runTimerCB);

  return 0;
}


void TimedPullSink::runTimer()
{
  // remove the timer id
  _timeCallback = 0;

  // Suck a tuple
  log(LoggerI::INFO, 0, "runTimer: pulling");
  TuplePtr result = input(0)->pull(_wakeupCB);
  if (result == 0) {
    // We have been pushed back.  Don't reschedule wakeup
    log(LoggerI::INFO, 0, "runTimer: sleeping");
  } else {
    // Reschedule me into the future
    _timeCallback = delaycb(_seconds,
                            _nseconds,
                            _runTimerCB);
    log(LoggerI::INFO, 0, "runTimer: rescheduling");
  }
}

void TimedPullSink::wakeup()
{
  // I'd better not be already scheduled
  assert(_timeCallback == 0);

  log(LoggerI::INFO, 0, "wakeup");

  // Okey dokey.  Reschedule me into the future
  _timeCallback = delaycb(_seconds,
                          _nseconds,
                          _runTimerCB);
}


