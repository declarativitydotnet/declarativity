// -*- c-basic-offset: 2; related-file-name: "print.h" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include <async.h>
#include <arpc.h>
#include <iostream>

#include "groupby.h"
#include "tuple.h"
#include "val_str.h"


const int GroupBy::MIN_AGG = 0;
const int GroupBy::MAX_AGG = 1;
const int GroupBy::AVG_AGG = 2;

GroupBy::GroupBy(str name, str newTableName, std::vector<int> primaryFields, std::vector<int> groupByFields, 
		 std::vector<int> aggFields, std::vector<int> aggTypes, 
		 double seconds): Element(name,1,1), 
				  _wakeupCB(wrap(this, &GroupBy::wakeup)),
				  _runTimerCB(wrap(this, &GroupBy::runTimer))
{
  _primaryFields = primaryFields;
  _groupByFields = groupByFields;
  _aggFields = aggFields;
  _aggTypes = aggTypes;
  _newTableName = newTableName;

  _seconds = (uint) floor(seconds);
  seconds -= _seconds;
  _nseconds = (uint) (seconds * 1000000000);

}

GroupBy::~GroupBy()
{
}

int GroupBy::initialize()
{
  log(LoggerI::INFO, 0, "initialize");
  // Schedule my timer
  _timeCallback = delaycb(_seconds,
                          _nseconds, _runTimerCB);

  return 0;
}


str GroupBy::getFieldStr(std::vector<int> fields, TupleRef p)
{
  strbuf fieldStr;
  for (int k = 0; k < fields.size(); k++) {
    ValuePtr key = (*p)[fields[k]];
    fieldStr << key->toString();
    if (k != (fields.size() - 1)) {
      fieldStr << ":";
    }
  }
  return fieldStr;
}

void GroupBy::computeAgg(str groupByStr, TupleRef newTuple)
{
  bool changed = false;
  
  _iterator = _aggValues.find(groupByStr);
 
  TupleRef newAggTuple = Tuple::mk();
  newAggTuple->append(Val_Str::mk(_newTableName));

  for (int k = 0; k < _groupByFields.size(); k++) {
    newAggTuple->append((*newTuple)[_groupByFields[k]]);
  }

  if (_iterator != _aggValues.end()) {
    TupleRef currentAgg = _iterator->second;
    
    // go through all the aggs, compute new one
    //std::cout << "Sizes "  << _aggFields.size() << " " << _aggTypes.size() << "\n";
    for (int k = 0; k < _aggFields.size(); k++) {
      ValueRef origVal = (*currentAgg)[k + 1 + _groupByFields.size()];
      ValueRef newVal = (*newTuple)[_aggFields[k]];
      // TOFIX: agg types
      if (_aggTypes[k] == MIN_AGG) {
	if (newVal->compareTo(origVal) < 0) {
	  // new value is smaller
	  origVal = newVal;
	  changed = true;
	}
      }

      if (_aggTypes[k] == MAX_AGG) {
	if (newVal->compareTo(origVal) > 0) {
	  // new value is bigger
	  origVal = newVal;
	  changed = true;
	}
      }
      newAggTuple->append(origVal);
    }
  } else {
    // the first time
    for (int k = 0; k < _aggFields.size(); k++) {
      newAggTuple->append((*newTuple)[_aggFields[k]]);
    }
    _pendingTuples.insert(std::make_pair(groupByStr, newAggTuple));
    _aggValues.insert(std::make_pair(groupByStr, newAggTuple));
  }

  if (changed == true) {
    _pendingTuples.insert(std::make_pair(groupByStr, newAggTuple));

    // update the agg in store
    _aggValues.erase(groupByStr);
    _aggValues.insert(std::make_pair(groupByStr, newAggTuple));
  } 
}

int GroupBy::push(int port, TupleRef p, cbv cb)
{  
  str indexStr = getFieldStr(_primaryFields, p);
  str groupByStr = getFieldStr(_groupByFields, p);

  // store the tuple. Replace by primary key if necessary. 
  for (_multiIterator = _tuples.lower_bound(groupByStr); _multiIterator != _tuples.upper_bound(groupByStr); _multiIterator++) {
    TupleRef t = _multiIterator->second;    
    if (getFieldStr(_primaryFields, t) == indexStr) {
      _tuples.erase(_multiIterator);
    }
  }
  _tuples.insert(std::make_pair(groupByStr, p));

  // compute new agg. Check if new tuple has changed the 
  // agg value. If so, add it to pending
  computeAgg(groupByStr, p);
  return 1;
}

void GroupBy::dumpTuples(str str)
{
  std::cout << "Key " << str << ": ";
  for (_multiIterator = _tuples.lower_bound(str); _multiIterator != _tuples.upper_bound(str); _multiIterator++) {
    TupleRef t = _multiIterator->second;    
    std::cout << t->toString() << ", ";
  }
  std::cout << "\n";
}

void GroupBy::dumpAggs(str str)
{
  std::cout << "Grouping Key " << str << ": ";
  for (_iterator = _aggValues.lower_bound(str); _iterator != _aggValues.upper_bound(str); _iterator++) {
    TupleRef t = _iterator->second;    
    std::cout << t->toString() << "\n";
  }
}

// periodically, push changes upsteam if possible until no more pending, 
// or upstream unable to accept more tuples
void GroupBy::runTimer()
{
  std::cout << "Flush pending " << _pendingTuples.size() << "\n";

  // remove the timer id
  _timeCallback = 0;

  // Attempt to push it
  for (_iterator = _pendingTuples.begin(); _iterator != _pendingTuples.end(); _iterator++) {
    TupleRef t = _iterator->second;
    int result = output(0)->push(t, _wakeupCB);
    if (result == 0) {
      // We have been pushed back.  Don't reschedule wakeup
      log(LoggerI::INFO, 0, "runTimer: sleeping");
      return;
    } else {
      std::cout << "Pushed " << t->toString() << "\n";
      _pendingTuples.erase(_iterator);
    }
  }

  // Reschedule me into the future
  //log(LoggerI::INFO, 0, "runTimer: rescheduling");
  _timeCallback = delaycb(_seconds,
			  _nseconds,
			  _runTimerCB);  
}

void GroupBy::wakeup()
{
  // I'd better not be already scheduled
  assert(_timeCallback == 0);

  log(LoggerI::INFO, 0, "wakeup");

  // Okey dokey.  Reschedule me into the future
  _timeCallback = delaycb(_seconds,
                          _nseconds,
                          _runTimerCB);
}

