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
#include <math.h>
#include "rcct.h"
#include "val_uint64.h"
#include "val_uint32.h"
#include "val_double.h"
#include "val_str.h"
#include "val_time.h"
#include "val_tuple.h"


/////////////////////////////////////////////////////////////////////
//
// Globals and MACROS
//

#define MAX_MBI     (64000)
#define ACK_SIG     1
#define ACK_DEST    ACK_SIG + 1
#define ACK_SEQ     ACK_SIG + 2
#define ACK_RATE    ACK_SIG + 3
#define ACK_LOSS    ACK_SIG + 4
#define ACK_TIME    ACK_SIG + 5
#define MAX_SURPLUS 10
#define RTT_FILTER  0.9

#define ACK(status, d, s) \
do { \
  TuplePtr tp = Tuple::mk(); \
  tp->append(Val_Str::mk((status))); \
  tp->append((d)); \
  tp->append(Val_UInt64::mk(s)); \
  tp->freeze(); \
  assert(output(1)->push(tp, 0)); \
} while (0)

#undef MAX
#define MAX(a, b) (((a) < (b)) ? (b) : (a))
#undef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

/////////////////////////////////////////////////////////////////////
//
// Transmit element
//

/**
 * Constructor: 
 * Input  0 (pull): Tuple to send, pulling a specific rate. 
 * Input  1 (push): Feedback packet
 * Output 0 (push): Tuple to send with cc info wrapper.
 * Output 1 (push): Status of a tuple that was recently sent.
 */
RateCCT::RateCCT(string name, int dest, int seq, int rtt, int ts) 
  : Element(name, 2, 2),
    data_on_(true),
    data_cbv_(0), 
    trate_(1),
    rtt_(100),
    rto_(4000),
    nofeedback_(NULL),
    dest_field_(dest),
    seq_field_(seq),
    rtt_field_(rtt),
    ts_field_(ts)
{
  getTime(tld_);
}

/**
 * The push method handles input on 2 ports.
 * port 0: Indicates a tuple to send.
 * port 1: Tuple received.
 */
int RateCCT::push(int port, TuplePtr tp, b_cbv cb)
{
  assert(port == 1);

  try {
    if (Val_Str::cast((*tp)[ACK_SIG]) == "ACK") {
      // Acknowledge tuple with rate feedback.
      ValuePtr dest = (*tp)[ACK_DEST];
      SeqNum   seq  = Val_UInt64::cast((*tp)[ACK_SEQ]);
      uint32_t rr   = Val_UInt32::cast((*tp)[ACK_RATE]);
      double   p    = Val_Double::cast((*tp)[ACK_LOSS]);
      boost::posix_time::ptime ts = Val_Time::cast(  (*tp)[ACK_TIME]);

      unmap(dest, seq);
      ACK("SUCCESS", dest, seq);
      feedback(delay(&ts), rr, p);
      return 1;
    }
  }
  catch (Value::TypeError e) { } 

  assert(output(1)->push(tp, 0)); // Pass data tuple through
  return 1;
}

/**
 * port 2: Return a tuple containing the internal CC state
 */
TuplePtr RateCCT::pull(int port, b_cbv cb)
{
  TuplePtr tp;

  if (port == 0) {
    if (tuplesInFlight() < trate_ && 
        (data_on_ = (tp = input(0)->pull(boost::bind(&RateCCT::data_ready, this))) != NULL)) {
      return package(tp);
    }
    data_cbv_ = cb;
  }
  return TuplePtr();
}

void RateCCT::map(ValuePtr dest, SeqNum seq)
{
  timeCBHandle *tcb = delayCB((0.0 + rto_) / 1000.0, 
                              boost::bind(&RateCCT::tuple_timeout, this, dest, seq));

  ValueSeqTimeCBMap::iterator iter_map = index_.find(dest);
  boost::shared_ptr<SeqTimeCBMap> time_map;
  if (iter_map != index_.end()) {
    time_map = iter_map->second;
  }
  else {
    time_map.reset(new SeqTimeCBMap);
    index_.insert(std::make_pair(dest, time_map));
  }
  time_map->insert(std::make_pair(seq, tcb));
}

