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
#include "val_tuple.h"
#include "val_null.h"
#include "val_uint64.h"
#include "val_uint32.h"
#include "val_opaque.h"
#include "xdrbuf.h"
#include "netglobals.h"

Defrag::Defrag(string name)
  : Element(name, 1, 1),
    _pull_cb(0) 
{
}


int Defrag::push(int port, TuplePtr t, b_cbv cb)
{
  // Is this the right port?
  assert(port == 0);

  defragment(t);

  // Unblock the puller if one is waiting
  if (tuples_.size() > 0 && _pull_cb != 0) {
    ELEM_INFO( "push: wakeup puller");
    _pull_cb();
    _pull_cb = 0;
  }
  return 1;
}

TuplePtr Defrag::pull(int port, b_cbv cb) 
{
  // Is this the right port?
  assert(port == 0);

  // Do I have a fragment?
  if (tuples_.size() > 0) {
    ELEM_INFO( "pull: will succeed");
    TuplePtr t = tuples_.front();
    tuples_.pop_front();

    return t;
  } 
  else {
    // I don't have a tuple.  Do I have a pull callback already?
    if (_pull_cb == 0) {
      // Accept the callback
      ELEM_INFO( "pull: raincheck");
      _pull_cb = cb;
    } 
    else {
      // I already have a pull callback
      ELEM_INFO( "pull: underrun");
    }
    return TuplePtr();
  }
}

void Defrag::defragment(TuplePtr t)
{
  uint64_t seq_num = Val_UInt64::cast((*t)[SEQ]);
  unsigned offset  = Val_UInt32::cast((*t)[SEQ+1]);
  unsigned chunks  = Val_UInt32::cast((*t)[SEQ+2]);

  for (FragMap::iterator iter = fragments_.find(seq_num); 
       iter != fragments_.end(); iter++) {
    if (Val_UInt32::cast((*iter->second)[SEQ+1]) == offset) {
      ELEM_INFO( "defragment: duplicate offset");
      return;
    }
  }

  fragments_.insert(std::make_pair(seq_num, t));

  if (fragments_.count(seq_num) == chunks) {  
    // Put fragments back together
    FdbufPtr fb(new Fdbuf());
    for (unsigned i = 0; i < chunks; i++) {
      TuplePtr p;
      for (FragMap::iterator iter = fragments_.find(seq_num); 
           iter != fragments_.end(); iter++) {
        if (Val_UInt32::cast((*iter->second)[SEQ+1]) == i) {
          p = iter->second;
          fragments_.erase(iter);
          break;
        }
      }
      assert(p);
      FdbufPtr payload = Val_Opaque::cast((*p)[SEQ+3]);
      fb->push_bytes(payload->cstr(), payload->length());
    }

    // Unmarhsal and expand the packaged tuple
    XDR xd;
    xdrfdbuf_create(&xd, fb.get(), false, XDR_DECODE);
    TuplePtr unmarshal = Tuple::xdr_unmarshal(&xd);
    xdr_destroy(&xd);

    TuplePtr defraged = Tuple::mk();
    for (unsigned i = 0 ; i < t->size(); ) {
      if (i == SEQ) {
        defraged->append((*t)[i]);	// Add the sequence number
        for (unsigned j = 0; j < unmarshal->size(); j++) {
          defraged->append((*unmarshal)[j]);
        }
        i += 4;	// Move all the way pass the marshal field
      }
      else {
        defraged->append((*t)[i++]);
      }
    }
    tuples_.push_back(defraged); 
  }
}
