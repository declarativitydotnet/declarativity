// -*- c-basic-offset: 2; related-file-name: "pingpong.h" -*-
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
#include <time.h>
#include "math.h"

#include "ping.h"
#include "val_null.h"
#include "val_str.h"
#include "val_int32.h"
#include "val_uint64.h"
#include "tuple.h"
#include "val_double.h"


/////////////////////////////////
/// Ping. 
/// numPings - The number of ping attemps for those with no replies
/// measureRTT - Report RTT measurements as well in the pingResult tuple, or -1 if timeout
/// seconds - Periodic push.
/// retry_interval is the interval between retries (in seconds). Assuming that after retry_interval,
/// the previous ping is lost or dead
/////////////////////////////////
Ping::Ping(string name, int numPings, int seconds, double retry_interval)
  : Element(name, 2, 2),
    _seconds(seconds),
    _wakeupCB(boost::bind(&Ping::wakeup, this)),
    _runTimerCB(boost::bind(&Ping::runTimer, this))
{
  _name = name;
  _numPings = numPings;
  _retry_interval = retry_interval;
}

int Ping::initialize()
{
  log(LoggerI::INFO, 0, "initialize");
  // Schedule my timer
  _timeCallback = delayCB(_seconds, _runTimerCB);

  return 0;
}

int Ping::push(int port, TuplePtr p, b_cbv cb)
{
  // when received a ping(X,Y) request
  // first, check generate a ping request to be sent out
  // second, store the ping request in a pending list
  if (p == Tuple::EMPTY) { return 1; }
  ValuePtr tableName = (*p)[0];
  ValuePtr defaultName = Val_Str::mk("Ping");
  if (port == 0 && tableName->compareTo(defaultName) == 0) {
    ValuePtr source = (*p)[1];
    ValuePtr destination = (*p)[2];

    TuplePtr pingRequestTuple = Tuple::mk();
    pingRequestTuple->append(Val_Str::mk("PingRequest"));
    pingRequestTuple->append(destination);  
    pingRequestTuple->append(source);  
    pingRequestTuple->freeze();

    // in future, have to check whether there is already one present
    _iterator = _pendingPings.find(destination->toString());
    if (_iterator == _pendingPings.end()) {
      // if we have a pending ping request on this, ignore the push
      Entry *e = new Entry(pingRequestTuple);
      e->numPings = 1;
      _pendingPings.insert(std::make_pair(destination->toString(), e)); 
    }
  }

  // when receive a pong(X,Y) response on a different port,
  // look up pending pings, send out ping result locally
  ValuePtr defaultResponse = Val_Str::mk("PingResponse");
  if (port == 1 && tableName->compareTo(defaultResponse) == 0) {
    // create a ping result tuple
    // generate a success
     generatePingSuccess(p);
  }

  return 1;
}

void Ping::generatePingFailure(TuplePtr p)
{
  ValuePtr source = (*p)[1];
  ValuePtr destination = (*p)[2];
  
  // generate a failure, put in pendingResults
  TuplePtr pingResultTuple = Tuple::mk();
  pingResultTuple->append(Val_Str::mk("PingResult"));
  pingResultTuple->append(source); 
  pingResultTuple->append(destination); 
  pingResultTuple->append(Val_Double::mk(-1));
  pingResultTuple->freeze();
  _pendingResults.erase(destination->toString());
  _pendingResults.insert(std::make_pair(destination->toString(), pingResultTuple));
}

void Ping::generatePingSuccess(TuplePtr p)
{
  // remove the pending
  ValuePtr source = (*p)[1];
  ValuePtr destination = (*p)[2];
  
  _iterator = _pendingPings.find(destination->toString());
  Entry *e = _iterator->second;

  if (_iterator != _pendingPings.end()) {
    // we haven't time this out
    double timeDifference = ((double) clock() - e->lastPingTime) / CLOCKS_PER_SEC;
    _pendingPings.erase(destination->toString()); 

    TuplePtr pingResultTuple = Tuple::mk();
    pingResultTuple->append(Val_Str::mk("PingResult"));
    pingResultTuple->append(source); 
    pingResultTuple->append(destination); 
    pingResultTuple->append(Val_Double::mk(timeDifference));
    pingResultTuple->freeze();
    
    _pendingResults.erase(destination->toString());
    _pendingResults.insert(std::make_pair(destination->toString(), pingResultTuple));        
  }


}

void Ping::runTimer()
{

  // remove the timer id
  _timeCallback = 0;

  for (_iterator = _pendingPings.begin(); _iterator != _pendingPings.end(); _iterator++) {
    Entry *e = _iterator->second;
    double timeDiffSeconds = ((double) clock() - e->lastPingTime) / CLOCKS_PER_SEC;

    TuplePtr t = e->p;
    if (e->numPings == 1 || (timeDiffSeconds > _retry_interval)) {
      // enough time has passed
      int result = output(1)->push(t, _wakeupCB);
      e->lastPingTime = (double) clock();

      if (e->numPings < _numPings) {
	e->numPings ++; // increase the number of retries
      } else {	
	// this is not good. we have to bail out with a negative pingResult
	generatePingFailure(e->p);
	_pendingPings.erase(_iterator->first);
	free(_iterator->second);
      }
      
      if (result == 0) {
	// We have been pushed back.  Don't reschedule wakeup
	log(LoggerI::INFO, 0, "runTimer: sleeping");
	return;
      } 
    }
  }

  // check here for results to push up
  for (_tupleIterator = _pendingResults.begin(); _tupleIterator != _pendingResults.end(); _tupleIterator++) {
    TuplePtr t = _tupleIterator->second;
    int result = output(0)->push(t, _wakeupCB);
    _pendingResults.erase(_tupleIterator->first);
      
    if (result == 0) {
      // We have been pushed back.  Don't reschedule wakeup
      log(LoggerI::INFO, 0, "runTimer: sleeping");
      return;
    }     
  }

  // Reschedule me into the future
  _timeCallback = delayCB(_seconds,
			  _runTimerCB);  

}

void Ping::wakeup()
{
  // I'd better not be already scheduled
  assert(_timeCallback == 0);

  log(LoggerI::INFO, 0, "wakeup");

  // Okey dokey.  Reschedule me into the future
  _timeCallback = delayCB(_seconds,
                          _runTimerCB);
}



