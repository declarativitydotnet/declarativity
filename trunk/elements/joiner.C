// -*- c-basic-offset: 2; related-file-name: "joiner.h" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include "joiner.h"
#include "store.h"              // For the end-of-search tag

Joiner::Joiner(str name)
  : Element(name, 2, 1),
    _fixed(0),
    _pushCallback(cbv_null),
    _pullCallback(cbv_null)
{
}

int Joiner::push(int port, TupleRef t, cbv cb)
{
  // Is this the right port?
  assert(port == 1);
  
  // Do I have a fixed tuple pending?
  if (_fixed == NULL) {
    // No pending tuple.  Take it in
    assert(_pushCallback == cbv_null);
    _fixed = t;

    // Do I have a puller pending?
    strbuf s("push: accepted fixed tuple ");
    s.cat(_fixed->toString());
    log(LoggerI::INFO, 0, s);
    
    // Unblock the puller if one is waiting
    if (_pullCallback != cbv_null) {
      log(LoggerI::INFO, 0, "push: wakeup puller");
      _pullCallback();
      _pullCallback = cbv_null;
    }

    // And stop the pusher since we have to wait until all joins on this
    // fixed tuple are done
    _pushCallback = cb;
    return 0;
  } else {
    // I'm psarry. I can't help ya.
    // XXX We might have to resynchronize the streams here.
    assert(_pushCallback != cbv_null);
    log(LoggerI::WARN, 0, "push: fixed tuple overrun");
    return 0;
  }
}

TuplePtr Joiner::pull(int port, cbv cb) 
{
  // Is this the right port?
  assert(port == 0);

  // Do I have a pending fixed tuple?
  if (_fixed == NULL) {
    // Nope, no pending join.  Deal with underruns.

    if (_pullCallback == cbv_null) {
      // Accept the callback
      log(LoggerI::INFO, 0, "pull: raincheck");
      _pullCallback = cb;
    } else {
      // I already have a pull callback
      log(LoggerI::INFO, 0, "pull: callback underrun");
    }
    return 0;
  } else {
    // OK, flow through me is enabled.  Now pretend to be a 1-in-1-out
    // element
    assert(_pullCallback == cbv_null);

    while (1) {
      TuplePtr p = input(0)->pull(cb);
      if (p) {
        TuplePtr result = mergeTuples(p);
        if (result != 0) {
          return result;
        } else {
          // This input yielded no result. Try again.
          log(LoggerI::WARN,
              -1,
              "pull: Fetched tuple did not match fixed one! Trying again.");
        }
      } else {
        // Didn't get any tuples from my input. Fail.
        return 0;
      }
    }
  }
}


TuplePtr Joiner::mergeTuples(TupleRef p)
{
  // XXX Merge tuples indiscriminately for now.
  assert(_fixed != NULL);

  // Keep it around because we might clean out the fixed tuple state.
  TupleRef myFixed = _fixed;

  // Is this the end of the current joining?
  if (p->tag(Store::END_OF_SEARCH) != NULL) {
    // Yup, this fetched tuple is tagged.  Consume both inputs
    log(LoggerI::INFO,
        -1,
        "mergeTuples: End of search found.");
    _fixed = NULL;

    // ... and wake up any push waiters.
    if (_pushCallback != cbv_null) {
      _pushCallback();
      _pushCallback = cbv_null;
    }
  }
    
  // Is my newly received tuple empty (because this was an empty
  // search?)
  if (p->size() == 0) {
    // I'd better have just finished this joining!
    assert(_fixed == NULL);

    // Okey, return nothing then.
    return NULL;
  } else {
    // I've got a real pair to report. Merge them.
    TupleRef newTuple = Tuple::mk();

    // Append the fixed
    newTuple->append(myFixed);
    // and the fetched
    newTuple->append(p);
    // and prepare it for shipping
    newTuple->freeze();

    return newTuple;
  }
}
