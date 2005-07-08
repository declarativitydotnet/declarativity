/*
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include "rdelivery.h"
#include "val_uint64.h"
#include "val_uint32.h"
#include "val_double.h"
#include "val_str.h"


#define MAP(s, t) tmap_.insert(std::make_pair((s), new RTuple((t))))

#define UNMAP(s) \
do { \
  RTupleIndex::iterator i = tmap_.find((s)); \
  delete i->second; \
  if (i != tmap_.end()) tmap_.erase(i); \
} while (0)


///////////////////////////////////////////////////////////////////////////////
//
// OTuple: Private Class 
//
class RTuple
{
public:
  RTuple(TuplePtr tp) : retry_cnt_(0), tp_(tp) { resetTime(); } 

  ~RTuple() { tp_ = NULL; }

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

/**
 * New tuple to send
 */
TuplePtr RDelivery::simple_action(TupleRef p) 
{
  /* Store the tuple for retry, if failure */
  SeqNum seq = Val_UInt64::cast((*p)[seq_field_]);
  MAP(seq, p);
  return p;			// forward tuple along
}

/**
 * The push method handles input on 2 ports.
 * port 1: Acknowledgement or Failure Tuple
 */
int RDelivery::push(int port, TupleRef tp, cbv cb)
{
  assert(port == 1);

  str   type  = Val_Str::cast((*tp)[0]);	// Tuple Type
  SeqNum seq  = Val_UInt64::cast((*tp)[1]);	// Sequence number

  if (type == "FAIL") {
    handle_failure(seq);
  }
  else if (type == "ACK") {
    UNMAP(seq);
  } else assert(0);

  return 1;
}

REMOVABLE_INLINE void RDelivery::handle_failure(SeqNum seq) 
{
  RTuple *rt = tmap_.find(seq)->second;
  
  if (rt->retry_cnt_ < max_retry_) {
    if (retry_on_) {
      retry_on_ = output(1)->push(rt->tp_, wrap(this,&RDelivery::element_cb)); 
    } else rtran_q_.push_back(rt);
  } else UNMAP(seq);
}

void RDelivery::element_cb()
{
  do {
    RTuple *rt = rtran_q_.front();
    rtran_q_.pop_front();
    retry_on_ = output(1)->push(rt->tp_, wrap(this,&RDelivery::element_cb)); 
  } while (retry_on_);
}
