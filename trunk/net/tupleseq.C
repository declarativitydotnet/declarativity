// -*- cr-basic-offset: 2; related-file-name: "tupleseq.h" -*-
/*
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */


#include "tupleseq.h"
#include "val_int64.h"
#include "val_str.h"
#include "val_tuple.h"

#define MAX_INIT_SEQ 2048

Sequence::Sequence(str n, str c, uint64_t s)
  : Element(n,1,1), cid_(c), seq_(s)
{
  if (!seq_) seq_ = uint64_t(rand() * MAX_INIT_SEQ);
}


TuplePtr Sequence::simple_action(TupleRef p)
{
  TupleRef n = Tuple::mk();
  TupleRef s = Tuple::mk();

  s->append(Val_Str::mk("SEQ"));
  s->append(Val_UInt64::mk(seq_++));
  if (cid_ != "") 
    s->append(Val_Str::mk(cid_));
  s->freeze();

  n->append(Val_Tuple::mk(s));
  for (uint i = 0; i < p->size(); i++)
    n->append((*p)[i]); 
  n->freeze();

  return n;
}