void RateCCT::unmap(ValuePtr dest, SeqNum seq)
{
  ValueSeqTimeCBMap::iterator iter_map = index_.find(dest);
  if (iter_map != index_.end()) { \
    SeqTimeCBMap::iterator iter_time = iter_map->second->find(seq);
    if (iter_time != iter_map->second->end()) {
      timeCBRemove(iter_time->second);
      iter_map->second->erase(iter_time);
      if (iter_map->second->size() == 0) {
        index_.erase(iter_map); 
      }
    }
  }

  if (tuplesInFlight() < trate_ && data_cbv_) {
    data_cbv_();
    data_cbv_ = 0;
  }

}

int RateCCT::tuplesInFlight() const {
  ValueSeqTimeCBMap::const_iterator iter_map = index_.begin();
  int total = 0;
  for (iter_map = index_.begin(); iter_map != index_.end(); iter_map++) {
    total += iter_map->second->size();
  }
  return total;
}

void RateCCT::data_ready()
{
  data_on_ = true;
  if (data_cbv_) {
    data_cbv_();
    data_cbv_ = 0;
  }
}

void RateCCT::tuple_timeout(ValuePtr d, SeqNum s) 
{
  ACK("FAIL", d, s);
  unmap(d, s);
}

void RateCCT::feedback_timeout() 
{
  nofeedback_ = NULL;
  if (trate_ > 2*rrate_) {
    rrate_ = MAX(trate_/2, 1U);
  }
  else {
    rrate_ = trate_ / 4;
  }
  feedback(rtt_, rrate_, 0);
}

REMOVABLE_INLINE uint32_t RateCCT::delay(boost::posix_time::ptime *ts)
{
  boost::posix_time::ptime  now;
  getTime(now);
  return((now - *ts).total_milliseconds());
}

REMOVABLE_INLINE TuplePtr RateCCT::package(TuplePtr tp)
{
  ValuePtr dest = (*tp)[dest_field_];
  SeqNum   seq  = Val_UInt64::cast((*tp)[seq_field_]);
  boost::posix_time::ptime now;
  getTime(now);
   
  map(dest, seq);	// Set a timer for this tuple

  (*tp)[rtt_field_] = Val_UInt32::mk(rtt_);
  (*tp)[ts_field_]  = Val_Time::mk(now); 
  return tp;
}

/**
 *
 */
REMOVABLE_INLINE void RateCCT::feedback(uint32_t rt, uint32_t X_recv, double p)
{
  if (nofeedback_ != NULL) {
    timeCBRemove(nofeedback_);
  }

  if (!rtt_) {
    // After the first measurement, set the rtt to the sample 
    rtt_ = rt;
    rto_ = (rt << 2);
  }
  else {
    rtt_ = uint32_t(RTT_FILTER * double(rtt_) + (1. - RTT_FILTER) * double(rt));
    rto_ = rtt_ << 2;
  }

  if (p > 0) {
    double   f      = sqrt(2.*p/3.) + (12.*sqrt(3.*p/8.) * p * (1.+32*pow(p, 2)));
    uint32_t X_calc = (uint32_t) (1000. / (rtt_ * f)); 
    trate_ = (uint32_t) MAX(MIN(X_calc, 2*X_recv), 1U);
  }
  else if (delay(&tld_) > rtt_) {
    trate_ = MAX(MIN(2*trate_, 2*X_recv), 2U);
    getTime(tld_);
  }

  rrate_ = X_recv;		// Save the receiver rate
  uint32_t tms = MAX(rto_, 8000/trate_);
  nofeedback_ = delayCB((0.0 + tms) / 1000.0,
                        boost::bind(&RateCCT::feedback_timeout, this));
}
