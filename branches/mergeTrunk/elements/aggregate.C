// -*- c-basic-offset: 2; related-file-name: "aggregate.h" -*-
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
 *
 */

#include "aggregate.h"
#include "val_str.h"
#include "val_int64.h"
#include "val_list.h"
#include "plumber.h"
#include <boost/bind.hpp>

Aggregate::Aggregate(string name,
                     CommonTable::Aggregate aggregate)
  : Element(name, 0, 1),
    _aggregate(aggregate),
    _pullCallback(0),
    _pending(false)
{
  // Place myself as a listener on the aggregate
  _aggregate->listener(boost::bind(&Aggregate::listener, this, _1));
}

/**
 * Generic constructor.
 * Arguments:
 * 2. Val_Str:    Element Name.
 * 3. Val_Str:    Table Name.
 * 4. Val_Str:    Aggregation function name.
 * 5. Val_UInt32: Aggregation field postion.
 * 6. Val_List:   Group by attributes.
 */
Aggregate::Aggregate(TuplePtr args)
  : Element(Val_Str::cast((*args)[2]), 0, 1),
    _pullCallback(0),
    _pending(false)
{
  string  tableName = Val_Str::cast((*args)[3]);
  string  function  = Val_Str::cast((*args)[4]);
  uint    fieldNo   = Val_Int64::cast((*args)[5]);
  ListPtr groupBy   = Val_List::cast((*args)[6]);
    
  CommonTablePtr table = Plumber::catalog()->table(tableName); 
  CommonTable::Key groupByKey;
  for (ValPtrList::const_iterator iter = groupBy->begin();
       iter != groupBy->end(); iter++)
   groupByKey.push_back(Val_Int64::cast(*iter));
  _aggregate = table->aggregate(groupByKey, fieldNo, function);
  _aggregate->listener(boost::bind(&Aggregate::listener, this, _1));
}

void
Aggregate::listener(TuplePtr t)
{
  if (_latest == NULL) {
    _latest = t;
    _pending = true;
  } else {
    if (_latest->compareTo(t) != 0) {
      // This is fresh and different
      _latest = t;
      _pending = true;
    } else {
      // Same old same old. Do nothing.  Don't reset pending, though, in
      // case the previous update is still pending.
      return;
    }
  }

  // If there's a pull callback, call it
  if (_pullCallback) {
    ELEM_INFO("listener: wakeup puller");
    _pullCallback();
    _pullCallback = 0;
  }
}

TuplePtr
Aggregate::pull(int port, b_cbv cb) 
{
  // Is this the right port?
  assert(port == 0);

  // Do I have a pending update?
  if (!_pending) {
    // Nope, no pending update.  Deal with underruns.
    if (!_pullCallback) {
      // Accept the callback
      ELEM_INFO("pull: raincheck");
      _pullCallback = cb;
    } else {
      // I already have a pull callback
      ELEM_INFO("pull: callback underrun");
    }
    return TuplePtr();
  } else {
    // I'd better have no callback pending and definitely a value
    assert(!_pullCallback);
    assert(_latest != NULL);

    // No longer pending
    _pending = false;

    // Return the latest
    return _latest;
  }
}


// This is necessary for the class to register itself with the
// stage registry.
DEFINE_ELEMENT_INITS(Aggregate,"Aggregate")
