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
#ifdef WIN32
#include "p2_win32.h"
#endif // WIN32
#include "tupleseq.h"
#include "val_uint64.h"
#include "val_uint32.h"
#include "val_str.h"
#include "val_tuple.h"
#include "netglobals.h"

#define MAX_INIT_SEQ 2048

Sequence::Sequence(string name, SeqNum start)
  : Element(name,1,1), _start_seq(start) { }


TuplePtr Sequence::simple_action(TuplePtr p)
{
  SeqNum seq = _start_seq;

  ValueSeqMap::iterator iter = _seq_map.find((*p)[DEST]);
  if (iter != _seq_map.end()) {
    seq = iter->second + 1;
  }
  else {
    seq = SeqNum(((double)rand()/RAND_MAX) * MAX_INIT_SEQ);
    _seq_map.insert(std::make_pair((*p)[DEST], seq));
  }
  _seq_map[(*p)[DEST]] = seq;

  TuplePtr tp = Tuple::mk();
  for (unsigned i = 0; i < p->size(); i++) {
    if (i == SEQ) {
      tp->append(Val_UInt64::mk(seq));
    }
    else {
      tp->append((*p)[i]);
    }
  }
  tp->freeze();
  return tp;
}
