/*
 * 
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 */

#include <iostream>
#include "rdelivery.h"
#include "p2Time.h"
#include "val_uint64.h"
#include "val_uint32.h"
#include "val_double.h"
#include "val_str.h"
#include "val_tuple.h"


///////////////////////////////////////////////////////////////////////////////
//
// RTuple: Private Class 
//
void RDelivery::RTuple::resetTime() { 
  getTime (timer_); 
}

int32_t RDelivery::RTuple::delay() {
  boost::posix_time::ptime  now;
  getTime(now);
  return((now - timer_).total_milliseconds());
}


RDelivery::RDelivery(string n, unsigned m, int dest, int seq)
  : Element(n, 2, 2),
    _out_cb(0),
    in_on_(true),
    max_retry_(m),
    dest_field_(dest),
    seq_field_(seq)
{
  max_seq_ = 0;
}

/**
 * New tuple to send
 */
TuplePtr RDelivery::pull(int port, b_cbv cb)
{
  assert (port == 0);

  TuplePtr tp;
  if (rtran_q_.empty() && in_on_ && 
      (in_on_ = ((tp = input(0)->pull(boost::bind(&RDelivery::input_cb,this))) != NULL))) {
    /* Store the tuple for retry, if failure */
    ValuePtr dest = (*tp)[dest_field_];
    SeqNum   seq  = Val_UInt64::cast((*tp)[seq_field_]);
    RTuplePtr rtp(new RTuple(tp));
    map(dest, seq, rtp);
    return tp;			// forward tuple along
  }
  else if (!rtran_q_.empty()) {
    RTuplePtr rtp = rtran_q_.front();
    rtran_q_.pop_front();
    return rtp->tp_;	// Already memoized, so just return it.
  }

  _out_cb = cb;
  return TuplePtr();
}

/**
 * The push method handles input on 2 ports.
 * port 1: Acknowledgement or Failure Tuple
 */
int RDelivery::push(int port, TuplePtr tp, b_cbv cb)
{
  assert(port == 1);

  string   type = Val_Str::cast((*tp)[0]);	// Tuple Type
  ValuePtr dest = (*tp)[1];			// Destination address
  SeqNum   seq  = Val_UInt64::cast((*tp)[2]);	// Sequence number

  if (type == "FAIL") {
    handle_failure(dest, seq);
  }
  else if (type == "SUCCESS") {
    unmap(dest, seq);				// And that's all folks.
  } else return output(1)->push(tp, cb);	// DATA DELIVERY

  return 1;
}

REMOVABLE_INLINE void RDelivery::handle_failure(ValuePtr dest, SeqNum seq) 
{
  RTuplePtr rtp = lookup(dest, seq);
  
  if (rtp->retry_cnt_ < max_retry_) {
    rtran_q_.push_back(rtp);
    if (_out_cb) {
      _out_cb();
      _out_cb = 0;
    }
  }
  else {
    TuplePtr f = Tuple::mk();
    f->append(Val_Str::mk("FAIL"));
    f->append(Val_Tuple::mk(rtp->tp_));
    f->freeze();
    // Push failed tuple upstream.
    assert(output(2)->push(f, 0));
    unmap(dest, seq);
  }
}

RDelivery::RTuplePtr RDelivery::lookup(ValuePtr dest, SeqNum seq)
{
  ValueSeqRTupleMap::iterator iter_map = index_.find(dest);
  if (iter_map != index_.end()) {
    SeqRTupleMap::iterator iter_rtuple = iter_map->second->find(seq);
    if (iter_rtuple != iter_map->second->end()) {
      return iter_rtuple->second;	// Found it.
    }
  }
  return RTuplePtr();			// Didn't find it. 
}

void RDelivery::map(ValuePtr dest, SeqNum seq, RTuplePtr rtp) 
{
  ValueSeqRTupleMap::iterator iter_map = index_.find(dest);
  if (iter_map != index_.end()) {
    iter_map->second->insert(std::make_pair(seq, rtp));
  }
  else {
    boost::shared_ptr<SeqRTupleMap> tuple_map(new SeqRTupleMap);
    tuple_map->insert(std::make_pair(seq, rtp));
    index_.insert(std::make_pair(dest,tuple_map)); 
  }
}

void RDelivery::unmap(ValuePtr dest, SeqNum seq)
{
  ValueSeqRTupleMap::iterator iter_map = index_.find(dest);
  if (iter_map != index_.end()) {
    SeqRTupleMap::iterator iter_rtuple = iter_map->second->find(seq);
    if (iter_rtuple != iter_map->second->end()) {
      iter_map->second->erase(iter_rtuple);	// Remove outstanding tuple.
    }
    if (iter_map->second->size() == 0) {
      index_.erase(iter_map);		// No more outstanding tuples
    }
  }
}

void RDelivery::input_cb()
{
  in_on_ = true;
  if (_out_cb) {
    _out_cb();
    _out_cb = 0;
  }
}
