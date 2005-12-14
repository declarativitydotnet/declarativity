/*
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include <algorithm>
#include "cctx.h"
#include "sysconf.h"
#include "val_uint64.h"
#include "val_uint32.h"
#include "val_double.h"
#include "val_str.h"


/////////////////////////////////////////////////////////////////////
//
// CCTuple: Private Class 
//
class CCTuple
{
public:
  CCTuple() : tcb_(NULL), wnd_(true), tp_(NULL) {}

  CCTuple(TuplePtr tp) : tcb_(NULL), wnd_(true), tp_(tp) 
    { resetTime(); }

  ~CCTuple() { tp_ = NULL; }

  void operator()(std::pair<const SeqNum, CCTuple*>& entry); 
  void resetTime() { clock_gettime (CLOCK_REALTIME, &tt_); }
  int32_t delay();

  timespec  tt_;		// Transmit time
  timecb_t  *tcb_;		// Used to cancel retransmit timer
  bool      wnd_;		// If true then window updated on timeout.
  TuplePtr  tp_;		// The tuple.
};

void CCTuple::operator()(std::pair<const SeqNum, CCTuple*>& entry) 
{
  (entry.second)->wnd_ = false;
}

int32_t CCTuple::delay()
{
  timespec  now;
  clock_gettime(CLOCK_REALTIME, &now);

  if (now.tv_nsec < tt_.tv_nsec) { 
    now.tv_nsec += 1000000000;
    now.tv_sec--; 
  } 

  return (((now.tv_sec - tt_.tv_sec)*1000) + 
          ((now.tv_nsec - tt_.tv_nsec)/1000000));	// Delay in milliseconds
}

/////////////////////////////////////////////////////////////////////
//
// Globals
//

#define MAX_RTO (5000)
#define MIN_RTO (500)

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
           uint32_t seq_field, uint32_t ack_seq_field, uint32_t ack_rwnd_field) 
  : Element(name, 2, 1), _dout_cb(b_cbv_null), data_on_(true), sa_(-1), sv_(0)
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
int CCTx::push(int port, TupleRef tp, b_cbv cb)
{
  assert(port < 2);

  switch(port) {
    case 0:	// Queue tuple and check window size.
    {
        CCTuple *otp = new CCTuple(tp);
        SeqNum   seq = Val_UInt64::cast((*otp->tp_)[seq_field_]);	// Sequence number
  
        map(seq, otp);

        if ((data_on_ = output(0)->push(otp->tp_, wrap(this, &CCTx::data_ready)))) {
          _dout_cb = cb;
          return 0; 
        }
        else if (current_window() >= max_window()) {
          log(LoggerI::INFO, 0, "CCTx::push WINDOW IS FULL"); 
          _dout_cb = cb;
          return 0;
        }
      break;
    }
    case 1:
    {
      // Acknowledge tuple and update measurements.
      SeqNum seq  = Val_UInt64::cast((*tp)[ack_seq_field_]);	// Sequence number
      //TODO: Use timestamps to track the latest rwnd value.
      rwnd_ = Val_Double::cast((*tp)[ack_rwnd_field_]);		// Receiver window

      add_rtt_meas(dealloc(seq, "ACK"));
      break;
    }
    default: assert(0);
  }
  return 1;
}

/**
 * Handles 2 output ports
 * port 0: Return the next tuple from either the retran or send queue
 * port 2: Return a tuple containing the internal CC state
 */
TuplePtr CCTx::pull(int port, b_cbv cb)
{
  TuplePtr tp = NULL;

  switch (port) {
    case 0:
      assert (rto_ >= MIN_RTO && rto_ <= MAX_RTO);
      if (current_window() < max_window() && 
          (data_on_ = (tp = input(0)->pull(wrap(this, &CCTx::data_ready))) != NULL)) {
        CCTuple *otp = new CCTuple(tp);
        SeqNum   seq = Val_UInt64::cast((*otp->tp_)[seq_field_]);	// Sequence number
        map(seq, otp);
      } else _dout_cb = cb;
      break;
    case 2:
      // Package up a Tuple containing my current CC state
      tp = Tuple::mk();
      tp->append(Val_Double::mk(rwnd_));		// Receiver window size (flow control)
      tp->append(Val_Double::mk(cwnd_));		// Current congestion window size
      tp->append(Val_Double::mk(ssthresh_));		// Slow start threshold
      tp->append(Val_UInt32::mk(rto_));			// Round trip time estimate
      tp->append(Val_UInt32::mk(current_window()));	// Current window size (# unacked tuples)
      break;
    default: assert(0);
  } 
  return tp;
}

