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
#include "val_str.h"

Aggwrap::Aggwrap(str name, str aggfn, int aggfield, str outputTableName)
  : Element(name, 2, 2),
    _aggfn(aggfn), 
    _aggfield(aggfield), 
    inner_accepting(true),
    ext_in_cb(b_cbv_null), 
    ext_out_cb(b_cbv_null)
{
  numJoins = 0;
  _outputTableName = outputTableName;
  if (aggfn == "min") {
    log(LoggerI::INFO, 0, str(strbuf() << "min aggregation " << _aggfield));
  } else if (aggfn == "max") {
    log(LoggerI::INFO, 0, str(strbuf() << "max aggregation " << _aggfield));
  } else if (aggfn == "count") { 
    log(LoggerI::INFO, 0, str(strbuf() << "count aggregation " << _aggfield));
  } else {
    log(LoggerI::INFO, 0, "HELP: Don't understand agg function '" 
	<< aggfn << "'");
  }
}

//
// A new tuple is coming in from outside. 
//
int Aggwrap::push(int port, TupleRef t, b_cbv cb)
{
  log(LoggerI::INFO, 0, str(strbuf() << " Push: " << port << "," << t->toString()));

  // if receive the next one when previous has not finished, then keep in queue?

  // Is this the right port?
  switch (port) {
  case 0:
    ext_in_cb = cb;
    switch(aggState) {
    case 0:  // Waiting
      log(LoggerI::INFO, 0, 
	  str(strbuf() << " received a tuple from outside!" 
	      << t->toString()));
      assert(inner_accepting);
      agg_init();
      _incomingTuple = t;
      inner_accepting = output(1)->push(t, boost::bind(&Aggwrap::int_push_cb, this));
      break;
    case 1:
      log(LoggerI::INFO, 0, "FAIL: pushed when processing!");
      // Discard tuple
      break;
    case 2:
      log(LoggerI::INFO, 0, "FAIL: pushed when done pending");
      break;
    default:
      log(LoggerI::INFO, 0, str(strbuf() << "FAIL: weird state " 
				<< aggState));
    }     
    log(LoggerI::INFO, 0, str(strbuf() << " Block downstream"));
    return 0;
  case 1:
    switch(aggState) {
    case 0:  // Waiting
      log(LoggerI::INFO, 0, 
	  "OOPS: unexpected result tuple when in state waiting");
      break;
    case 1:
      agg_accum(t);
      break;
    case 2:
      log(LoggerI::INFO, 0, "FAIL: pushed when done pending");
      break;
    default:
      log(LoggerI::INFO, 0, str(strbuf() << "FAIL: weird state " 
				<< aggState));
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
  log(LoggerI::INFO, 
      0, str(strbuf() 
	     << "Callback from inner graph on successful push" << aggState));
  if (aggState == 0 && ext_in_cb) {
    log(LoggerI::INFO, 0, str(strbuf() << "Invoke ext_in_cb"));
    ext_in_cb();
    ext_in_cb = b_cbv_null;
  }
}


//
// Completion callback
//
void Aggwrap::comp_cb(int jnum)
{
  TRC_FN;
  log(LoggerI::INFO, 0, str(strbuf() << "Join " << jnum << " completed."));
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
  log(LoggerI::INFO, 0, str(strbuf() << "Joins so far: " << numJoins + 1));
  return boost::bind(&Aggwrap::comp_cb, this, numJoins++);
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
    aggResult = t;
    log(LoggerI::INFO, 0, str(strbuf() << "After Agg accumulation: " 
			      << aggResult->toString()));
    return;
  } 

  if ( aggResult == NULL ) {
    aggResult = t;
    log(LoggerI::INFO, 0, str(strbuf() << "After Agg accumulation: " 
			      << aggResult->toString()));
    return;
  }

  log(LoggerI::INFO, 0, str(strbuf() << "Before Agg accumulation: " 
			    << aggResult->toString()));
  int cr = (*t)[_aggfield]->compareTo((*aggResult)[_aggfield]);
  if ((cr == -1 && _aggfn == "min") || (cr == 1 && _aggfn == "max")) {
    aggResult = t;
  }
  log(LoggerI::INFO, 0, str(strbuf() << "After Agg accumulation: " 
			    << aggResult->toString() << cr 
			    << " " << (*t)[_aggfield]->toString() << " " 
			    << (*aggResult)[_aggfield]->toString()));
}

void Aggwrap::agg_finalize() {
  TRC_FN; 
  if (_aggfn == "count") {       
    if (_incomingTuple) {
      TupleRef aggResultToRet = Tuple::mk();
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
    log(LoggerI::INFO, 0, 
	str(strbuf() << " finalize: Pushing tuple" << aggResult->toString()));
    output(0)->push(aggResult, b_cbv_null);
  } else {
    log(LoggerI::INFO, 0, "Finalize: Alas, nothing to push");
  }
  if (ext_in_cb) {
    log(LoggerI::INFO, 0, "Invoke push callback for more tuples");
    // accept new tuples to be pushed in via outer regardless of any outputs
    ext_in_cb(); 
    ext_in_cb = b_cbv_null;
  }
  aggState = 0;
}

void Aggwrap::registerGroupbyField(int field)
{ 
  _groupByFields.push_back(field); 
  warn << " Register group by" << field << "\n";
}
