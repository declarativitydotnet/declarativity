/*
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

#include <algorithm>
#include <stdlib.h>
#include "basicAck.h"
#include "val_uint64.h"
#include "val_uint32.h"
#include "val_str.h"
#include "val_tuple.h"
#include "netglobals.h"


/////////////////////////////////////////////////////////////////////
//
// Receive element
//

/**
 * Constructor: 
 * name: The name given to this element.
 */
BasicAck::BasicAck(string name) 
  : Element(name, 1, 2), _ack_cb(0)
{
}

/**
 * Acknowledge tuple p if ack_q is empty and output1 is open.
 * Otherwise, add to ack_q and enable callback.
 */
TuplePtr BasicAck::simple_action(TuplePtr p) 
{
  ValuePtr src  = (*p)[SRC]; 

/*
  if ((rand() / (float)RAND_MAX) < 0.5)
    return p;
*/

  TuplePtr ack  = Tuple::mk();
  ack->append(src);			// Source location
  ack->append(Val_Str::mk("ACK"));
  for(unsigned i = 0; i < STACK_SIZE; i++)
    ack->append((*p)[i]);
  ack->freeze();
  ack_q_.push_back(ack);		// Append to ack queue

  if (_ack_cb) {
    _ack_cb();				// Notify new ack
    _ack_cb = 0;
  } 
  return p;				// Forward data tuple
}

/*
int BasicAck::push(int port, TuplePtr p, b_cbv cb)
{
  if ((rand() / (float)RAND_MAX) < 0.5)
    return this->Element::push(port, p, cb); 	
  return 1;
}
*/

/**
 * Pulls the next acknowledgement in ack_q_ to send to the
 * receiver.
 */
TuplePtr BasicAck::pull(int port, b_cbv cb)
{
  if (port == 1) {
    if (!ack_q_.empty()) {
      TuplePtr ack = ack_q_.front();
      ack_q_.pop_front();
      return ack;
    }
    _ack_cb = cb;
    return TuplePtr();
  } 

  // Need this to go through regular pull to call simple_action
  return this->Element::pull(port, cb); 	
}
