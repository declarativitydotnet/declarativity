// -*- c-basic-offset: 2; related-file-name: "element.C" -*-
/*
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#ifndef __CC_H__
#define __CC_H__

#include <map>
#include <async.h>
#include "tuple.h"
#include "element.h"
#include "inlines.h"


class otuple;
typedef ref<otuple> otuple_ref;
typedef ptr<otuple> otuple_ptr;
typedef uint64_t    SeqNum; 

class otuple
{
 public:
  otuple() : tt_ms_(0), tcb_(NULL), wnd_(true), tp_(NULL) {}

  otuple(u_int64_t tt_ms, TuplePtr tp)
    : tt_ms_(tt_ms), tcb_(NULL), wnd_(true), tp_(tp) {}


  void operator()(std::pair<const SeqNum, otuple_ref>& entry); 

  u_int64_t tt_ms_;	// Transmit time in milliseconds
  timecb_t  *tcb_;	// Used to cancel retransmit timer
  bool      wnd_;	// If true then window updated on timeout.
  TuplePtr  tp_;	// The tuple.
};

class CC { 
public:

  /**
   * The Rx element acknowledges incoming cc tuples on output1 
   * as they are passed from input0 to output0. The element does 
   * not care about the tuple order.
   */
  class Rx : public Element {
  public:
    Rx(const CC&, double, double);
    const char *class_name() const		{ return "CC::Rx";};
    const char *processing() const		{ return "hh/l"; };
    const char *flow_code() const		{ return "/-"; };

    TuplePtr simple_tuple(TupleRef p);		// Ack on output1 before passing to output0.

    TuplePtr pull(int port, cbv cb);		// Pull next acknowledgement from ack_q

    void set_rwnd(double wnd) { rwnd_ = wnd; }	// Set receive window

    double get_rwnd() const { return rwnd_; }	// Get receive window

  private:
    cbv _ack_cb; 				// Callback to send an ack 

    const CC *cc_;

    double max_wnd_;				// Max window size
    double rwnd_;				// Receiver window size

    std::vector <TuplePtr> ack_q_;		// Output ack queue
  };
  
  /**
   * The Tx element pulls tuples from input0, wraps them in a
   * control structure, and then pushes it to the output. It
   * receives acks on input1, which are used to move the sliding
   * window forward. Unacknowledged packets are retransmitted after
   * begin in transit for a time of 4*RTT. This element implements
   * slow-start and AIMD as its congestion control protcol. 
   */
  class Tx : public Element {
  public:
    Tx(const CC&, double, double);
    const char *class_name() const		{ return "CC::Tx";};
    const char *processing() const		{ return "hh/l"; };
    const char *flow_code() const		{ return "-/"; };

    int push(int port, TupleRef tp, cbv cb);	// Incoming, either add to send_q or ack
    TuplePtr pull(int port, cbv cb);		// Rate limited output tuple stream

  private:
    cbv _element_cb; 				// Callback for element push
    cbv _send_cb; 				// Callback for send. Pulls from send_q.

    void timeout_cb(otuple_ref otr);		// Callback for to retry sending a tuple
    REMOVABLE_INLINE void add_rtt_meas(long m);	// Update sa, sv, and rto based on m
    REMOVABLE_INLINE void timeout();		// Update sa, sv, and rto based on m
    REMOVABLE_INLINE int current_window();	// Returns the current window size
    REMOVABLE_INLINE int max_window();		// Returns the current window size

    
    const CC *cc_;

    long sa_;					// Scaled avg. RTT (ms) scaled by 8
    long sv_;					// Scaled variance RTT (ms) scaled by 4
    long rto_;					// The round-trip timeout

    double max_wnd_;				// Max window size
    double rwnd_;				// Receiver window size
    double cwnd_;				// Current congestion window size
    double ssthresh_;				// Slow start threshold

    std::vector <TuplePtr> send_q_; 		// Primary queue containing tuples not yet sent
    std::vector <otuple_ref> rtran_q_;		// Retransmit queue 

    typedef std::map<SeqNum, otuple_ref> OTupleIndex;
    OTupleIndex ot_map_;			// Map containing unacked in transit tuples
  };
  
  //
  // Now the CC object itself.
  //
  CC(double init_wnd, double max_wnd, int seq_field=0);

  // Get sequence number field
  int get_seq_field() const { return seq_field_; }

  // Accessing the individual elements
  ref< CC::Rx > get_rx() { return rx_; };
  ref< CC::Tx > get_tx() { return tx_; };

private:
  int seq_field_;

  // Elements 
  ref< Rx > rx_;
  ref< Tx > tx_;
};

#endif /* __CC_H_ */
