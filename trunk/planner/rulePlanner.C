
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
 * DESCRIPTION: Overlog planner
 *
 */

#ifndef __PL_RULEPLANNER_C__
#define __PL_RULEPLANNER_C__


#include <list>
#include <map>
#include <iostream>
#include <stdlib.h>
#include <fstream>

#include "ol_context.h"
#include "value.h"
#include "parser_util.h"
#include "ol_lexer.h"
#include "tuple.h"
#include "plumber.h"
#include "val_int32.h"
#include "val_str.h"
#include "print.h"
#include "discard.h"
#include "pelTransform.h"
#include "duplicate.h"
#include "dupElim.h"
#include "filter.h"
#include "timedPullPush.h"
#include "udp.h"
#include "marshalField.h"
#include "unmarshalField.h"
#include "unboxField.h"
#include "mux.h"
#include "demux.h"
#include "strToSockaddr.h"
#include "slot.h"
#include "timedPullSink.h"
#include "timestampSource.h"
#include "hexdump.h"
#include "table2.h"
#include "lookup2.h"
#include "insert.h"
#include "queue.h"
#include "printTime.h"
#include "roundRobin.h"
#include "noNullField.h"
#include "functorSource.h"
#include "delete.h"
#include "tupleSource.h"
#include "printWatch.h"
#include "aggregate.h"
#include "duplicateConservative.h"
#include "aggwrap.h"
#include "tupleseq.h"
#include "loop.h"
#include "ruleTracer.h"
#include "tap.h"
#include "traceTuple.h"
#include "update.h"
#include "removed.h"

#include "tableStore.h"
#include "eca_context.h"
#include "ruleStrand.h"
#include "netPlanner.h"

////////////////////////////////////////////////////////////
// Function to output into a dataflow graph specification
////////////////////////////////////////////////////////////

void debugRule(PlanContext* pc, string msg)
{
  warn << "DEBUG RULE: " << pc->_ruleStrand->getRuleID() << " " 
       << pc->_ruleStrand->getStrandID() << " " << msg; 
}

#include "rulePel.C"

void addWatch(PlanContext* pc, string b)
{
  if (pc->_outputDebugFile == NULL) {
    ElementSpecPtr print = 
      pc->createElementSpec(ElementPtr(new PrintWatch(b, 
						   pc->_tableStore->getWatchTables())));
    pc->addElementSpec(print);  
  } else {
    ElementSpecPtr print = 
      pc->createElementSpec(ElementPtr(new PrintWatch(b, 
						      pc->_tableStore->getWatchTables(), 
						      pc->_outputDebugFile)));
    pc->addElementSpec(print);  
  }
}

void generateInsertEvent(PlanContext* pc)
{
  RuleStrand* rs = pc->_ruleStrand;
  ECA_Rule* curRule = pc->getRule();
  pc->_namesTracker = new PlanContext::FieldNamesTracker(pc->getRule()->_event->_pf);

  debugRule(pc, "Insert event NamesTracker " + pc->_namesTracker->toString() + "\n");
  Table2Ptr tablePtr = pc->_tableStore->getTableByName(rs->eventFunctorName());
  ElementSpecPtr updateTable =
    pc->createElementSpec(ElementPtr(new Update("Update|" + curRule->_ruleID 
						+ "|" + rs->eventFunctorName()
						+ "|" + pc->_nodeID, tablePtr)));
  pc->addElementSpec(updateTable);
  
  // add a debug element
  addWatch(pc, "DebugInsertEvent|"+curRule->_ruleID+"|"+pc->_nodeID);
  
  if (curRule->_probeTerms.size() > 0) {
    // if we are doing a join
    ElementSpecPtr pullPush = 
      pc->createElementSpec(ElementPtr(new TimedPullPush("InsertEventTimedPullPush|"
							 + curRule->_ruleID 
							 + "|" + pc->_nodeID, 0)));
    pc->addElementSpec(pullPush);
  }
}

void generateDeleteEvent(PlanContext* pc)
{
  RuleStrand* rs = pc->_ruleStrand;
  ECA_Rule* curRule = pc->getRule();
  pc->_namesTracker = new PlanContext::FieldNamesTracker(pc->getRule()->_event->_pf);

  debugRule(pc, "Delete event NamesTracker " 
	    + pc->_namesTracker->toString() + "\n");
  Table2Ptr tablePtr = pc->_tableStore->getTableByName(rs->eventFunctorName());
  ElementSpecPtr removedTable =
    pc->createElementSpec(ElementPtr(new Removed("Removed|" 
						 + curRule->_ruleID 
						 + "|" 
						 + rs->eventFunctorName()
						 + "|" 
						 + pc->_nodeID, tablePtr)));
  pc->addElementSpec(removedTable);
  
  // add a debug element
  addWatch(pc, "DebugInsertEvent|"+curRule->_ruleID+"|"+pc->_nodeID);
  
  if (curRule->_probeTerms.size() > 0) {
    // if we are doing a join
    ElementSpecPtr pullPush = 
      pc->createElementSpec(ElementPtr(new TimedPullPush("RemovedEventTimedPullPush|"
							 + curRule->_ruleID 
							 + "|" + pc->_nodeID, 
							 0)));
    pc->addElementSpec(pullPush);
  }
}

