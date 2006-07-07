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

#ifndef __RDELIVERY_H__
#define __RDELIVERY_H__

#include <map>
#include <deque>
#include "tuple.h"
#include "element.h"
#include "loop.h"
#include "inlines.h"
#include "tupleseq.h"


class RDelivery : public Element {
public:
  RDelivery(string name, uint max_retry=5, uint dest=0, uint rto=2, uint seq=3);
  const char *class_name() const { return "RDelivery";};
  const char *processing() const { return "lh/lh"; };
  const char *flow_code() const	 { return "--/--"; };

  TuplePtr pull(int port, b_cbv cb);
  int push(int port, TuplePtr tp, b_cbv cb);	// Incoming ack or timeout

private:
  struct RTuple
  {
    RTuple(ValuePtr dest, SeqNum seq, double rto, TuplePtr tp) 
      : dest_(dest), seq_(seq), rto_(rto), tp_(tp), tcb_(NULL), retry_cnt_(0) { } 

    ValuePtr      dest_;
    SeqNum        seq_;
    double        rto_;		// Round trip time
    TuplePtr      tp_;		// The tuple.
    timeCBHandle* tcb_;       	// Used to cancel retransmit timer
    uint          retry_cnt_;	// Transmit counter.
  };
  typedef boost::shared_ptr<RTuple> RTuplePtr;

  // Helper methods to manipulate memorized tuples
  RTuplePtr lookup(ValuePtr dest, SeqNum seq);
  void      map(ValuePtr, SeqNum, RTuplePtr rtp);
  void      unmap(ValuePtr, SeqNum);

  // Handles a failure to deliver a tuple
  REMOVABLE_INLINE void handle_failure(ValuePtr, SeqNum);

  void input_cb();
  b_cbv _out_cb;

  bool      in_on_;
  uint      max_retry_;			// Max number of retries for a tuple
  uint      dest_field_;
  uint      seq_field_;
  uint      rto_field_;

  std::deque <RTuplePtr>  rtran_q_;	// Retransmit queue 

  typedef std::map<SeqNum, RTuplePtr> SeqRTupleMap;
  typedef std::map<ValuePtr, 
                   boost::shared_ptr<SeqRTupleMap>, 
                   Value::Less> ValueSeqRTupleMap;
  ValueSeqRTupleMap index_;		// Map containing unacked in transit tuples
};
  
#endif /* __RDELIVERY_H_ */
