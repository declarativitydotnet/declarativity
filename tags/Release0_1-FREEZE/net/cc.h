// -*- c-basic-offset: 2; related-file-name: "element.C" -*-
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

#ifndef __CC_H__
#define __CC_H__

#include <map>
#include <deque>
#include "tuple.h"
#include "element.h"
#include "inlines.h"


class OTuple;
typedef uint64_t SeqNum; 

/**
 * The Rx element acknowledges incoming cc tuples on output1 
 * as they are passed from input0 to output0. The element does 
 * not care about the tuple order.
 */
class CCRx : public Element {
public:
  CCRx(string name, double rwnd=512., int seq=0, int src=1);
  const char *class_name() const { return "CCRx";};
  const char *processing() const { return "a/al"; };
  const char *flow_code() const	 { return "x/x-"; };

  TuplePtr simple_action(TuplePtr p);		// Ack on output1 before passing to output0.

  TuplePtr pull(int port, b_cbv cb);		// Pull next acknowledgement from ack_q

  int push(int port, TuplePtr tp, b_cbv cb);	// Flow control input

private:
  b_cbv _ack_cb; 					// Callback to send an ack 

  double   rwnd_;				// Receiver window size
  int      seq_field_;
  int      src_field_;
  std::deque <TuplePtr> ack_q_;		// Output ack queue
};
  
/**
 * The Tx element pulls tuples from input0, wraps them in a
 * control structure, and then pushes it to the output. It
 * receives acks on input1, which are used to move the sliding
 * window forward. Unacknowledged packets are retransmitted after
 * begin in transit for a time of 4*RTT. This element implements
 * slow-start and AIMD as its congestion control protcol. 
 */
class CCTx : public Element {
public:
  CCTx(string name, double init_wnd, double max_wnd, uint32_t max_retry=5,
       uint32_t seq_field = 0, uint32_t ack_seq_field=1, uint32_t ack_rwnd_field=2);
  const char *class_name() const { return "CC::Tx";};
  const char *processing() const { return "hh/ll"; };
  const char *flow_code() const	 { return "--/--"; };

  int push(int port, TuplePtr tp, b_cbv cb);	// Incoming, either add to send_q or ack
  TuplePtr pull(int port, b_cbv cb);		// Rate limited output tuple stream

private:
  void timeout_cb(OTuple *otp);				// Callback for to retry sending a tuple
  REMOVABLE_INLINE int32_t dealloc(SeqNum seq);		// Remove OTuple from map
  REMOVABLE_INLINE void map(SeqNum seq, OTuple *otp);	// Map tuple and set timeout
  REMOVABLE_INLINE void add_rtt_meas(int32_t m);	// Update sa, sv, and rto based on m
  REMOVABLE_INLINE void timeout();			// Update sa, sv, and rto based on m
  REMOVABLE_INLINE int  current_window();		// Returns the current window size
  REMOVABLE_INLINE int  max_window();			// Returns the current window size

  b_cbv _din_cb; 					// Callback for data input ready
  b_cbv _dout_cb; 				// Callback for data output ready
    
  int32_t sa_;					// Scaled avg. RTT (ms) scaled by 8
  int32_t sv_;					// Scaled variance RTT (ms) scaled by 4
  int32_t rto_;					// The round-trip timeout

  double    max_wnd_;				// Max window size
  double    rwnd_;				// Receiver window size
  double    cwnd_;				// Current congestion window size
  double    ssthresh_;				// Slow start threshold
  uint32_t  max_retry_;				// Max number of retries for a tuple
  uint32_t  seq_field_;
  uint32_t  ack_seq_field_;
  uint32_t  ack_rwnd_field_;

  std::deque <TuplePtr> send_q_; 		// Primary queue containing tuples 
						// not yet sent
  std::deque <OTuple*>  rtran_q_;		// Retransmit queue 

  typedef std::map<SeqNum, OTuple*> OTupleIndex;
  OTupleIndex ot_map_;			// Map containing unacked in transit tuples
};
  
#endif /* __CC_H_ */
