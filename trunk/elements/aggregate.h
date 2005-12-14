// -*- c-basic-offset: 2; related-file-name: "aggregate.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached
 * INTEL-LICENSE file.  If you do not find these files, copies can be
 * found by writing to: Intel Research Berkeley, 2150 Shattuck Avenue,
 * Suite 1300, Berkeley, CA, 94704.  Attention: Intel License Inquiry.
 *
 * The continuous aggregate element.  It has a single pull output.
 * Whenever the aggregate changes, it allows itself to be pulled.
 * 
 */

#ifndef __AGGREGATEELEMENT_H__
#define __AGGREGATEELEMENT_H__

#include "table.h"
#include "element.h"
#include "async.h"

class Aggregate : public Element {
 public:
  Aggregate(str name,
            Table::MultAggregate aggregate);
  
  const char *class_name() const		{ return "Aggregate";}
  const char *processing() const		{ return "/l"; }
  const char *flow_code() const			{ return "/-"; }
  
  /** Return an updated aggregate. */
  TuplePtr pull(int port, b_cbv cb);
  
 private:
  /** My aggregate */
  Table::MultAggregate _aggregate;

  /** My latest aggregate */
  TuplePtr _latest;

  /** My listener method */
  void listener(TupleRef t);

  /** My puller's callback */
  b_cbv _pullCallback;

  /** Is the latest pending transmission? */
  bool _pending;
};


#endif /* __AGGREGATEELEMENT_H_ */
