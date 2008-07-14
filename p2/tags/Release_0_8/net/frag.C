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

#include "frag.h"
#include "val_tuple.h"
#include "val_null.h"
#include "val_uint64.h"
#include "val_uint32.h"
#include "val_opaque.h"
#include "math.h"
#include "xdrbuf.h"
#include "netglobals.h"

Frag::Frag(string name, unsigned bs, unsigned mqs)
  : Element(name, 1, 1),
    _push_cb(0), _pull_cb(0),
    block_size_(bs), max_queue_size_(mqs)
{
  assert (bs % 4 == 0);
}


int Frag::push(int port, TuplePtr t, b_cbv cb)
{
  // Is this the right port?
  assert(port == 0);

  if (_push_cb != 0) {
    // Complain and do nothing
    ELEM_INFO( "push: callback overrun"); 
  } 

  ELEM_INFO( "push: put accepted");
  fragment(t);

  // Unblock the puller if one is waiting
  if (fragments_.size() > 0 && _pull_cb != 0) {
    ELEM_INFO( "push: wakeup puller");
    _pull_cb();
    _pull_cb = 0;
  }

  if (fragments_.size() >= max_queue_size_) {
    _push_cb = cb;
    return 0;
  }
  return 1;
}

TuplePtr Frag::pull(int port, b_cbv cb) 
{
  // Is this the right port?
  assert(port == 0);

  // Do I have a fragment?
  if (fragments_.size() > 0) {
    ELEM_INFO( "pull: will succeed");
    TuplePtr t = fragments_.front();
    fragments_.pop_front();

    if (fragments_.size() < max_queue_size_ && 
        _push_cb != 0) {
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
  TuplePtr payload = Tuple::mk();
  for (unsigned i = SEQ+1; i < t->size(); i++) {
    payload->append((*t)[i]);
  }
  payload->freeze();

  FdbufPtr payload_fb(new Fdbuf());
  XDR xe;
  xdrfdbuf_create(&xe, payload_fb.get(), false, XDR_ENCODE);
  payload->xdr_marshal(&xe);
  xdr_destroy(&xe);

  unsigned num_chunks = 
    static_cast<unsigned>(ceil((double)payload_fb->length() / block_size_)); 
  for(uint32_t bn=0; payload_fb->length(); bn++) {
    TuplePtr p = Tuple::mk();
    // Get everything up to and including the sequence number
    for (unsigned i = 0; i <= SEQ; i++) {
      p->append((*t)[i]);
    }

    FdbufPtr fb(new Fdbuf());
    payload_fb->pop_to_fdbuf(*fb, block_size_);
    p->append(Val_UInt32::mk(bn));		// Block Number
    p->append(Val_UInt32::mk(num_chunks));	// Total blocks
    p->append(Val_Opaque::mk(fb));		// The payload
    p->freeze();
    fragments_.push_back(p);
  }
}
