// -*- c-basic-offset: 2; related-file-name: "rangeLookup.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached
 * LICENSE file.  If you do not find this file, copies can be
 * found by writing to: Intel Research Berkeley, 2150 Shattuck Avenue,
 * Suite 1300, Berkeley, CA, 94704.  Attention: Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 *
 * Modified from lookup2 element.
 * The range lookup element for the new table structure. 
 * The lookup/index key are used for specifying the 1D range attr.
 * 
 * On the output all matches for the lookup are
 * returned with successive pulls.  Outputs consist of tuples that
 * contain two embedded tuples, the first holding the lookup tuple and
 * the second holding the result if any.  The last output tuple to be
 * returned for a given search is tagged with END_OF_SEARCH.  If a
 * lookup yields no results, an output tuple with the lookup tuple as
 * the first value and an empty tuple as the second value is returned
 * (also tagged with END_OF_SEARCH).
 */

#ifndef __RANGE_LOOKUP_H__
#define __RANGE_LOOKUP_H__

#include "element.h"
#include "elementRegistry.h"
#include "commonTable.h"
#include "value.h"
#include "val_tuple.h"
#include "val_null.h"
#include "iStateful.h"

class RangeLookup : public Element {
public:

  /**
	Table: 		the table to be selected & joined
	lKey, rKey: 	the attrs on the incoming tuple 
			to perform 1D range select
			EMPTY keys would be treated as Infinity
	indexKey: 	the attr positions on the table
	openL/openR:	denotes if the left/right boundary is open/close
  */
  RangeLookup(string name,
          CommonTablePtr table,
          CommonTable::Key lKey,
	  CommonTable::Key rKey,
          CommonTable::Key indexKey,
	  bool openL,
 	  bool openR,
          b_cbv completion_cb = 0);

  RangeLookup(TuplePtr);

  ~RangeLookup();
  
  
  const char *class_name() const { return "RangeLookup";}
  const char *processing() const { return "h/l"; }
  const char *flow_code() const	 { return "-/-"; }

  /** Receive a new lookup tuple */
  int
  push(int port, TuplePtr, b_cbv cb);

  
  /** Return a match to the current lookup */
  TuplePtr
  pull(int port, b_cbv cb);

  
  /** The END_OF_SEARCH tuple tag. */
  static string END_OF_SEARCH;
  



  DECLARE_PUBLIC_ELEMENT_INITS

private:
  int initialize();

  /** My stateful proxy */
  IStatefulPtr _stateProxy;

  /** My table */
  CommonTablePtr _table;
  

  /** My pusher's callback */
  b_cbv _pushCallback;

  
  /** My puller's callback */
  b_cbv _pullCallback;

  
  /** My completion callback */
  b_cbv _compCallback;

  
  /** My current lookup tuple */
  TuplePtr _lookupTuple;

  
  /** My encapsulated lookup tuple for all results */
  ValuePtr _lookupTupleValue;

  
  /** My current iterator */
  CommonTable::Iterator _iterator;


  /** My index key */
  CommonTable::Key _indexKey;

  /** My range search key**/
  CommonTable::Key _lKey,_rKey;

  bool _openL, _openR;


  DECLARE_PRIVATE_ELEMENT_INITS
};

#endif /* __LOOKUP2_H_ */
