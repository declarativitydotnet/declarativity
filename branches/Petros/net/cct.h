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

#ifndef __CCT_H__
#define __CCT_H__

#include <map>
#include <deque>
#include "tuple.h"
#include "element.h"
#include "elementRegistry.h"
#include "inlines.h"
#include "tupleseq.h"


class CCTuple;

/**
 * The Tx element pulls tuples from input0, wraps them in a
 * control structure, and then pushes it to the output. It
 * receives acks on input1, which are used to move the sliding
 * window forward. Unacknowledged packets are retransmitted after
 * begin in transit for a time of 4*RTT. This element implements
 * slow-start and AIMD as its congestion control protcol. 
 *
 * To summarize:
 * Input  0 (agnostic): Tuple to send. 
 * Input  1 (push)    : Acknowledgement of some (possibly) outstanding tuple.
 * Output 0 (agnostic): Tuple to send with cc info wrapper.
 * Output 1 (push)    : Status of a tuple that was recently sent.
 */
class CCT : public Element {
public:
  CCT(string name, double init_wnd=1., double max_wnd=2048.);
  CCT(TuplePtr args);
  const char *class_name() const { return "CCT";   };
  const char *processing() const { return "lh/lh"; };
  const char *flow_code() const	 { return "--/--"; };

  int push(int port, TuplePtr tp, b_cbv cb);	// Incoming, either add to send_q or ack
  TuplePtr pull(int port, b_cbv cb);		// Rate limited output tuple stream

  DECLARE_PUBLIC_ELEMENT_INITS

private:
  void timeout_cb(CCTuple*);			// Callback for to retry sending a tuple
  void data_ready();				// Callback for input data ready
  REMOVABLE_INLINE void dealloc(ValuePtr,SeqNum); // Remove CCTuple from map
  REMOVABLE_INLINE void map(CCTuple*, double);	// Map tuple and set timeout
  REMOVABLE_INLINE void successTransmit();	// Successful tuple transmission 
  REMOVABLE_INLINE void failureTransmit();	// Failed tuple transmission 
  REMOVABLE_INLINE int  current_window();	// Returns the current window size
  REMOVABLE_INLINE int  max_window();		// Returns the max window size

  b_cbv   _data_cb; 				// Callback for data output ready
  bool    data_on_;

  double    max_wnd_;				// Max window size
  double    cwnd_;				// Current congestion window size
  double    ssthresh_;				// Slow start threshold

  typedef std::map<ValuePtr, 
                   boost::shared_ptr<std::map<SeqNum, CCTuple*> >, 
                   Value::Comparator> CCTupleIndex;
  CCTupleIndex tmap_;			// Map containing unacked in transit tuples

  DECLARE_PRIVATE_ELEMENT_INITS
};
  
#endif /* __CCTX_H_ */
