/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include "agg.h"
#include "tuple.h"
#include "val_double.h"
#include<iostream>

Agg::Agg(string name, std::vector<unsigned int> groupByFields, 
	 int aggField, unsigned uniqueField, string aggStr)
  : Element(name, 1, 1),
    _pullCB(0),
    _pushCB(0)
{
  _aggField = aggField;
  _groupByFields = groupByFields;
  _uniqueField = uniqueField;
  _aggStr = aggStr;
}

Agg::~Agg()
{
}

void Agg::updateBestTuple(TuplePtr p)
{
  log(LoggerI::INFO, 0, "Update best " + p->toString());
  std::map<string, TuplePtr>::iterator iter = _bestSoFar.find(getGroupByFields(p));
  if (iter != _bestSoFar.end()) {
    _bestSoFar.erase(iter);
  }
  _bestSoFar.insert(std::make_pair(getGroupByFields(p), p)); 

  iter = _buffer.find(getGroupByFields(p));
  if (iter != _buffer.end()) {
    _buffer.erase(iter);
  }
  
  _buffer.insert(std::make_pair(getGroupByFields(p), p));  

  log(LoggerI::INFO, 0, "Update best " + p->toString());
}

bool Agg::checkBestTuple(TuplePtr p)
{
  std::map<string, TuplePtr>::iterator iter = _bestSoFar.find(getGroupByFields(p));
  if (iter == _bestSoFar.end()) { // no best yet
    updateBestTuple(p);
    return true;
  } 

  TuplePtr oldTuple = iter->second;  
  string oldUniqueVal = (*oldTuple)[_uniqueField]->toString();
  string newUniqueVal = (*p)[_uniqueField]->toString();
  if (oldUniqueVal == newUniqueVal) {
    std::map<string, TuplePtr>::iterator iter;
    TuplePtr bestTuple;
    for (iter = _allValues.begin(); iter != _allValues.end(); iter++) {
      TuplePtr nextTuple = iter->second;
      if (getGroupByFields(p) != getGroupByFields(nextTuple)) {
	continue;
      }
      if (bestTuple == NULL) {
	bestTuple = nextTuple;
      } else {
	ValuePtr oldValue = (*bestTuple)[_aggField];
	ValuePtr newValue = (*nextTuple)[_aggField];
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

  ValuePtr oldValue = (*oldTuple)[_aggField];
  ValuePtr newValue = (*p)[_aggField];
  if ((_aggStr == "min" && oldValue->compareTo(newValue) == 1) ||
      (_aggStr == "max" && oldValue->compareTo(newValue) == -1)) {
    updateBestTuple(p);
    return true;
  }  
  return false;
}

string Agg::getGroupByFields(TuplePtr p)
{
  ostringstream b;
  for (unsigned int k = 0; k < _groupByFields.size(); k++) {
    b << (*p)[_groupByFields.at(k)]->toString() << ",";
  }
  return b.str();
}

int Agg::push(int port, TuplePtr p, b_cbv cb)
{
  string uniqueVal = (*p)[_uniqueField]->toString();
  std::map<string, TuplePtr>::iterator iter = _allValues.find(uniqueVal);
  if (iter != _allValues.end()) {
    _allValues.erase(iter);
  }
  _allValues.insert(std::make_pair(uniqueVal, p));

  bool changed = checkBestTuple(p);

  if (changed == true) {
    // we may need to wake up puller
    if (_pullCB) {
      _pullCB();
      _pullCB = 0;
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
    return TuplePtr(); 
  }
  
  TuplePtr p = _buffer.begin()->second;
  _buffer.erase(_buffer.begin());

  if (_pushCB) {
    _pushCB();
    _pushCB = 0;
  }

  ostringstream oss;
  oss << "Pull succeed " << p->toString() << ", queuesize=" << _buffer.size();
  log(LoggerI::INFO, 0, oss.str());
  return p;
}



