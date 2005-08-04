// -*- c-basic-offset: 2; related-file-name: "aggwrap.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Element that "wraps" a dataflow graph and performs one
 * aggegate calculation for each input tuple. 
 * Inputs:  0, push: incoming event tuples
 *          1, push: output from the inner graph
 * Outputs: 0, push: aggregates (external output)
 *          1, push: push events to the inner graph
 *
 */

#ifndef __AGGWRAP_H__
#define __AGGWRAP_H__

#include "element.h"

class Aggwrap : public Element { 
public:

  Aggwrap(str aggfn, int aggfield, str outputTableName);

  const char *class_name() const		{ return "Aggwrap";}
  const char *processing() const		{ return "hh/hh"; }; //"ha/hh"; }
  const char *flow_code() const			{ return "--/--"; }

  /** Receive a new fixed tuple */
  int push(int port, TupleRef, cbv cb);

  void registerGroupbyField(int field); 
  void comp_cb(int jnum);
  cbv get_comp_cb();
  int numJoins;

private:
  str _aggfn;
  int _aggfield;
  std::vector<int> _groupByFields;
    
  int curJoin;
  
  bool inner_accepting;

  // External callbacks
  cbv ext_in_cb;
  cbv ext_out_cb;

  int aggState;
  TuplePtr aggResult, _incomingTuple;
  int count;
  str _outputTableName;

  void int_push_cb();
  void agg_init();
  void agg_accum(TupleRef t);
  void agg_finalize();
    
};

#endif /* __AGGWRAP_H_ */
