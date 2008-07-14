// -*- c-basic-offset: 2; related-file-name: "aggwrap.h" -*-
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

#include "aggwrap.h"
#include "trace.h"
#include "val_int32.h"
#include "val_str.h"
#include "loop.h"

Aggwrap::Aggwrap(string name, string aggfn, int aggfield, string outputTableName)
  : Element(name, 2, 2),
    _aggfn(aggfn), 
    _aggfield(aggfield), 
    inner_accepting(true),
    ext_in_cb(0), 
    ext_out_cb(0)
{
  numJoins = 0;
  _outputTableName = outputTableName;
  ostringstream os;
  if (aggfn == "MIN") {
    os << "min aggregation " << _aggfield;
  } else if (aggfn == "MAX") {
    os << "max aggregation " << _aggfield;
  } else if (aggfn == "COUNT") { 
    os << "count aggregation " << _aggfield;
  } else {
    os << "HELP: Don't understand agg function '" << aggfn << "'";
  }
  ELEM_INFO( os.str() );
}

//
// A new tuple is coming in from outside. 
//
int Aggwrap::push(int port, TuplePtr t, b_cbv cb)
{
  ostringstream os;
  os << " Push: " << port << "," << t->toString();
  ELEM_INFO( os.str() );

  // if receive the next one when previous has not finished, then keep in queue?

  // Is this the right port?
  switch (port) {
  case 0:
    ext_in_cb = cb;
    switch(aggState) {
    case 0:  // Waiting
      ELEM_INFO( " received a tuple from outside!" + t->toString());
      assert(inner_accepting);
      agg_init();
      _incomingTuple = t;
      inner_accepting = output(1)->push(t, boost::bind( &Aggwrap::int_push_cb, this ));
      break;
    case 1:
      ELEM_INFO( "FAIL: pushed when processing!");
      // Discard tuple
      break;
    case 2:
      ELEM_INFO( "FAIL: pushed when done pending");
      break;
    default:
      ostringstream os;
      os << "FAIL: weird state " << aggState;
      ELEM_INFO( os.str() );
    }     
    ELEM_INFO( " Block downstream");
    return 0;
  case 1:
    switch(aggState) {
    case 0:  // Waiting
      ELEM_INFO( 
	  "OOPS: unexpected result tuple when in state waiting");
      break;
    case 1:
      agg_accum(t);
      break;
    case 2:
      ELEM_INFO( "FAIL: pushed when done pending");
      break;
    default:
      ostringstream os; 
      os << "FAIL: weird state " << aggState;
      ELEM_INFO( os.str());
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
  ostringstream os;
  os << "Callback from inner graph on successful push" << aggState;
  ELEM_INFO( os.str());
  if (aggState == 0 && ext_in_cb) {
    ELEM_INFO( "Invoke ext_in_cb");
    ext_in_cb();
    ext_in_cb = 0;
  }
}


//
// Completion callback
//
void Aggwrap::comp_cb(int jnum)
{
  TRC_FN;
  ostringstream os;
  os << "Join " << jnum << " completed.";
  ELEM_INFO( os.str());
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
b_cbv Aggwrap::get_comp_cb()
{
  ostringstream os;
  os << "Joins so far: " << numJoins + 1;
  ELEM_INFO( os.str());
  return boost::bind(&Aggwrap::comp_cb,this,numJoins++);
}

//
// Finally, the aggregation functions
//
void Aggwrap::agg_init() {
  TRC_FN;
  curJoin = -1;
  aggState = 1;
  if ( _aggfn == "COUNT") {
    count = 0;
  } else {
    aggResult.reset();
  }
}

void Aggwrap::agg_accum(TuplePtr t) {
  TRC_FN;
  if ( _aggfn == "COUNT") {
    count++;
    aggResult = t;
    ELEM_INFO( "After Agg accumulation: " + aggResult->toString());
    return;
  } 

  if ( aggResult == NULL ) {
    aggResult = t;
    ELEM_INFO( "After Agg accumulation: " + aggResult->toString());
    return;
  }

  ELEM_INFO( "Before Agg accumulation: " + aggResult->toString());
  int cr = (*t)[_aggfield]->compareTo((*aggResult)[_aggfield]);
  if ((cr == -1 && _aggfn == "MIN") || (cr == 1 && _aggfn == "MAX")) {
    aggResult = t;
  }
  ELEM_INFO( "After Agg accumulation: " + aggResult->toString()
      + "\n " + (*t)[_aggfield]->toString() + " " 
      + (*aggResult)[_aggfield]->toString());
}

void Aggwrap::agg_finalize() {
  TRC_FN; 
  if (_aggfn == "COUNT") {       
    if (_incomingTuple) {
      TuplePtr aggResultToRet = Tuple::mk();
      aggResultToRet->append(Val_Str::mk(_outputTableName));
      for (uint k = 0; k < _groupByFields.size(); k++) {
	/*warn << _incomingTuple->toString() << " " 
	  << _groupByFields.at(k) + 1 << "\n";     */
	aggResultToRet->append((*_incomingTuple)[_groupByFields.at(k)+1]);
      }
      aggResultToRet->append(Val_Int32::mk(count));
      aggResultToRet->freeze();
      
      aggResult = Tuple::mk();
      aggResult->concat(aggResultToRet);
      aggResult->freeze();     
    }
  }
  if (aggResult) {
    ELEM_INFO( " finalize: Pushing tuple" + aggResult->toString());
    output(0)->push(aggResult, 0);
  } else {
    ELEM_INFO( "Finalize: Alas, nothing to push");
  }
  if (ext_in_cb) {
    ELEM_INFO( "Invoke push callback for more tuples");
    // accept new tuples to be pushed in via outer regardless of any outputs
    ext_in_cb(); 
    ext_in_cb = 0;
  }
  aggState = 0;
}

void Aggwrap::registerGroupbyField(int field)
{ 
  _groupByFields.push_back(field); 
  warn << "Register group by " << field << "\n";
}
