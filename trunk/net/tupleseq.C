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

TupleSeq::Package::Package(str name, u_int64_t start_seq)
  : Element(name,1, 1), seq_(start_seq)
{
}


TuplePtr TupleSeq::Package::simple_action(TupleRef p)
{
  TupleRef n = Tuple::mk();

  n->append(Val_UInt64::mk(MAKE_SEQ(seq_++, 0)));
  n->append(p);
  n->freeze();

  return n;
}

TupleSeq::Unpackage::Unpackage(str name) 
  : Element(name, 1, 1)
{
}

TuplePtr TupleSeq::Unpackage::simple_action(TupleRef p)
{
  TupleRef t = Tuple::mk();

  // Project sequence number, assumed field 0
  for (size_t i = 1; i < p->size(); i++) {
    t->append((*p)[i]);
  }

  return t;
}
