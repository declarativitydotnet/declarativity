
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

#include "aggFactory.h"
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
#include "refresh.h"
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
#include "noNull.h"
#include "functorSource.h"
#include "delete.h"
#include "tupleSource.h"
#include "printWatch.h"
#include "aggregate.h"
#include "duplicateConservative.h"
#include "aggwrap2.h"
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

//#include "dataflow.C"

////////////////////////////////////////////////////////////
// Function to output into a dataflow graph specification
////////////////////////////////////////////////////////////

void createSecondaryIndex(PlanContext* pc, 
			  CommonTablePtr table,
			  CommonTable::Key key);


#include "rulePel.C"


/** Add a print element if the functor name is watched.*/
void
addPrint(PlanContext* pc,
         string b,
         string functorName,
         string modifier)
{
  OL_Context::WatchTableType  watchedNames =
    pc->_tableStore->getWatchTables();
  OL_Context::WatchTableType::iterator i =
    watchedNames.find(functorName);
  if (i == watchedNames.end()) {
    // Didn't find that name. Do nothing
  } else {
    std::string modifiers = i->second;

    if (modifiers.size() == 0) {
      // All modifiers are allowed
    } else if (modifiers.find(modifier, 0) != std::string::npos) {
      // Found this modifier
    } else {
      // Didn't find this modifier
      return;
    }

    string output = b + "!" + functorName +
      "!" + pc->getRule()->_ruleID + "!" + pc->_nodeID;
    ElementSpecPtr print = 
      pc->createElementSpec(ElementPtr(new Print(output)));
    pc->addElementSpec(print);  
  }
}


/** Add a print element if the functor name is watched.*/
void
addPrint(TableStore* tableStore,
         Plumber::DataflowPtr _conf,
         string nodeID,
         StageStrand* strand,
         string b,
         string functorName,
         string modifier)
{
  OL_Context::WatchTableType watchedNames =
    tableStore->getWatchTables();
  OL_Context::WatchTableType::iterator i =
    watchedNames.find(functorName);
  if (i == watchedNames.end()) {
    // Didn't find that name. Do nothing
  } else {
    std::string modifiers = i->second;

    if (modifiers.size() == 0) {
      // All modifiers are allowed
    } else if (modifiers.find(modifier, 0) != std::string::npos) {
      // Found this modifier
    } else {
      // Didn't find this modifier
      return;
    }

    string output = b + "!" + functorName +
      "!" + strand->getStrandID() + "!" + nodeID;
    strand->addElement(_conf,
                       ElementPtr(new Print(output)));
  }
}


void
generateInsertEvent(PlanContext* pc)
{
  RuleStrand* rs = pc->_ruleStrand;
  ECA_Rule* curRule = pc->getRule();
  pc->_namesTracker = new PlanContext::FieldNamesTracker(pc->getRule()->_event->_pf);

  PLANNER_WORDY(pc, "Insert event NamesTracker " << pc->_namesTracker->toString());
  string tableName = rs->eventFunctorName();
  CommonTablePtr tablePtr = pc->_tableStore->getTableByName(tableName);
  ElementSpecPtr updateTable =
    pc->createElementSpec(ElementPtr(new Update("Update!" + curRule->_ruleID 
						+ "!" + rs->eventFunctorName()
						+ "!" + pc->_nodeID, tablePtr)));
  pc->addElementSpec(updateTable);
  
  // add a print element
  addPrint(pc, "InsertEvent", tableName, "i");
  
  if (curRule->_probeTerms.size() > 0) {
    // if we are doing a join
    ElementSpecPtr pullPush = 
      pc->createElementSpec(ElementPtr(new TimedPullPush("InsertEventTimedPullPush!"
							 + curRule->_ruleID 
							 + "!" + pc->_nodeID, 0)));
    pc->addElementSpec(pullPush);
  }
}