void generateReceiveEvent(PlanContext* pc)
{
  ECA_Rule* curRule = pc->getRule();
  pc->_namesTracker = new PlanContext::FieldNamesTracker(pc->getRule()->_event->_pf);

  debugRule(pc, "Recv event NamesTracker " 
	    + pc->_namesTracker->toString() + "\n");
  addWatch(pc, "DebugRecvEvent|" + curRule->_ruleID + "|" + pc->_nodeID);
  
  if (curRule->_probeTerms.size() == 0) {
    ElementSpecPtr slot = 
      pc->createElementSpec(ElementPtr(new Slot("RecvEventSlot|" 
					     + curRule->_ruleID 
					     + "|" + pc->_nodeID)));
    pc->addElementSpec(slot);
  }
}

void generateEventElement(PlanContext* pc)
{
  RuleStrand* rs = pc->_ruleStrand;

  // update, create an updater
  if (rs->eventType() == Parse_Event::INSERT) {
    generateInsertEvent(pc);
  }
  
  if (rs->eventType() == Parse_Event::DELETE) {
    generateDeleteEvent(pc);
  }
  
  if (rs->eventType() == Parse_Event::RECV) {
    generateReceiveEvent(pc);
  }
}

void generateAddAction(PlanContext* pc)
{
  ECA_Rule* curRule = pc->getRule();
  RuleStrand* rs = pc->_ruleStrand;

  debugRule(pc, "Generate Add Action for " + rs->actionFunctorName() + "\n");

  // add a debug element
  ElementSpecPtr insertPullPush = 
    pc->createElementSpec(ElementPtr(new TimedPullPush("Insert|" 
						       + curRule->_ruleID 
						       + "|" + pc->_nodeID, 
						       0)));
  
  addWatch(pc, "DebugInsertAction|" + curRule->_ruleID + "|" + 
	   rs->actionFunctorName() + "|" + pc->_nodeID);
  
  Table2Ptr tablePtr 
    = pc->_tableStore->getTableByName(rs->actionFunctorName()); 
  
  ElementSpecPtr insertElement
    = pc->createElementSpec(ElementPtr(new Insert("Insert|" 
					       + curRule->_ruleID + "|" 
					       + rs->actionFunctorName() 
					       + "|" + pc->_nodeID, 
					       tablePtr)));
  
  ElementSpecPtr sinkS 
    = pc->createElementSpec(ElementPtr(new Discard("DiscardInsert")));
  
  pc->addElementSpec(insertPullPush);
  pc->addElementSpec(insertElement);
  pc->addElementSpec(sinkS);
}

void generateDeleteAction(PlanContext* pc)
{
  ECA_Rule* curRule = pc->getRule();
  RuleStrand* rs = pc->_ruleStrand;

  debugRule(pc, "Generate Delete Action for " + rs->actionFunctorName() + "\n");

  // add a debug element
  addWatch(pc, "DebugDeleteAction|" + curRule->_ruleID + "|" + 
	   rs->actionFunctorName() + "|" + pc->_nodeID);
  
  ElementSpecPtr deletePullPush = 
    pc->_conf->addElement(ElementPtr(new TimedPullPush("Delete|" 
						       + curRule->_ruleID 
						       + "|" + pc->_nodeID, 
						       0)));
  
  Table2Ptr tablePtr 
    = pc->_tableStore->getTableByName(rs->actionFunctorName()); 
  
  ElementSpecPtr deleteElement
    = pc->_conf->addElement(ElementPtr(new Delete("Delete|" 
						  + curRule->_ruleID + "|" 
						  + rs->actionFunctorName() 
						  + "|" + pc->_nodeID, 
						  tablePtr)));
  pc->addElementSpec(deletePullPush);
  pc->addElementSpec(deleteElement);
}

