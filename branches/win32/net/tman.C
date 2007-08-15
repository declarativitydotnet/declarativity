// -*- c-basic-offset: 2; related-file-name: "timedPushSource.h" -*-
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

#ifdef WIN32
#include "p2_win32.h"
#endif // WIN32
#include <element.h>
#include <iostream>
#include <math.h>
#include "tman.h"
#include "loop.h"
#include "val_str.h"
#include "val_uint32.h"
#include "val_tuple.h"
#include "val_time.h"
#include <boost/bind.hpp>

TrafficManager::TrafficManager(string n, string a, uint k, uint r, double s)
  : Element(n, 1, 2),
    _seconds(s), 
    _wakeupCB(boost::bind(&TrafficManager::wakeup, this)),
    _runTimerCB(boost::bind(&TrafficManager::runTimer, this)),
    my_addr_(a),
    my_key_(k),
    key_range_(r),
    total_received_(0)
{
}

int TrafficManager::push(int port, TuplePtr tp, b_cbv cb) {
  assert(port == 0);
  ValuePtr vp = ValuePtr();

  uint key = getKey(tp);
  if (key == my_key_) {
    total_received_++;
    assert(output(1)->push(mkResponse(tp), 0));
  }
  else if (!processResponse(tp)) {
    assert(0);
  }
  return 1;
}      
    
int TrafficManager::initialize()
{
  log(Reporting::INFO, 0, "initialize");
  // Schedule my timer
  if (_seconds != 0.0)
    _timeCallback = delayCB(_seconds, _runTimerCB, this);
  return 0;
}


void TrafficManager::runTimer()
{
  // remove the timer id
  _timeCallback = NULL;

  boost::posix_time::ptime now;
  getTime(now);

  // Create a tuple
  TuplePtr tuple = Tuple::mk();
  tuple->append(Val_Str::mk("LOOKUP"));
  tuple->append(Val_UInt32::mk(genLookupKey()));
  tuple->append(Val_Str::mk(my_addr_));
  tuple->append(Val_Time::mk(now));
  tuple->append(Val_UInt32::mk(0));	// Hop count
  tuple->append(Val_UInt32::mk(0));	// Retries
  tuple->freeze();

  // Attempt to push it
  if (!output(0)->push(tuple, _wakeupCB)) {
    // We have been pushed back.  Don't reschedule wakeup
    log(Reporting::INFO, 0, "runTimer: sleeping");
  } else {
    // Reschedule me into the future
    log(Reporting::INFO, 0, "runTimer: rescheduling");
    _timeCallback = delayCB(_seconds, _runTimerCB, this);
  }
}

void TrafficManager::wakeup()
{
  // I'd better not be already scheduled
  assert(_timeCallback == NULL);

  log(Reporting::INFO, 0, "wakeup");

  // Okey dokey.  Reschedule me into the future
  _timeCallback = delayCB(_seconds, _runTimerCB, this);
}

REMOVABLE_INLINE uint TrafficManager::genLookupKey() {
  uint k = my_key_;
  while (k == my_key_) k = rand() % key_range_;
  return k;
}

REMOVABLE_INLINE int TrafficManager::getKey(TuplePtr tp) {
  for (uint i = 0; i < tp->size(); i++) {
    try {
      if (Val_Str::cast((*tp)[i]) == "LOOKUP") {
        return Val_UInt32::cast((*tp)[i+1]);
      }
    }
    catch (Value::TypeError e) { } 
  }
  return -1;
}

REMOVABLE_INLINE bool TrafficManager::processResponse(TuplePtr tp) {
  for (uint i = 0; i < tp->size(); i++) {
    try {
      if (Val_Str::cast((*tp)[i]) == "RESPONSE") {
		boost::posix_time::ptime t = Val_Time::cast((*tp)[i+2]); 
        uint    hc = Val_UInt32::cast((*tp)[i+3]);
        uint    rc = Val_UInt32::cast((*tp)[i+4]);
        TELL_INFO << "RECEIVE RESPONSE: delay " << delay(&t) << ", hop count "
                  << hc << ", retry count " << rc << std::endl;
        return true;
      }
    }
    catch (Value::TypeError e) { } 
  }
  return false;
}

REMOVABLE_INLINE TuplePtr TrafficManager::mkResponse(TuplePtr tp) {
  TuplePtr resp = Tuple::mk();
  resp->append(Val_Str::mk("RESPONSE"));
   
  for (uint i = 0; i < tp->size(); i++) {
    try {
      if (Val_Str::cast((*tp)[i]) == "LOOKUP") {
        resp->append((*tp)[i+2]);		// Response address
        resp->append((*tp)[i+3]);		// Lookup time
        resp->append((*tp)[i+4]);		// Hop Count
        resp->append((*tp)[i+5]);		// Retry count
        resp->append(Val_UInt32::mk(0));	// HACK: PAD FOR DATA PEL TRANSFORM
        resp->append(Val_UInt32::mk(0));	// HACK: PAD FOR DATA PEL TRANSFORM
        resp->append(Val_UInt32::mk(0));	// HACK: PAD FOR DATA PEL TRANSFORM
        resp->freeze();
        return resp;
      }
    }
    catch (Value::TypeError e) { } 
  }
  return TuplePtr();

}

REMOVABLE_INLINE uint32_t TrafficManager::delay(boost::posix_time::ptime *ts)
{
  boost::posix_time::ptime now;
  getTime(now);

  return((uint32_t)(now - *ts).total_milliseconds());
}
