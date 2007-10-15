/*
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: 
 *
 */

#include "ecaContext.h"
#include "plumber.h"
#include "systemTable.h"
#include "value.h"
#include "val_str.h"
#include "val_null.h"
#include "val_uint32.h"
#include "val_int32.h"
#include "val_list.h"
#include "val_tuple.h"
#include "oper.h"

namespace compile {
  namespace eca {
    using namespace opr;

    DEFINE_ELEMENT_INITS_NS(Context, "EcaContext", compile::eca)

    Context::Context(string name)
    : compile::Context(name) { }

    Context::Context(TuplePtr args)
    : compile::Context((*args)[2]->toString()) { }

    void
    Context::rule(CommonTable::ManagerPtr catalog, TuplePtr rule)
    {
      CommonTablePtr functorTbl = catalog->table(FUNCTOR);
      CommonTable::Iterator funcIter;

      /** Separate the event and probe functor terms */
      TuplePtr event;
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
	uint32_t eventPos = -1;

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
	eventPos = Val_UInt32::cast((*ecaEvent)[functorPosPos]);
        ecaEvent->set(functorPosPos, Val_UInt32::mk(1));
        ecaEvent->freeze();
        functorTbl->insert(ecaEvent); // Commit the updated event functor
      
        headEca(catalog, rule, ecaHead);

        /* Ensure the head appears in position 0. */
        ecaHead->set(functorPosPos, Val_UInt32::mk(0));
        ecaHead->freeze();
        if (!functorTbl->insert(ecaHead)) {
	  throw Exception("ECA Rewrite: Can't update head. " + ecaHead->toString());
        }
      
        /** Set all table predicates to ECA type PROBE. Also set
            the probe positions to follow the event tuple. */
        unsigned position = 2;
        for (std::deque<TuplePtr>::iterator iter = probes.begin(); 
             iter != probes.end(); iter++) {
          TuplePtr tp = (*iter)->clone();
          tp->set(functorEcaPos, Val_Str::mk("PROBE"));
	  uint32_t oldPos = Val_UInt32::cast((*tp)[catalog->attribute(FUNCTOR, "POSITION")]);
	  if(oldPos <= eventPos){
	      tp->set(functorPosPos, Val_UInt32::mk(oldPos+1));
	  }
	  position++;
          tp->freeze();
          functorTbl->insert(tp);
        }

        // Copy over all assignments and selection predicates from old rule.
        CommonTable::Key key;
        CommonTable::Iterator Iter;
	key.push_back(catalog->attribute(ASSIGN, "RID"));

	Iter = catalog->table(ASSIGN)->lookup(CommonTable::theKey(CommonTable::KEY2), key, rule); 
        while (!Iter->done()) {
          TuplePtr assign = Iter->next()->clone();
	  uint32_t oldPos = Val_UInt32::cast((*assign)[catalog->attribute(ASSIGN, "POSITION")]);
	  if(oldPos <= eventPos){
	    assign->set(catalog->attribute(ASSIGN, "POSITION"), Val_UInt32::mk(oldPos+1));
            assign->freeze();
            if (!catalog->table(ASSIGN)->insert(assign)){
	      throw Exception("Rewrite View: Can't insert assignment. " + assign->toString());
	    }
          }
	  position++;
        }

        key.clear();

        key.push_back(catalog->attribute(SELECT, "RID"));
        Iter = catalog->table(SELECT)->lookup(CommonTable::theKey(CommonTable::KEY2), key, rule); 
        while (!Iter->done()) {
          TuplePtr select = Iter->next()->clone();
	  uint32_t oldPos = Val_UInt32::cast((*select)[catalog->attribute(SELECT, "POSITION")]);
	  if(oldPos <= eventPos){
	    select->set(catalog->attribute(SELECT, "POSITION"), Val_UInt32::mk(oldPos + 1));
            select->freeze();
            if (!catalog->table(SELECT)->insert(select)){
	      throw Exception("Rewrite View: Can't insert selection. " + select->toString());
            }
	  }
	  position++;
        }
      }
    } 

