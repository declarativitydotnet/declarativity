/*
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: Post-eca rewrite stage that does the following tasks:
 * 1> converts locSpec based accesses to transformed location based accesses in form of locSpecTable and versionTable
 * 2> changes the materialized "table" into materialized "tableversion" with corresponding modifications in the keys
 * Input: code with constructor and container creation rules. 
 * Assumption: constructors have local rhs
 * Placement: before eca stage
 *
 */

#include<iostream>
#include<vector>
#include "rewrite1Context.h"
#include "plumber.h"
#include "systemTable.h"
#include "value.h"
#include "val_str.h"
#include "val_null.h"
#include "val_uint32.h"
#include "val_int32.h"
#include "val_list.h"
#include "set.h"
#include "val_tuple.h"

namespace compile {
  namespace rewrite1{
    using namespace opr;

    DEFINE_ELEMENT_INITS_NS(Context, "Rewrite1Context", compile::rewrite1)

    Context::Context(string name)
    : compile::Context(name) { }

    Context::Context(TuplePtr args)
    : compile::Context((*args)[2]->toString()) { }

    void
    Context::rule(CommonTable::ManagerPtr catalog, TuplePtr rule)
    {
      SetPtr locSpecSet(new Set());
      SetPtr locationSet(new Set());
      CommonTablePtr functorTbl = catalog->table(FUNCTOR);
      CommonTable::Iterator funcIter;
      ValuePtr eventLocSpec;
      uint32_t termCount = Val_UInt32::cast((*rule)[catalog->attribute(RULE, "TERM_COUNT")]);
      TuplePtr *termList = new TuplePtr[termCount];
      Vector<TuplePtr> newTermList;
      int varSuffix = 0;

      int refPosPos = catalog->attribute(REF, "LOCSPECFIELD");
      
      for (funcIter = functorTbl->lookup(CommonTable::theKey(CommonTable::KEY2), 
                                         CommonTable::theKey(CommonTable::KEY3), rule);
           !funcIter->done(); ) {
        TuplePtr functor = funcIter->next();
	int functorPos = (*functor)[catalog->attribute(FUNCTOR, "POSITION")];
	termList[functorPos] = functor;
        if ((*functor)[TUPLE_ID] != (*rule)[catalog->attribute(RULE, "HEAD_FID")]) {
	  ListPtr attributes = Val_List::cast((*functor)[catalog->attribute(FUNCTOR, "ATTRIBUTES")]);
          if ((*functor)[catalog->attribute(FUNCTOR, "TID")] != Val_Null::mk()) {
	    locationSet->insert(attributes->front());
	    CommonTablePtr refTbl = catalog->table(REF);
	    CommonTable::Iterator refIter;
	      
	    for (refIter = refTbl->lookup(CommonTable::theKey(CommonTable::KEY4), CommonTable::theKey(CommonTable::KEY4), functor);
		 !refIter->done(); ) {
	      TuplePtr ref = refIter->next();
	      int32_t refPosVal = Val_Int32::cast((*ref)[refPosPos]);
	      locSpecSet->insert(attributes->at(refPosVal));
	    }
          } else {
	    eventLocSpec = attributes->front();
	  }
        }
      }

      // copy assign terms
      CommonTablePtr assignTbl = catalog->table(ASSIGN);
      CommonTable::Iterator assignIter;

      for (assignIter = assignTbl->lookup(CommonTable::theKey(CommonTable::KEY2), 
                                         CommonTable::theKey(CommonTable::KEY3), rule);
           !assignIter->done(); ) {
        TuplePtr assign = assignIter->next();
	int assignPos = (*assign)[catalog->attribute(ASSIGN, "POSITION")];
	termList[assignPos] = assign;
      }

      // copy select terms
      CommonTablePtr selectTbl = catalog->table(SELECT);
      CommonTable::Iterator selectIter;

      for (selectIter = selectTbl->lookup(CommonTable::theKey(CommonTable::KEY2), 
                                         CommonTable::theKey(CommonTable::KEY3), rule);
           !selectIter->done(); ) {
        TuplePtr select = selectIter->next();
	int selectPos = (*select)[catalog->attribute(SELECT, "POSITION")];
	termList[selectPos] = select;
      }

      if(!eventLocSpec){
	SetPtr possibleViewLocation = locationSet->difference(locSpecSet);
	if(possibleViewLocation->size() == 0){
	  throw compile::rewrite1::Exception("No event in eca processed rule" + rule->toString());
	}
	else{
	  // pick up the first non locSpec member as the eventLocSpec
	  eventLocSpec = (*possibleViewLocation->begin());
	}
      }

      CommonTablePtr tableTbl = catalog->table(TABLE);
      CommonTable::Iterator tIter;

      // now start sequential processing
      for(uint32_t i = 0; i < termCount; i++){
	// if non-functor then simply push it in the new term list
	// note that "new" functors are automatically local due to the extended new tuple
	if((*termList[i])[0] != FUNCTOR){
	  newTermList.push_back(termList[i]);
	  termList[i] = NULL;
	}
	else{
	  // handle functors
	  // check if materialized or not?
	  CommonTable::Key nameKey;
	  nameKey.push_back(catalog->attribute(FUNCTOR, "NAME"));
	  tIter = tableTbl->lookup(nameKey, CommonTable::theKey(CommonTable::KEY3), termList[i]);
	  if (!tIter->done()) {
	    // materialized
	    ListPtr attributes = Val_List::cast((*termList[i])[catalog->attribute(FUNCTOR, "ATTRIBUTES")]);
	    ValuePtr tupleLoc = attributes->front();
	    //if locSpec in locSpecSet, then add locSpec(@eventLocSpec, S, N, V), tableVersion(@N, V,...)
	    // otherwise: simply change the name to add version, and add the 0 version
	    if(locSpecSet->member(tupleLoc) == 1){
	      TuplePtr version = makeVersionedTuple(&varSuffix, termList[i]);
	      ListPtr verAttr = Val_List::cast((*version)[catalog->attribute(FUNCTOR, "ATTRIBUTES")]);
	      TuplePtr locSpecTuple = createLocSpecTuple(eventLocSpec, tupleLoc, verAttr->front(), verAttr->at(1));
	      newTermList.push_back(locSpecTuple);
	      newTermList.push_back(version);
	    }
	    else{
	      TuplePtr version = makeVersionedTuple(&varSuffix, termList[i], tupleLoc);
	      ListPtr verAttr = Val_List::cast((*version)[catalog->attribute(FUNCTOR, "ATTRIBUTES")]);
	      TuplePtr assign = createCurVerAssign(verAttr->at(1));
	      newTermList.push_back(assign);
	      newTermList.push_back(locSpecTuple);
	    }
	    functorTbl->remove(termList[i]);
	  }
	  else {
	    //event: simply copy
	    newTermList.push_back(termList[i]);
	    termList[i] = NULL;
	  }
	}
      }

      delete []termList;
#ifdef BLAH
      /** Separate the event and probe functor terms */
      
      std::deque<TuplePtr> probes; 
      for (funcIter = functorTbl->lookup(CommonTable::theKey(CommonTable::KEY2), 
                                         CommonTable::theKey(CommonTable::KEY3), rule);
           !funcIter->done(); ) {
        TuplePtr functor = funcIter->next();
        if ((*functor)[TUPLE_ID] != (*rule)[catalog->attribute(RULE, "HEAD_FID")]) {
          if ((*functor)[catalog->attribute(FUNCTOR, "TID")] == Val_Null::mk()) {
            if (event) throw compile::eca::Exception("More than one event in rule");
            event = functor;
          } else probes.push_back(functor);
        }
      }

      if (!event) {
        /* No event. We are dealing with a materialized view.
           Rewrite into a set of DELTA rule. */
        rewriteView(catalog, rule, probes);
      }
      else {
        // Fill in the base event and action types
        funcIter = functorTbl->lookup(CommonTable::theKey(CommonTable::KEY5), 
                                      CommonTable::theKey(CommonTable::KEY2), rule);
        if (funcIter->done()) {
          throw compile::eca::Exception("Rule functor head does not exist!");
        }
    
        TuplePtr ecaHead         = funcIter->next()->clone();
        TuplePtr ecaEvent        = event->clone();
        int      functorNamePos  = catalog->attribute(FUNCTOR, "NAME");
        int      functorEcaPos   = catalog->attribute(FUNCTOR, "ECA");
        int      functorPosPos   = catalog->attribute(FUNCTOR, "POSITION");
        string   eventName       = (*event)[functorNamePos]->toString();
      
        if (eventName.size() >= 8 && 
            eventName.substr(eventName.size()-8, eventName.size()) == "periodic") {
          if (probes.size() > 0) {
            /* Create a periodic trigger rule that will replace the periodic 
               event in the current rule. */
            rewritePeriodic(catalog, rule, ecaEvent, 
                            eventName.substr(0, eventName.size()-8));
          }
          else {
            /* Set the ECA type of the periodic event to be insert. */
            ecaEvent->set(functorEcaPos, Val_Str::mk("INSERT"));
          }
          ecaEvent->set(functorNamePos, Val_Str::mk("periodic"));
        }
        else {
          /* All other event types are receive (like network packet).
             The reason being that we can not express table side effect
             events in OverLog. */
          ecaEvent->set(functorEcaPos, Val_Str::mk("RECV"));
        }
        /* Make sure the event appears in position 1 of the rule. */
        ecaEvent->set(functorPosPos, Val_UInt32::mk(1));
        ecaEvent->freeze();
        functorTbl->insert(ecaEvent); // Commit the updated event functor
      
        if ((*ecaHead)[catalog->attribute(FUNCTOR, "TID")] == Val_Null::mk()) {
          /* The head predicate does not refer to an existing table. This
             means it is an event to the local node and gets the ECA value 
             SEND. */
          ecaHead->set(functorEcaPos, Val_Str::mk("SEND"));
        }
        else if ((*rule)[catalog->attribute(RULE, "DELETE")] == Val_UInt32::mk(1)) {
          /* The rule contains a deletion marker, so the functor ECA type
             is DELETE. Should we ensure that the head references a local table!! */
          ecaHead->set(functorEcaPos, Val_Str::mk("DELETE"));
        }
        else {
          /* The event is an insert into a table. Should ensure that
             the head references a local table!! */
          ecaHead->set(functorEcaPos, Val_Str::mk("ADD"));
        }

        /* Ensure the head appears in position 0. */
        ecaHead->set(functorPosPos, Val_UInt32::mk(0));
        functorTbl->insert(ecaHead); // Commit changes.
      
        /** Set all table predicates to ECA type PROBE. Also set
            the probe positions to follow the event tuple. */
        unsigned position = 2;
        for (std::deque<TuplePtr>::iterator iter = probes.begin(); 
             iter != probes.end(); iter++) {
          TuplePtr tp = (*iter)->clone();
          tp->set(functorEcaPos, Val_Str::mk("PROBE"));
          tp->set(functorPosPos, Val_UInt32::mk(position++));
          functorTbl->insert(tp);
        }

        // Copy over all assignments and selection predicates from old rule.
        CommonTable::Key key;
        CommonTable::Iterator Iter;
	key.push_back(catalog->attribute(ASSIGN, "RID"));

	Iter = catalog->table(ASSIGN)->lookup(CommonTable::theKey(CommonTable::KEY2), key, rule); 
        while (!Iter->done()) {
          TuplePtr assign = Iter->next()->clone();
          assign->set(catalog->attribute(ASSIGN, "POSITION"), Val_UInt32::mk(position++));
          assign->freeze();
          if (!catalog->table(ASSIGN)->insert(assign))
            throw Exception("Rewrite View: Can't insert assignment. " + rule->toString());
        }

        key.clear();

        key.push_back(catalog->attribute(SELECT, "RID"));
        Iter = catalog->table(SELECT)->lookup(CommonTable::theKey(CommonTable::KEY2), key, rule); 
        while (!Iter->done()) {
          TuplePtr select = Iter->next()->clone();
          select->set(catalog->attribute(SELECT, "POSITION"), Val_UInt32::mk(position++));
          select->freeze();
          if (!catalog->table(SELECT)->insert(select))
            throw Exception("Rewrite View: Can't insert selection. " + rule->toString());
        }

      }
#endif
    } 

