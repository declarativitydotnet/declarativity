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
  RDelivery(string name, uint max_retry=3);
  const char *class_name() const { return "RDelivery";};
  const char *processing() const { return "lh/lh"; };
  const char *flow_code() const	 { return "--/--"; };

  TuplePtr pull(int port, b_cbv cb);
  int push(int port, TuplePtr tp, b_cbv cb);	// Incoming ack or timeout

private:
  typedef std::deque<TuplePtr> TupleQ;
  TupleQ  _retry_q;	// Retransmit queue 

  struct Connection {
    struct Tuple {
      Tuple(SeqNum seq, TuplePtr tp) 
        : _seq(seq), _tp(tp), _retry_cnt(0) { } 

      SeqNum   _seq;		// Sequence number
      TuplePtr _tp;		// The tuple.
      uint     _retry_cnt;	// Transmit counter.
    };
    typedef boost::shared_ptr<Tuple> TuplePtr;

    Connection(double rtt, SeqNum cum_seq)
      : _rtt(rtt), _cum_seq(cum_seq), _tcb(NULL) {} 

    std::deque<TuplePtr> _outstanding; 	// All outstanding tuples
    double               _rtt;		// The round trip time
    SeqNum               _cum_seq;	// The cumulative sequence
    timeCBHandle*        _tcb; 		// Used to cancel retransmit timer
  };
  typedef boost::shared_ptr<Connection> ConnectionPtr;

  REMOVABLE_INLINE void handle_failure(ConnectionPtr); 

  // Helper methods to manipulate memorized connections
  ConnectionPtr lookup(ValuePtr);
  void map(ValuePtr, ConnectionPtr);
  void unmap(ValuePtr);

  TuplePtr tuple(TuplePtr tp);
  void ack(TuplePtr tp);


  void input_cb();
  b_cbv _out_cb;

  bool      _in_on;
  uint      _max_retry;			// Max number of retries for a tuple

  typedef std::map<ValuePtr, ConnectionPtr, Value::Less> ValueConnectionMap;
  ValueConnectionMap _index;		// Map containing outstanding connections
};
  
#endif /* __RDELIVERY_H_ */