void
generateRefreshEvent(PlanContext* pc)
{
  RuleStrand* rs = pc->_ruleStrand;
  ECA_Rule* curRule = pc->getRule();
  pc->_namesTracker = new PlanContext::FieldNamesTracker(pc->getRule()->_event->_pf);

  PLANNER_WORDY(pc, "Refresh event NamesTracker " << pc->_namesTracker->toString());
  string tableName = rs->eventFunctorName();
  CommonTablePtr tablePtr = pc->_tableStore->getTableByName(tableName);
  ElementSpecPtr refreshTable =
    pc->createElementSpec(ElementPtr(new Refresh("Refresh!" + curRule->_ruleID 
						+ "!" + rs->eventFunctorName()
						+ "!" + pc->_nodeID, tablePtr)));
  pc->addElementSpec(refreshTable);
  
  // add a debug element
  addPrint(pc, "RefreshEvent", tableName, "r");
  
  if (curRule->_probeTerms.size() > 0) {
    // if we are doing a join
    ElementSpecPtr pullPush = 
      pc->createElementSpec(ElementPtr(new TimedPullPush("RefreshEventTimedPullPush!"
							 + curRule->_ruleID 
							 + "!" + pc->_nodeID, 0)));
    pc->addElementSpec(pullPush);
  }
}


void
generateDeleteEvent(PlanContext* pc)
{
  RuleStrand* rs = pc->_ruleStrand;
  ECA_Rule* curRule = pc->getRule();
  pc->_namesTracker = new PlanContext::FieldNamesTracker(pc->getRule()->_event->_pf);

  PLANNER_WORDY(pc, "Delete event NamesTracker " 
                << pc->_namesTracker->toString());
  string tableName = rs->eventFunctorName();
  CommonTablePtr tablePtr = pc->_tableStore->getTableByName(tableName);
  ElementSpecPtr removedTable =
    pc->createElementSpec(ElementPtr(new Removed("Removed!" 
						 + curRule->_ruleID 
						 + "!" 
						 + rs->eventFunctorName()
						 + "!" 
						 + pc->_nodeID, tablePtr)));
  pc->addElementSpec(removedTable);
  
  // add a debug element
  addPrint(pc, "DeleteEvent", tableName, "d");
  
  if (curRule->_probeTerms.size() > 0) {
    // if we are doing a join
    ElementSpecPtr pullPush = 
      pc->createElementSpec(ElementPtr(new TimedPullPush("RemovedEventTimedPullPush!"
							 + curRule->_ruleID 
							 + "!" + pc->_nodeID, 
							 0)));
    pc->addElementSpec(pullPush);
  }
}


void
generateReceiveEvent(PlanContext* pc)
{
  ECA_Rule* curRule = pc->getRule();
  pc->_namesTracker = new PlanContext::FieldNamesTracker(curRule->_event->_pf);

  PLANNER_WORDY(pc, "Recv event NamesTracker " 
                << pc->_namesTracker->toString());
  addPrint(pc, "RecvEvent", curRule->getEventName(), "c");
  
  if (curRule->_probeTerms.size() == 0) {
    ElementSpecPtr slot = 
      pc->createElementSpec(ElementPtr(new Slot("RecvEventSlot!" 
                                                + curRule->_ruleID 
                                                + "!" + pc->_nodeID)));
    pc->addElementSpec(slot);
  }
}


