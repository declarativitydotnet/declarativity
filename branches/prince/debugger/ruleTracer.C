/***
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776,
 * Berkeley, CA,  94707. Attention: P2 Group.
 *
 *
 */



#include <p2Time.h>
#include "ruleTracer.h"
#include "execRecord.h"
#include "val_tuple.h"
#include "val_time.h"
#include "val_str.h"
#include "val_uint32.h"
#include <math.h>

#ifndef SKIP
#define SKIP 

RuleTracer:: RuleTracer(string name, string ruleName, string node, 
			int startPort, int endPort, int ruleNum,
			Table2Ptr ruleExec, Table2Ptr tupleTable)
  : Element(name, endPort+1, 0)
{
  _ruleName = ruleName;
  _node = node;
  _ruleNum = ruleNum;
  _startPort = startPort;
  _endPort = endPort;
  _ruleExecTable = ruleExec;
  _tupleTable = tupleTable;
  for(int i = 0; i < ExecRecord::MAX_STAGES; i++)
    _records[i] = 0;
}

RuleTracer:: ~RuleTracer()
{
}

/**
 * Port #: identifies the stage for this execution
 *
 * Algorithm:
 * 1. find the corresponding record
 * 2. fill appropriate slot
 * 3. if this is the last slot, push out an execTuple
 **/

int RuleTracer::push(int port, TuplePtr p, b_cbv cb)
{
  // set the local node for a given tuple
  setLocalNode(p);

  TuplePtr t_tupleTable = createTupleTableTuple(p);
  
  if (t_tupleTable) {
    insertInTupleTable(t_tupleTable);
  }

  //TELL_INFO << " INSERTED " << t_tupleTable->toString() << "\n";
#ifdef SKIP
  // undefine the SKIP at the beginning if you want
  // to trace the debugging rules too
  if(skip(p))
    return 1;

#endif

  ELEM_INFO("Pushing tuple " << p->toString());

  if(_startPort == _endPort){
    ELEM_INFO("For element " 
              << name()
              << ", start and end ports are same");
  }

  if (port == _startPort &&
      _startPort < _endPort) {
    startRule(p, port);
  } else if(port == _endPort) {
    endRule(p, port);
  } else {
    appendRule(p, port);
  }
  
  return 1;
}


/**
 * Algorithm:
 * Find the record whose range contains the given port, i.e. start < port < end
 * If no such record exists, then do the following
 * If port >= end for a record, (there should be only one such record)
 *       use that record and increment end
 * If port == start for a record (again, there should be only one such record)
 *       increment the start for that record (dont use it)
 * If there exists a record s.t. start >= end, flush it and use it
 *
 * Else, use an unused record (there should exist one)
 *
 **/
int RuleTracer::findRecordIndex(int port)
{
  ExecRecord * e;

  for(int i = 0;i < ExecRecord::MAX_STAGES; i++){
    e = _records[i];
    if(e != NULL){
      if(e->range_start < port && e->range_end > port)
	return i;
      else if(port >= e->range_end){
	return i;
      }
      else if(port == e->range_start){
	e->range_start ++;
	i--; // restart looking from this index
	continue;
      }
      else if(e->range_start >= e->range_end){
	// this is in-active now, so we can use it
	e->flush();
	return i;
      }
    }
  }
  // now, use any un-used records
  for(int i = 0; i < ExecRecord::MAX_STAGES; i++){
    e = _records[i];
    if(e == NULL){
      e = new ExecRecord(_ruleName);
      _records[i] = e;
      return i;
    }
  }
  return 0;
}

void RuleTracer::startRule(TuplePtr t, int port)
{
  int index = findRecordIndex(port);
  ExecRecord * e = _records[index];

  e->flush();
  e->tupleIn = t->ID();
  getTime(e->timeIn);
  e->finished = false;
  
  return;
}

void
RuleTracer::endRule(TuplePtr t, int port)
{
  int index = findRecordIndex(port);
  ExecRecord * e = _records[index];
 
  e->tupleOut = t->ID();
  getTime(e->timeOut);
  e->finished = true;

  // destination node for this tuple is the 2nd field
  e->remoteNode = ((*t)[1])->toString();


  TuplePtr t_exec = createExecTuple(e);
  
  insertInRuleTable(t_exec);

  std::vector<TuplePtr> * vv = new std::vector<TuplePtr>();
  int count = createExecTupleUsingPrecond(e, vv);
  
  for(int i = 0; i < count; i++){
    insertInRuleTable(vv->at(i));
  }

  // create the exec tuple and send it to the insert element
  return;
}


void
RuleTracer::appendRule(TuplePtr t, int port)
{
  int index = findRecordIndex(port);
  ExecRecord * e = _records[index];
  
  ELEM_INFO("AppendRule "
            <<  t->toString()
            << " Rule "
            << _ruleName);
  TuplePtr t2 = Val_Tuple::cast((*t)[1]);
  setLocalNode(t2);

  TuplePtr t_tupleTable = createTupleTableTuple(t2);

  if(t_tupleTable) {
    insertInTupleTable(t_tupleTable);
  }

  e->flushTillIndex(port);

  e->precond[port] = t2->ID();
  e->numEntries ++;
  //TELL_INFO << e->toString();
  return;
}

