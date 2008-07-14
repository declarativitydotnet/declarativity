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
#include "netglobals.h"
#include <boost/bind.hpp>

DEFINE_ELEMENT_INITS(CCT, "CCT")

/////////////////////////////////////////////////////////////////////
//
// CCTuple: Private Class 
//
class CCTuple
{
public:
  CCTuple() {}
  CCTuple(ValuePtr dest, SeqNum seq) 
    : dest_(dest), seq_(seq), tcb_(NULL), wnd_(true) { }

  void operator()(std::pair<const SeqNum, CCTuple*>& entry); 

  ValuePtr      dest_;
  SeqNum        seq_;	// Tuple sequence number
  timeCBHandle* tcb_;	// Used to cancel retransmit timer
  bool          wnd_;	// If true then window updated on timeout.
};

void CCTuple::operator()(std::pair<const SeqNum, CCTuple*>& entry) 
{
  (entry.second)->wnd_ = false;
}

/////////////////////////////////////////////////////////////////////
//
// Transmit element
//

/**
 * Constructor: 
 * Input  0 (push): Tuple to send. 
 * Input  1 (push): Acknowledgement of some outstanding tuple.
 * Output 0 (pull): Tuple to send with cc info wrapper.
 * Output 1 (push): Acknowledgement of some outstanding tuple.
 */
CCT::CCT(string name, double init_wnd, double max_wnd) 
  : Element(name, 2, 2),
    _data_cb(0),
    data_on_(true)
{
  max_wnd_        = max_wnd;
  cwnd_           = init_wnd;
  ssthresh_       = max_wnd;
}

CCT::CCT(TuplePtr args) 
  : Element((*args)[2]->toString(), 2, 2),
    _data_cb(0),
    data_on_(true)
{
  cwnd_           = Val_Double::cast((*args)[3]);
  max_wnd_        = Val_Double::cast((*args)[4]);
  ssthresh_       = max_wnd_;
}
/**
 * The push method handles input on 2 ports.
 * port 0: Indicates a tuple to send.
 * port 1: Indicates the acknowledgement of some outstanding tuple.
 */
int CCT::push(int port, TuplePtr tp, b_cbv cb)
{
  assert(port == 1);

  if ((*tp)[1]->typeCode() == Value::STR && Val_Str::cast((*tp)[1]) == "ACK") {
    // Acknowledge tuple and update measurements.
    ValuePtr dest = (*tp)[DEST+2];			// Destination address
    SeqNum   seq  = Val_UInt64::cast((*tp)[SEQ+2]);	// Sequence number
    dealloc(dest, seq);
    successTransmit();
  }
  return output(1)->push(tp, cb);
}

/**
 * port 0: Return the next tuple
 */
TuplePtr CCT::pull(int port, b_cbv cb)
{
  TuplePtr tp;

  assert (port == 0);

  if (current_window() < max_window() && 
      (data_on_ = (tp = input(0)->pull(boost::bind(&CCT::data_ready, this))) != NULL)) {
    SeqNum   seq  = Val_UInt64::cast((*tp)[SEQ]);
    ValuePtr dest = (*tp)[DEST];
    double   rtt  = Val_Double::cast((*tp)[RTT]);

    CCTuple  *otp = new CCTuple(dest, seq);
    map(otp, rtt);
  } else _data_cb = cb;

  return tp;
}

void CCT::map(CCTuple *otp, double rtt) 
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
    delayCB((0.0 + rtt) / 1000.0,
            boost::bind(&CCT::timeout_cb, this, otp), this); 
}

void CCT::dealloc(ValuePtr dest, SeqNum seq)
{
  CCTupleIndex::iterator iter = tmap_.find(dest);
  if (iter != tmap_.end()) { 
    std::map<SeqNum, CCTuple*>::iterator record_iter = iter->second->find(seq); 
    if (record_iter != iter->second->end()) {
      CCTuple *record = record_iter->second;

      if (record->tcb_ != NULL) {
        timeCBRemove(record->tcb_);
      }

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
    ELEM_INFO("CCT::push receive unknown ack, possible duplicate"); 
  }
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
    failureTransmit(); 
    for (CCTupleIndex::iterator i = tmap_.begin(); i != tmap_.end(); i++) {
      // Update window sizes and enter slow start
      for_each(i->second->begin(), i->second->end(), CCTuple());
    }
  }
  otp->tcb_ = NULL;
  dealloc(otp->dest_, otp->seq_);
}

/**
 *
 */
REMOVABLE_INLINE void CCT::successTransmit()
{
  if (cwnd_ < ssthresh_) 
    cwnd_ *= 2.0; 			// slow start
  else		 
    cwnd_ += 1.0; 			// additive increase 

  if (cwnd_ > max_wnd_)
    cwnd_ = max_wnd_;
}


REMOVABLE_INLINE void CCT::failureTransmit() 
{
  ssthresh_ = cwnd_ / 2.0;	// multiplicative decrease
  cwnd_     = 2.0;		// Enter slow start
  if (ssthresh_ < 2.0) ssthresh_ = 2.0;
}

REMOVABLE_INLINE int CCT::current_window() {
  return tmap_.size();
}

REMOVABLE_INLINE int CCT::max_window() {
  return int(cwnd_);
}
