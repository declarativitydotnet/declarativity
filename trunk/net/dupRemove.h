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

#ifndef __DUPREMOVE_H__
#define __DUPREMOVE_H__

#include <map>
#include <vector>
#include "tuple.h"
#include "element.h"
#include "inlines.h"
#include "tupleseq.h"
#include "loop.h"


class DupRemove : public Element {
public:
  DupRemove(string n) : Element(n, 1, 1) {}

  const char *class_name() const { return "DupRemove";};
  const char *processing() const { return "h/h"; };
  const char *flow_code() const	 { return "-/-"; };

  int push(int port, TuplePtr tp, b_cbv cb);	// Incoming ack or timeout

private:
  class Connection {
    public:
      Connection(uint cs) 
        : _tcb(NULL), _cum_seq(cs) { touch(); }

      bool received(TuplePtr);
      long touch_duration() const;
      void touch();

      boost::posix_time::ptime last_touched;
      timeCBHandle*          _tcb;
      SeqNum                 _cum_seq;
      std::map<SeqNum, bool> _receiveMap; 
  };
  typedef boost::shared_ptr<Connection> ConnectionPtr;

  REMOVABLE_INLINE void map(ValuePtr, ConnectionPtr);
  REMOVABLE_INLINE void unmap(ValuePtr);
  REMOVABLE_INLINE ConnectionPtr lookup(ValuePtr);

  typedef std::map<ValuePtr, ConnectionPtr, Value::Less> ConnectionIndex;
  ConnectionIndex cmap_;	// Map containing unacked in transit tuples
};
  
#endif /* __DUPREMOVE_H_ */
