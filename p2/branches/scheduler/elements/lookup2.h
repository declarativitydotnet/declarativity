// -*- c-basic-offset: 2; related-file-name: "lookup2.C" -*-
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
 * The lookup element for the new table structure.  It has a single
 * input where a lookup tuple arrives. The search of the table is
 * performed according to the index (if any) held for the table's
 * indexKey.  The lookup tuple is matched according to a 1-1 mapping
 * from its fields described by the lookupKey to the table's field
 * described by the index key (as per the definition of
 * Table2::lookup()).  On the output all matches for the lookup are
 * returned with successive pulls.  Outputs consist of tuples that
 * contain two embedded tuples, the first holding the lookup tuple and
 * the second holding the result if any.  The last output tuple to be
 * returned for a given search is empty (i.e., contains no fields).  If
 * a lookup yields no results, its only output tuple is the empty tuple.
 *
 * A lookup constructor may optionally take a state change callback,
 * which is called with true every time a lookup becomes busy (i.e.,
 * acquires a new probe tuple) and with false every time it becomes free
 * (i.e., it gives out its last result).
 */

#ifndef __LOOKUP2_H__
#define __LOOKUP2_H__

#include "element.h"
#include "elementRegistry.h"
#include "commonTable.h"
#include "val_tuple.h"
#include "val_null.h"

class Lookup2 : public Element {
public:
  Lookup2(string name,
          CommonTablePtr table,
          CommonTable::Key lookupKey,
          CommonTable::Key indexKey,
          b_cbv state_cb = 0);

  Lookup2(TuplePtr args);

  ~Lookup2();
  
  
  const char *class_name() const		{ return "Lookup2";}
  const char *processing() const		{ return "h/l"; }
  const char *flow_code() const			{ return "-/-"; }
  

  /** Receive a new lookup tuple */
  int
  push(int port, TuplePtr, b_cbv cb);

  
  /** Return a match to the current lookup */
  TuplePtr
  pull(int port, b_cbv cb);


  DECLARE_PUBLIC_ELEMENT_INITS

private:
  int initialize();

  /** My table */
  CommonTablePtr _table;
  

  /** My pusher's callback */
  b_cbv _pushCallback;

  
  /** My puller's callback */
  b_cbv _pullCallback;

  
  /** My state callback */
  b_cbv _stateCallback;


  /** My current lookup tuple */
  TuplePtr _lookupTuple;

  
  /** My encapsulated lookup tuple for all results */
  ValuePtr _lookupTupleValue;

  
  /** My current iterator */
  CommonTable::Iterator _iterator;


  /** My lookup key */
  CommonTable::Key _lookupKey;
  
  
  /** My index key */
  CommonTable::Key _indexKey;


  /** Are my lookup key and index key different?  If so, I need to
      project before matching (from the lookup tuple to the schema of
      the looked-up table). Otherwise I don't need to project. */
  bool _project;

  DECLARE_PRIVATE_ELEMENT_INITS
};

#endif /* __LOOKUP2_H_ */