void
generateAggEvent(PlanContext* pc)
{
  
  ECA_Rule* curRule = pc->getRule();
  pc->_namesTracker = new PlanContext::FieldNamesTracker(curRule->_event->_pf);

  CommonTable::Key groupByFields;      
  
  PlanContext::FieldNamesTracker* aggregateNamesTracker 
    = new PlanContext::FieldNamesTracker();
  Parse_Functor* action_functor = curRule->_action->_pf;
  Parse_Functor* event_functor = curRule->_event->_pf;

  ostringstream oss;
  for (int k = 0; k < action_functor->args(); k++) {
    // go through the functor head, but skip the aggField itself    
    Parse_Var* pv = dynamic_cast< Parse_Var* > (action_functor->arg(k));
    if (pv == NULL) {
      continue;
    }
    int pos = pc->_namesTracker->fieldPosition(pv->toString());
    if (k != -1 && k != action_functor->aggregate()) {
      groupByFields.push_back((uint) pos + 1);
      oss << (pos + 1) << " ";
      aggregateNamesTracker->fieldNames.push_back(pv->toString());
    }
  }

  Parse_Agg* pa 
    = dynamic_cast<Parse_Agg* > (action_functor->arg(action_functor->aggregate()));
  string aggVarname = pa->v->toString();
  aggregateNamesTracker->fieldNames.push_back(aggVarname);

  int aggFieldBaseTable =
    pc->_namesTracker->fieldPosition(aggVarname) + 1;

  // get the table, create the index
  CommonTablePtr aggTable = pc->_tableStore->getTableByName(event_functor->fn->name);  
  createSecondaryIndex(pc, aggTable, groupByFields);  

  CommonTable::Aggregate tableAgg;
  try {
    tableAgg =
      aggTable->aggregate(groupByFields,
                          aggFieldBaseTable, // the agg field
                          pa->oper);
    oss << "," << aggFieldBaseTable;
    PLANNER_WORDY(pc, "Group by " << oss.str());
  } catch (AggFactory::AggregateNotFound a) {
    // Ooops, I couldn't create an aggregate. Complain.
    error(pc, "Could not create aggregate \"" + pa->oper
          + "\". I only know aggregates " +
          AggFactory::aggList());
    return;
  }

  PLANNER_WORDY(pc, "Agg NamesTracker " << aggregateNamesTracker->toString());

  ElementSpecPtr aggElement =
    pc->createElementSpec(ElementPtr(new Aggregate("Agg!" + curRule->_ruleID +
						   "!" + pc->_nodeID + "!" 
						   + pa->oper, tableAgg)));
  
  ostringstream pelTransformStr;
  pelTransformStr << "\"" << "aggResult!" 
		  << curRule->_ruleID << "\" pop";
  for (uint k = 0;
       k < aggregateNamesTracker->fieldNames.size();
       k++) {
    pelTransformStr << " $" << k << " pop";
  }
  PLANNER_WORDY(pc, "Agg Pel Expr " << pelTransformStr.str());

  // apply PEL to add a table name
  ElementSpecPtr addTableName =
    pc->createElementSpec(ElementPtr(new PelTransform("Aggregation!"
						      + curRule->_ruleID
						      + "!" + pc->_nodeID,
						      pelTransformStr.str())));

  pc->addElementSpec(aggElement);
  pc->addElementSpec(addTableName);

  pc->_namesTracker = aggregateNamesTracker;  
}


void
generatePeriodicEvent(PlanContext* pc)
{
  ECA_Rule* curRule = pc->getRule();
    
  TuplePtr functorTuple = Tuple::mk();
  functorTuple->append(Val_Str::mk("periodic_" + curRule->_ruleID));
  functorTuple->append(Val_Str::mk(pc->_nodeID)); 
  functorTuple->freeze();

  ElementSpecPtr source =
    pc->createElementSpec(ElementPtr(new TupleSource("FunctorSource!" + curRule->_ruleID 
						     + "!" + pc->_nodeID,
						     functorTuple)));
  pc->addElementSpec(source);

  Parse_Functor* pf = dynamic_cast<Parse_Functor* > (curRule->_event->_pf);
  if (pf == NULL) {
    error(pc, curRule->getEventName() + " must be a functor");
    return;
  }

  if (pf->args() < 3) {
    error(pc, "Malformed periodic predicate");
  }

  //pc->_namesTracker->fieldNames.push_back("T"); // time interval

  string period = pf->arg(2)->toString();
  int count = 0;
  if (pf->args() > 3) {
    count = atoi(pf->arg(3)->toString().c_str());
    //pc->_namesTracker->fieldNames.push_back("C"); // count
  }

   
  // a pel transform that puts in the periodic stuff
  ElementSpecPtr pelRand = 
    pc->createElementSpec(ElementPtr(new PelTransform("FunctorSourcePel!" +
                                                      curRule->_ruleID +
                                                      "!" + pc->_nodeID,
                                                      "$0 pop $1 pop rand pop")));
  pc->addElementSpec(pelRand);

  PLANNER_WORDY(pc, "Periodic Event "
                << pc->_namesTracker->toString() << " "
                << atof(period.c_str()) << " " << count);


  // The timed pusher
  ElementSpecPtr pushFunctor =
    pc->createElementSpec(ElementPtr(new TimedPullPush("FunctorPush!" 
						       + curRule->_ruleID +
                                                       "!" + pc->_nodeID,
                                                       atof(period.c_str()),
                                                       count)));

  pc->addElementSpec(pushFunctor);
  
  if (curRule->_probeTerms.size() == 0) {
    ElementSpecPtr functorSlot 
      = pc->createElementSpec(ElementPtr(new Slot("functorSlot:" 
						  + curRule->_ruleID + ":" 
						  + pc->_nodeID)));      
    pc->addElementSpec(functorSlot);
  }
  addPrint(pc, "PeriodicEvent", "periodic", "p");
}


