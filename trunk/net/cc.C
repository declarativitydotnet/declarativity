// -*- c-basic-offset: 2; related-file-name: "element.C" -*-
/*
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include <sys/time.h>
#include <iostream>

#include "cc.h"
#include "val_uint64.h"
#include "val_uint32.h"
#include "val_double.h"

/////////////////////////////////////////////////////////////////////
//
// Macros
//

#define MAX_RTO        (5*1000)
#define ACK_SEQ_FIELD  0
#define ACK_RWND_FIELD 1

void otuple::operator()(std::pair<const SeqNum, otuple_ref>& entry) 
{
  (entry.second)->wnd_ = false;
}


/////////////////////////////////////////////////////////////////////
//
// Receive element
//

/**
 * Constructor: 
 * Input  0 (push): Stream of tuples. 
 * Output 0 (push): Stream of tuples (possibly out of order and with dups).	
 * Output 1 (pull): Acknowledgements of individual tuples.
 */
CC::Rx::Rx(const CC &cc, double init_wnd, double max_wnd) 
  : Element("CC::Rx", 1, 2), _ack_cb(cbv_null), cc_(&cc), rwnd_(init_wnd), max_wnd_(max_wnd)
{
}

/**
 * Acknowledge tuple p if ack_q is empty and output1 is open.
 * Otherwise, add to ack_q and enable callback.
 */
TuplePtr CC::Rx::simple_tuple(TupleRef p) 
{
  /* Get sequence number and ack tuple */
  SeqNum seq = Val_UInt64::cast((*p)[cc_->get_seq_field()]);
  TupleRef   ack = Tuple::mk();
  ack->append(Val_UInt64::mk(seq));
  ack->append(Val_Double::mk(get_rwnd()));
  ack->freeze();

  ack_q_.push_back(ack);

  if (_ack_cb != cbv_null) {
    (*_ack_cb)();		// Notify new ack
    _ack_cb = cbv_null;
  }

  return p;			// forward tuple along
}

/**
 * Pulls the next acknowledgement in ack_q_ to send to the
 * receiver.
 */
TuplePtr CC::Rx::pull(int port, cbv cb)
{
  assert(port == 1);

  if (!ack_q_.empty()) {
    TuplePtr ack = ack_q_[0];
    ack_q_.erase(ack_q_.begin());
    return ack;
  }
  _ack_cb = cb;
  return NULL;
}

/////////////////////////////////////////////////////////////////////
//
// Transmit element
//

/**
 * Constructor: 
 * Input  0 (push): Tuple to send. 
 * Input  1 (push): Acknowledgement of some (possibly) outstanding tuple.
 * Output 0 (pull): Tuple to send with cc info wrapper.
 */
CC::Tx::Tx(const CC &cc, double init_wnd, double max_wnd) 
  : Element("CC::Tx", 2, 1), cc_(&cc), _element_cb(cbv_null), _send_cb(cbv_null) 
{
  sa_       = -1;
  sv_       = 0;
  rto_      = MAX_RTO;
  max_wnd_  = max_wnd;
  cwnd_     = max_wnd;
  ssthresh_ = max_wnd;
  rwnd_     = init_wnd;
}

/**
 * The push method handles input on 2 ports.
 * port 0: Indicates a tuple to send.
 * port 1: Indicates the acknowledgement of some outstanding tuple.
 */
int CC::Tx::push(int port, TupleRef tp, cbv cb)
{
  assert(port < 2);

  switch(port) {
  case 0:	// Queue tuple and check window size.
    send_q_.push_back(tp);
    if (current_window() >= max_window()) {
      _send_cb = cb;
      return 0;
    }
    break;
  case 1:	// Acknowledge tuple and update measurements.
    SeqNum seq  = Val_UInt64::cast((*tp)[ACK_SEQ_FIELD]);	// Sequence number
    double rwnd = Val_Double::cast((*tp)[ACK_RWND_FIELD]);	// Receiver window

    OTupleIndex::iterator iter = ot_map_.find(seq);
    if (iter != ot_map_.end()) { 
      timecb_remove((iter->second)->tcb_);
      add_rtt_meas((iter->second)->tt_ms_);
      ot_map_.erase(iter);
    }
    else {
      // Log event: possibly due to duplicate ack.
    }

    break;
  }
  return 1;
}

