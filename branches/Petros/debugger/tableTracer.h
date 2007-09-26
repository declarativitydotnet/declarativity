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
 * DESCRIPTION: Element that wraps around a table and exports a stream
 * of the operations that the table performs, presumably to log or
 * otherwise utilize.
 *
 * It appears as a subclass of Table and Element. It passes through all
 * table operations.  As an element it has 0 inputs and a single push
 * output.
 */


#ifndef __TABLETRACER_H__
#define __TABLETRACER_H__

#include "element.h"
#include "table2.h"
#include "val_tuple.h"

class TableTracer : public Table2
{
public:
  TableTracer(string tableName,
              Key& key,
              uint32_t maxSize,
              boost::posix_time::time_duration& lifetime);

  
  TableTracer(string tableName,
              Key& key,
              uint32_t maxSize,
              string lifetime);
  

  TableTracer(string tableName,
              Key& key,
              uint32_t maxSize);
  
  
  TableTracer(string tableName,
              Key& key);
  
  
  ~TableTracer();
  
  
  /** Lookup overload. Pushes out all requests. */
  Iterator
  lookup(Key& lookupKey, Key& indexKey, TuplePtr t);


  /** Lookup overload. Pushes out all requests. */
  Iterator
  lookup(Key& indexKey, TuplePtr t);
  

  /** My element interface */
  class TableTracerElement : public Element
  {
  public:
    /** My typical element constructor */
    TableTracerElement(string instanceName,
                       unsigned ninputs,
                       unsigned noutputs,
                       TableTracer* tt);

    const char*
    class_name() const {return "TableTracerElement";}
    
    
    const char*
    processing() const {return "/h";}
    
    
    const char*
    flow_code() const {return "/-"; }
    
    
  private:
    /** My containing table tracer */
    TableTracer* _tt;
  };


  /** Fetch my element face */
  ElementPtr
  getElementPtr();




private:
  
  /** My actual element */
  ElementPtr _e;
};

#endif /* __TABLETRACER_H_ */