void
generateEventElement(PlanContext* pc)
{
  RuleStrand* rs = pc->_ruleStrand;
  int aggField = pc->getRule()->_action->_pf->aggregate();

  if (rs->eventType() == Parse_Event::REFRESH) {
    generateRefreshEvent(pc);
  }

  // update, create an updater
  if (rs->eventType() == Parse_Event::INSERT) {

    // is this an agg table type?
    if (aggField >= 0) {
      generateAggEvent(pc);
      return;
    }

    if (pc->getRule()->getEventName() == "periodic") {
      generatePeriodicEvent(pc);
      return;
    }

    generateInsertEvent(pc);

    // check for periodic as well
  }
  
  if (rs->eventType() == Parse_Event::DELETE) {
    generateDeleteEvent(pc);
  }
  
  if (rs->eventType() == Parse_Event::RECV) {
    generateReceiveEvent(pc);
  }
}


void
generateAddAction(PlanContext* pc)
{
  ECA_Rule* curRule = pc->getRule();
  RuleStrand* rs = pc->_ruleStrand;

  PLANNER_WORDY(pc, "Generate Add Action for " << rs->actionFunctorName());

  // add a debug element
  ElementSpecPtr insertPullPush = 
    pc->createElementSpec(ElementPtr(new TimedPullPush("Insert!" 
						       + curRule->_ruleID 
						       + "!" + pc->_nodeID, 
						       0)));
  
  addPrint(pc, "AddAction", rs->actionFunctorName(), "a");
  
  CommonTablePtr tablePtr 
    = pc->_tableStore->getTableByName(rs->actionFunctorName()); 
  
  ElementSpecPtr insertElement
    = pc->createElementSpec(ElementPtr(new Insert("Insert!" 
					       + curRule->_ruleID + "!" 
					       + rs->actionFunctorName() 
					       + "!" + pc->_nodeID, 
					       tablePtr)));
  
  ElementSpecPtr sinkS 
    = pc->createElementSpec(ElementPtr(new Discard("DiscardInsert")));
  
  pc->addElementSpec(insertPullPush);
  pc->addElementSpec(insertElement);
  pc->addElementSpec(sinkS);
}


void
generateDeleteAction(PlanContext* pc)
{
  ECA_Rule* curRule = pc->getRule();
  RuleStrand* rs = pc->_ruleStrand;

  PLANNER_WORDY(pc, "Generate Delete Action for " << rs->actionFunctorName());

  // add a debug element
  addPrint(pc, "DeleteAction", rs->actionFunctorName(), "z");
  
  ElementSpecPtr deletePullPush = 
    pc->_conf->addElement(ElementPtr(new TimedPullPush("Delete!" 
						       + curRule->_ruleID 
						       + "!" + pc->_nodeID, 
						       0)));
  
  CommonTablePtr tablePtr 
    = pc->_tableStore->getTableByName(rs->actionFunctorName()); 
  
  ElementSpecPtr deleteElement
    = pc->_conf->addElement(ElementPtr(new Delete("Delete!" 
						  + curRule->_ruleID + "!" 
						  + rs->actionFunctorName() 
						  + "!" + pc->_nodeID, 
						  tablePtr)));
  pc->addElementSpec(deletePullPush);
  pc->addElementSpec(deleteElement);
}

