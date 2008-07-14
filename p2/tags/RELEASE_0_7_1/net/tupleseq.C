// -*- cr-basic-offset: 2; related-file-name: "tupleseq.h" -*-
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

#include <iostream>
#include "tupleseq.h"
#include "val_uint64.h"
#include "val_uint32.h"
#include "val_str.h"
#include "val_tuple.h"

#define MAX_INIT_SEQ 2048

Sequence::Sequence(string n, SeqNum b, unsigned s, int d)
  : Element(n,1,1), seq_(b), seq_field_(s), dest_field_(d)
{
  if (!seq_) seq_ = SeqNum(rand() * MAX_INIT_SEQ);
}


TuplePtr Sequence::simple_action(TuplePtr p)
{
  SeqNum next_seq = seq_;

  if (dest_field_ < 0) {
    next_seq = seq_++;
  }
  else {
    std::map<ValuePtr, SeqNum>::iterator i = seq_nums_.find((*p)[dest_field_]);
    if (i != seq_nums_.end()) {
      next_seq = i->second + 1;
    }
    seq_nums_[(*p)[dest_field_]] = next_seq;
  }

  TuplePtr tp = Tuple::mk();
  for (unsigned i = 0; i < seq_field_; i++) {
    tp->append((*p)[i]);
  }
  tp->append(Val_UInt64::mk(next_seq));
  for (unsigned i = seq_field_; i < p->size(); i++) {
    tp->append((*p)[i]);
  }
  tp->freeze();
  return tp;
}
