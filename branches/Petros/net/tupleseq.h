// -*- c-basic-offset: 2; related-file-name: "element.C" -*-
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

#ifndef __TupleSeq_H__
#define __TupleSeq_H__

#include <map>
#include "element.h"
#include "elementRegistry.h"
#include "val_uint64.h"

typedef uint64_t SeqNum;

class Sequence : public Element {
public:
  Sequence(string name="sequence", SeqNum start=0);
  Sequence(TuplePtr args);
  const char *class_name() const { return "Sequence";};
  const char *processing() const { return "a/a"; };
  const char *flow_code() const	 { return "x/x"; };

  TuplePtr simple_action(TuplePtr p);

  DECLARE_PUBLIC_ELEMENT_INITS

private:
  SeqNum _start_seq;

  typedef std::map<ValuePtr, SeqNum, Value::Comparator> ValueSeqMap;
  ValueSeqMap _seq_map;

  DECLARE_PRIVATE_ELEMENT_INITS
};

#endif /* __TupleSeq_H_ */
