/*
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 */

#ifndef __BASICACK_H__
#define __BASICACK_H__

#include <deque>
#include "tuple.h"
#include "element.h"
#include "inlines.h"


/**
 */
class BasicAck : public Element {
public:
  BasicAck(string name);
  const char *class_name() const { return "BasicAck";};
  const char *processing() const { return "a/al"; };
  const char *flow_code() const	 { return "x/x-"; };

  TuplePtr simple_action(TuplePtr p);	// Ack on output1 before passing to output0.
  TuplePtr pull(int port, b_cbv cb);	// Pull next acknowledgement from ack_q
  // int push(int port, TuplePtr p, b_cbv cb);

private:
  b_cbv    _ack_cb; 			// Callback to send an ack 
  std::deque <TuplePtr> ack_q_;		// Output ack queue
};
  
#endif /* __BASICACK_H_ */
