/*
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include <iostream>
#include "rdelivery.h"
#include "val_uint64.h"
#include "val_uint32.h"
#include "val_double.h"
#include "val_str.h"
#include "val_tuple.h"


#define MAP(s, t) \
do { \
  if (max_seq_ && s != max_seq_+1) \
    std::cerr << "MAX SEQ ERROR: " << max_seq_ << ", NEXT SEQ: " << s << std::endl; \
  max_seq_ = s; \
  tmap_.insert(std::make_pair((s), new RTuple((t)))); \
} while (0)

#define UNMAP(s) \
do { \
  RTupleIndex::iterator i = tmap_.find((s)); \
  if (i != tmap_.end()) { \
    delete i->second; \
    tmap_.erase(i); \
  } \
} while (0)


///////////////////////////////////////////////////////////////////////////////
//
// OTuple: Private Class 
//
class RTuple
{
public:
  RTuple(TuplePtr tp) : retry_cnt_(0), tp_(tp) { resetTime(); } 

  ~RTuple() { tp_.reset(); }

  void resetTime() { 
    clock_gettime (CLOCK_REALTIME, &timer_); 
  }
  int32_t delay() {
    timespec  now;
    clock_gettime(CLOCK_REALTIME, &now);

    if (now.tv_nsec < timer_.tv_nsec) { 
      now.tv_nsec += 1000000000;
      now.tv_sec--; 
    } 

    return (((now.tv_sec - timer_.tv_sec)*1000) + 
            ((now.tv_nsec - timer_.tv_nsec)/1000000)); // Delay in milliseconds
  }

  timespec  timer_;		// Transmit time
  uint32_t  retry_cnt_;		// Transmit counter.
  TuplePtr  tp_;		// The tuple.
};

RDelivery::RDelivery(str n, bool r, uint32_t m)
  : Element(n, 2, 3),
    _out_cb(0),
    in_on_(true),
    retry_(r),
    max_retry_(m)
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
    SeqNum seq = getSeq(tp); 
    MAP(seq, tp);
    return tp;			// forward tuple along
  }
  else if (!rtran_q_.empty()) {
    RTuple *rt = rtran_q_.front();
    rtran_q_.pop_front();
    return rt->tp_;
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

  str   type  = Val_Str::cast((*tp)[0]);	// Tuple Type
  SeqNum seq  = Val_UInt64::cast((*tp)[1]);	// Sequence number

  if (type == "FAIL") {
    handle_failure(seq);
  }
  else if (type == "SUCCESS") {
    UNMAP(seq);					// And that's all folks.
  } else return output(1)->push(tp, cb);	// DATA DELIVERY

  return 1;
}

REMOVABLE_INLINE void RDelivery::handle_failure(SeqNum seq) 
{
  RTuple *rt = tmap_.find(seq)->second;
  
  if (retry_ && rt->retry_cnt_ < max_retry_) {
    rtran_q_.push_back(rt);
    if (_out_cb) {
      _out_cb();
      _out_cb = 0;
    }
  }
  else {
    TuplePtr f = Tuple::mk();
    f->append(Val_Str::mk("FAIL"));
    f->append(Val_Tuple::mk(rt->tp_));
    f->freeze();
    // Push failed tuple upstream.
    assert(output(2)->push(f, 0));
    UNMAP(seq);
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

REMOVABLE_INLINE SeqNum RDelivery::getSeq(TuplePtr tp) {
  for (uint i = 0; i < tp->size(); i++) {
    try {
      TuplePtr t = Val_Tuple::cast((*tp)[i]); 
      if (Val_Str::cast((*t)[0]) == "SEQ") {
         if (Val_Str::cast((*t)[2]) == "localhost:10000" && Val_UInt32::cast((*t)[3]) == 0) 
           std::cerr << "SEQ NUMBER: " << (*t)[1]->toString() << std::endl;
         return Val_UInt64::cast((*t)[1]);
      }
    }
    catch (Value::TypeError& e) { } 
  }
  return 0;
}
