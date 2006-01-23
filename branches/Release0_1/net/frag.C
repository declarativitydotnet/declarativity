// -*- c-basic-offset: 2; related-file-name: "frag.h" -*-
/*
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include "frag.h"
#include "val_uint64.h"
#include "val_uint32.h"
#include "val_opaque.h"
#include "tupleseq.h"
#include "math.h"

Frag::Frag(string name, uint32_t block_size)
  : Element(name, 1, 1),
    _push_cb(0), _pull_cb(0),
    block_size_(block_size)
{
}


int Frag::push(int port, TuplePtr t, b_cbv cb)
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

  // Do I have fragments?
  if (fragments_.size() > 0) {
    // Nope, accept and fragment the tuple
    ELEM_INFO( "push: put accepted");
    fragment(t);

    // Unblock the puller if one is waiting
    if (_pull_cb != 0) {
      ELEM_INFO( "push: wakeup puller");
      _pull_cb();
      _pull_cb = 0;
    }
  } else {
    // I already have a tuple so the one I just accepted is dropped
    ELEM_INFO( "push: tuple overrun");
  }
  return 0;
}

TuplePtr Frag::pull(int port, b_cbv cb) 
{
  // Is this the right port?
  assert(port == 0);

  // Do I have a fragment?
  if (!fragments_.empty()) {
    ELEM_INFO( "pull: will succeed");
    TuplePtr t = fragments_.front();
    fragments_.pop_front();

    if (fragments_.empty() && _push_cb != 0) {
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

void Frag::fragment(TuplePtr t)
{
  uint64_t seq_num = Val_UInt64::cast((*t)[SEQ_FIELD]);
  FdbufPtr pfb = Val_Opaque::cast((*t)[PLD_FIELD]);

  if (pfb->length() <= CHUNK_SIZE) {
    // No need to fragment.
    t->tag(NUM_CHUNKS, Val_UInt32::mk(1));
    fragments_.push_back(t);
  }
  else {
    size_t num_chunks = (pfb->length() + CHUNK_SIZE - 1) / CHUNK_SIZE; 
    size_t offset = 0;
    while( pfb->length() ) {
      TuplePtr p = Tuple::mk();
      p->append(Val_UInt64::mk(seq_num));
      FdbufPtr fb(new Fdbuf());
      offset += pfb->pop_to_fdbuf(*fb, CHUNK_SIZE);
      p->append(Val_Opaque::mk(fb));
      p->tag(NUM_CHUNKS, Val_UInt32::mk(num_chunks));
      fragments_.push_back(p);
    }
  }
}
