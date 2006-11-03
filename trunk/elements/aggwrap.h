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
 *
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

  /** The constructor takes the name of the element instance, the name
      of the aggregation function applied, the field number on which to
      apply the aggregation function with regards to the expected input
      schema received from the inner graph on input 1, and the name of
      the exported tuple */
  Aggwrap(string name, string aggfn, int aggfield, string outputTableName);


  /** What's my class? */
  const char *class_name() const		{ return "Aggwrap";}


  /** What's my processing personality? Push all around. */
  const char *processing() const		{ return "hh/hh"; };


  /** Any flow recommendations? None since no ports are agnostic. */
  const char *flow_code() const			{ return "--/--"; }


  /** Receive a new fixed tuple */
  int push(int port, TuplePtr, b_cbv cb);



  void registerGroupbyField(int field); 


  void comp_cb(int jnum);


  b_cbv get_comp_cb();


  int numJoins;

private:
  /** My aggregation function name */
  string _aggfn;

  
  /** The aggregated field number of my internal inputs */
  int _aggfield;


  /** The group-by fields numbers on my internal inputs */
  std::vector<int> _groupByFields;
    

  /** The current join I'm expecting a conclusion from */
  int curJoin;
  

  /** Am I still accepting inner tuples? */
  bool inner_accepting;


  /** External callbacks from my external input. Call it when I'm ready
      to take more push.  */
  b_cbv ext_in_cb;


  /** My aggregation state */
  int aggState;


  /** My current aggregate result */
  TuplePtr aggResult;


  /** The latest (external) input tuple that triggered an aggregation */
  TuplePtr _incomingTuple;


  
  int count;


  /** The name of tuples I produce */
  string _outputTableName;


  /** My internal push callback */
  void int_push_cb();


  /** Start aggregation internally */
  void agg_init();


  /** Process an internal input */
  void agg_accum(TuplePtr t);


  /** Finalize my aggregation */
  void agg_finalize();
    
};

#endif /* __AGGWRAP_H_ */