    void
    Context::headEca(CommonTable::ManagerPtr catalog, TuplePtr rule, TuplePtr head) 
    {
      if ((*head)[catalog->attribute(FUNCTOR, "TID")] != Val_Null::mk()) {
        std::string headName = (*head)[catalog->attribute(FUNCTOR, "NAME")]->toString();
        if (headName.compare(0, 5, "sys::") == 0) {
          /* System tables do not require this rewrite, since remote updates disallowed!! */
          if ((*rule)[catalog->attribute(RULE, "DELETE")] == Val_UInt32::mk(1)) {
            head->set(catalog->attribute(FUNCTOR, "ECA"), Val_Str::mk("DELETE"));
          }
          else {
            head->set(catalog->attribute(FUNCTOR, "ECA"), Val_Str::mk("ADD"));
          }
        } else {
          /* Create a row in the SIDE_EFFECT table */
          TuplePtr sideEffectTp = Tuple::mk(SIDE_EFFECT, true);
          sideEffectTp->append((*head)[catalog->attribute(FUNCTOR, "NAME")]);
          if ((*rule)[catalog->attribute(RULE, "DELETE")] == Val_UInt32::mk(1)) {
            sideEffectTp->append(Val_Str::mk("DELETE"));
          }
          else {
            sideEffectTp->append(Val_Str::mk("ADD"));
          }
          sideEffectTp->freeze();
          /* Check if we've already done this rewrite for this table. */
          CommonTablePtr sideEffectTbl = catalog->table(SIDE_EFFECT);
          CommonTable::Iterator iter = 
            sideEffectTbl->lookup(CommonTable::theKey(CommonTable::KEY34),
                                  CommonTable::theKey(CommonTable::KEY34), 
                                  sideEffectTp);

          if (iter->done()) {
            /* Do the rewrite. */
            string rname = (*rule)[catalog->attribute(RULE, "NAME")]->toString();
            rname += "_SideEffect_" + (*sideEffectTp)[catalog->attribute(SIDE_EFFECT, "TYPE")]->toString();
   
            TuplePtr sideEffectRule = rule->clone(RULE, true);
            TuplePtr sideEffectHead = head->clone(FUNCTOR, true);
            TuplePtr sideEffectEvent = head->clone(FUNCTOR, true);
            sideEffectRule->set(catalog->attribute(RULE, "HEAD_FID"), (*sideEffectHead)[TUPLE_ID]);
            sideEffectRule->set(catalog->attribute(RULE, "NAME"), Val_Str::mk(rname));
            sideEffectHead->set(catalog->attribute(FUNCTOR, "RID"), (*sideEffectRule)[TUPLE_ID]);
            sideEffectEvent->set(catalog->attribute(FUNCTOR, "RID"), (*sideEffectRule)[TUPLE_ID]);
            sideEffectHead->set(catalog->attribute(FUNCTOR, "POSITION"), Val_UInt32::mk(0));
            sideEffectEvent->set(catalog->attribute(FUNCTOR, "POSITION"), Val_UInt32::mk(1));

            sideEffectEvent->set(catalog->attribute(FUNCTOR, "ECA"), Val_Str::mk("RECV"));
            sideEffectHead->set(catalog->attribute(FUNCTOR, "ECA"), 
                                (*sideEffectTp)[catalog->attribute(SIDE_EFFECT, "TYPE")]);

            sideEffectRule->freeze();
            sideEffectHead->freeze();
            sideEffectEvent->freeze();
            catalog->table(FUNCTOR)->insert(sideEffectHead);
            catalog->table(FUNCTOR)->insert(sideEffectEvent);
            catalog->table(RULE)->insert(sideEffectRule);
            catalog->table(SIDE_EFFECT)->insert(sideEffectTp);
          }
          /* Make the head ECA a send, which will loop around to the above rewrite. */
          head->set(catalog->attribute(FUNCTOR, "ECA"), Val_Str::mk("SEND"));
        }
      }
      else {
          head->set(catalog->attribute(FUNCTOR, "ECA"), Val_Str::mk("SEND"));
      }
    }
  
