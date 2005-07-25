/*
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include <algorithm>

#include "ccr.h"
#include "sysconf.h"
#include "val_uint64.h"
#include "val_double.h"
#include "val_str.h"
#include "val_tuple.h"


/////////////////////////////////////////////////////////////////////
//
// Receive element
//

/**
 * Constructor: 
 * name: The name given to this element.
 * rwnd: Initial receiver window size (default given in ccrx.h).
 */
CCR::CCR(str name, double rwnd, uint src, bool flow) 
  : Element(name, (flow ? 2 : 1), 2), _ack_cb(cbv_null),
    rwnd_(rwnd), src_field_(src), flow_(flow)
{
}

/**
 * Acknowledge tuple p if ack_q is empty and output1 is open.
 * Otherwise, add to ack_q and enable callback.
 */
TuplePtr CCR::simple_action(TupleRef p) 
{
  uint64_t seq  = 0;
  ValuePtr src  = NULL; 
  ValuePtr port = NULL;

  for (uint i = 0; i < p->size(); i++) {
    try {
      TupleRef t = Val_Tuple::cast((*p)[i]); 
      if (Val_Str::cast((*t)[0]) == "SEQ") {
        seq  = Val_UInt64::cast((*t)[1]);
        src  = (*t)[2];
        port = (*t)[3];
      }
    }
    catch (Value::TypeError& e) { } 
  }
  if (!src || !port) return p;		// Punt

  TupleRef ack  = Tuple::mk();
  ack->append(src);			// Source location
  ack->append(port);			// Source location
  ack->append(Val_Str::mk("ACK"));
  ack->append(Val_UInt64::mk(seq));	// The sequence number
  ack->append(Val_Double::mk(rwnd_));	// Receiver window size
  ack->freeze();
  ack_q_.push_back(ack);		// Append to ack queue

  if (_ack_cb != cbv_null) {
    (*_ack_cb)();			// Notify new ack
    _ack_cb = cbv_null;
  } 
  return p;				// Forward data tuple
}

/**
 * Pulls the next acknowledgement in ack_q_ to send to the
 * receiver.
 */
TuplePtr CCR::pull(int port, cbv cb)
{
  if (port == 1) {
    if (!ack_q_.empty()) {
      TuplePtr ack = ack_q_.front();
      ack_q_.pop_front();
      return ack;
    }
    _ack_cb = cb;
    return NULL;
  } 

  // Need this to go through regular pull
  return this->Element::pull(port, cb); 	
}

int CCR::push(int port, TupleRef tp, cbv cb)
{
  if (port == 1) {
    try {
      if (Val_Str::cast((*tp)[0]) == "RWND")
        rwnd_ = Val_Double::cast((*tp)[1]);
    }
    catch (Value::TypeError *e) {
      log(LoggerI::WARN, 0, "CCR::push TypeError Thrown on port 1"); 
    } 
    return int(rwnd_);
  }

  // Need this to go through regular push
  return this->Element::push(port, tp, cb); 	
}
