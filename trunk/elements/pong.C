// -*- c-basic-offset: 2; related-file-name: "pong.h" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include <utility>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <iostream>
#include <async.h>
#include <arpc.h>

#include "pong.h"
#include "val_null.h"
#include "val_str.h"
#include "val_int32.h"
#include "xdr_suio.h"
#include "tuple.h"
#include "math.h"


/////////////////////////////
/// Pong 
////////////////////////////
Pong::Pong(str name, int seconds)
    : Element(name, 1, 1), _wakeupCB(boost::bind(&Pong::wakeup, this)), _runTimerCB(boost::bind(&Pong::runTimer, this))
{
  _name = name;
  _seconds = (uint) floor(seconds);
  seconds -= _seconds;
  _nseconds = (uint) (seconds * 1000000000);
}

int Pong::initialize()
{
  log(LoggerI::INFO, 0, "initialize");
  // Schedule my timer
  _timeCallback = delaycb(_seconds,
                          _nseconds, _runTimerCB);

  return 0;
}


int Pong::push(int port, TupleRef p, b_cbv cb)
{  
  if (p == Tuple::EMPTY) { return 1; }
  ValueRef tableName = (*p)[0];
  // is it a ping request from port 1? If so, send back a reply
  ValueRef defaultName = Val_Str::mk("PingRequest");
  if (port == 0 && tableName->compareTo(defaultName) == 0)  {	
    ValueRef destination = (*p)[1];
    ValueRef source = (*p)[2];
    
    TupleRef pingRequestTuple = Tuple::mk();
    pingRequestTuple->append(Val_Str::mk("PingResponse"));
    pingRequestTuple->append(source);  
    pingRequestTuple->append(destination);  
    pingRequestTuple->freeze();
    _pendingResults.push(pingRequestTuple);
  }
  return 1;
}



void Pong::wakeup()
{
  // I'd better not be already scheduled
  assert(_timeCallback == 0);

  log(LoggerI::INFO, 0, "wakeup");

  // Okey dokey.  Reschedule me into the future
  _timeCallback = delaycb(_seconds,
                          _nseconds,
                          _runTimerCB);
}


void Pong::runTimer()
{
 // remove the timer id
  _timeCallback = 0;

  // check here for results to push up
  while (_pendingResults.size() > 0) {
    TupleRef t = _pendingResults.front();
    _pendingResults.pop();
    int result = output(0)->push(t, _wakeupCB);
      
    if (result == 0) {
      // We have been pushed back.  Don't reschedule wakeup
      log(LoggerI::INFO, 0, "runTimer: sleeping");
      return;
    }     
  }

  // Reschedule me into the future
  //log(LoggerI::INFO, 0, "runTimer: rescheduling");
  _timeCallback = delaycb(_seconds,
			  _nseconds,
			  _runTimerCB);  
}

