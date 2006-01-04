/*
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include <algorithm>
#include <iostream>
#include "cct.h"
#include "val_uint64.h"
#include "val_uint32.h"
#include "val_double.h"
#include "val_str.h"
#include "val_tuple.h"


/////////////////////////////////////////////////////////////////////
//
// CCTuple: Private Class 
//
class CCTuple
{
public:
  CCTuple() {}
  CCTuple(SeqNum seq) : seq_(seq), tcb_(NULL), wnd_(true) { resetTime(); }

  void operator()(std::pair<const SeqNum, CCTuple*>& entry); 
  void resetTime() { clock_gettime (CLOCK_REALTIME, &tt_); }
  int32_t delay();

  timespec  tt_;	// Transmit time
  SeqNum    seq_;	// Tuple sequence number
  timeCBHandle *tcb_;	// Used to cancel retransmit timer
  bool      wnd_;	// If true then window updated on timeout.
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
 * Output 1 (push): Status of a tuple that was recently sent.
 * Optional:
 * Output 2 (pull): Status of the CC element.
 */
CCT::CCT(string name, double init_wnd, double max_wnd, bool tstat, bool stat) 
  : Element(name, 2, (stat ? 3 : 2)),
    _data_cb(0),
    data_on_(true),
    sa_(-1),
    sv_(0)
{
  rto_            = MAX_RTO;
  max_wnd_        = max_wnd;
  cwnd_           = init_wnd;
  ssthresh_       = max_wnd;
  rwnd_           = max_wnd;
  tstat_          = tstat;
  stat_           = stat;
}

/**
 * The push method handles input on 2 ports.
 * port 0: Indicates a tuple to send.
 * port 1: Indicates the acknowledgement of some outstanding tuple.
 */
int CCT::push(int port, TuplePtr tp, b_cbv cb)
{
  assert(port == 1);

  try {
    if (Val_Str::cast((*tp)[2]) == "ACK") {
      // Acknowledge tuple and update measurements.
      SeqNum seq  = Val_UInt64::cast((*tp)[3]);	// Sequence number
      //TODO: Use timestamps to track the latest rwnd value.
      rwnd_ = Val_Double::cast((*tp)[4]);	// Receiver window
      add_rtt_meas(dealloc(seq, "SUCCESS"));
      return 1;
    }
  }
  catch (Value::TypeError& e) { } 

  assert(output(1)->push(tp, 0)); // Pass data tuple through
  return 1;
}

/**
 * Handles 2 output ports
 * port 0: Return the next tuple from either the retran or send queue
 * port 2: Return a tuple containing the internal CC state
 */
TuplePtr CCT::pull(int port, b_cbv cb)
{
  TuplePtr tp;

  switch (port) {
    case 0:
      assert (rto_ >= MIN_RTO && rto_ <= MAX_RTO);
      if (current_window() < max_window() && 
          (data_on_ = (tp = input(0)->pull(boost::bind(&CCT::data_ready, this))) != NULL)) {
        SeqNum   seq = getSeq(tp);
        CCTuple *otp = new CCTuple(seq);
        map(seq, otp);
      } else _data_cb = cb;
      break;
    case 2:
      // Package up a Tuple containing my current CC state
      tp = Tuple::mk();
      tp->append(Val_Double::mk(rwnd_));		// Receiver window size (flow control)
      tp->append(Val_Double::mk(cwnd_));		// Current congestion window size
      tp->append(Val_Double::mk(ssthresh_));		// Slow start threshold
      tp->append(Val_UInt32::mk(rto_));			// Retransmit timeout
      tp->append(Val_UInt32::mk(current_window()));	// Current window size (# unacked tuples)
      break;
    default: assert(0);
  } 
  return tp;
}

void CCT::map(SeqNum seq, CCTuple *otp) 
{
  tmap_.insert(std::make_pair(seq, otp));
  otp->tcb_ =
    delayCB((0.0 + rto_) / 1000.0,
            boost::bind(&CCT::timeout_cb, this, otp)); 
}

int32_t CCT::dealloc(SeqNum seq, string status)
{
  int32_t d = -1;
  CCTupleIndex::iterator iter = tmap_.find(seq);
  if (iter != tmap_.end()) { 
    d = iter->second->delay();

    if (iter->second->tcb_ != NULL) {
      timeCBRemove((iter->second)->tcb_);
    }

    delete iter->second;
    tmap_.erase(iter);
    if (current_window() < max_window() && data_on_ && _data_cb) {
      _data_cb();
      _data_cb = 0;
    }

    if (tstat_) {
      TuplePtr tp = Tuple::mk();
      tp->append(Val_Str::mk(status));		// Signal drop
      tp->append(Val_UInt64::mk(seq));		// Sequence number 
      tp->freeze();
      assert(output(1)->push(tp, 0));
    }
  }
  else {
    // Log event: possibly due to duplicate ack.
    log(LoggerI::INFO, 0, "CCT::push receive unknown ack, possible duplicate"); 
  }
  return d;
}

void CCT::data_ready() {
  data_on_ = true;
  if (_data_cb) {
    _data_cb();
    _data_cb = 0;
  }
}

/**
 * This function performs the following actions.
 * 1. Update window size
 * 2. Retransmit the packet
 */
void CCT::timeout_cb(CCTuple *otp)
{
  SeqNum seq = otp->seq_;

  if (otp->wnd_ == true) {
    // Update window sizes and enter slow start
    timeout(); 		
    for_each(tmap_.begin(), tmap_.end(), CCTuple());
  }

  otp->tcb_ = NULL;
  dealloc(seq, "FAIL");
}

/**
 *
 */
REMOVABLE_INLINE void CCT::add_rtt_meas(int32_t m)
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
}


REMOVABLE_INLINE void CCT::timeout() 
{
  rto_ <<= 1;

  if (rto_ > MAX_RTO)      rto_ = MAX_RTO;
  else if (rto_ < MIN_RTO) rto_ = MIN_RTO;

  ssthresh_ = cwnd_ / 2.0;	// multiplicative decrease
  cwnd_     = 2.0;		// Enter slow start
  if (ssthresh_ < 2.0) ssthresh_ = 2.0;
}

REMOVABLE_INLINE int CCT::current_window() {
  return tmap_.size();
}

REMOVABLE_INLINE int CCT::max_window() {
  return (cwnd_ < rwnd_) ? int(cwnd_) : int(rwnd_);
}

REMOVABLE_INLINE SeqNum CCT::getSeq(TuplePtr tp) {
  SeqNum seq = 0;
  for (uint i = 0; i < tp->size(); i++) {
    try {
      TuplePtr t = Val_Tuple::cast((*tp)[i]); 
      if (Val_Str::cast((*t)[0]) == "SEQ") {
        seq = Val_UInt64::cast((*t)[1]);
      }
    }
    catch (Value::TypeError& e) { } 
  }
  return seq;
}