void generateSendAction(PlanContext* pc)
{
  ECA_Rule* curRule = pc->getRule();
  RuleStrand* rs = pc->_ruleStrand;

  debugRule(pc, "Generate Send Action for " + rs->actionFunctorName() + "\n");

  // compute the field for location specifier
  Parse_Functor* head = curRule->_action->_pf;
  string loc = head->fn->loc;
  int locationIndex = 0;
  for (int k = 0; k < head->args(); k++) {
    Parse_Var* parse_var = dynamic_cast<Parse_Var*>(head->arg(k));
    if (parse_var != NULL && parse_var->toString() == loc) {
      locationIndex = k+1;
    }
  }
  // copy that location specifier field first, encapsulate rest of tuple
  addWatch(pc, "DebugSendAction|"+ curRule->_ruleID + "|" + pc->_nodeID);
  ostringstream oss;
  oss << "$" << locationIndex << " pop swallow pop";
  ElementSpecPtr sendPelTransform =
    pc->createElementSpec(ElementPtr(new PelTransform("SendActionAddress|" 
						      + curRule->_ruleID 
						      + "|" + pc->_nodeID,
						      oss.str())));
  
  pc->addElementSpec(sendPelTransform);   
}

void generateActionElement(PlanContext* pc)
{
  RuleStrand* rs = pc->_ruleStrand;

  // add, insert
  if (rs->actionType() == Parse_Action::ADD) {    
    generateAddAction(pc);
  }

  if (rs->actionType() == Parse_Action::DELETE) {    
    generateDeleteAction(pc);
  }


  if (rs->actionType() == Parse_Action::SEND) {    
    generateSendAction(pc);
  }   
}


void createSecondaryIndex(PlanContext* pc, 
			  Table2Ptr table,
			  Table2::Key key)
{
  ostringstream uniqStr;
  uniqStr << table->name() << ":";
  std::vector< unsigned >::iterator iter = key.begin();
  while (iter != key.end()) {
    uniqStr << (*iter) << "_";
    iter++;
  }
  if (pc->_tableStore->checkSecondaryIndex(uniqStr.str()) == false) {
    // not there yet
    table->secondaryIndex(key);
    debugRule(pc, "AddMultTableIndex: Mult index added " + uniqStr.str() 
	      + "\n");
  } else {
    debugRule(pc, "AddMultTableIndex: Mult index already exists " 
	      + uniqStr.str() + "\n");
  }
}


void generateProbeElements(PlanContext* pc, 
			   Parse_Functor* innerFunctor,
			   b_cbv *comp_cb = 0)
{
  // we use the outer to denote the relation used for probing
  // inner to denote the relation being probed
  PlanContext::FieldNamesTracker* outerNamesTracker = pc->_namesTracker;
  PlanContext::FieldNamesTracker *innerNamesTracker
    = new PlanContext::FieldNamesTracker(innerFunctor);   

  Table2::Key innerIndexKey;         
  Table2::Key outerLookupKey;        
  Table2::Key innerRemainingKey; 
  innerNamesTracker->joinKeys(outerNamesTracker, outerLookupKey, 
			      innerIndexKey, innerRemainingKey);
  
  string innerTableName = innerFunctor->fn->name;
  
  if (outerLookupKey.size() == 0 || innerIndexKey.size() == 0) {
    error("No join keys " + innerTableName + " ", pc);
  }

  ostringstream joinKeyStr;
  joinKeyStr << "Size of join keys " << outerLookupKey.size() 
	     << " " << innerIndexKey.size() 
	     << " " << innerRemainingKey.size() << "\n";
  debugRule(pc, joinKeyStr.str());
  
  // feter the inner table
  Table2Ptr innerTable = pc->_tableStore->getTableByName(innerTableName);

  // The NoNull filter for the join sequence
  OL_Context::TableInfo* tableInfo 
    = pc->_tableStore->getTableInfo(innerTableName);  
  ostringstream nonulloss;
  nonulloss << "NoNull:" << pc->getRule()->_ruleID;
  ElementSpecPtr noNull =
    pc->createElementSpec(ElementPtr(new NoNullField(nonulloss.str(), 1)));
  
  // The connector slot for the output of my join
  ElementSpecPtr last_el(new ElementSpec
                         (ElementPtr(new Slot("dummySlotProbeElements"))));

  // The lookup
  ostringstream lookuposs;
  lookuposs << "Lookup2:" << pc->getRule()->_ruleID;

  last_el =
    pc->createElementSpec(ElementPtr(new Lookup2(lookuposs.str(),
						 innerTable,
						 outerLookupKey,
						 innerIndexKey)));

  if (tableInfo->primaryKeys == innerIndexKey) {
    // This is a primary key join so we don't need a secondary index
  } else {
    // Ensure there's a secondary index on the indexKey
    createSecondaryIndex(pc, innerTable, innerIndexKey);
  }
  
  int numFieldsProbe = outerNamesTracker->fieldNames.size();
  debugRule(pc, "Probe before merge " + outerNamesTracker->toString() + "\n");
  outerNamesTracker->mergeWith(innerNamesTracker->fieldNames); 
  debugRule(pc, "Probe after merge " + outerNamesTracker->toString() + "\n");

  pc->addElementSpec(last_el);
  pc->addElementSpec(noNull);
  
  // Take the joined tuples and produce the resulting path form the pel
  // projection.  Keep all fields from the probe, and all fields from
  // the base table that were not join keys.
  ostringstream pelProject;
  pelProject << "\"join:"
             << pc->getRule()->getEventName() 
             << ":" << innerTableName
             << ":" 
	     << pc->getRule()->_ruleID
             << "\" pop "; // table name
  for (int k = 0; k < numFieldsProbe; k++) {
    pelProject << "$0 " << k + 1 << " field pop ";
  }

  // And also pop all remaining base field numbers (those that did not
  // participate in the join.
  for (Table2::Key::iterator i = innerRemainingKey.begin();
       i != innerRemainingKey.end();
       i++) {
    pelProject << "$1 " << (*i) << " field pop ";
  }

  string pelProjectStr = pelProject.str();
  ostringstream oss1; 
  oss1 << "joinPel_" << pc->getRule()->_ruleID 
       << " " << pelProjectStr << "\n";

  ElementSpecPtr transS =
    pc->createElementSpec(ElementPtr(new PelTransform(oss1.str(),
						      pelProjectStr)));
  debugRule(pc, "Join Pel Transform " + oss1.str());
  
  pc->addElementSpec(transS);
}		   


 
void generateMultipleProbeElements(PlanContext* pc)
{
  ECA_Rule* curRule = pc->_ruleStrand->getRule();

  for (uint k = 0; k < curRule->_probeTerms.size(); k++) {    
    Parse_Functor* pf = curRule->_probeTerms.at(k);
   
    debugRule(pc, "Probing " + curRule->getEventName()
		      + " " + pf->toString() + "\n");
    
    generateProbeElements(pc, pf);

    if (curRule->_probeTerms.size() - 1 != k) {
      ElementSpecPtr pullPush =
	pc->createElementSpec(ElementPtr(new TimedPullPush("ProbePullPush|" 
							   + curRule->_ruleID + "|" 
							   + pc->_nodeID, 0)));
      pc->addElementSpec(pullPush);
    }
  }
}

