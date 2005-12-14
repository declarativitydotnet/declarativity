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

Frag::Frag(str name, uint32_t block_size)
  : Element(name, 1, 1),
    _push_cb(b_cbv_null), _pull_cb(b_cbv_null),
    block_size_(block_size)
{
}


int Frag::push(int port, TupleRef t, b_cbv cb)
{
  // Is this the right port?
  assert(port == 0);

  if (_push_cb != b_cbv_null) {
    // Complain and do nothing
    log(LoggerI::INFO, 0, "push: callback overrun"); 
  } else {
    // Accept the callback
    _push_cb = cb;
    log(LoggerI::INFO, 0, "push: raincheck");
  }

  // Do I have fragments?
  if (fragments_.size() > 0) {
    // Nope, accept and fragment the tuple
    log(LoggerI::INFO, 0, "push: put accepted");
    fragment(t);

    // Unblock the puller if one is waiting
    if (_pull_cb != b_cbv_null) {
      log(LoggerI::INFO, 0, "push: wakeup puller");
      _pull_cb();
      _pull_cb = b_cbv_null;
    }
  } else {
    // I already have a tuple so the one I just accepted is dropped
    log(LoggerI::INFO, 0, "push: tuple overrun");
  }
  return 0;
}

TuplePtr Frag::pull(int port, b_cbv cb) 
{
  // Is this the right port?
  assert(port == 0);

  // Do I have a fragment?
  if (!fragments_.empty()) {
    log(LoggerI::INFO, 0, "pull: will succeed");
    TuplePtr t = fragments_.front();
    fragments_.pop_front();

    if (fragments_.empty() && _push_cb != b_cbv_null) {
      log(LoggerI::INFO, 0, "pull: wakeup pusher");
      _push_cb();
      _push_cb = b_cbv_null;
    }

    return t;
  } else {
    // I don't have a tuple.  Do I have a pull callback already?
    if (_pull_cb == b_cbv_null) {
      // Accept the callback
      log(LoggerI::INFO, 0, "pull: raincheck");
      _pull_cb = cb;
    } else {
      // I already have a pull callback
      log(LoggerI::INFO, 0, "pull: underrun");
    }
    return NULL;
  }
}

void Frag::fragment(TupleRef t)
{
  uint64_t seq_num = SEQ_NUM(Val_UInt64::cast((*t)[SEQ_FIELD]));
  ref< suio > puio = Val_Opaque::cast((*t)[PLD_FIELD]);

  if (puio->resid() <= CHUNK_SIZE) {
    // No need to fragment.
    t->tag(NUM_CHUNKS, Val_UInt32::mk(1));
    fragments_.push_back(t);
  }
  else {
    int num_chunks = (int) ceil(float(puio->resid()) / float(CHUNK_SIZE));
    for (int offset = 0; puio->resid() > 0; offset++) {
      ref<suio> uio = New refcounted<suio>();
      puio->copy(uio, min<int>(CHUNK_SIZE, puio->resid()));
      puio->rembytes(uio->resid());

      TupleRef p = Tuple::mk();
      p->append(Val_UInt64::mk(MAKE_SEQ(seq_num, offset)));
      p->append(Val_Opaque::mk(uio));
      p->tag(NUM_CHUNKS, Val_UInt32::mk(num_chunks));
      fragments_.push_back(p);
    }
  }
}
