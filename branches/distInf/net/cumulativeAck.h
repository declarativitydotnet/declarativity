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

#ifndef __CUMULATIVEACK_H__
#define __CUMULATIVEACK_H__

#include <map>
#include <vector>
#include "tuple.h"
#include "element.h"
#include "elementRegistry.h"
#include "inlines.h"
#include "tupleseq.h"
#include "loop.h"


class CumulativeAck : public Element {
public:
  CumulativeAck(string name);
  CumulativeAck(TuplePtr args);
  const char *class_name() const { return "CumulativeAck";};
  const char *processing() const { return "a/a"; };
  const char *flow_code() const	 { return "-/-"; };

  TuplePtr simple_action(TuplePtr p);

  DECLARE_PUBLIC_ELEMENT_INITS

private:
  class Connection {
    public:
      Connection() 
        : _tcb(NULL), _cum_seq(0) { }

      void confirm(SeqNum);

      timeCBHandle*         _tcb;
      SeqNum                _cum_seq;
      std::vector<SeqNum>   _queue; 
  };
  typedef boost::shared_ptr<Connection> ConnectionPtr;

  REMOVABLE_INLINE void map(ValuePtr, ConnectionPtr);
  REMOVABLE_INLINE void unmap(ValuePtr);
  REMOVABLE_INLINE ConnectionPtr lookup(ValuePtr);

  typedef std::map<ValuePtr, ConnectionPtr, Value::Comparator> ConnectionMap;
  ConnectionMap _cmap;

  DECLARE_PRIVATE_ELEMENT_INITS
};
  
#endif /* __CUMULATIVEACK_H_ */
