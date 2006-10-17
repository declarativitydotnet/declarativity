// -*- c-basic-offset: 2; related-file-name: "aggwrap.C" -*-
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

  Aggwrap(string name,
          string aggfn,
          int aggfield,
          string outputTableName);

  const char *class_name() const		{ return "Aggwrap";}
  const char *processing() const		{ return "hh/hh"; }; //"ha/hh"; }
  const char *flow_code() const			{ return "--/--"; }

  /** Receive a new fixed tuple */
  int push(int port, TuplePtr, b_cbv cb);

  void registerGroupbyField(int field); 
  void comp_cb(int jnum);
  b_cbv get_comp_cb();
  int numJoins;

private:
  string _aggfn;
  int _aggfield;
  std::vector<int> _groupByFields;
    
  int curJoin;
  
  bool inner_accepting;

  // External callbacks
  b_cbv ext_in_cb;
  b_cbv ext_out_cb;

  int aggState;
  TuplePtr aggResult, _incomingTuple;
  int count;
  string _outputTableName;

  void int_push_cb();
  void agg_init();
  void agg_accum(TuplePtr t);
  void agg_finalize();
    
};

#endif /* __AGGWRAP_H_ */