    void
    Context::rewriteView(CommonTable::ManagerPtr catalog, TuplePtr rule, 
                         std::deque<TuplePtr>& baseTables)
    {
      CommonTable::Key key;

      TuplePtr head;
      key.push_back(catalog->attribute(FUNCTOR, "FID"));
      CommonTable::Iterator Iter;
      Iter = catalog->table(FUNCTOR)->lookup(CommonTable::theKey(CommonTable::KEY5),
                                             key, rule);
      if (Iter->done()) 
        throw Exception("No head table predicate in rule: " + rule->toString());
      head = Iter->next()->clone();

      std::string headName = (*head)[catalog->attribute(FUNCTOR, "NAME")]->toString();
      headEca(catalog, rule, head);
      head->set(catalog->attribute(FUNCTOR, "POSITION"), Val_UInt32::mk(0));

      for (std::deque<TuplePtr>::iterator iter = baseTables.begin();
           iter != baseTables.end(); iter++) {
        if ((**iter)[catalog->attribute(FUNCTOR, "NOTIN")] == Val_UInt32::mk(true)) {
          continue; // Don't create DELTA rule for notin table predicate.
        }

        // Make the new delta rule and predicate head.
	uint32_t eventPos = -1;
        TuplePtr deltaRule = rule->clone(RULE, true);
        TuplePtr deltaHead = head->clone(FUNCTOR, true);
        deltaRule->set(catalog->attribute(RULE, "HEAD_FID"), (*deltaHead)[TUPLE_ID]);
	ostringstream oss;
	oss<<RULENAMEPREFIX<<ruleCounter++;
	deltaRule->set(catalog->attribute(RULE, "NAME"), Val_Str::mk(oss.str()));
        deltaHead->set(catalog->attribute(FUNCTOR, "RID"), (*deltaRule)[TUPLE_ID]);

        deltaRule->freeze();
        deltaHead->freeze();
        if (!catalog->table(RULE)->insert(deltaRule))
          throw Exception("Rewrite View: Can't insert delta rule. " + rule->toString());
        if (!catalog->table(FUNCTOR)->insert(deltaHead))
          throw Exception("Rewrite View: Can't insert delta head. " + rule->toString());

        // Create delta rule event 
        TuplePtr deltaEvent = (*iter)->clone(FUNCTOR, true);
	eventPos = Val_UInt32::cast((*deltaEvent)[catalog->attribute(FUNCTOR, "POSITION")]);
        deltaEvent->set(catalog->attribute(FUNCTOR, "ECA"), Val_Str::mk("DELTA"));
        deltaEvent->set(catalog->attribute(FUNCTOR, "RID"), (*deltaRule)[TUPLE_ID]);
        deltaEvent->set(catalog->attribute(FUNCTOR, "POSITION"), Val_UInt32::mk(1));
        deltaEvent->freeze();
        if (!catalog->table(FUNCTOR)->insert(deltaEvent))
          throw Exception("Rewrite View: Can't insert delta event. " + rule->toString());

        // Clone all probes, and reference to the new delta rule
        unsigned position = 2;
        for (std::deque<TuplePtr>::iterator iter2 = baseTables.begin();
             iter2 != baseTables.end(); iter2++) {
          if (*iter != *iter2) {
            TuplePtr deltaProbe = (*iter2)->clone(FUNCTOR, true);
            deltaProbe->set(catalog->attribute(FUNCTOR, "ECA"), Val_Str::mk("PROBE"));
            deltaProbe->set(catalog->attribute(FUNCTOR, "RID"), (*deltaRule)[TUPLE_ID]);
	    uint32_t oldPos = Val_UInt32::cast((*deltaProbe)[catalog->attribute(FUNCTOR, "POSITION")]);
	    if(oldPos <= eventPos){
	      deltaProbe->set(catalog->attribute(FUNCTOR, "POSITION"), Val_UInt32::mk(oldPos + 1));
	    }
	    position++;
            deltaProbe->freeze();
            if (!catalog->table(FUNCTOR)->insert(deltaProbe))
              throw Exception("Rewrite View: Can't insert probe. " + rule->toString());
          }
        }
        
        // Copy over all assignments and selection predicates from old rule.
        key.clear();
	key.push_back(catalog->attribute(ASSIGN, "RID"));
	Iter = catalog->table(ASSIGN)->lookup(CommonTable::theKey(CommonTable::KEY2),
                                              key, rule); 

        while (!Iter->done()) {
          TuplePtr assign = Iter->next()->clone(ASSIGN, true);
          assign->set(catalog->attribute(ASSIGN, "RID"), (*deltaRule)[TUPLE_ID]);
	  uint32_t oldPos = Val_UInt32::cast((*assign)[catalog->attribute(ASSIGN, "POSITION")]);
	  if(oldPos <= eventPos){
	    assign->set(catalog->attribute(ASSIGN, "POSITION"), Val_UInt32::mk(oldPos + 1));
	  }
	  position++;
          assign->freeze();
          if (!catalog->table(ASSIGN)->insert(assign)){
	    throw Exception("Rewrite View: Can't insert assignment. " + assign->toString());
	  }
        }

        key.clear();
        key.push_back(catalog->attribute(SELECT, "RID"));
        CommonTable::Iterator Iter;
        Iter = catalog->table(SELECT)->lookup(CommonTable::theKey(CommonTable::KEY2),
                                              key, rule); 
        while (!Iter->done()) {
          TuplePtr select = Iter->next()->clone(SELECT, true);
          select->set(catalog->attribute(SELECT, "RID"), (*deltaRule)[TUPLE_ID]);
	  uint32_t oldPos = Val_UInt32::cast((*select)[catalog->attribute(SELECT, "POSITION")]);
	  if(oldPos <= eventPos){
	    select->set(catalog->attribute(SELECT, "POSITION"), Val_UInt32::mk(oldPos+1));
	  }
	  position++;
          select->freeze();
          if (!catalog->table(SELECT)->insert(select)){
	    throw Exception("Rewrite View: Can't insert selection. " + select->toString());
	  }
        }
      }

      // Now clean up the mess we've made.
      if (!catalog->table(RULE)->remove(rule)) {
        throw Exception("Rewrite View: Can't remove old rule. " + rule->toString());
      }
    }

