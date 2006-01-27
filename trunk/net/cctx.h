/*
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 */

#ifndef __CCTX_H__
#define __CCTX_H__

#include <map>
#include <deque>
#include "tuple.h"
#include "element.h"
#include "inlines.h"


class CCTuple;
typedef uint64_t SeqNum; 

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
  CCTx(string name, double init_wnd, double max_wnd,
       uint32_t seq_field = 0, uint32_t ack_seq_field=1, uint32_t ack_rwnd_field=2);
  const char *class_name() const { return "CC::Tx";};
  const char *processing() const { return "ah/ahl"; };
  const char *flow_code() const	 { return "--/--"; };

  int push(int port, TupleRef tp, b_cbv cb);	// Incoming, either add to send_q or ack
  TuplePtr pull(int port, b_cbv cb);		// Rate limited output tuple stream

private:
  void timeout_cb(CCTuple*);			// Callback for to retry sending a tuple
  void data_ready();				// Callback for input data ready
  REMOVABLE_INLINE int32_t dealloc(SeqNum,string);	// Remove CCTuple from map
  REMOVABLE_INLINE void map(SeqNum, CCTuple*);	// Map tuple and set timeout
  REMOVABLE_INLINE void add_rtt_meas(int32_t);	// Update sa, sv, and rto based on m
  REMOVABLE_INLINE void timeout();		// Update sa, sv, and rto based on m
  REMOVABLE_INLINE int  current_window();	// Returns the current window size
  REMOVABLE_INLINE int  max_window();		// Returns the current window size

  b_cbv _dout_cb; 				// Callback for data output ready
    
  bool    data_on_;
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

  typedef std::map<SeqNum, CCTuple*> CCTupleIndex;
  CCTupleIndex tmap_;			// Map containing unacked in transit tuples
};
  
#endif /* __CCTX_H_ */