void RuleTracer::setLocalNode(TuplePtr t)
{
  string node = getNode();
  setLocalNode(t, node);
  return;
}

TuplePtr RuleTracer::createExecTuple(ExecRecord *e)
{
  TuplePtr t = Tuple::mk();
  t->append(Val_Str::mk("ruleExecTable"));
  t->append(Val_Str::mk(getNode()));
  t->append(Val_Str::mk(e->ruleId));
  t->append(Val_UInt32::mk(e->tupleIn));
  t->append(Val_UInt32::mk(e->tupleOut));
  t->append(Val_Str::mk(e->remoteNode));
  t->append(Val_Time::mk(e->timeIn));
  t->append(Val_Time::mk(e->timeOut));
  t->append(Val_Str::mk("EVENT"));
  ELEM_INFO(t->toString());
  return t;

  // todo: insert tuples for each precondition also
}

int RuleTracer::createExecTupleUsingPrecond(ExecRecord *e,
					    std::vector<TuplePtr> * execs)
{
  int i;
  for(i = 0; i < e->numEntries; i++){
    TuplePtr t = Tuple::mk();
    t->append(Val_Str::mk("ruleExecTable"));
    t->append(Val_Str::mk(getNode()));
    t->append(Val_Str::mk(e->ruleId));
    t->append(Val_UInt32::mk(e->precond[i]));
    t->append(Val_UInt32::mk(e->tupleOut));
    t->append(Val_Str::mk(e->remoteNode));
    t->append(Val_Time::mk(e->timeIn));
    t->append(Val_Time::mk(e->timeOut));
    t->append(Val_Str::mk("PRECOND"));
    ELEM_INFO(t->toString());
    execs->push_back(t);
  }
  return e->numEntries;
}





TuplePtr
RuleTracer::createTupleTableTuple(TuplePtr t)
{
  if(t->ID() == 0)
    return TuplePtr();

  // This assumes that the tuple table has a primary key on field 2.
  (*TUPLETABLETUPLE)[2] = Val_UInt32::mk(t->ID());
  Table2::Iterator it = _tupleTable->lookup(Table2::theKey(CommonTable::KEY2), TUPLETABLETUPLE);
  int count = 0;
  while(!it->done()){
    it->next();
    count++;
  }
  if(count > 0)
    return TuplePtr();

  TuplePtr t_new = Tuple::mk();
  t_new->append(Val_Str::mk("tupleTable"));
  t_new->append(Val_Str::mk(getNode()));
  t_new->append(Val_UInt32::mk(t->ID()));
  //t_new->append(Val_Tuple::mk(t));
  if(getSourceNode(t) != "-")
    t_new->append(Val_Str::mk(getSourceNode(t)));
  else
    t_new->append(Val_Str::mk(getLocalNode(t)));
  if(getIdAtSource(t) != 0)
    t_new->append(Val_UInt32::mk(getIdAtSource(t)));
  else
    t_new->append(Val_UInt32::mk(t->ID()));

  //boost::posix_time::ptime timedd;
  //getTime(timedd);
  //t_new->append(Val_Time::mk(timedd));

  ELEM_INFO(t_new->toString());

  return t_new;
}

void RuleTracer::insertInRuleTable(TuplePtr t)
{
  _ruleExecTable->insert(t);

}

void RuleTracer::insertInTupleTable(TuplePtr t)
{
  _tupleTable->insert(t);
  
}

bool RuleTracer::skip(TuplePtr t)
{
  string sub_str = _ruleName.substr(0,1);
  // Debugging rules start with "x"
  if(sub_str == "x")
    return true;
  else
    return false;
}

uint32_t RuleTracer::getIdAtSource(TuplePtr t)
{ 
  if(t->tag("ID"))
    return Val_UInt32::cast(t->tag("ID"));
  else
    return 0;
}

string RuleTracer::getLocalNode(TuplePtr t)
{ 
  if(t->tag("localNode"))
    return Val_Str::cast(t->tag("localNode"));
  else
    return "-";
}

string RuleTracer::getSourceNode(TuplePtr t)
{ 
  if(t->tag("sourceNode"))
    return Val_Str::cast(t->tag("sourceNode"));
  else
    return "-";
}

void RuleTracer::setLocalNode(TuplePtr t, string ln)
{ 
  t->tag("localNode", Val_Str::mk(ln));
}



// Initialize the search tuple table tuple
TuplePtr
RuleTracer::TUPLETABLETUPLE = Tuple::mk();


RuleTracer::Initializer::Initializer()
{
  // The tuple table tuple needs up to field 2
  TUPLETABLETUPLE->append(Val_Str::mk("tupleTable"));
  TUPLETABLETUPLE->append(Val_Str::mk("nodeID"));
  TUPLETABLETUPLE->append(Val_UInt32::mk(0));
}


/** Run the static initialization */
RuleTracer::Initializer RuleTracer::_INITIALIZER;


#endif /* SKIP */
