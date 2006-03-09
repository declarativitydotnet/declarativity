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

#include "element.h"
#include "val_uint64.h"

#define SEQ_FIELD 1

class Sequence : public Element {
public:
  Sequence(string n="Sequence", uint64_t s=1);
  const char *class_name() const { return "Sequence";};
  const char *processing() const { return "a/a"; };
  const char *flow_code() const	 { return "-/-"; };

  TuplePtr simple_action(TuplePtr p);

private:
  REMOVABLE_INLINE bool isSeq(ValuePtr);

  uint64_t seq_;
};

#endif /* __TupleSeq_H_ */
