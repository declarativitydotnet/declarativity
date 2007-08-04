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
#include "roundTripTimer.h"
#include "loop.h"
#include "val_uint64.h"
#include "val_uint32.h"
#include "val_double.h"
#include "val_str.h"
#include "val_tuple.h"
#include "val_null.h"
#include "netglobals.h"
#include <boost/bind.hpp>


/////////////////////////////////////////////////////////////////////
//
// TupleTimestamp: Private Class 
//
class TupleTimestamp
{
public:
  TupleTimestamp(ValuePtr dest, SeqNum seq) 
    : dest_(dest), seq_(seq) { getTime(tt_); }

  int32_t delay();

  boost::posix_time::ptime tt_;	// Transmit time
  ValuePtr      dest_;		// Tuple destination
  SeqNum        seq_;		// Tuple sequence number
  timeCBHandle* tcb_;    	// Used to cancel timer

};

int32_t TupleTimestamp::delay()
{
  boost::posix_time::ptime  now;
  getTime(now);

  return((int32_t)(now - tt_).total_milliseconds());
}

/////////////////////////////////////////////////////////////////////
//
// Globals
//

#define MAX_RTO (5000)
#define MIN_RTO (500)

/////////////////////////////////////////////////////////////////////
//
//
RoundTripTimer::RTTRec::RTTRec() : sa_(-1), sv_(0), rto_(MAX_RTO) {} 

/**
 */
RoundTripTimer::RoundTripTimer(string name) 
  : Element(name, 2, 1)
{
}

/**
 */
int RoundTripTimer::push(int port, TuplePtr tp, b_cbv cb)
{
  if (port == 1) {
    if ((*tp)[1]->typeCode() == Value::STR && Val_Str::cast((*tp)[1]) == "ACK") {
      // Acknowledge tuple and update measurements.
      ValuePtr dest = (*tp)[DEST+2];			// Destination address
      SeqNum   seq  = Val_UInt64::cast((*tp)[SEQ+2]);	// Sequence number
      RTTIndex::iterator iter = rttmap_.find(dest);
      if (iter == rttmap_.end()) {
        TELL_INFO << "RTT TIMER UNKNOWN DESTINATION: " << dest->toString() << std::endl;
      }
      else {
        int32_t delay = dealloc(dest, seq);
        add_rtt_meas(dest, delay);
      }
    }
  } else return this->Element::push(port, tp, cb); 	
  return 1;
}

/**
 */
TuplePtr RoundTripTimer::simple_action(TuplePtr p)
{
  SeqNum   seq  = Val_UInt64::cast((*p)[SEQ]);
  ValuePtr dest = (*p)[DEST];

  RTTIndex::iterator iter = rttmap_.find(dest);
  boost::shared_ptr<RTTRec> rttrec; 
  if (iter == rttmap_.end()) {
    rttrec.reset(new RTTRec());
    rttmap_.insert(std::make_pair(dest, rttrec));
  }
  else {
    rttrec = iter->second; 
  }

  TupleTimestamp *tt = new TupleTimestamp(dest, seq);
  map(tt);
  TuplePtr tp = Tuple::mk();
  for (unsigned i = 0; i < p->size(); i++) {
    if (i == RTT) 
      tp->append(Val_Double::mk(rttrec->rto_));
    else
      tp->append((*p)[i]);
  }
  tp->freeze();
  return tp;
}

void RoundTripTimer::map(TupleTimestamp *tt) 
{
  RTTIndex::iterator iter = rttmap_.find(tt->dest_);
  assert(iter != rttmap_.end());
  boost::shared_ptr<RTTRec> rttrec = iter->second; 

  boost::shared_ptr<std::map<SeqNum, TupleTimestamp*> > m;
  TupleTimestampIndex::iterator i = tmap_.find(tt->dest_);
  if (i == tmap_.end()) {
    m.reset(new std::map<SeqNum, TupleTimestamp*>);
    tmap_.insert(std::make_pair(tt->dest_, m));
  }
  else 
    m = i->second;
  m->insert(std::make_pair(tt->seq_, tt));

  tt->tcb_ =
    delayCB((0.0 + rttrec->rto_) / 1000.0,
            boost::bind(&RoundTripTimer::timeout_cb, this, tt), this); 
}

int32_t RoundTripTimer::dealloc(ValuePtr dest, SeqNum seq)
{
  int32_t delay = -1;
  TupleTimestampIndex::iterator iter = tmap_.find(dest);
  if (iter != tmap_.end()) { 
    std::map<SeqNum, TupleTimestamp*>::iterator record_iter = iter->second->find(seq); 
    if (record_iter != iter->second->end()) {
      TupleTimestamp *record = record_iter->second;
      delay = record->delay();

      if (record->tcb_ != NULL) {
        timeCBRemove(record->tcb_);
      }
      delete record;
      iter->second->erase(record_iter);
      if (iter->second->size() == 0)
        tmap_.erase(iter);
    }
  }
  else {
    // Log event: possibly due to duplicate ack.
    log(Reporting::INFO, 0, "RoundTripTimer::push receive unknown ack, possible duplicate"); 
  }
  return delay;
}

/**
 */
void RoundTripTimer::timeout_cb(TupleTimestamp *tt)
{
  timeout(tt->dest_); 
  tt->tcb_ = NULL;
  dealloc(tt->dest_, tt->seq_);
}

/**
 *
 */
REMOVABLE_INLINE void RoundTripTimer::add_rtt_meas(ValuePtr dest, int32_t m)
{
  RTTIndex::iterator iter = rttmap_.find(dest);
  assert(iter != rttmap_.end());
  boost::shared_ptr<RTTRec> rttrec = iter->second; 

  if (m < 0) return;

  if (rttrec->sa_ == -1) {
    // After the first measurement, set the timeout to four
    // times the RTT.

    rttrec->sa_ = m << 3;
    rttrec->sv_ = 0;
    // the 10 is for local queuing delay
    rttrec->rto_ = (m << 2) + 10; 
  }
  else {
    m -= (rttrec->sa_ >> 3);
    rttrec->sa_ += m;
    if (m < 0)
      m = -1*m;
    m -= (rttrec->sv_ >> 2);
    rttrec->sv_ += m;
    // the 10 is for local queuing delay
    rttrec->rto_ = (rttrec->sa_ >> 3) + rttrec->sv_ + 10; 
  }

  // Don't backoff past 5 seconds.
  if (rttrec->rto_ > MAX_RTO)      rttrec->rto_ = MAX_RTO;
  else if (rttrec->rto_ < MIN_RTO) rttrec->rto_ = MIN_RTO;
}


REMOVABLE_INLINE void RoundTripTimer::timeout(ValuePtr dest) 
{
  RTTIndex::iterator iter = rttmap_.find(dest);
  assert(iter != rttmap_.end());
  boost::shared_ptr<RTTRec> rttrec = iter->second; 

  rttrec->rto_ <<= 1;

  if (rttrec->rto_ > MAX_RTO)      rttrec->rto_ = MAX_RTO;
  else if (rttrec->rto_ < MIN_RTO) rttrec->rto_ = MIN_RTO;
}
