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
#include "val_double.h"
#include "val_uint32.h"
#include <boost/bind.hpp>

DEFINE_ELEMENT_INITS(TimedPullPush, "TimedPullPush")

TimedPullPush::TimedPullPush(string name,
                             double seconds,
                             int tuples)
  : Element(name, 1, 1),
    _seconds(seconds),
    _tuples(tuples),
    _counter(0),
    _unblockPull(boost::bind(&TimedPullPush::pullWakeup, this)),
    _pendingPull(false),
    _unblockPush(boost::bind(&TimedPullPush::pushWakeup, this)),
    _pendingPush(false),
    _runTimerCB(boost::bind(&TimedPullPush::runTimer, this)),
    _timeCallback(NULL)
{
  assert(_tuples >= 0);
}

TuplePtr
TimedPullPush::pull(int port, b_cbv cb)
{
  ELEM_ERROR("Should not allow pull!");
  assert(0);
}

/**
 * Generic constructor.
 * Arguments:
 * 2. Val_Str:    Element Name.
 * 3. Val_Double: Seconds.
 * 4. Val_UInt32: Tuples.
 */
TimedPullPush::TimedPullPush(TuplePtr args)
  : Element(Val_Str::cast((*args)[2]), 1, 1),
    _seconds(Val_Double::cast((*args)[3])),
    _tuples(0),
    _counter(0),
    _unblockPull(boost::bind(&TimedPullPush::pullWakeup, this)),
    _pendingPull(false),
    _unblockPush(boost::bind(&TimedPullPush::pushWakeup, this)),
    _pendingPush(false),
    _runTimerCB(boost::bind(&TimedPullPush::runTimer, this)),
    _timeCallback(NULL)
{
  if (args->size() > 4)
    _tuples = Val_UInt32::cast((*args)[4]);
  //std::cout<<"\nTUPLES = "<<_tuples<<std::endl; std::cout.flush();
}


int
TimedPullPush::initialize()
{
  ELEM_INFO("initialize");
  // Schedule my timer
  reschedule();
  return 0;
}


void
TimedPullPush::runTimer()
{
  // remove the timer id
  _timeCallback = 0;

  ELEM_INFO("runTimer: called back");
  
  if(_pendingPush || _pendingPull){
	  reschedule(); return;
  }


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
      ELEM_INFO("runTimer: push blocked");
      _pendingPush = true;
    } 
    // Reschedule me
    reschedule();

  } else {
    // Didn't get any tuples from my input. Don't reschedule me until my
    // _unblockPull is invoked
    ELEM_INFO("runTimer: pull blocked");
    _pendingPull = true;
  }
}


void
TimedPullPush::pullWakeup()
{
  ELEM_INFO("pullWakeup");

  _pendingPull = false;
  if(_timeCallback != 0)
	  return;

  // Okey dokey.  Reschedule me into the future
  reschedule();
}


void
TimedPullPush::pushWakeup()
{
  ELEM_INFO("pushWakeup");

  _pendingPush = false;
  if(_timeCallback != 0)
	  return;
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

    ELEM_INFO("reschedule: rescheduling");
    // Okey dokey.  Reschedule me into the future
    _timeCallback = delayCB(_seconds, _runTimerCB, this);
  } else {
    ELEM_INFO("reschedule: DONE!");
  }
}
