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
#include "val_str.h"

/////////////////////////////////////////////////////////////////////
//
// Macros
//

#define MAX_RTO        (5*1000)
#define MIN_RTO        (500)

void otuple::operator()(std::pair<const SeqNum, otuple_ref>& entry) 
{
  (entry.second)->wnd_ = false;
}

uint64_t now_ms()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);

  return uint64_t (tv.tv_usec / 1000);	// Time in milliseconds
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
CCRx::CCRx(str name, double max_wnd, int seq, int src) 
  : Element(name, 1, 2), _ack_cb(cbv_null), max_wnd_(max_wnd), 
    rwnd_(max_wnd), seq_field_(seq), src_field_(src)
{
}

/**
 * Acknowledge tuple p if ack_q is empty and output1 is open.
 * Otherwise, add to ack_q and enable callback.
 */
TuplePtr CCRx::simple_action(TupleRef p) 
{
  log(LoggerI::INFO, 0, "CCRx::simple_action"); 

  /* Get source location, sequence number and ack tuple, signal my window also */
  SeqNum seq = Val_UInt64::cast((*p)[seq_field_]);
  str    src = Val_Str::cast((*p)[src_field_]);
  TupleRef ack = Tuple::mk();
  ack->append(Val_Str::mk(src));
  ack->append(Val_UInt64::mk(seq));
  ack->append(Val_Double::mk(get_rwnd()));
  ack->freeze();

  ack_q_.push_back(ack);

  if (_ack_cb != cbv_null) {
    log(LoggerI::INFO, 0, "CC::Rx::simple_action notify ack ready to send"); 
    (*_ack_cb)();		// Notify new ack
    _ack_cb = cbv_null;
  } 
  return p;			// forward tuple along
}

/**
 * Pulls the next acknowledgement in ack_q_ to send to the
 * receiver.
 */
