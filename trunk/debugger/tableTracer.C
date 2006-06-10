/*
 * @(#)$Id$
 *
 * Copyright (c) 2005 Intel Corporation. All rights reserved.
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 */


#include "tableTracer.h"

TableTracer::TableTracer(string tableName,
                         Table2::Key& key,
                         size_t maxSize,
                         boost::posix_time::time_duration& lifetime)
  : Table2(tableName, key, maxSize, lifetime),
    _e(new TableTracerElement(tableName + ".tracer", 0, 1, this))
{
}

  
TableTracer::TableTracer(string tableName,
                         Table2::Key& key,
                         size_t maxSize,
                         string lifetime)
  : Table2(tableName, key, maxSize, lifetime),
    _e(new TableTracerElement(tableName + ".tracer", 0, 1, this))
{
}
  

TableTracer::TableTracer(string tableName,
                         Table2::Key& key,
                         size_t maxSize)
  : Table2(tableName, key, maxSize),
    _e(new TableTracerElement(tableName + ".tracer", 0, 1, this))
{
}
  
  
TableTracer::TableTracer(string tableName,
                         Table2::Key& key)
  : Table2(tableName, key),
    _e(new TableTracerElement(tableName + ".tracer", 0, 1, this))
{
}
  
  
TableTracer::~TableTracer()
{
}


Table2::Iterator
TableTracer::lookup(Key& lookupKey, Key& indexKey, TuplePtr t)
{
  // Form a trace tuple 
  TuplePtr theTuple = Tuple::mk();
  
  return Table2::lookup(lookupKey, indexKey, t);
}


Table2::Iterator
TableTracer::lookup(Key& indexKey, TuplePtr t)
{
  // Form a trace tuple 
  TuplePtr theTuple = Tuple::mk();
  
  return Table2::lookup(indexKey, t);
}


TableTracer::TableTracerElement::TableTracerElement(string instanceName,
                                                    unsigned ninputs,
                                                    unsigned noutputs,
                                                    TableTracer* tt)
  : Element(instanceName, ninputs, noutputs),
    _tt(tt)
{
}
  

ElementPtr
TableTracer::getElementPtr()
{
  return _e;
}
