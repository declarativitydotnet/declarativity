// -*- c-basic-offset: 2; related-file-name: "timedPushSource.h" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include <timedPushSource.h>
#include <element.h>
#include <math.h>

#include "val_str.h"
#include "val_uint64.h"

TimedPushSource::TimedPushSource(string name,
                                 double seconds)
  : Element(name, 0, 1),
    _seconds(seconds),
    _wakeupCB(boost::bind(&TimedPushSource::wakeup, this)),
    _runTimerCB(boost::bind(&TimedPushSource::runTimer, this))
{
}
    
int TimedPushSource::initialize()
{
  log(LoggerI::INFO, 0, "initialize");
  // Schedule my timer
  _timeCallback = delayCB(_seconds, _runTimerCB);
  
  return 0;
}


void TimedPushSource::runTimer()
{
  // remove the timer id
  _timeCallback = 0;

  // Create a tuple
  TuplePtr tuple = Tuple::mk();
  if (tuple == 0) {
    // Couldn't create a new tuple.  Bzzzt
    log(LoggerI::ERROR, 0, "runTimer: Failed to create new tuple");
    return;
  }

  log(LoggerI::INFO, 0, "runTimer: Creating new tuple");
  // Fill it up with the current timeval
  struct timespec t;
  getTime(t);
  
  tuple->append(Val_Str::mk("Time"));
  tuple->append(Val_UInt64::mk(t.tv_sec));
  tuple->append(Val_UInt64::mk(t.tv_nsec));
  tuple->append(Val_Str::mk("End of time"));
  tuple->freeze();

  // Attempt to push it
  int result = output(0)->push(tuple, _wakeupCB);
  if (result == 0) {
    // We have been pushed back.  Don't reschedule wakeup
    log(LoggerI::INFO, 0, "runTimer: sleeping");
  } else {
    // Reschedule me into the future
    log(LoggerI::INFO, 0, "runTimer: rescheduling");
    _timeCallback = delayCB(_seconds,
                            _runTimerCB);
  }
}

void TimedPushSource::wakeup()
{
  // I'd better not be already scheduled
  assert(_timeCallback == 0);

  log(LoggerI::INFO, 0, "wakeup");

  // Okey dokey.  Reschedule me into the future
  _timeCallback = delayCB(_seconds,
                          _runTimerCB);
}
