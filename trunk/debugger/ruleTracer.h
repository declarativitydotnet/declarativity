/*
 * @(#)$Id$
 *
 * DESCRIPTION: For each rule, we have a ruleTracer
 * element. It has k input and 0 output port. Each input
 * port is associated with the output port #1 of different
 * Tap elements present in a rule strand.
 *
 * This element stores the different tuples coming from
 * different stages of a rule strand and when a rule execution
 * finishes, it pushes out ruleExecTuple and tupleTableTuple.
 * It handles pipelining of rule execution.
 *
 */

#ifndef __RULETRACER_H__
#define __RULETRACER_H__

#include "element.h"
#include "execRecord.h"
#include "table.h"
#include "val_uint32.h"
#include "val_str.h"

class RuleTracer : public Element {
 public:

  RuleTracer(string name, string ruleName, string node, 
	     int startPort, int endPort, int ruleNum,
	     TablePtr ruleExec, TablePtr tupleTable);
  
  ~RuleTracer();

  
  
  /** Overridden to perform handling of intermediate tuples
   *  for a rule strand
   **/

  int push(int port, TuplePtr t, b_cbv cb);

  const char *class_name() const		{ return "RuleTracer";}
  const char *processing() const		{ return "a/a"; }
  const char *flow_code() const			{ return "-/-"; }

 private:
  /** the port which will represent the input to a rule */
  int _startPort;

  /** the port which will represent the output of a rule */
  int _endPort;
  
  /** rule number */
  int _ruleNum;

  /** rule name given by the overlog program, multiple rules
      may have similar names erroneously */
  string _ruleName;

  /** local node ID */
  string _node;

  /** Multiple records containing the execution state of each one */
  ExecRecord * _records[ExecRecord::MAX_STAGES];

  /** table to store the ruleExecTable tuples */
  TablePtr _ruleExecTable;

  /** table to store the tupleTable tuples */
  TablePtr _tupleTable;

  // helper functions
  /** finds the record depending on the port number */
  int findRecordIndex(int port);

  /** handles a new execution record */
  void startRule(TuplePtr, int);

  /** handles a exec record finishing */
  void endRule(TuplePtr, int);

  /** handles a precondition for a record */
  void appendRule(TuplePtr, int);

  /** sets the local node for a tuple */
  void setLocalNode(TuplePtr);
  
  string getNode(){ return _node;};

  /** enable/disable the rule tracing of debugging rules 
      themselves. Use SKIP flag to do that. */
  bool skip(TuplePtr );

  /** creates a tuple for the ruleExec table from a record */
  TuplePtr createExecTuple(ExecRecord *e);

  /** creates multiple tuples for the ruleExec table, 
      depending on the number of preconditios */
  int createExecTupleUsingPrecond(ExecRecord *,
				       std::vector<TuplePtr> *);

  /** creates a tuple for the tupleTable for a given tuple */
  TuplePtr createTupleTableTuple(TuplePtr t);


  /** inserts tuples in respective tables */
  void insertInRuleTable(TuplePtr);
  void insertInTupleTable(TuplePtr);

  /** Access functions for finding tags from tuples **/

  uint32_t getIdAtSource(TuplePtr t);
  string getLocalNode(TuplePtr t);
  string getSourceNode(TuplePtr t);
  void setLocalNode(TuplePtr t, string ln);

};

#endif /** __RULETRACER_H__ **/
