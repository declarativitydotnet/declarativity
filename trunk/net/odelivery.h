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
#include "inlines.h"
#include "tupleseq.h"
#include "loop.h"


class ODelivery : public Element {
public:
  ODelivery(string name, uint init_next_seq=1, 
            uint src_field=1, uint rto_field=2, uint seq_field=3);
  const char *class_name() const { return "ODelivery";};
  const char *processing() const { return "h/h"; };
  const char *flow_code() const	 { return "-/-"; };

  int push(int port, TuplePtr tp, b_cbv cb);	// Incoming ack or timeout

private:
  class Connection {
    public:
      Connection(uint seq, uint ns) 
        : tcb_(NULL), seq_field_(seq), next_seq_(ns) { touch(); }

      void insert(TuplePtr);
      long touch_duration() const;
      void touch();

      boost::posix_time::ptime last_touched;
      timeCBHandle*         tcb_;
      uint                  seq_field_;
      SeqNum                next_seq_;
      std::vector<TuplePtr> queue_; 
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

  uint init_next_seq_;		// The initialial next sequence value
  uint src_field_;
  uint seq_field_;
  uint rto_field_;

  typedef std::map<ValuePtr, ConnectionPtr, Value::Less> ConnectionIndex;
  ConnectionIndex cmap_;	// Map containing unacked in transit tuples
};
  
#endif /* __ODELIVERY_H_ */