TuplePtr CCRx::pull(int port, cbv cb)
{
  assert(port == 1);

  log(LoggerI::INFO, 0, "CC::Rx::pull check ack q"); 
  if (!ack_q_.empty()) {
    log(LoggerI::INFO, 0, "CC::Rx::pull has ack"); 
    TuplePtr ack = ack_q_[0];
    ack_q_.erase(ack_q_.begin());
    return ack;
  }
  log(LoggerI::INFO, 0, "CC::Rx::pull no ack, store callback"); 
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
CCTx::CCTx(str name, double init_wnd, double max_wnd, 
           int seq_field, int ack_seq_field, int ack_rwnd_field) 
  : Element(name, 2, 1), _input_cb(cbv_null), _output_cb(cbv_null), sa_(-1), sv_(0)
{
  rto_            = MAX_RTO;
  max_wnd_        = max_wnd;
  cwnd_           = init_wnd;
  ssthresh_       = max_wnd;
  rwnd_           = max_wnd;
  seq_field_      = seq_field;
  ack_seq_field_  = ack_seq_field;
  ack_rwnd_field_ = ack_rwnd_field;
}

/**
 * The push method handles input on 2 ports.
 * port 0: Indicates a tuple to send.
 * port 1: Indicates the acknowledgement of some outstanding tuple.
 */
int CCTx::push(int port, TupleRef tp, cbv cb)
{
  int retval = 1;
  assert(port < 2);

  switch(port) {
  case 0:	// Queue tuple and check window size.
    send_q_.push_back(tp);

    if (current_window() >= max_window()) {
      log(LoggerI::INFO, 0, "CCTx::push WINDOW IS FULL"); 
      _input_cb = cb;
      retval = 0;
    }
    break;
  case 1:
    // Acknowledge tuple and update measurements.
    SeqNum seq  = Val_UInt64::cast((*tp)[ack_seq_field_]);	// Sequence number
    //TODO: Use timestamps to track the latest rwnd value.
    rwnd_ = Val_Double::cast((*tp)[ack_rwnd_field_]);		// Receiver window

    log(LoggerI::INFO, 0, "CCTx::push receive acknowledgement"); 
    OTupleIndex::iterator iter = ot_map_.find(seq);
    if (iter != ot_map_.end()) { 
      long delay = now_ms() - (iter->second)->tt_ms_;

      if ((iter->second)->tcb_ != NULL) {
        timecb_remove((iter->second)->tcb_);
        (iter->second)->tcb_ = NULL;
      }
      if (delay > 0) 
        add_rtt_meas(delay);

      ot_map_.erase(iter);
      if (current_window() < max_window() && _input_cb != cbv_null) {
        (*_input_cb)();
        _input_cb = cbv_null;
      }
    }
    else {
      // Log event: possibly due to duplicate ack.
      log(LoggerI::WARN, 0, "CCTx::push receive unknown ack, possible duplicate"); 
    }
    log(LoggerI::INFO, 0, "CCTx::push ack received."); 

    break;
  }

  if (!send_q_.empty() && _output_cb != cbv_null) {
    (*_output_cb)();
    _output_cb = cbv_null;
  }
  return retval;
}

/**
 * Return the next tuple from send_q_, prepending a timestamp.
 * If no tuples exist then store callback and return NULL
 */
TuplePtr CCTx::pull(int port, cbv cb)
{
  assert(port == 0);

  // All retransmit packets go first.
  if (!rtran_q_.empty()) {
    otuple_ref otr = rtran_q_[0];
    rtran_q_.erase(rtran_q_.begin());
    otr->tcb_ = delaycb(rto_ / 1000, rto_ * 1000000, wrap(this, &CCTx::timeout_cb, otr)); 

    return otr->tp_;
  }
  else if (!send_q_.empty()) {
    TuplePtr  tp    = send_q_[0];
    SeqNum    seq   = Val_UInt64::cast((*tp)[seq_field_]);	// Sequence number
    uint64_t  tt_ms = now_ms();

    send_q_.erase(send_q_.begin());

    otuple_ref otr = New refcounted<otuple>(tt_ms, tp);
    otr->tcb_ = delaycb(rto_ / 1000, rto_ * 1000000, wrap(this, &CCTx::timeout_cb, otr)); 
    ot_map_.insert(std::make_pair(seq, otr));

    return tp;
  }

  _output_cb = cb;
  return NULL;

}

/**
 * This function performs the following actions.
 * 1. Update window size
 * 2. Retransmit the packet
 */
void CCTx::timeout_cb(otuple_ref otr)
{
  SeqNum seq = Val_UInt64::cast((*otr->tp_)[seq_field_]);	// Sequence number
  log(LoggerI::INFO, 0, strbuf() << "TIMEOUT: Packet seq(" << seq << ") timeout after " 
			<< (now_ms() - otr->tt_ms_) << "ms");

  otr->tcb_ = NULL;
  if (otr->wnd_ == true) {
    log(LoggerI::INFO, 0, "Adjusting window size after timeout."); 

    timeout(); 							// Update window sizes and enter slow start
    std::for_each(ot_map_.begin(), ot_map_.end(), otuple()); 	// Set all current otuple::wnd_ = false.
  }

  rtran_q_.push_back(otr);				 	// Add to retransmit queue
  if (_output_cb != cbv_null) {
    (*_output_cb)();
    _output_cb = cbv_null;
  }

  log(LoggerI::INFO, 0, "DONE TIMEOUT."); 
}

/**
 *
 */
REMOVABLE_INLINE void CCTx::add_rtt_meas(long m)
{
  long orig_m = m;
  assert(m > 0);

  if (sa_ == -1) {
    // After the first measurement, set the timeout to four
    // times the RTT.

    sa_ = m << 3;
    sv_ = 0;
    rto_ = (m << 2) + 10; 		// the 10 is to accont for GC
  }
  else {
    m -= (sa_ >> 3);
    sa_ += m;
    if (m < 0)
      m = -1*m;
    m -= (sv_ >> 2);
    sv_ += m;
    rto_ = (sa_ >> 3) + sv_ + 10; 	// the 10 is to accont for GC
  }

  // Don't backoff past 1 second.
  if (rto_ > MAX_RTO) {
    log(LoggerI::WARN, 1, strbuf() << "HUGE RTO: m=" << long(orig_m)); 
    rto_ = MAX_RTO;
  }

  if (cwnd_ < ssthresh_) 
    cwnd_ *= 2.0; 		// slow start
  else		 
    cwnd_ += 1.0 / cwnd_; 	// additive increase 

  if (cwnd_ > max_wnd_)
    cwnd_ = max_wnd_;

  log(LoggerI::INFO, 0, strbuf() << "CWND(" << long(cwnd_) << ") MAX_WND(" << long(max_wnd_) 
			<< ") SSTHRESH(" << long(ssthresh_) << ")\n");
}


REMOVABLE_INLINE void CCTx::timeout() 
{
  rto_ <<= 1;

  // Don't backoff past 1 second.
  if (rto_ > MAX_RTO)      rto_ = MAX_RTO;
  else if (rto_ < MIN_RTO) rto_ = MIN_RTO;

  ssthresh_ = cwnd_ / 2.0;	// multiplicative decrease
  cwnd_     = 1.0;

  log(LoggerI::INFO, 0, strbuf() << "CONGESTION ADJUST: RTO = " << long(rto_)
			<< " | SSTHRESH = " << long(ssthresh_) << " | CWND = " << long(cwnd_));
}

REMOVABLE_INLINE int CCTx::current_window() {
  return send_q_.size() + ot_map_.size();
}

REMOVABLE_INLINE int CCTx::max_window() {
  return (cwnd_ < rwnd_) ? int(cwnd_) : int(rwnd_);
}
