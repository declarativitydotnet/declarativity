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

#include "ccr.h"
#include "val_uint64.h"
#include "val_uint32.h"
#include "val_double.h"
#include "val_str.h"
#include "val_tuple.h"
#include "val_null.h"
#include "netglobals.h"


/////////////////////////////////////////////////////////////////////
//
// Receive element
//

/**
 * Constructor: 
 * name: The name given to this element.
 * rwnd: Initial receiver window size (default given in ccrx.h).
 */
CCR::CCR(string name) 
  : Element(name, 1, 2), _ack_cb(0) { }

/**
 * Acknowledge tuple p if ack_q is empty and output1 is open.
 * Otherwise, add to ack_q and enable callback.
 */
TuplePtr CCR::simple_action(TuplePtr p) 
{
  ValuePtr src    = (*p)[SRC]; 
  ValuePtr dest   = (*p)[DEST]; 
  ValuePtr seq    = (*p)[SEQ];

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

/**
 * Pulls the next acknowledgement in ack_q_ to send to the
 * receiver.
 */
TuplePtr CCR::pull(int port, b_cbv cb)
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

  // Need this to go through regular pull
  return this->Element::pull(port, cb); 	
}

/*
int CCR::push(int port, TuplePtr p, b_cbv cb)
{
  if ((rand() / (float)RAND_MAX) < 0.9)
    return this->Element::push(port, p, cb); 	
  return 1;
}
*/
