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
#include "sysconf.h"
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
#define ACK_ADDR    1
#define ACK_CID     2
#define ACK_SEQ     3
#define ACK_RATE    4
#define ACK_LOSS    5
#define ACK_TIME    6
#define MAX_SURPLUS 10
#define RTT_FILTER  0.9

#define MAP(key) \
do { \
  timecb_t *t = delaycb(rto_/1000, (rto_ % 1000)*1000000, \
                        wrap(this, &RateCCT::tuple_timeout, (key))); \
  tmap_.insert(std::make_pair((key), t)); \
} while (0)

#define UNMAP(key, c) \
do { \
  TupleTOIndex::iterator i = tmap_.find((key)); \
  if (i != tmap_.end()) { \
    if (c) timecb_remove(i->second); \
    tmap_.erase(i); \
  } \
} while (0)

#define ACK(status, c, s) \
do { \
  TuplePtr tp = Tuple::mk(); \
  tp->append(Val_Str::mk((status))); \
  tp->append(Val_Str::mk(c)); \
  tp->append(Val_UInt64::mk(s)); \
  assert(output(1)->push(tp, cbv_null)); \
} while (0)

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
RateCCT::RateCCT(str name) 
  : Element(name, 2, 2), data_on_(true), data_cbv_(cbv_null), 
    trate_(1), rtt_(100), rto_(4000), nofeedback_(NULL)
{
  clock_gettime(CLOCK_REALTIME, &tld_);
}

/**
 * The push method handles input on 2 ports.
 * port 0: Indicates a tuple to send.
 * port 1: Indicates the acknowledgement of some outstanding tuple.
 */
int RateCCT::push(int port, TupleRef tp, cbv cb)
{
  assert(port == 1);

  str name  = Val_Str::cast((*tp)[0]);
  if (name == "ACK") {
    // Acknowledge tuple with rate feedback.
    str     cid = Val_Str::cast(   (*tp)[ACK_CID]);
    SeqNum  seq = Val_UInt64::cast((*tp)[ACK_SEQ]);
    uint32_t rr = Val_UInt32::cast((*tp)[ACK_RATE]);
    double   p  = Val_Double::cast((*tp)[ACK_LOSS]);
    timespec ts = Val_Time::cast(  (*tp)[ACK_TIME]);

    log(LoggerI::INFO, 0, strbuf() << "CID: " << cid << ", SEQ: " << seq
				   << ", RRATE: " << rr << ", LOSS RATE: " << long(p)
				   << ", DELAY: " << delay(&ts));

    UNMAP(strbuf() << cid << ":" << seq, true);
    ACK("SUCCESS", cid, seq);
    feedback(delay(&ts), rr, p);
    if (tmap_.size() < trate_ && data_cbv_ != cbv_null) {
      (*data_cbv_)();
      data_cbv_ = cbv_null;
    }
    std::cerr << "WINDOW SIZE: " << tmap_.size() << std::endl;
  }
  return 1;
}

/**
 * port 2: Return a tuple containing the internal CC state
 */
TuplePtr RateCCT::pull(int port, cbv cb)
{
  TuplePtr tp = NULL;

  if (port == 0) {
    if (tmap_.size() < trate_ && 
        (data_on_ = (tp = input(0)->pull(wrap(this, &RateCCT::data_ready))) != NULL)) {
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
  return NULL;
}

void RateCCT::data_ready()
{
  data_on_ = true;
  if (data_cbv_ != cbv_null) {
    (*data_cbv_)();
    data_cbv_ = cbv_null;
  }
}

void RateCCT::tuple_timeout(str key) 
{
  std::cerr << "\nTUPLE TIMEOUT" << std::endl;
  const char * rstr = (key).cstr();
  char *       sep  = strchr(rstr, ':');
  SeqNum       s    = strtoull(sep+1,(char **)NULL, 0);
  str c(rstr, sep - rstr);

  ACK("FAIL", c, s);
  UNMAP(key, false);
}

void RateCCT::feedback_timeout() 
{
  nofeedback_ = NULL;
  if (trate_ > 2*rrate_) {
    rrate_ = max(trate_/2, 1U);
  }
  else {
    rrate_ = trate_ / 4;
  }
  feedback(rtt_, rrate_, 0);
}

REMOVABLE_INLINE uint32_t RateCCT::delay(timespec *ts)
{
  timespec  now;
  clock_gettime(CLOCK_REALTIME, &now);

  if (now.tv_nsec < ts->tv_nsec) { 
    now.tv_nsec += 1000000000;
    now.tv_sec--; 
  } 

  return (((now.tv_sec - ts->tv_sec)*1000) + 
          ((now.tv_nsec - ts->tv_nsec)/1000000)); // Delay in milliseconds
}

REMOVABLE_INLINE TuplePtr RateCCT::package(TuplePtr tp)
{
  str      cid = "";
  SeqNum   seq = 0;
  timespec now;
  clock_gettime(CLOCK_REALTIME, &now);
  for (uint i = 0; i < tp->size(); i++) {
    try {
      TupleRef t = Val_Tuple::cast((*tp)[i]); 
      if (Val_Str::cast((*t)[0]) == "SEQ") {
        seq = Val_UInt64::cast((*t)[1]);
        if (t->size() == 3) cid = Val_Str::cast((*t)[2]);
      }
    }
    catch (Value::TypeError& e) { } 
  }
   
  MAP(strbuf() << cid << ":" << seq);

  TuplePtr otp   = Tuple::mk();
  TuplePtr tinfo = Tuple::mk();
  tinfo->append(Val_Str::mk("TINFO"));
  tinfo->append(Val_UInt32::mk(rtt_));	// Round trip time
  tinfo->append(Val_Time::mk(now));		// Time stamp
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
    timecb_remove(nofeedback_);
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
    trate_ = (uint32_t) max(min(X_calc, 2*X_recv), 1U);
  }
  else if (delay(&tld_) > rtt_) {
    trate_ = max(min(2*trate_, 2*X_recv), 2U);
    clock_gettime(CLOCK_REALTIME, &tld_);
  }

  rrate_ = X_recv;		// Save the receiver rate
  uint32_t tms = max(rto_, 4000/trate_);
  nofeedback_ = delaycb(tms/1000, (tms % 1000)*1000000,
                        wrap(this, &RateCCT::feedback_timeout));
}
