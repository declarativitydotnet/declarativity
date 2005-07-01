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

Sequence::Sequence(str name, u_int64_t start_seq)
  : Element(name,1, 1), seq_(start_seq)
{
}


TuplePtr Sequence::simple_action(TupleRef p)
{
  TupleRef n = Tuple::mk();

  n->append(Val_UInt64::mk(MAKE_SEQ(seq_++, 0)));
  n->concat(p);
  n->freeze();

  return n;
}
