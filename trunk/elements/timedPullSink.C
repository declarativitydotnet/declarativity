// -*- c-basic-offset: 2; related-file-name: "timedPullSink.h" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 */

#include <timedPullSink.h>
#include <element.h>
#include <math.h>

TimedPullSink::TimedPullSink(string name,
                             double seconds)
  : Element(name, 1, 0),
    _seconds(seconds),
    _wakeupCB(boost::bind(&TimedPullSink::wakeup, this)),
    _runTimerCB(boost::bind(&TimedPullSink::runTimer, this))
{
}
    
int TimedPullSink::initialize()
{
  log(LoggerI::INFO, 0, "initialize: TIMEDPULLSINK/init");
  // Schedule my timer
  _timeCallback = delayCB(_seconds, _runTimerCB);

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
    _timeCallback = delayCB(_seconds,
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
  _timeCallback = delayCB(_seconds,
                          _runTimerCB);
}


