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

#ifndef __ODELIVERY_H__
#define __ODELIVERY_H__

#include <map>
#include <vector>
#include "tuple.h"
#include "element.h"
#include "elementRegistry.h"
#include "inlines.h"
#include "tupleseq.h"
#include "eventLoop.h"

class ODelivery : public Element {
public:
  ODelivery(string name); 
  ODelivery(TuplePtr args);
  const char *class_name() const { return "ODelivery";};
  const char *processing() const { return "h/h"; };
  const char *flow_code() const	 { return "-/-"; };

  int push(int port, TuplePtr tp, b_cbv cb);	// Incoming ack or timeout

  DECLARE_PUBLIC_ELEMENT_INITS

private:
  class Connection {
  public:
    Connection(uint ns) : tcb_(0), next_seq_(ns) { touch(); }
    
    void insert(TuplePtr);
    long touch_duration();
    void touch();
    
    boost::posix_time::ptime	last_touched;
    EventLoop::TimerID		tcb_;
    SeqNum			next_seq_;
    std::vector<TuplePtr>	queue_; 
  };
  typedef boost::shared_ptr<Connection> ConnectionPtr;

  REMOVABLE_INLINE void out_cb();

  REMOVABLE_INLINE void map(ValuePtr, ConnectionPtr);
  REMOVABLE_INLINE void unmap(ValuePtr);
  REMOVABLE_INLINE ConnectionPtr lookup(ValuePtr);

  REMOVABLE_INLINE void flush(ConnectionPtr cp);
  REMOVABLE_INLINE void flushConnectionQ(ValuePtr src);

  b_cbv     _in_cb;
  bool      out_on_;		//  Output port ready status

  typedef std::map<ValuePtr, ConnectionPtr, Value::Comparator> ConnectionIndex;
  ConnectionIndex cmap_;	// Map containing unacked in transit tuples

  DECLARE_PRIVATE_ELEMENT_INITS
};
  
#endif /* __ODELIVERY_H_ */
