// -*- c-basic-offset: 2; related-file-name: "timedPullPush.h" -*-
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

#include <timedPullPush.h>
#include <element.h>
#include <math.h>

#include "loop.h"
#include "val_str.h"
#include "val_uint64.h"

TimedPullPush::TimedPullPush(string name,
                             double seconds,
                             int tuples)
  : Element(name, 1, 1),
    _seconds(seconds),
    _tuples(tuples),
    _counter(0),
    _unblockPull(boost::bind(&TimedPullPush::pullWakeup, this)),
    _unblockPush(boost::bind(&TimedPullPush::pushWakeup, this)),
    _runTimerCB(boost::bind(&TimedPullPush::runTimer, this)),
    _timeCallback(NULL)
{
  assert(_tuples >= 0);
}

int TimedPullPush::initialize()
{
  log(Reporting::INFO, 0, "initialize");
  // Schedule my timer
  reschedule();
  return 0;
}


void TimedPullPush::runTimer()
{
  // remove the timer id
  _timeCallback = 0;

  log(Reporting::INFO, 0, "runTimer: called back");

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
      log(Reporting::INFO, 0, "runTimer: push blocked");
    } else {
      // No.  Reschedule me
      reschedule();
    }
  } else {
    // Didn't get any tuples from my input. Don't reschedule me until my
    // _unblockPull is invoked
    log(Reporting::INFO, 0, "runTimer: pull blocked");
  }
}

void TimedPullPush::pullWakeup()
{
  log(Reporting::INFO, 0, "pullWakeup");

  // Okey dokey.  Reschedule me into the future
  reschedule();
}

void TimedPullPush::pushWakeup()
{
  log(Reporting::INFO, 0, "pushWakeup");

  // Okey dokey.  Reschedule me into the future
  reschedule();
}

void
TimedPullPush::reschedule()
{
  assert(_timeCallback == 0);

  if ((_tuples == 0) ||
      ((_tuples > 0) && (_counter < _tuples))) {
    _counter++;

    log(Reporting::INFO, 0, "reschedule: rescheduling");
    // Okey dokey.  Reschedule me into the future
    _timeCallback = delayCB(_seconds, _runTimerCB, this);
  } else {
    log(Reporting::INFO, 0, "reschedule: DONE!");
  }
}
