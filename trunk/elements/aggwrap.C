// -*- c-basic-offset: 2; related-file-name: "aggwrap.h" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include "aggwrap.h"
#include "trace.h"
#include "val_int32.h"

Aggwrap::Aggwrap(str aggfn, int aggfield)
  : Element(strbuf() << "Aggwrap<" << aggfn << ">", 2, 2),
    _aggfn(aggfn), _aggfield(aggfield),
    inner_accepting(true),
    ext_in_cb(cbv_null), 
    ext_out_cb(cbv_null)
{
  numJoins = 0;
  if (aggfn == "min") {
    TRC("min aggregation");
  } else if (aggfn == "max") {
    TRC("max aggregation");
  } else if (aggfn == "count") { 
    TRC("count aggregation");
  } else {
    DBG("HELP: Don't understand agg function '" << aggfn << "'");
  }
}

//
// A new tuple is coming in from outside. 
//
int Aggwrap::push(int port, TupleRef t, cbv cb)
{
  // Is this the right port?
  switch (port) {
  case 0:
    ext_in_cb = cb;
    switch(aggState) {
    case 0:  // Waiting
      TRC("Got a tuple from outside!");
      assert(inner_accepting);
      agg_init();
      inner_accepting = output(1)->push(t, wrap(this, &Aggwrap::int_push_cb));
      break;
    case 1:
      DBG("FAIL: pushed when processing!");
      // Discard tuple
      break;
    case 2:
      DBG("FAIL: pushed when done pending");
      break;
    default:
      DBG("FAIL: weird state " << aggState);
    } 
    return 0;
  case 1:
    switch(aggState) {
    case 0:  // Waiting
      DBG("OOPS: unexpected result tuple when in state waiting");
      break;
    case 1:
      agg_accum(t);
      break;
    case 2:
      DBG("FAIL: pushed when done pending");
      break;
    default:
      DBG("FAIL: weird state " << aggState);
    } 
    return 1;
  default:
    assert(port < 2);
    return  0;
  }

}

// 
// Callback from the inner graph when we can push more tuples. 
// 
void Aggwrap::int_push_cb()
{
  TRC_FN;
  inner_accepting = true;
  if (aggState == 0 && ext_in_cb) {
    ext_in_cb();
  }
}

//
// From the inner graph, schedule a callback to pull all the new
// aggregate stuff. 
void Aggwrap::int_pull_cb()
{
  TRC_FN;
  delaycb(0, wrap(this,&Aggwrap::pull_everything));
}
//
// Called from inner pull cb when we can pull more tuples
//
void Aggwrap::pull_everything()
{
  TRC_FN;
  //  Naughty: pull everything we can.
  TuplePtr t;
  do {
    t = input(1)->pull( wrap(this, &Aggwrap::int_pull_cb ));
    switch(aggState) {
    case 0: // Waiting to start
      DBG("Got inner tuples before starting");
      // Discard tuple
      break;
    case 1: // Aggregating
      if (t) {
	agg_accum(t);
      }
      break;
    case 2: // Done aggregating
      DBG("Ooops - got tuple after aggregation is finished.");
      break;
    default:
      DBG("Ooops: Badd aggState of " << aggState);
    }
  } while(t);
}

//
// Completion callback
//
void Aggwrap::comp_cb(int jnum)
{
  TRC_FN;
  TRC("Join " << jnum << " completed.");
  if (curJoin < jnum) { 
    curJoin = jnum;
  }
  
  if (curJoin >= numJoins - 1) {
    // Presumably, we're now done.
    agg_finalize();
  }
}

//
// Getting hold of a closure for the above
//
cbv Aggwrap::get_comp_cb()
{
  TRC("Joins so far: " << numJoins+1);
  return wrap(this,&Aggwrap::comp_cb,numJoins++);
}

//
// Finally, the aggregation functions
//
void Aggwrap::agg_init() {
  TRC_FN;
  curJoin = -1;
  aggState = 1;
  if ( _aggfn == "count") {
    count = 0;
  } else {
    aggResult = NULL;
  }
}

void Aggwrap::agg_accum(TupleRef t) {
  TRC_FN;
  if ( _aggfn == "count") {
    count++;
    return;
  } 

  if ( aggResult == NULL ) {
    aggResult = t;
    return;
  }

  int cr = (*t)[_aggfield]->compareTo((*aggResult)[_aggfield]);
  if ((cr == -1 && _aggfn == "min") || (cr==1 && _aggfn == "max")) {
    aggResult = t;
  }
}

void Aggwrap::agg_finalize() {
  TRC_FN;
  if (_aggfn == "count") {
    aggResult = Tuple::mk();
    aggResult->append(Val_Int32::mk(count));
    aggResult->freeze();
  }
  if (aggResult) {
    TRC("Pushing tuple");
    output(0)->push(aggResult, cbv_null);
    if (ext_out_cb) { 
      ext_out_cb();
    }
  } else {
    TRC("Alas, nothing to push");
  }
  aggState = 0;
}
