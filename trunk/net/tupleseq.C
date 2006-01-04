// -*- cr-basic-offset: 2; related-file-name: "tupleseq.h" -*-
/*
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include <iostream>
#include "tupleseq.h"
#include "val_uint64.h"
#include "val_uint32.h"
#include "val_str.h"
#include "val_tuple.h"

#define MAX_INIT_SEQ 2048

Sequence::Sequence(string n, string src, uint32_t p, uint64_t s)
  : Element(n,1,1), src_(src), port_(p), seq_(s)
{
  if (!seq_) seq_ = uint64_t(rand() * MAX_INIT_SEQ);
}


TuplePtr Sequence::simple_action(TuplePtr p)
{
  TuplePtr n = Tuple::mk();
  TuplePtr s = Tuple::mk();

  s->append(Val_Str::mk("SEQ"));
  s->append(Val_UInt64::mk(seq_++));
  s->append(Val_Str::mk(src_));
  s->append(Val_UInt32::mk(port_));
  s->freeze();

  n->append(Val_Tuple::mk(s));
  for (uint i = 0; i < p->size(); i++)
    if (!isSeq((*p)[i])) n->append((*p)[i]); 
  n->freeze();
  return n;
}

REMOVABLE_INLINE bool Sequence::isSeq(ValuePtr vp) {
  try {
    TuplePtr t = Val_Tuple::cast(vp);
    if (Val_Str::cast((*t)[0]) == "SEQ") return true;
  }
  catch (Value::TypeError& e) { } 
  return false;
}
