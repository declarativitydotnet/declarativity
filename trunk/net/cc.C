// -*- c-basic-offset: 2; related-file-name: "element.C" -*-
/*
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

// #include <sys/time.h>
#include <iostream>

#include "sysconf.h"
#include "cc.h"
#include "val_uint64.h"
#include "val_uint32.h"
#include "val_double.h"
#include "val_str.h"


/////////////////////////////////////////////////////////////////////
//
// OTuple: Private Class 
//
class OTuple
{
public:
  OTuple() : tcb_(NULL), wnd_(true), tran_cnt_(0), tp_(NULL) {}

  OTuple(TuplePtr tp) : tcb_(NULL), wnd_(true), tran_cnt_(0), tp_(tp) 
    { resetTime(); }

  ~OTuple() { tp_ = NULL; }

  void operator()(std::pair<const SeqNum, OTuple*>& entry); 
  void resetTime() { clock_gettime (CLOCK_REALTIME, &tt_); }

  timespec  tt_;		// Transmit time
  timecb_t  *tcb_;		// Used to cancel retransmit timer
  bool      wnd_;		// If true then window updated on timeout.
  uint32_t  tran_cnt_;		// Transmit counter.
  TuplePtr  tp_;		// The tuple.
};

void OTuple::operator()(std::pair<const SeqNum, OTuple*>& entry) 
{
  (entry.second)->wnd_ = false;
}

/////////////////////////////////////////////////////////////////////
//
// Macros
//

#define MAX_RTO (5000)
#define MIN_RTO (500)

/////////////////////////////////////////////////////////////////////
//
// Helper Functions
//

int32_t delay(timespec *ts)
{
  timespec  now;
  clock_gettime(CLOCK_REALTIME, &now);

  if (now.tv_nsec < ts->tv_nsec) { 
    now.tv_nsec += 1000000000;
    now.tv_sec--; 
  } 

  return (((now.tv_sec - ts->tv_sec)*1000) + 
          ((now.tv_nsec - ts->tv_nsec)/1000000));	// Delay in milliseconds
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

  if (!ack_q_.empty()) {
    TuplePtr ack = ack_q_[0];
    ack_q_.erase(ack_q_.begin());
    return ack;
  }
  _ack_cb = cb;
  return NULL;
}

int CCRx::push(int port, TupleRef tp, cbv cb)
{
  if (port == 1) {
    try {
      if (Val_Str::cast((*tp)[0]) == "RWND")
        rwnd_ = Val_Double::cast((*tp)[1]);
    }
    catch (Value::TypeError *e) {
      log(LoggerI::WARN, 0, "CCRx::push TypeError Thrown on port 1"); 
    } 
    return int(rwnd_);
  }
  return this->Element::push(port, tp, cb);
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
CCTx::CCTx(str name, double init_wnd, double max_wnd, uint32_t max_retry, 
           uint32_t seq_field, uint32_t ack_seq_field, uint32_t ack_rwnd_field) 
  : Element(name, 2, 1), _din_cb(cbv_null), _dout_cb(cbv_null), sa_(-1), sv_(0)
{
  rto_            = MAX_RTO;
  max_wnd_        = max_wnd;
  cwnd_           = init_wnd;
  ssthresh_       = max_wnd;
  rwnd_           = max_wnd;
  max_retry_      = max_retry;
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
      _din_cb = cb;
      retval = 0;
    }
    break;
  case 1:
    // Acknowledge tuple and update measurements.
    SeqNum seq  = Val_UInt64::cast((*tp)[ack_seq_field_]);	// Sequence number
    //TODO: Use timestamps to track the latest rwnd value.
    rwnd_ = Val_Double::cast((*tp)[ack_rwnd_field_]);		// Receiver window

    OTupleIndex::iterator iter = ot_map_.find(seq);
    if (iter != ot_map_.end()) { 
      int32_t d = delay(&(iter->second)->tt_);

      timecb_remove((iter->second)->tcb_);
      add_rtt_meas(d);

      delete iter->second;
      ot_map_.erase(iter);
      if (current_window() < max_window() && _din_cb != cbv_null) {
        (*_din_cb)();
        _din_cb = cbv_null;
      }
    }
    else {
      // Log event: possibly due to duplicate ack.
      log(LoggerI::WARN, 0, "CCTx::push receive unknown ack, possible duplicate"); 
    }
    break;
  }

  if (!send_q_.empty() && _dout_cb != cbv_null) {
    (*_dout_cb)();
    _dout_cb = cbv_null;
  }
  return retval;
}

/**
 * Handles 2 output ports
 * port 0: Return the next tuple from either the retran or send queue
 * port 1: Return a tuple containing the internal CC state
 */
