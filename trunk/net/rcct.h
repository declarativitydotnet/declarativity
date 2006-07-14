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

#ifndef __RCCT_H__
#define __RCCT_H__

#include <map>
#include <deque>
#include "tuple.h"
#include "element.h"
#include "inlines.h"
#include "tupleseq.h"

struct timeCBHandle;

class RateCCT : public Element {
public:
  RateCCT(string name);
  const char *class_name() const { return "RateCCT";};
  const char *processing() const { return "lh/lh"; };
  const char *flow_code() const	 { return "--/--"; };

  int push(int port, TuplePtr tp, b_cbv cb); 
  TuplePtr pull(int port, b_cbv cb);

  // Difference between current time and that given in timespec
  static REMOVABLE_INLINE uint32_t delay(boost::posix_time::ptime*);	
private:
  unsigned tuplesInFlight() const;
  void map(ValuePtr dest, SeqNum seq);
  void unmap(ValuePtr dest, SeqNum seq);
  void data_ready();			// Callback for tuple output ready
  void tuple_timeout(ValuePtr,SeqNum);	// Callback indicating tuple timeout
  void feedback_timeout();

  REMOVABLE_INLINE TuplePtr package(TuplePtr);		
  // Update rtt_, rto_, and sending rate
  REMOVABLE_INLINE void feedback(uint32_t, uint32_t, double);	

  bool     data_on_;
  b_cbv    data_cbv_;

  uint32_t  trate_;			// Allowable tuple rate (tuples/sec)
  uint32_t  rrate_;			// Receiver tuple rate (tuples/sec)
  uint32_t  rtt_;			// Estimated round trip time
  uint32_t  rto_;			// The round-trip timeout
  timeCBHandle  *nofeedback_;		// No feedback timer
  boost::posix_time::ptime  tld_;	// Time last doubled (for slow start)

  typedef std::map<SeqNum, timeCBHandle*> SeqTimeCBMap;
  typedef std::map<ValuePtr, boost::shared_ptr<SeqTimeCBMap>, Value::Less> ValueSeqTimeCBMap;
  ValueSeqTimeCBMap index_;		// Map containing unacked in transit tuples
};
  
#endif /* __RCCT_H_ */