/**
 * Return the next tuple from send_q_, prepending a timestamp.
 * If no tuples exist then store callback and return NULL
 */
TuplePtr CC::Tx::pull(int port, cbv cb)
{
  assert(port == 0);

  // All retransmit packets go first.
  if (!rtran_q_.empty()) {
    otuple_ref otr = rtran_q_[0];
    SeqNum     seq = Val_UInt64::cast((*otr->tp_)[cc_->get_seq_field()]);

    rtran_q_.erase(rtran_q_.begin());
    otr->tcb_ = delaycb(rto_ / 1000, rto_ * 1000000, wrap(this, &CC::Tx::timeout_cb, otr)); 

    return otr->tp_;
  }
  else if (!send_q_.empty()) {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    TuplePtr  tp    = send_q_[0];
    SeqNum    seq   = Val_UInt64::cast((*tp)[cc_->get_seq_field()]);	// Sequence number
    u_int64_t tt_ms = (tv.tv_sec / 1000) + (tv.tv_usec * 1000);		// Time in milliseconds

    send_q_.erase(send_q_.begin());

    otuple_ref otr = New refcounted<otuple>(tt_ms, tp);
    otr->tcb_ = delaycb(rto_ / 1000, rto_ * 1000000, wrap(this, &CC::Tx::timeout_cb, otr)); 
    ot_map_.insert(std::make_pair(seq, otr));

    return tp;
  }

  _send_cb = cb;
  return NULL;

}

/**
 * This function performs the following actions.
 * 1. Update window size
 * 2. Retransmit the packet
 */
void CC::Tx::timeout_cb(otuple_ref otr)
{
  SeqNum seq = Val_UInt64::cast((*otr->tp_)[cc_->get_seq_field()]);	// Sequence number

  ot_map_.erase(ot_map_.find(seq));				

  if (otr->wnd_ == true) {
    timeout(); 							// Update window sizes and enter slow start
    std::for_each(ot_map_.begin(), ot_map_.end(), otuple()); 	// Set all current otuple::wnd_ = false.
  }

  rtran_q_.push_back(otr);				 	// Add to retransmit queue
}

/**
 *
 */
REMOVABLE_INLINE void CC::Tx::add_rtt_meas(long m)
{
  long orig_m = m;

  if (sa_ == -1) {
    // After the first measurement, set the timeout to four
    // times the RTT.

    sa_ = m << 3;
    sv_ = 0;
    rto_ = (m << 2) + 10; // the 10 is to accont for GC
  }
  else {
    m -= (sa_ >> 3);
    sa_ += m;
    if (m < 0)
      m = -1*m;
    m -= (sv_ >> 2);
    sv_ += m;
    rto_ = (sa_ >> 3) + sv_ + 10; // the 10 is to accont for GC
  }

  // Don't backoff past 1 second.
  if (rto_ > MAX_RTO) {
    // log("huge rto: conn=" + 0 + " m=" + orig_m + " sa=" + sa_ + " sv=" + sv_ + " rto=" + rto_);

    rto_ = MAX_RTO;
  }

  if (cwnd_ < ssthresh_) // slow start
    cwnd_ += 1.0;
  else		 // increment by one
    cwnd_ += 1.0 / cwnd_;

  if (cwnd_ > max_wnd_)
    cwnd_ = max_wnd_;
}


REMOVABLE_INLINE void CC::Tx::timeout() {
  rto_ <<= 1;

  // Don't backoff past 1 second.
  if (rto_ > MAX_RTO) rto_ = MAX_RTO;

  ssthresh_ = cwnd_ / 2.0;
  cwnd_ = 1.0;
}

REMOVABLE_INLINE int CC::Tx::current_window() {
  return send_q_.size() + rtran_q_.size();
}

REMOVABLE_INLINE int CC::Tx::max_window() {
  return (cwnd_ < rwnd_) ? int(cwnd_) : int(rwnd_);
}

////////////////////////////////////////////////////////////////////
//
// The main object itself
//
CC::CC(double init_wnd, double max_wnd, int seq_field) 
  : rx_(New refcounted< CC::Rx >(*this, init_wnd, max_wnd)),
    tx_(New refcounted< CC::Tx >(*this, init_wnd, max_wnd)),
    seq_field_(seq_field)
{
}

