// -*- c-basic-offset: 2; related-file-name: "pong.h" -*-
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

#include <utility>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <iostream>

#include "pong.h"
#include "val_null.h"
#include "val_str.h"
#include "val_int32.h"
#include "tuple.h"
#include "math.h"


/////////////////////////////
/// Pong 
////////////////////////////
Pong::Pong(string name, int seconds)
  : Element(name, 1, 1),
    _seconds(seconds),
    _wakeupCB(boost::bind(&Pong::wakeup, this)),
    _runTimerCB(boost::bind(&Pong::runTimer, this))
{
  _name = name;
}

int Pong::initialize()
{
  log(LoggerI::INFO, 0, "initialize");
  // Schedule my timer
  _timeCallback = delayCB(_seconds,
                          _runTimerCB);

  return 0;
}


int Pong::push(int port, TuplePtr p, b_cbv cb)
{  
  if (p == Tuple::EMPTY) { return 1; }
  ValuePtr tableName = (*p)[0];
  // is it a ping request from port 1? If so, send back a reply
  ValuePtr defaultName = Val_Str::mk("PingRequest");
  if (port == 0 && tableName->compareTo(defaultName) == 0)  {	
    ValuePtr destination = (*p)[1];
    ValuePtr source = (*p)[2];
    
    TuplePtr pingRequestTuple = Tuple::mk();
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
  _timeCallback = delayCB(_seconds,
                          _runTimerCB);
}


void Pong::runTimer()
{
 // remove the timer id
  _timeCallback = 0;

  // check here for results to push up
  while (_pendingResults.size() > 0) {
    TuplePtr t = _pendingResults.front();
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
  _timeCallback = delayCB(_seconds,
			  _runTimerCB);  
}