void generateSendAction(PlanContext* pc)
{
  ECA_Rule* curRule = pc->getRule();
  RuleStrand* rs = pc->_ruleStrand;

  // compute the field for location specifier
  Parse_Functor* head = curRule->_action->_pf;
  string loc = head->getlocspec();
  int locationIndex = -1;
  for (int k = 0; k < head->args(); k++) {
    Parse_Var* parse_var = dynamic_cast<Parse_Var*>(head->arg(k));
    if (parse_var != NULL && fieldNameEq(parse_var->toString(), loc)) {
      locationIndex = k+1;
    }
	else {
	  Parse_Agg* aggExpr = dynamic_cast<Parse_Agg*>(head->arg(k));
	  if (aggExpr != NULL && fieldNameEq(aggExpr->v->toString(), loc)) {
		locationIndex = k+1;
	  }    
	}
  }

  if (locationIndex == -1) {
    error(pc, "Cannot find location specifier for " 
	  + loc + " in rule " + curRule->toString() + "\n");
  }

  ostringstream oss;
  oss << "$" << locationIndex << " pop swallow pop";
  ElementSpecPtr sendPelTransform =
    pc->createElementSpec(ElementPtr(new PelTransform("SendActionAddress!" 
						      + curRule->_ruleID 
						      + "!" + pc->_nodeID,
						      oss.str())));

  PLANNER_WORDY(pc, "Generate Send Action for " << rs->actionFunctorName() 
                << " " + oss.str());

  addPrint(pc, "SendAction", rs->actionFunctorName(), "s");  
  pc->addElementSpec(sendPelTransform);   
  //addPrint(pc, "SendActionMsg");

  // copy that location specifier field first, encapsulate rest of tuple
}

void generateSendAction(TableStore* tableStore,
                        string nodeID,
                        StageStrand* strand,
                        const OL_Context::ExtStageSpec* aSpec,
                        Plumber::DataflowPtr _conf)
{
  // The first field after the tuple name is always the location
  // specifier
  PLANNER_WORDY_NOPC("Generate Send Action for "
                     << aSpec->outputTupleName
                     << " $1 pop swallow pop");

  addPrint(tableStore,
           _conf,
           nodeID,
           strand,
           "SendAction",
           aSpec->outputTupleName,
           "s");  
  strand->addElement(_conf,
                     ElementPtr(new PelTransform("SendActionAddress!" 
                                                 + strand->getStrandID()
                                                 + "!" + nodeID,
                                                 "$1 pop swallow pop")));
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
			  CommonTablePtr table,
			  CommonTable::Key key)
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
    table->createSecondaryIndex(key);
    PLANNER_WORDY(pc, "AddMultTableIndex: Mult index added " << uniqStr.str());
  } else {
    PLANNER_WORDY(pc, "AddMultTableIndex: Mult index already exists " 
                  << uniqStr.str());
  }
}


/** This method creates a single join from tuples of type probeName
    returning the name of the output (temporary) tuples. */