    TuplePtr makeVersionedTuple(int &varSuffix, TuplePtr functor, ValuePtr location){
      TuplePtr     functorTp = Tuple::mk(FUNCTOR, true);
      ValuePtr ruleId = (*functor)[catalog->attribute(FUNCTOR, "RID")];
      functorTp->append(ruleId);
      string name  = Val_String::cast((*functor)[catalog->attribute(FUNCTOR, "NAME")]);

      functorTp->append(Val_Str::mk(name + compile::versionSuffix));   // Functor name
  
      // Fill in table reference if functor is materialized
      CommonTable::Key nameKey;
      nameKey.push_back(catalog->attribute(FUNCTOR, "NAME"));
      CommonTablePtr tableTbl = catalog->table(TABLE);
      CommonTable::Iterator tIter = 
        tableTbl->lookup(nameKey, CommonTable::theKey(CommonTable::KEY3), functorTp);
      if (!tIter->done()) 
        functorTp->append((*tIter->next())[TUPLE_ID]);
      else {
        functorTp->append(Val_Null::mk());
      }
  
      functorTp->append((*functor)[catalog->attribute(FUNCTOR, "ECA")]);          // The ECA flag
      functorTp->append(Val_Null::mk());          // The attributes field
      functorTp->append(Val_Null::mk());          // The position field
      functorTp->append((*functor)[catalog->attribute(FUNCTOR, "AM")]);          // The access method
      functorTp->append((*functor)[catalog->attribute(FUNCTOR, "NEW")]);        // The new field

      // Now take care of the functor arguments and variable dependencies
      ListPtr attributes = Val_List::cast((*functor)[catalog->attribute(FUNCTOR, "ATTRIBUTES")]);
      attributes->pop_front();
      //make version variable and push it
      attributes->
	// if location == NULL, create locationVariable and push it else push location variable
      functorTp->set(catalog->attribute(FUNCTOR, "ATTRIBUTES"), Val_List::mk(attributes));

      return functorTp;

    }

