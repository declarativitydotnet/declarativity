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

#include <algorithm>
#include <iostream>
#include "loop.h"
#include "cct.h"
#include "val_uint64.h"
#include "val_uint32.h"
#include "val_double.h"
#include "val_str.h"
#include "val_tuple.h"
#include "val_null.h"


/////////////////////////////////////////////////////////////////////
//
// CCTuple: Private Class 
//
class CCTuple
{
public:
  CCTuple() {}
  CCTuple(ValuePtr dest, SeqNum seq) 
    : dest_(dest), seq_(seq), tcb_(NULL), wnd_(true) { resetTime(); }

  void operator()(std::pair<const SeqNum, CCTuple*>& entry); 
  void resetTime() { getTime (tt_); }
  int32_t delay();

  boost::posix_time::ptime tt_;	// Transmit time
  ValuePtr  dest_;
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
  boost::posix_time::ptime  now;
  getTime(now);

  return((now - tt_).total_milliseconds());
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
CCT::CCT(string name, double init_wnd, double max_wnd, int dest_field, int seq_field) 
  : Element(name, 2, 2),
    _data_cb(0),
    data_on_(true),
    sa_(-1),
    sv_(0),
    dest_field_(dest_field),
    seq_field_(seq_field)
{
  rto_            = MAX_RTO;
  max_wnd_        = max_wnd;
  cwnd_           = init_wnd;
  ssthresh_       = max_wnd;
  rwnd_           = max_wnd;
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
    for (uint i = 0;
         i < tp->size();
         i++) {
      if ((*tp)[i]->typeCode() == Value::STR && Val_Str::cast((*tp)[i]) == "ACK") {
        // Acknowledge tuple and update measurements.
        ValuePtr dest = (*tp)[i+1];			// Destination address
        SeqNum   seq  = Val_UInt64::cast((*tp)[i+2]);	// Sequence number
        //TODO: Use timestamps to track the latest rwnd value.
        rwnd_ = Val_Double::cast((*tp)[i+3]);		// Receiver window
        add_rtt_meas(dealloc(dest, seq, "SUCCESS"));
        return 1;
      }
    }
  }
  catch (Value::TypeError e) { } 
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

  assert (port == 0);
  assert (rto_ >= MIN_RTO && rto_ <= MAX_RTO);

  if (current_window() < max_window() && 
      (data_on_ = (tp = input(0)->pull(boost::bind(&CCT::data_ready, this))) != NULL)) {
    SeqNum   seq  = Val_UInt64::cast((*tp)[seq_field_]);
    ValuePtr dest = (dest_field_ < 0) ? Val_Null::mk() : (*tp)[dest_field_];
    CCTuple  *otp = new CCTuple(dest, seq);
    map(otp);
  } else _data_cb = cb;
  return tp;
}

void CCT::map(CCTuple *otp) 
{
  boost::shared_ptr<std::map<SeqNum, CCTuple*> > m;
  CCTupleIndex::iterator i = tmap_.find(otp->dest_);
  if (i == tmap_.end()) {
    m.reset(new std::map<SeqNum, CCTuple*>);
    tmap_.insert(std::make_pair(otp->dest_, m));
  }
  else 
    m = i->second;
  m->insert(std::make_pair(otp->seq_, otp));

  otp->tcb_ =
    delayCB((0.0 + rto_) / 1000.0,
            boost::bind(&CCT::timeout_cb, this, otp), this); 
}

int32_t CCT::dealloc(ValuePtr dest, SeqNum seq, string status)
{
  int32_t d = -1;
  CCTupleIndex::iterator iter = tmap_.find(dest);
  if (iter != tmap_.end()) { 
    std::map<SeqNum, CCTuple*>::iterator record_iter = iter->second->find(seq); 
    if (record_iter != iter->second->end()) {
      CCTuple *record = record_iter->second;
      d = record->delay();

      if (record->tcb_ != NULL) {
        timeCBRemove(record->tcb_);
      }

      TuplePtr tp = Tuple::mk();
      tp->append(Val_Str::mk(status));		// Signal drop
      tp->append(record->dest_);		// Destination address 
      tp->append(Val_UInt64::mk(seq));		// Sequence number 
      tp->freeze();
      assert(output(1)->push(tp, 0));

      delete record;
      iter->second->erase(record_iter);
      if (iter->second->size() == 0)
        tmap_.erase(iter);
 
      if (current_window() < max_window() && data_on_ && _data_cb) {
        _data_cb();
        _data_cb = 0;
      }
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
  if (otp->wnd_ == true) {
    timeout(); 		
    for (CCTupleIndex::iterator i = tmap_.begin(); i != tmap_.end(); i++) {
      // Update window sizes and enter slow start
      for_each(i->second->begin(), i->second->end(), CCTuple());
    }
  }

  otp->tcb_ = NULL;
  dealloc(otp->dest_, otp->seq_, "FAIL");
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
