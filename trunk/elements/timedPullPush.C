// -*- c-basic-offset: 2; related-file-name: "timedPullPush.h" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include <timedPullPush.h>
#include <element.h>
#include <async.h>
#include <math.h>

#include "val_str.h"
#include "val_uint64.h"

TimedPullPush::TimedPullPush(str name,
                             double seconds)
  : Element(name, 1, 1),
    _unblockPull(wrap(this, &TimedPullPush::pullWakeup)),
    _unblockPush(wrap(this, &TimedPullPush::pushWakeup)),
    _runTimerCB(wrap(this, &TimedPullPush::runTimer))
{
  _seconds = (uint) floor(seconds);
  seconds -= _seconds;
  _nseconds = (uint) (seconds * 1000000000);
}

int TimedPullPush::initialize()
{
  log(LoggerI::INFO, 0, "initialize");
  // Schedule my timer
  _timeCallback = delaycb(_seconds,
                          _nseconds, _runTimerCB);

  return 0;
}


void TimedPullPush::runTimer()
{
  // remove the timer id
  _timeCallback = 0;

  // Attempt to fetch a tuple. If it's there, it will certainly be
  // delivered.
  TuplePtr p = input(0)->pull(_unblockPull);

  // Was it there?
  if (p) {
    // Goody, just push it out
    int result = output(0)->push(p, _unblockPush);

    // Were we pushed back?
    if (result == 0) {
      // Yup.  Don't reschedule until my _unblockPush is invoked.
      log(LoggerI::INFO, 0, "runTimer: push blocked");
    } else {
      // No.  Reschedule me
      log(LoggerI::INFO, 0, "runTimer: rescheduling");
      _timeCallback = delaycb(_seconds,
                              _nseconds,
                              _runTimerCB);
    }
  } else {
    // Didn't get any tuples from my input. Don't reschedule me until my
    // _unblockPull is invoked
    log(LoggerI::INFO, 0, "runTimer: pull blocked");
  }
}

void TimedPullPush::pullWakeup()
{
  // I'd better not be already scheduled
  assert(_timeCallback == 0);

  log(LoggerI::INFO, 0, "pullWakeup");

  // Okey dokey.  Reschedule me into the future
  _timeCallback = delaycb(_seconds,
                          _nseconds,
                          _runTimerCB);
}

void TimedPullPush::pushWakeup()
{
  // I'd better not be already scheduled
  assert(_timeCallback == 0);

  log(LoggerI::INFO, 0, "pushWakeup");

  // Okey dokey.  Reschedule me into the future
  _timeCallback = delaycb(_seconds,
                          _nseconds,
                          _runTimerCB);
}