    TuplePtr createLocSpecTuple(ValuePtr location, ValuePtr locSpec, ValuePtr refLocation, ValuePtr ver){
    }
    
    TuplePtr createCurVerAssign(ValuePtr ver){
    }
 
    TuplePtr 
    Context::program(CommonTable::ManagerPtr catalog, TuplePtr program) 
    {
      CommonTable::Key indexKey;
      CommonTable::Iterator iter;

      indexKey.push_back(catalog->attribute(REF, "PID"));
      iter =
        catalog->table(REF)->lookup(CommonTable::theKey(CommonTable::KEY2), indexKey, program); 
      while (!iter->done()) {
        TuplePtr ref = iter->next();                                        // The row in the fact table

      }

      //add materialization for locSpecTable
      string scopedName = "::" + compile::locSpecTable;
      Table2::Key _keys;
      _keys.push_back(Val_UInt32::mk(0));
      _keys.push_back(Val_UInt32::mk(1));
      _keys.push_back(Val_UInt32::mk(2));
      _keys.push_back(Val_UInt32::mk(3));

      catalog->createTable(scopedName, _keys, Table2::NO_SIZE, Table2::NO_EXPIRATION);

      TuplePtr newProgram = this->compile::Context::program(catalog, program);

      //change the materialization statements for other tables to include the extended name: 
      //version suffix and corresponding changes in the key

      return newProgram;

    }

  }
}
