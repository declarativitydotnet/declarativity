// -*- c-basic-offset: 2; related-file-name: "tupleSource.C" -*-
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
 * DESCRIPTION: Element that produces the same tuple whenever pulled.
 * It never blocks.  A specialization of FunctorSource.
 */


#ifndef __TUPLE_SOURCE_H__
#define __TUPLE_SOURCE_H__

#include <element.h>
#include <boost/shared_ptr.hpp>
#include "functorSource.h"

class TupleSource : public FunctorSource { 
public:
  
  struct TupleGenerator : public FunctorSource::Generator
  {
  private:
    /** My tuple */
    TuplePtr _tuple;
    
  public:
    TupleGenerator(TuplePtr tuple) : _tuple(tuple) { }

    TuplePtr operator()() {
      return _tuple;
    }
  };

  TupleSource(string, TuplePtr);

  const char *class_name() const		{ return "TupleSource"; }
  const char *flow_code() const			{ return "/-"; }
  const char *processing() const		{ return "/l"; }

private:
  /** My generator */
  boost::shared_ptr<TupleGenerator> _tupleGenerator;
};

#endif /* __TUPLE_SOURCE_H_ */
