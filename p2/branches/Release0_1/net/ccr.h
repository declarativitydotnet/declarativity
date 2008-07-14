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

#ifndef __CCRX_H__
#define __CCRX_H__

#include <deque>
#include "tuple.h"
#include "element.h"
#include "inlines.h"


typedef uint64_t SeqNum;

/**
 * The CCR element acknowledges incoming CCT tuples on port 1 
 * as they are passed from input port 0 to output port 0. 
 * The element does not care about the tuple order.
 * A CCR object needs to know where the sequence number and
 * source location (e.g., IP address, ID) fields are in a tuple. 
 * CCR will build an acknowledgement tuple with the following schema:
 * <source_location, sequence_number, receiver_window> 
 * The receiver_window is the only local state maintained by CCR,
 * which is set by sending a tuple to input port 1 containing the
 * desired window size in the first position (flow control). 
 * To summarize:
 * Input  0 (agnostic): Stream of data tuples with require CC info. 
 * Input  1 (push):     Receiver window size (flow control). (Optional)
 * Output 0 (agnostic): Stream of data tuples (possibly out of order, with dups).	
 * Output 1 (pull):     Acknowledgements of individual tuples.
 */
class CCR : public Element {
public:
  CCR(string name, double rwnd=512., uint src=0, bool flow=false);
  const char *class_name() const { return "CCR";};
  const char *processing() const { return flow_ ? "ah/al" : "a/al"; };
  const char *flow_code() const	 { return flow_ ? "--/--" : "-/--"; };

  TuplePtr simple_action(TuplePtr p);		// Ack on output1 before passing to output0.

  TuplePtr pull(int port, b_cbv cb);		// Pull next acknowledgement from ack_q

  int push(int port, TuplePtr tp, b_cbv cb);	// Flow control input

private:
  b_cbv _ack_cb; 					// Callback to send an ack 

  double   rwnd_;				// Receiver window size
  uint     src_field_;
  bool     flow_;
  std::deque <TuplePtr> ack_q_;			// Output ack queue
};
  
#endif /* __CCRX_H_ */
