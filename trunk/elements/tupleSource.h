// -*- c-basic-offset: 2; related-file-name: "tupleSource.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Element that produces the same tuple whenever pulled.
 * It never blocks.  A specialization of FunctorSource.
 */


#ifndef __TUPLE_SOURCE_H__
#define __TUPLE_SOURCE_H__

#include <element.h>
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

    virtual ~TupleGenerator() {};
    
    TuplePtr operator()() {
      return _tuple;
    }
  };

  TupleSource(str, TuplePtr);
  virtual ~TupleSource();

  const char *class_name() const		{ return "TupleSource"; }
  const char *flow_code() const			{ return "/-"; }
  const char *processing() const		{ return "/l"; }

private:
  /** My generator */
  TupleGenerator* _tupleGenerator;
};

#endif /* __TUPLE_SOURCE_H_ */