string
generateProbeElements(PlanContext* pc, 
                      Parse_Functor* innerFunctor,
                      string probeName,
                      b_cbv *comp_cb = 0)
{
  // we use the outer to denote the relation used for probing
  // inner to denote the relation being probed
  PlanContext::FieldNamesTracker* outerNamesTracker = pc->_namesTracker;
  PlanContext::FieldNamesTracker* innerNamesTracker
    = new PlanContext::FieldNamesTracker(innerFunctor);   
  ECA_Rule* curRule = pc->_ruleStrand->getRule();

  CommonTable::Key innerIndexKey;         
  CommonTable::Key outerLookupKey;        
  CommonTable::Key innerRemainingKey; 
  innerNamesTracker->joinKeys(outerNamesTracker, outerLookupKey, 
			      innerIndexKey, innerRemainingKey);

  addPrint(pc, "BeforeJoin", probeName, "b");
  
  string innerTableName = innerFunctor->fn->name;
  PLANNER_WORDY(pc, "InnerFunctorName " << innerTableName);

  if (outerLookupKey.size() == 0 || innerIndexKey.size() == 0) {
    error(pc, "No join keys " + innerTableName + " ");
  }

  ostringstream joinKeyStr;
  joinKeyStr << "Size of join keys: "
             << innerIndexKey.size()
             << ". InnerIndex: ";
  for (uint k = 0; k < innerIndexKey.size(); k++) {
    joinKeyStr << innerIndexKey.at(k) << " ";
  }
  joinKeyStr << ", OuterLookup: ";
  for (uint k = 0; k < outerLookupKey.size(); k++) {
    joinKeyStr << outerLookupKey.at(k) << " ";
  }
  joinKeyStr << ", Remaining: ";
  for (uint k = 0; k < innerRemainingKey.size(); k++) {
    joinKeyStr << innerRemainingKey.at(k) << " ";
  }
  joinKeyStr << "\n";
  PLANNER_WORDY(pc, joinKeyStr.str());
  
  // feter the inner table
  CommonTablePtr innerTable = pc->_tableStore->getTableByName(innerTableName);

  // The NoNull filter for the join sequence
  OL_Context::TableInfo* tableInfo 
    = pc->_tableStore->getTableInfo(innerTableName);  
  ostringstream nonulloss;
  nonulloss << "NoNull:" << pc->getRule()->_ruleID;
  ElementSpecPtr noNull =
    pc->createElementSpec(ElementPtr(new NoNull(nonulloss.str())));
  
  // The connector slot for the output of my join
  ElementSpecPtr last_el(new ElementSpec
                         (ElementPtr(new Slot("dummySlotProbeElements"))));

  // The lookup
  ostringstream lookuposs;
  lookuposs << "Lookup2:" << pc->getRule()->_ruleID
            << "!" << innerTableName
            << "!" << pc->_nodeID;

  if (curRule->_aggWrap == false) {
    PLANNER_WORDY(pc, "Lookup " << innerTableName << " no callbacks");
    last_el =
      pc->createElementSpec(ElementPtr(new Lookup2(lookuposs.str(),
						   innerTable,
						   outerLookupKey,
						   innerIndexKey)));
  } else {
    PLANNER_WORDY(pc, "Lookup " << innerTableName << " callbacks");
    last_el =
      pc->createElementSpec(ElementPtr(new Lookup2(lookuposs.str(),
						   innerTable,
						   outerLookupKey,
						   innerIndexKey,
						   *comp_cb)));
  }

  if (tableInfo->primaryKeys == innerIndexKey) {
    // This is a primary key join so we don't need a secondary index
  } else {
    // Ensure there's a secondary index on the indexKey
    createSecondaryIndex(pc, innerTable, innerIndexKey);
  }
  
  int numFieldsProbe = outerNamesTracker->fieldNames.size();
  PLANNER_WORDY(pc, "Probe before merge " << outerNamesTracker->toString());
  outerNamesTracker->mergeWith(innerNamesTracker->fieldNames); 
  PLANNER_WORDY(pc, "Probe after merge " << outerNamesTracker->toString());

  pc->addElementSpec(last_el);
  pc->addElementSpec(noNull);
  
  // Take the joined tuples and produce the resulting path form the pel
  // projection.  Keep all fields from the probe, and all fields from
  // the base table that were not join keys.
  string temporaryResultName = "join"
    + pc->getRule()->getEventName() 
    + innerTableName	     
    + pc->getRule()->_ruleID;
  ostringstream pelProject;
  pelProject << "\""
             << temporaryResultName
             << "\" pop "; // table name
  for (int k = 0; k < numFieldsProbe; k++) {
    pelProject << "$0 " << k + 1 << " field pop ";
  }

  // And also pop all remaining base field numbers (those that did not
  // participate in the join.
  for (CommonTable::Key::iterator i = innerRemainingKey.begin();
       i != innerRemainingKey.end();
       i++) {
    pelProject << "$1 " << (*i) << " field pop ";
  }

  string pelProjectStr = pelProject.str();
  ostringstream oss1; 
  oss1 << "joinPel_" << pc->getRule()->_ruleID
       << "!"
       << temporaryResultName
       << "!";

  ElementSpecPtr transS =
    pc->createElementSpec(ElementPtr(new PelTransform(oss1.str(),
						      pelProjectStr)));
  PLANNER_WORDY(pc, "Join Pel Transform " << oss1.str());
  addPrint(pc, "AfterJoin", temporaryResultName, "j");  
  pc->addElementSpec(transS);

  return temporaryResultName;
}		   


 
void generateMultipleProbeElements(PlanContext* pc)
{
  ECA_Rule* curRule = pc->_ruleStrand->getRule();

  string joinProbeName = curRule->getEventName();
  for (uint k = 0;
       k < curRule->_probeTerms.size();
       k++) {    
    Parse_Functor* pf = curRule->_probeTerms.at(k);

    PLANNER_WORDY(pc, "Probing "
                  << joinProbeName
                  << " " + pf->toString());
    
    if (curRule->_aggWrap == false) {
      joinProbeName = generateProbeElements(pc, pf, joinProbeName);
    } else {
      b_cbv comp_cb = 0;
      comp_cb = pc->_agg_el->get_comp_cb();
      joinProbeName = generateProbeElements(pc, pf, joinProbeName, &comp_cb);
    }

     if (curRule->_probeTerms.size() - 1 != k) {
      ElementSpecPtr pullPush =
	pc->createElementSpec(ElementPtr(new TimedPullPush("ProbePullPush!" 
							   + curRule->_ruleID + "!" 
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
      PLANNER_WORDY(pc, "Selection term " << parse_select->toString() << " " 
                    << curRule->_ruleID);
      pelSelect(pc, parse_select, j); 
    }
    Parse_Assign* parse_assign 
      = dynamic_cast<Parse_Assign *>(curRule->_selectAssignTerms.at(j));
    if (parse_assign != NULL) {
      PLANNER_WORDY(pc, "Assignment term " << parse_assign->toString() << " " 
			+ curRule->_ruleID);
      pelAssign(pc, parse_assign, j);
    }
  }
}


void
generateProjectElements(PlanContext* pc)
{
  ECA_Rule* curRule = pc->getRule();
  Parse_Functor* pf = curRule->_action->_pf;
  PlanContext::FieldNamesTracker* curNamesTracker = pc->_namesTracker;
  PlanContext::FieldNamesTracker* newNamesTracker = 
    new PlanContext::FieldNamesTracker();

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
      newNamesTracker->fieldNames.push_back(parse_var->toString());
    }

    // Is this term in the functor an aggregate?
    Parse_Agg* aggExpr = dynamic_cast<Parse_Agg*>(pf->arg(k));
    if (aggExpr != NULL) {
      // It seems to be an aggregate
      pos = curNamesTracker->fieldPosition(aggExpr->v->toString());

      // Only add it if it's not a don't care
      if (pos != -1) {
        newNamesTracker->fieldNames.push_back(aggExpr->v->toString());
      }
    }

    if (pos == -1) {
      continue;
    }
    indices.push_back(pos + 1);
  }
 
  ostringstream pel;
  pel << "\"" << pf->fn->name << "\" pop";

  for (unsigned int k = 0; k < indices.size(); k++) {
    pel << " $" << indices.at(k) << " pop";
  }


  string pelTransformStr = pel.str();
  PLANNER_WORDY(pc, "Project head " << curNamesTracker->toString()
                << " " + pelTransformStr);
 
  ElementSpecPtr projectHeadPelTransform =
    pc->createElementSpec(ElementPtr(new PelTransform("ProjectHead!" 
						      + curRule->_ruleID 
						      + "!" + pc->_nodeID,
						      pelTransformStr)));

  pc->addElementSpec(projectHeadPelTransform);  
  addPrint(pc, "HeadProjection", pf->fn->name, "h");


  pc->_namesTracker = newNamesTracker;
  PLANNER_WORDY(pc, "Project head names tracker " << pc->_namesTracker->toString());
}


void
initializeAggWrap(PlanContext* pc)
{
  ECA_Rule* r = pc->getRule();
  if (r->_aggWrap == false) {
    return;
  }
  PLANNER_WORDY(pc, "Generate agg wrap "
                << r->toString() << " "
                << pc->_namesTracker->toString());

  Parse_Functor* headFunctor = r->_action->_pf;

  int aggField = headFunctor->aggregate();
  // there is an aggregate and involves an event, we need an agg wrap      
  Parse_Agg* aggExpr =
    dynamic_cast< Parse_Agg* > (headFunctor->arg(aggField));

  ostringstream oss;
  if (aggExpr == NULL) {
    oss << "Invalid aggregate field " << aggField
	<< " for rule " << r->_ruleID;  
    error(pc, oss.str());
  }

  // Is this a DONT_CARE aggregate?
  bool dontCare = (aggExpr->v == Parse_Agg::DONT_CARE->v);
      
  oss << "Aggwrap:" << r->_ruleID << ":" << pc->_nodeID;
  
  pc->_agg_el = new Aggwrap2(oss.str(),
                             aggExpr->aggName(), 
                             aggField + 1,
                             dontCare,
                             headFunctor->fn->name);
}


void
generateAggWrap(PlanContext* pc)
{
  ECA_Rule* r = pc->getRule();
  if (r->_aggWrap == false) {
    return;
  }
  Parse_Functor* headFunctor = r->_action->_pf;
  int aggField = headFunctor->aggregate();

  // We need a name tracker for the event functor of the Aggwrap.  We
  // could probably save this one from earlier but it isn't that big a
  // cost anyway...
  PlanContext::FieldNamesTracker eventNamesTracker(pc->getRule()
                                                   ->_event->_pf);
  ElementSpecPtr agg_elSpec 
    = pc->createElementSpec(ElementPtr(pc->_agg_el));

  pc->_ruleStrand->aggWrapperElement(pc->_conf, agg_elSpec);

  for (int k = 0;
       k < headFunctor->args();
       k++) {
    if (k != aggField) {
      // for each groupby value, figure out its location in the
      // initial event tuple, if not present, throw an error
      int pos =
	eventNamesTracker.fieldPosition(headFunctor->arg(k)->toString());
      if (pos == -1) {
        // This field was not found in the event. Throw an error
        error(pc, "Group-by field " 
              + headFunctor->arg(k)->toString()
              + " does not come from the event in the aggregation rule. "
              + " Currently, only group-by fields from the event "
              + "are supported for event-table aggregates, ");
        return;
      } else {
        pc->_agg_el->registerGroupbyField(pos + 1);
      }
    }
  }
}


void compileECARule(PlanContext* pc)
{
  PLANNER_INFO(pc, "Process rule " << pc->getRule()->toString());

  initializeAggWrap(pc);

  // first generate the incoming event  
  generateEventElement(pc);        

  // do all the joins
  generateMultipleProbeElements(pc);

  // do selections and assignments
  generateSelectionAssignmentElements(pc);

  // do projection based on head
  generateProjectElements(pc);

  // once strand is completed, generate a wrapper if necessary
  generateAggWrap(pc);

  // to complete the strand, generate the necessary action
  generateActionElement(pc);



  PLANNER_INFO(pc, "Finish processing rule "
               << pc->getRule()->toString());
}

#endif
