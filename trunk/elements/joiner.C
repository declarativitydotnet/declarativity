// -*- c-basic-offset: 2; related-file-name: "joiner.h" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 */

#include "joiner.h"
#include "store.h"              // For the end-of-search tag

Joiner::Joiner(string name)
  : Element(name, 2, 1),
    _pushCallback(0),
    _pullCallback(0)
{
}

int Joiner::push(int port, TuplePtr t, b_cbv cb)
{
  // Is this the right port?
  assert(port == 1);
  
  // Do I have a fixed tuple pending?
  if (_fixed == NULL) {
    // No pending tuple.  Take it in
    assert(!_pushCallback);
    _fixed = t;

    // Do I have a puller pending?
    string s("push: accepted fixed tuple ");
    s.append(_fixed->toString());
    log(LoggerI::INFO, 0, s);
    
    // Unblock the puller if one is waiting
    if (_pullCallback) {
      log(LoggerI::INFO, 0, "push: wakeup puller");
      _pullCallback();
      _pullCallback = 0;
    }

    // And stop the pusher since we have to wait until all joins on this
    // fixed tuple are done
    _pushCallback = cb;
    return 0;
  } else {
    // I'm psarry. I can't help ya.
    // XXX We might have to resynchronize the streams here.
    assert(_pushCallback);
    log(LoggerI::WARN, 0, "push: fixed tuple overrun");
    return 0;
  }
}

TuplePtr Joiner::pull(int port, b_cbv cb) 
{
  // Is this the right port?
  assert(port == 0);

  // Do I have a pending fixed tuple?
  if (_fixed == NULL) {
    // Nope, no pending join.  Deal with underruns.

    if (!_pullCallback) {
      // Accept the callback
      log(LoggerI::INFO, 0, "pull: raincheck");
      _pullCallback = cb;
    } else {
      // I already have a pull callback
      log(LoggerI::INFO, 0, "pull: callback underrun");
    }
    return TuplePtr();
  } else {
    // OK, flow through me is enabled.  Now pretend to be a 1-in-1-out
    // element
    assert(!_pullCallback);

    while (1) {
      TuplePtr p = input(0)->pull(cb);
      if (p) {
        TuplePtr result = mergeTuples(p);
        if (result != 0) {
          return result;
        }
      } else {
        // Didn't get any tuples from my input. Fail.
        return TuplePtr();
      }
    }
  }
}


TuplePtr Joiner::mergeTuples(TuplePtr p)
{
  // XXX Merge tuples indiscriminately for now.
  assert(_fixed != NULL);

  // Keep it around because we might clean out the fixed tuple state.
  TuplePtr myFixed = _fixed;

  // Is this the end of the current joining?
  if (p->tag(Store::END_OF_SEARCH) != NULL) {
    // Yup, this fetched tuple is tagged.  Consume both inputs
    log(LoggerI::INFO,
        -1,
        "mergeTuples: End of search found.");
    _fixed.reset();

    // ... and wake up any push waiters.
    if (_pushCallback) {
      _pushCallback();
      _pushCallback = 0;
    }
  }
    
  // Is my newly received tuple empty (because this was an empty
  // search?)
  if (p->size() == 0) {
    // I'd better have just finished this joining!
    assert(_fixed == NULL);

    // Okey, return nothing then.
    return TuplePtr();
  } else {
    // I've got a real pair to report. Merge them.
    TuplePtr newTuple = Tuple::mk();

    // Append the fixed
    newTuple->concat(myFixed);
    // and the fetched
    newTuple->concat(p);
    // and prepare it for shipping
    newTuple->freeze();

    return newTuple;
  }
}
