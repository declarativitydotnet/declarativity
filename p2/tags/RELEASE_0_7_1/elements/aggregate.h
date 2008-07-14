// -*- c-basic-offset: 2; related-file-name: "aggregate.C" -*-
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
 * The continuous aggregate element.  It has a single pull output.
 * Whenever the aggregate changes, it allows itself to be pulled.
 * 
 */

#ifndef __AGGREGATEELEMENT_H__
#define __AGGREGATEELEMENT_H__

#include "table2.h"
#include "element.h"

class Aggregate : public Element {
 public:
  Aggregate(string name,
            Table2::Aggregate aggregate);

  
  const char*
  class_name() const {return "Aggregate";}


  const char* 
  processing() const {return "/l";}


  const char*
  flow_code() const {return "/-";}
  

  /** Return an updated aggregate. */
  TuplePtr
  pull(int port, b_cbv cb);



  
 private:
  /** My aggregate */
  Table2::Aggregate _aggregate;


  /** My latest aggregate */
  TuplePtr _latest;


  /** My listener method */
  void listener(TuplePtr t);


  /** My puller's callback */
  b_cbv _pullCallback;


  /** Is the latest pending transmission? */
  bool _pending;
};


#endif /* __AGGREGATEELEMENT_H_ */