    void
    Context::rewritePeriodic(CommonTable::ManagerPtr catalog, TuplePtr rule, 
                             TuplePtr periodic, string name_space)
    {
      CommonTablePtr ruleTbl    = catalog->table(RULE);
      CommonTablePtr functorTbl = catalog->table(FUNCTOR);
  
      TuplePtr event = Tuple::mk(FUNCTOR, true);
  
      /** Create the trigger rule */ 
      string ruleName    = (*rule)[catalog->attribute(FUNCTOR, "NAME")]->toString();
      string triggerName = name_space + "periodicTrigger_" + ruleName;
      int functorRidPos  = catalog->attribute(FUNCTOR, "RID");
      int functorEcaPos  = catalog->attribute(FUNCTOR, "ECA");
      int functorPosPos  = catalog->attribute(FUNCTOR, "POSITION");

      ListPtr periodicSchema = Val_List::cast((*periodic)[catalog->attribute(FUNCTOR, "ATTRIBUTES")]);
      ListPtr schema         = List::mk();
      /** Get rid of any non-variable attributes associated with periodic. */
      for (ValPtrList::const_iterator iter = periodicSchema->begin();
           iter != periodicSchema->end(); iter++) {
        if ((*Val_Tuple::cast(*iter))[TNAME]->toString() == VAR ||
            (*Val_Tuple::cast(*iter))[TNAME]->toString() == LOC) {
          schema->append(*iter);
        }
      }
  
      /** Setup the trigger rule tuple */
      TuplePtr triggerRule  = Tuple::mk(RULE, true);
      TuplePtr head  = Tuple::mk(FUNCTOR, true);
      triggerRule->append((*rule)[catalog->attribute(RULE, "PID")]); // Same program identifier.
      triggerRule->append(Val_Str::mk(string("periodicTrigger_") + ruleName)); // Rule name
      triggerRule->append((*head)[TUPLE_ID]);                        // Head tuple identifier
      triggerRule->append(Val_Null::mk());                           // P2DL text for this rule
      triggerRule->append(Val_UInt32::mk(0));                        // Not delete rule
      triggerRule->append(Val_UInt32::mk(2));                        // Rule contains head and event.
      triggerRule->freeze();
      ruleTbl->insert(triggerRule);

      /** Point the periodic event tuple to the new trigger rule tuple */
      periodic->set(functorRidPos, (*triggerRule)[TUPLE_ID]); // Point periodic to new rule
      periodic->set(functorEcaPos, Val_Str::mk("INSERT"));    // Periodic ECA type
      periodic->set(functorPosPos, Val_UInt32::mk(1));        // Periodic Position
  
      /** Create the trigger rule head tuple */
      head->append((*triggerRule)[TUPLE_ID]); // The "trigger" rule identifier
      head->append(Val_UInt32::mk(false));      // NOTIN
      head->append(Val_Str::mk(triggerName)); // Event name
      head->append(Val_Null::mk());           // Reference table (event -> none)
      head->append(Val_Str::mk("SEND"));      // ECA action type
      head->append(Val_List::mk(schema));     // Attributes
      head->append(Val_UInt32::mk(0));        // Position
      head->append(Val_Null::mk());           // Access method
      head->append(Val_UInt32::mk(0));        // NEW?
      head->freeze();
      functorTbl->insert(head);
  
      /** Create the trigger rule event tuple, assigning it to the origianl rule */
      event->append((*rule)[TUPLE_ID]);        // The original rule identifier
      event->append(Val_UInt32::mk(false));      // NOTIN
      event->append(Val_Str::mk(triggerName)); // Event name
      event->append(Val_Null::mk());           // Reference table (event -> none)
      event->append(Val_Str::mk("RECV"));      // ECA event type
      event->append(Val_List::mk(schema));     // Attributes
      event->append(Val_UInt32::mk(1));        // Position
      event->append(Val_Null::mk());           // Access method
      event->append(Val_UInt32::mk(0));           // NEW?
      event->freeze();
      functorTbl->insert(event);

    }
  }
}
