// -*- c-basic-offset: 2; related-file-name: "pingpong.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: UDP Ping/Pong
 */

#ifndef __PONG_H__
#define __PONG_H__

#include "element.h"
#include "val_opaque.h"

#include <map>
#include <vector>
#include <queue>

/* Accepts ping requests, generate ping responses */
class Pong : public Element {
public:
  Pong(string name, int seconds);
  int push(int port, TuplePtr p, b_cbv cb);      
  int initialize();  

  const char *class_name() const		{ return "Pong";}
  const char *processing() const		{ return "h/h"; }
  const char *flow_code() const		{ return "x/x"; }
  
private:

  void wakeup();
  void runTimer();

 /** The the interval in seconds */
  uint _seconds;

  /** My time callback ID. */
  timeCBHandle * _timeCallback;

  /** My wakeup callback */
  b_cbv _wakeupCB, _runTimerCB;

  std::queue<TuplePtr> _pendingResults;
};
  
  


#endif /* __PONG_H_ */