void generateSelectionAssignmentElements(PlanContext* pc)
{
  ECA_Rule* curRule = pc->getRule();
  for (unsigned int j = 0; j < curRule->_selectAssignTerms.size(); j++) {
    Parse_Select* parse_select 
      = dynamic_cast<Parse_Select *>(curRule->_selectAssignTerms.at(j));
    if (parse_select != NULL) {
      debugRule(pc, "Selection term " + parse_select->toString() + " " 
			+ curRule->_ruleID + "\n");
      pelSelect(pc, parse_select, j); 
    }
    Parse_Assign* parse_assign 
      = dynamic_cast<Parse_Assign *>(curRule->_selectAssignTerms.at(j));
    if (parse_assign != NULL) {
      debugRule(pc, "Assignment term " + parse_assign->toString() + " " 
			+ curRule->_ruleID + "\n");
      pelAssign(pc, parse_assign, j);
    }
  }
}

void generateProjectElements(PlanContext* pc)
{
  ECA_Rule* curRule = pc->getRule();
  Parse_Functor* pf = curRule->_action->_pf;
  PlanContext::FieldNamesTracker* curNamesTracker = pc->_namesTracker;
  
  // determine the projection fields, and the first address to return. 
  // Add 1 for table name     
  std::vector<unsigned int> indices;  
  // iterate through all functor's output
  for (int k = 0; k < pf->args(); k++) {
    Parse_Var* parse_var = dynamic_cast<Parse_Var*>(pf->arg(k));
    int pos = -1;
    if (parse_var != NULL) {
      // care only about vars    
      pos = curNamesTracker->fieldPosition(parse_var->toString());    
    }
    if (pos == -1) { continue; }    
    indices.push_back(pos + 1);
  }
 
  ostringstream pel;
  pel << "\"" << pf->fn->name << "\" pop";

  for (unsigned int k = 0; k < indices.size(); k++) {
    pel << " $" << indices.at(k) << " pop";
  }


  string pelTransformStr = pel.str();
  debugRule(pc, "Project head " + curNamesTracker->toString() 
		    + " " + pelTransformStr + "\n");
 
  ElementSpecPtr projectHeadPelTransform =
    pc->createElementSpec(ElementPtr(new PelTransform("ProjectHead|" 
						      + curRule->_ruleID 
						      + "|" + pc->_nodeID,
						      pelTransformStr)));

  pc->addElementSpec(projectHeadPelTransform);  
}


void compileECARule(PlanContext* pc)
{

  std::cout << "Process rule " << pc->getRule()->toString() << "\n";

  // first generate the incoming event
  generateEventElement(pc);        

  // do all the joins
  generateMultipleProbeElements(pc);

  // do selections and assignments
  generateSelectionAssignmentElements(pc);

  // do projection based on head
  generateProjectElements(pc);

  // do action. For send, later hook to round robin
  generateActionElement(pc);
}

#endif
