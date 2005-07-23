/*
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#ifndef __RCCT_H__
#define __RCCT_H__

#include <map>
#include <deque>
#include <async.h>
#include "tuple.h"
#include "element.h"
#include "inlines.h"

typedef uint64_t SeqNum;

class RateCCT : public Element {
public:
  RateCCT(str name);
  const char *class_name() const { return "RateCCT";};
  const char *processing() const { return "lh/lh"; };
  const char *flow_code() const	 { return "--/--"; };

  int push(int port, TupleRef tp, cbv cb); 
  TuplePtr pull(int port, cbv cb);

private:
  void data_ready();			// Callback for tuple output ready
  void tuple_timeout(SeqNum);		// Callback indicating tuple timeout
  void feedback_timeout();

  // Difference between current time and that given in timespec
  REMOVABLE_INLINE uint32_t delay(timespec*);	
  REMOVABLE_INLINE TuplePtr package(TuplePtr);		
  // Update rtt_, rto_, and sending rate
  REMOVABLE_INLINE void feedback(uint32_t, uint32_t, double);	

  bool     data_on_;
  cbv      data_cbv_;

  uint32_t  trate_;			// Allowable tuple rate (tuples/sec)
  uint32_t  rrate_;			// Receiver tuple rate (tuples/sec)
  uint32_t  rtt_;			// Estimated round trip time
  uint32_t  rto_;			// The round-trip timeout
  timecb_t  *nofeedback_;		// No feedback timer
  timespec  tld_;			// Time last doubled (for slow start)

  typedef std::map<SeqNum, timecb_t*> TupleTOIndex;
  TupleTOIndex tmap_;			// Map containing unacked in transit tuples
};
  
#endif /* __RCCT_H_ */
