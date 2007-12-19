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

#ifndef __ROUNDTRIPTIMER_H__
#define __ROUNDTRIPTIMER_H__

#include <map>
#include "tuple.h"
#include "element.h"
#include "elementRegistry.h"
#include "inlines.h"
#include "tupleseq.h"


class TupleTimestamp;

class RoundTripTimer : public Element {
public:
  RoundTripTimer(string name); 
  RoundTripTimer(TuplePtr args);

  const char *class_name() const { return "RoundTripTimer";};
  const char *processing() const { return "ah/a"; };
  const char *flow_code() const	 { return "x-/x"; };

  TuplePtr simple_action(TuplePtr p);
  int push(int port, TuplePtr tp, b_cbv cb);

  DECLARE_PUBLIC_ELEMENT_INITS

private:
  void timeout_cb(TupleTimestamp*);
  // Remove TupleTimestamp from map
  REMOVABLE_INLINE int32_t dealloc(ValuePtr,SeqNum); 	
  // Map tuple and set timeout
  REMOVABLE_INLINE void map(TupleTimestamp*);		
  // Update sa, sv, and rto based on m
  REMOVABLE_INLINE void add_rtt_meas(ValuePtr,int32_t);	
  // Remove tuple from time consideration
  REMOVABLE_INLINE void timeout(ValuePtr);

  struct RTTRec {
    RTTRec();
    int32_t sa_;	// Scaled avg. RTT (ms) scaled by 8
    int32_t sv_;	// Scaled variance RTT (ms) scaled by 4
    int32_t rto_;	// The round-trip timeout
  };

  typedef std::map<ValuePtr, boost::shared_ptr<RTTRec>, 
                   Value::Comparator> RTTIndex;
  RTTIndex rttmap_;

  typedef std::map<ValuePtr, 
                   boost::shared_ptr<std::map<SeqNum, TupleTimestamp*> >,
                   Value::Comparator> TupleTimestampIndex;
  TupleTimestampIndex tmap_;

  DECLARE_PRIVATE_ELEMENT_INITS
};
  
#endif /* __ROUNDTRIPTIMER_H_ */
