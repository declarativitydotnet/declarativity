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
//
RoundTripTimer::RTTRec::RTTRec() : sa_(-1), sv_(0), rto_(MAX_RTO) {} 

/**
 */
RoundTripTimer::RoundTripTimer(string name, uint dest_field, uint seq_field, uint rto_field) 
  : Element(name, 2, 1),
    dest_field_(dest_field),
    seq_field_(seq_field),
    rto_field_(rto_field)
{
}

/**
 */
int RoundTripTimer::push(int port, TuplePtr tp, b_cbv cb)
{
  if (port == 0) {
    SeqNum   seq  = Val_UInt64::cast((*tp)[seq_field_]);
    ValuePtr dest = (*tp)[dest_field_];
    
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

    TuplePtr tpl = Tuple::mk();
    for (unsigned i = 0; i < rto_field_; i++) {
      tpl->append((*tp)[i]);
    }
    tpl->append(Val_Double::mk(rttrec->rto_));
    for (unsigned i = rto_field_; i < tp->size(); i++) {
      tpl->append((*tp)[i]);
    }
    tpl->freeze();
    return output(0)->push(tpl, cb);
  }
  else if (port == 1) {
    try {
      for (uint i = 0;
           i < tp->size();
           i++) {
        if ((*tp)[i]->typeCode() == Value::STR && Val_Str::cast((*tp)[i]) == "ACK") {
          // Acknowledge tuple and update measurements.
          ValuePtr dest = (*tp)[i+1];			// Destination address
          SeqNum   seq  = Val_UInt64::cast((*tp)[i+2]);	// Sequence number
          int32_t delay = dealloc(dest, seq);
          add_rtt_meas(dest, delay);
          return 1;
        }
      }
    }
    catch (Value::TypeError e) { } 
  } else assert(0);
  return 1;
}

/**
 */
TuplePtr RoundTripTimer::pull(int port, b_cbv cb)
{
  TuplePtr p;

  assert (port == 0);

  if ((p = input(0)->pull(cb)) != NULL) {
    SeqNum   seq  = Val_UInt64::cast((*p)[seq_field_]);
    ValuePtr dest = (dest_field_ < 0) ? Val_Null::mk() : (*p)[dest_field_];

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
    for (unsigned i = 0; i < rto_field_; i++) {
      tp->append((*p)[i]);
    }
    tp->append(Val_Double::mk(rttrec->rto_));
    for (unsigned i = rto_field_; i < p->size(); i++) {
      tp->append((*p)[i]);
    }
    tp->freeze();
    return tp;
  }
  return p;
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
    log(LoggerI::INFO, 0, "RoundTripTimer::push receive unknown ack, possible duplicate"); 
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
