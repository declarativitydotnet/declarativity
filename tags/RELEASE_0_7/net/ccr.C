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

#include "ccr.h"
#include "val_uint64.h"
#include "val_uint32.h"
#include "val_double.h"
#include "val_str.h"
#include "val_tuple.h"
#include "val_null.h"


/////////////////////////////////////////////////////////////////////
//
// Receive element
//

/**
 * Constructor: 
 * name: The name given to this element.
 * rwnd: Initial receiver window size (default given in ccrx.h).
 */
CCR::CCR(string name, double rwnd, int dest, int src, int seq) 
  : Element(name, 1, 2),
    _ack_cb(0),
    rwnd_(rwnd),
    src_field_(src),
    dest_field_(dest),
    seq_field_(seq)
{
}

/**
 * Acknowledge tuple p if ack_q is empty and output1 is open.
 * Otherwise, add to ack_q and enable callback.
 */
TuplePtr CCR::simple_action(TuplePtr p) 
{
  ValuePtr src    = (*p)[src_field_]; 
  ValuePtr dest   = (dest_field_ < 0) ? Val_Null::mk() : (*p)[dest_field_]; 
  ValuePtr seq    = (*p)[seq_field_];

  if (!seq) {
    log(LoggerI::ERROR, 0, "CCR::simple_action NO SEQUENCE NUMBER WITH TUPLE!!"); 
    return p;
  }

  if (!src) {
    log(LoggerI::ERROR, 0, "CCR::simple_action NO SOURCE ADDRESS"); 
    return p;			// Punt
  }

  TuplePtr ack  = Tuple::mk();
  ack->append(src);			// Source location
  ack->append(Val_Str::mk("ACK"));
  ack->append(dest);			// Destination address
  ack->append(seq);			// The sequence number
  ack->append(Val_Double::mk(rwnd_));	// Receiver window size
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

int CCR::push(int port, TuplePtr tp, b_cbv cb)
{
  if (port == 1) {
    try {
      if (Val_Str::cast((*tp)[0]) == "RWND")
        rwnd_ = Val_Double::cast((*tp)[1]);
    }
    catch (Value::TypeError e) {
      log(LoggerI::WARN, 0, "CCR::push TypeError Thrown on port 1"); 
    } 
    return int(rwnd_);
  }

  // Need this to go through regular push
  return this->Element::push(port, tp, cb); 	
}