TuplePtr CCTx::pull(int port, cbv cb)
{
  if (port == 0) {
    assert (rto_ >= MIN_RTO && rto_ <= MAX_RTO);
  
    // All retransmit packets go first.
    if (!rtran_q_.empty()) {
      OTuple *otp = rtran_q_[0];
      rtran_q_.erase(rtran_q_.begin());
      otp->tcb_ = delaycb(rto_/1000, (rto_ % 1000)*1000000, wrap(this, &CCTx::timeout_cb, otp)); 
      otp->tran_cnt_++;
  
      return otp->tp_;
    }
    else if (!send_q_.empty()) {
      TuplePtr  tp    = send_q_[0];
      SeqNum    seq   = Val_UInt64::cast((*tp)[seq_field_]);	// Sequence number
  
      send_q_.erase(send_q_.begin());
  
      OTuple *otp = new OTuple(tp);
      otp->tcb_ = delaycb(rto_/1000, (rto_ % 1000)*1000000, wrap(this, &CCTx::timeout_cb, otp)); 
      otp->tran_cnt_++;
      ot_map_.insert(std::make_pair(seq, otp));
  
      return tp;
    }
    _dout_cb = cb;
  }
  else if (port == 1) {
    // Package up a Tuple containing my current CC state
    TuplePtr state = Tuple::mk();
    state->append(Val_Double::mk(rwnd_));		// Receiver window size (flow control)
    state->append(Val_Double::mk(cwnd_));		// Current congestion window size
    state->append(Val_Double::mk(ssthresh_));		// Slow start threshold
    state->append(Val_UInt32::mk(rto_));		// Round trip time estimate
    state->append(Val_UInt32::mk(current_window()));	// Current window size (# unacked tuples)

    return state;
  } else assert(0);

  return NULL;

}

/**
 * This function performs the following actions.
 * 1. Update window size
 * 2. Retransmit the packet
 */
void CCTx::timeout_cb(OTuple *otp)
{
  SeqNum seq = Val_UInt64::cast((*otp->tp_)[seq_field_]);

  otp->tcb_ = NULL;
  if (otp->wnd_ == true) {
    // Update window sizes and enter slow start
    timeout(); 		
    // Set all current OTuple::wnd_ = false.
    std::for_each(ot_map_.begin(), ot_map_.end(), OTuple());
  }
  else if (otp->tran_cnt_ > max_retry_) {
    log(LoggerI::WARN, 0, 
        strbuf()<<"MAX NUMBER OF RETRIES REACHED FOR TUPLE seq("<<seq<<")");
    OTupleIndex::iterator iter = ot_map_.find(seq);
    assert(iter != ot_map_.end());
    delete iter->second;
    ot_map_.erase(iter);
    if (current_window() < max_window() && _din_cb != cbv_null) {
      (*_din_cb)();
      _din_cb = cbv_null;
    }
    return;
  }

  rtran_q_.push_back(otp);	// Add to retransmit queue
  if (_dout_cb != cbv_null) {
    (*_dout_cb)();
    _dout_cb = cbv_null;
  }

  log(LoggerI::INFO, 0, 
      strbuf()<< "TIMEOUT: Packet seq(" << seq << ") timeout after " 
              << (delay(&otp->tt_)) << "ms");
}

/**
 *
 */
REMOVABLE_INLINE void CCTx::add_rtt_meas(int32_t m)
{
  assert(m >= 0);

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

  // Don't backoff past 5 seconds.
  if (rto_ > MAX_RTO)      rto_ = MAX_RTO;
  else if (rto_ < MIN_RTO) rto_ = MIN_RTO;

  if (cwnd_ < ssthresh_) 
    cwnd_ *= 2.0; 		// slow start
  else		 
    cwnd_ += 1.0 / cwnd_; 	// additive increase 

  if (cwnd_ > max_wnd_)
    cwnd_ = max_wnd_;

  log(LoggerI::INFO, 0, strbuf() << "CURRENT WINDOW: " << current_window()
			<< "\n\tCWND(" << long(cwnd_) << ") MAX_WND("
			<< long(max_wnd_) << ") SSTHRESH(" << long(ssthresh_)
			<< ")\n");
}


REMOVABLE_INLINE void CCTx::timeout() 
{
  rto_ <<= 1;

  if (rto_ > MAX_RTO)      rto_ = MAX_RTO;
  else if (rto_ < MIN_RTO) rto_ = MIN_RTO;

  ssthresh_ = cwnd_ / 2.0;	// multiplicative decrease
  cwnd_     = 1.0;		// Enter slow start

  log(LoggerI::INFO, 0, strbuf() << "CONGESTION ADJUST: RTO = " << long(rto_)
			<< " | SSTHRESH = " << long(ssthresh_) 
			<< " | CWND = " << long(cwnd_));
}

REMOVABLE_INLINE int CCTx::current_window() {
  return send_q_.size() + rtran_q_.size() + ot_map_.size();
}

REMOVABLE_INLINE int CCTx::max_window() {
  return (cwnd_ < rwnd_) ? int(cwnd_) : int(rwnd_);
}
