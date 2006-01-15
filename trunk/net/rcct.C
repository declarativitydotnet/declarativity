/*
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
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
#define ACK_SIG     2
#define ACK_SEQ     ACK_SIG + 1
#define ACK_RATE    ACK_SIG + 2
#define ACK_LOSS    ACK_SIG + 3
#define ACK_TIME    ACK_SIG + 4
#define MAX_SURPLUS 10
#define RTT_FILTER  0.9

#define MAP(key) \
do { \
  timeCBHandle *t = delayCB((0.0 + rto_) / 1000.0, boost::bind(&RateCCT::tuple_timeout, this, (key))); \
  tmap_.insert(std::make_pair((key), t)); \
} while (0)

#define UNMAP(key, c) \
do { \
  TupleTOIndex::iterator i = tmap_.find((key)); \
  if (i != tmap_.end()) { \
    if (c) timeCBRemove(i->second); \
    tmap_.erase(i); \
  } \
  if (tmap_.size() < trate_ && data_cbv_) { \
      data_cbv_(); \
      data_cbv_ = 0; \
    } \
} while (0)

#define ACK(status, s) \
do { \
  if (tstat_) { \
    TuplePtr tp = Tuple::mk(); \
    tp->append(Val_Str::mk((status))); \
    tp->append(Val_UInt64::mk(s)); \
    assert(output(1)->push(tp, 0)); \
  } \
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
 * Output 2 (pull): Status of the CC element. (Optional)
 */
RateCCT::RateCCT(string name, bool tstat) 
  : Element(name, 2, 2),
    data_on_(true),
    data_cbv_(0), 
    trate_(1),
    rtt_(100),
    rto_(4000),
    nofeedback_(NULL),
    tstat_(tstat)
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
    string name  = Val_Str::cast((*tp)[ACK_SIG]);
    if (name == "ACK") {
      // Acknowledge tuple with rate feedback.
      SeqNum  seq = Val_UInt64::cast((*tp)[ACK_SEQ]);
      uint32_t rr = Val_UInt32::cast((*tp)[ACK_RATE]);
      double   p  = Val_Double::cast((*tp)[ACK_LOSS]);
      timespec ts = Val_Time::cast(  (*tp)[ACK_TIME]);

      UNMAP(seq, true);
      ACK("SUCCESS", seq);
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
    if (tmap_.size() < trate_ && 
        (data_on_ = (tp = input(0)->pull(boost::bind(&RateCCT::data_ready, this))) != NULL)) {
      return package(tp);
    }
    data_cbv_ = cb;
  }
  else if (port == 2) {
    tp = Tuple::mk();
    tp->append(Val_Double::mk(trate_));		// Transmit rate
    tp->append(Val_Double::mk(rrate_));		// Receiver rate
    tp->append(Val_UInt32::mk(rtt_));		// Round trip time
    tp->append(Val_UInt32::mk(rto_));		// Retransmit timeout
    return tp;
  } else assert (0);
  return TuplePtr();
}

void RateCCT::data_ready()
{
  data_on_ = true;
  if (data_cbv_) {
    data_cbv_();
    data_cbv_ = 0;
  }
}

void RateCCT::tuple_timeout(SeqNum s) 
{
  ACK("FAIL", s);
  UNMAP(s, false);
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

REMOVABLE_INLINE uint32_t RateCCT::delay(timespec *ts)
{
  timespec  now;
  getTime(now);
  if (now.tv_nsec < ts->tv_nsec) { 
    if (now.tv_nsec + 1000000000 < 0) {
      now.tv_nsec -= 1000000000;
      now.tv_sec++;
    }
    else {
      now.tv_nsec += 1000000000;
      now.tv_sec--; 
    }
  } 

  return (((now.tv_sec - ts->tv_sec)*1000) + 
          ((now.tv_nsec - ts->tv_nsec)/1000000)); // Delay in milliseconds
}

REMOVABLE_INLINE TuplePtr RateCCT::package(TuplePtr tp)
{
  string   cid = "";
  SeqNum   seq = 0;
  timespec now;
  getTime(now);
  for (uint i = 0; i < tp->size(); i++) {
    try {
      TuplePtr t = Val_Tuple::cast((*tp)[i]); 
      if (Val_Str::cast((*t)[0]) == "SEQ") {
        seq = Val_UInt64::cast((*t)[1]);
        if (t->size() == 3) cid = Val_Str::cast((*t)[2]);
      }
    }
    catch (Value::TypeError e) { } 
  }
   
  MAP(seq);

  TuplePtr otp   = Tuple::mk();
  TuplePtr tinfo = Tuple::mk();
  tinfo->append(Val_Str::mk("TINFO"));
  tinfo->append(Val_UInt32::mk(rtt_));	// Round trip time
  tinfo->append(Val_Time::mk(now));	// Time stamp
  tinfo->freeze();
  otp->append(Val_Tuple::mk(tinfo));
  for (uint i = 0; i < tp->size(); i++)
    otp->append((*tp)[i]);
  otp->freeze();
  return otp;
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