void CCTx::map(SeqNum seq, CCTuple *otp) 
{
  tmap_.insert(std::make_pair(seq, otp));
  otp->tcb_ = delaycb(rto_/1000, (rto_ % 1000)*1000000, wrap(this, &CCTx::timeout_cb, otp)); 
}

int32_t CCTx::dealloc(SeqNum seq, str status)
{
  int32_t d = -1;
  CCTupleIndex::iterator iter = tmap_.find(seq);
  if (iter != tmap_.end()) { 
    log(LoggerI::INFO, 0, strbuf()<<"DEALLOCATING seq("<<seq<<")"); 
    d = iter->second->delay();

    if (iter->second->tcb_ != NULL) timecb_remove((iter->second)->tcb_);

    delete iter->second;
    tmap_.erase(iter);
    if (data_on_ && _dout_cb != b_cbv_null) {
      (*_dout_cb)();
      _dout_cb = b_cbv_null;
    }

    TuplePtr tp = Tuple::mk();
    tp->append(Val_Str::mk(status));		// Signal drop
    tp->append(Val_UInt64::mk(seq));		// Sequence number 
    assert(output(1)->push(tp, b_cbv_null));
  }
  else {
    // Log event: possibly due to duplicate ack.
    log(LoggerI::INFO, 0, "CCTx::push receive unknown ack, possible duplicate"); 
  }
  return d;
}

void CCTx::data_ready() {
  data_on_ = true;
  if (_dout_cb != b_cbv_null) {
    (*_dout_cb)();
    _dout_cb = b_cbv_null;
  }
}

/**
 * This function performs the following actions.
 * 1. Update window size
 * 2. Retransmit the packet
 */
void CCTx::timeout_cb(CCTuple *otp)
{
  SeqNum seq = Val_UInt64::cast((*otp->tp_)[seq_field_]);

  if (otp->wnd_ == true) {
    // Update window sizes and enter slow start
    timeout(); 		
    for_each(tmap_.begin(), tmap_.end(), CCTuple());
  }

  otp->tcb_ = NULL;
  dealloc(seq, "FAIL");

  log(LoggerI::INFO, 0, 
      strbuf()<< "TIMEOUT: Packet seq(" << seq << ") timeout after " 
              << otp->delay() << "ms");
}

/**
 *
 */
REMOVABLE_INLINE void CCTx::add_rtt_meas(int32_t m)
{
  if (m < 0) return;

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
    cwnd_ *= 2.0; 			// slow start
  else		 
    cwnd_ += 1.0; 			// additive increase 

  if (cwnd_ > max_wnd_)
    cwnd_ = max_wnd_;

  log(LoggerI::INFO, 0, strbuf() << "CURRENT WINDOW: " << current_window()
			<< "\tCWND(" << long(cwnd_) << ") MAX_WND("
			<< long(max_wnd_) << ") SSTHRESH(" << long(ssthresh_)
			<< ")\n");
}


REMOVABLE_INLINE void CCTx::timeout() 
{
  rto_ <<= 1;

  if (rto_ > MAX_RTO)      rto_ = MAX_RTO;
  else if (rto_ < MIN_RTO) rto_ = MIN_RTO;

  ssthresh_ = cwnd_ / 2.0;	// multiplicative decrease
  cwnd_     = 2.0;		// Enter slow start
  if (ssthresh_ < 2.0) ssthresh_ = 2.0;

  log(LoggerI::INFO, 0, strbuf() << "CONGESTION ADJUST: RTO = " << long(rto_)
			<< " | SSTHRESH = " << long(ssthresh_) 
			<< " | CWND = " << long(cwnd_));
}

REMOVABLE_INLINE int CCTx::current_window() {
  return tmap_.size();
}

REMOVABLE_INLINE int CCTx::max_window() {
  return (cwnd_ < rwnd_) ? int(cwnd_) : int(rwnd_);
}
