// -*- c-basic-offset: 2; related-file-name: "frag.h" -*-
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

#include "defrag.h"
#include "frag.h"
#include "val_uint64.h"
#include "val_uint32.h"
#include "val_opaque.h"
#include "tupleseq.h"

#define PLD_FIELD 1

Defrag::Defrag(string name)
  : Element(name, 1, 1),
    _push_cb(0), _pull_cb(0)
{
}


int Defrag::push(int port, TuplePtr t, b_cbv cb)
{
  // Is this the right port?
  assert(port == 0);

  if (_push_cb != 0) {
    // Complain and do nothing
    ELEM_INFO( "push: callback overrun"); 
  } else {
    // Accept the callback
    _push_cb = cb;
    ELEM_INFO( "push: raincheck");
  }

  ELEM_INFO( "push: put accepted");
  defragment(t);

  // Unblock the puller if one is waiting
  if (_pull_cb != 0) {
    ELEM_INFO( "push: wakeup puller");
    _pull_cb();
    _pull_cb = 0;
  }
  return 0;
}

TuplePtr Defrag::pull(int port, b_cbv cb) 
{
  // Is this the right port?
  assert(port == 0);

  // Do I have a fragment?
  if (!tuples_.empty()) {
    ELEM_INFO( "pull: will succeed");
    TuplePtr t = tuples_.front();
    tuples_.pop_front();

    if (tuples_.empty() && _push_cb != 0) {
      ELEM_INFO( "pull: wakeup pusher");
      _push_cb();
      _push_cb = 0;
    }
    return t;
  } else {
    // I don't have a tuple.  Do I have a pull callback already?
    if (_pull_cb == 0) {
      // Accept the callback
      ELEM_INFO( "pull: raincheck");
      _pull_cb = cb;
    } else {
      // I already have a pull callback
      ELEM_INFO( "pull: underrun");
    }
    return TuplePtr();
  }
}

void Defrag::defragment(TuplePtr t)
{
  uint64_t seq_num = Val_UInt64::cast((*t)[SEQ_FIELD]);
  int      offset  = Val_UInt64::cast((*t)[SEQ_FIELD]);
  uint32_t chunks  = Val_UInt32::cast(t->tag(NUM_CHUNKS));

  if (chunks == 1) {
    tuples_.push_back(t);
  }
  else {
    for (FragMap::iterator iter = fragments_.find(seq_num); iter != fragments_.end(); iter++) {
      if (Val_UInt64::cast((*iter->second)[SEQ_FIELD]) == (uint64_t) offset) {
        ELEM_INFO( "defragment: duplicate offset");
        return;
      }
    }

    fragments_.insert(std::make_pair(seq_num, t));

    if (fragments_.count(seq_num) == chunks) {  
      // Put fragments back together
      FdbufPtr fb(new Fdbuf());
      for (uint64_t i = 0;
           i < chunks;
           i++) {
        TuplePtr p;
        for (FragMap::iterator iter = fragments_.find(seq_num); iter != fragments_.end(); iter++) {
          if (Val_UInt64::cast((*iter->second)[SEQ_FIELD]) == i) {
            p = iter->second;
            fragments_.erase(iter);
            break;
          }
        }
        assert(p);
        FdbufPtr payload = Val_Opaque::cast((*p)[PLD_FIELD]);
        fb->push_bytes(payload->cstr(), payload->length());
      }
      TuplePtr defraged = Tuple::mk();
      defraged->append(Val_UInt64::mk(seq_num));
      defraged->append(Val_Opaque::mk(fb));
      tuples_.push_back(defraged); 
    }
  }
}
