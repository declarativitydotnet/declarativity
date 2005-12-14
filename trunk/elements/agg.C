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
#include "agg.h"
#include "tuple.h"
#include "val_double.h"
#include<iostream>

Agg::Agg(str name, std::vector<unsigned int> groupByFields, 
	 int aggField, unsigned uniqueField, str aggStr)
  : Element(name, 1, 1), _pullCB(b_cbv_null), _pushCB(b_cbv_null)
{
  _aggField = aggField;
  _groupByFields = groupByFields;
  _uniqueField = uniqueField;
  _aggStr = aggStr;
}

Agg::~Agg()
{
}

void Agg::updateBestTuple(TupleRef p)
{
  log(LoggerI::INFO, 0, str(strbuf() << "Update best " << p->toString()));
  std::map<str, TupleRef>::iterator iter = _bestSoFar.find(getGroupByFields(p));
  if (iter != _bestSoFar.end()) {
    _bestSoFar.erase(iter);
  }
  _bestSoFar.insert(std::make_pair(getGroupByFields(p), p)); 

  iter = _buffer.find(getGroupByFields(p));
  if (iter != _buffer.end()) {
    _buffer.erase(iter);
  }
  
  _buffer.insert(std::make_pair(getGroupByFields(p), p));  

  log(LoggerI::INFO, 0, str(strbuf() << "Update best " << p->toString()));
  getGroupByFields(p) << "\n";
}

bool Agg::checkBestTuple(TupleRef p)
{
  std::map<str, TupleRef>::iterator iter = _bestSoFar.find(getGroupByFields(p));
  if (iter == _bestSoFar.end()) { // no best yet
    updateBestTuple(p);
    return true;
  } 

  TupleRef oldTuple = iter->second;  
  str oldUniqueVal = (*oldTuple)[_uniqueField]->toString();
  str newUniqueVal = (*p)[_uniqueField]->toString();
  if (oldUniqueVal == newUniqueVal) {
      p->toString() << " " << getGroupByFields(p) << "\n";  
    std::map<str, TupleRef>::iterator iter;
    TuplePtr bestTuple = NULL;
    for (iter = _allValues.begin(); iter != _allValues.end(); iter++) {
      TupleRef nextTuple = iter->second;
      if (getGroupByFields(p) != getGroupByFields(nextTuple)) {
	continue;
      }
      if (bestTuple == NULL) {
	bestTuple = nextTuple;
      } else {
	ValueRef oldValue = (*bestTuple)[_aggField];
	ValueRef newValue = (*nextTuple)[_aggField];
	if ((_aggStr == "min" && oldValue->compareTo(newValue) == 1) ||
	    (_aggStr == "max" && oldValue->compareTo(newValue) == -1)) {
	  bestTuple = nextTuple;
	}	
      }
    }
    assert(bestTuple != NULL);    
    updateBestTuple(bestTuple);
    return true;
  }

  ValueRef oldValue = (*oldTuple)[_aggField];
  ValueRef newValue = (*p)[_aggField];
  if ((_aggStr == "min" && oldValue->compareTo(newValue) == 1) ||
      (_aggStr == "max" && oldValue->compareTo(newValue) == -1)) {
    updateBestTuple(p);
      oldValue->toString() << " " << newValue->toString() << 
      " " << getGroupByFields(p) << "\n";  
    return true;
  }  
  return false;
}

str Agg::getGroupByFields(TupleRef p)
{
  strbuf b;
  for (unsigned int k = 0; k < _groupByFields.size(); k++) {
    b << (*p)[_groupByFields.at(k)]->toString() << ",";
  }
  return str(b);
}

int Agg::push(int port, TupleRef p, b_cbv cb)
{
  str uniqueVal = (*p)[_uniqueField]->toString();
  std::map<str, TupleRef>::iterator iter = _allValues.find(uniqueVal);
  if (iter != _allValues.end()) {
    _allValues.erase(iter);
  }
  _allValues.insert(std::make_pair(uniqueVal, p));

  bool changed = checkBestTuple(p);

  if (changed == true) {
    // we may need to wake up puller
    if (_pullCB != b_cbv_null) {
      _pullCB();
      _pullCB = b_cbv_null;
    } else {
      log(LoggerI::INFO, 0, "No pending pull callbacks");
    }
  }
  return 1; // always let a push succeed
}


/* pull. When pull, drain the queue. Do nothing if queue is empty but register callback. */
TuplePtr Agg::pull(int port, b_cbv cb)
{
  if (_buffer.size() == 0) { 
    log(LoggerI::INFO, 0, "Buffer is empty during pull");
    _pullCB = cb;
    return 0; 
  }
  
  TuplePtr p = _buffer.begin()->second;
  _buffer.erase(_buffer.begin());

  if (_pushCB != b_cbv_null) {
    _pushCB();
    _pushCB = b_cbv_null;
  }

  log(LoggerI::INFO, 0, str(strbuf() << "Pull succeed " << p->toString() 
			    << ", queuesize=" << _buffer.size()));
  return p;
}



