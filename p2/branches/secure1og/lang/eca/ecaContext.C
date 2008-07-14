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
        key.push_back(catalog->attribute(SELECT, "RID"));
        CommonTable::Iterator Iter;
        Iter = catalog->table(SELECT)->lookup(CommonTable::theKey(CommonTable::KEY2), key, rule); 
        while (!Iter->done()) {
          TuplePtr select = Iter->next()->clone();
          select->set(catalog->attribute(SELECT, "POSITION"), Val_UInt32::mk(position++));
          select->freeze();
          if (!catalog->table(SELECT)->insert(select))
            throw Exception("Rewrite View: Can't insert selection. " + rule->toString());
        }

        key.clear();
	key.push_back(catalog->attribute(ASSIGN, "RID"));
	Iter = catalog->table(ASSIGN)->lookup(CommonTable::theKey(CommonTable::KEY2), key, rule); 
        while (!Iter->done()) {
          TuplePtr assign = Iter->next()->clone();
          assign->set(catalog->attribute(ASSIGN, "POSITION"), Val_UInt32::mk(position++));
          assign->freeze();
          if (!catalog->table(ASSIGN)->insert(assign))
            throw Exception("Rewrite View: Can't insert assignment. " + rule->toString());
        }
      }
    } 
  
    void
    Context::rewriteView(CommonTable::ManagerPtr catalog, TuplePtr rule, 
                         std::deque<TuplePtr>& baseTables)
    {
      CommonTable::Key key;

      TuplePtr head;
      key.push_back(catalog->attribute(FUNCTOR, "RID"));
      CommonTable::Iterator Iter;
      Iter = catalog->table(FUNCTOR)->lookup(CommonTable::theKey(CommonTable::KEY2),
                                             key, rule);
      if (Iter->done()) 
        throw Exception("No head table predicate in rule: " + rule->toString());
      head = Iter->next();

      for (std::deque<TuplePtr>::iterator iter1 = baseTables.begin();
           iter1 != baseTables.end(); iter1++) {
        // Make the new delta rule and predicate head.
        TuplePtr deltaRule = rule->clone(RULE, true);
        TuplePtr deltaHead = head->clone(FUNCTOR, true);
        deltaRule->set(catalog->attribute(RULE, "HEAD_FID"), (*deltaHead)[TUPLE_ID]);
        deltaHead->set(catalog->attribute(FUNCTOR, "RID"), (*deltaRule)[TUPLE_ID]);

        if ((*deltaHead)[catalog->attribute(FUNCTOR, "TID")] == Val_Null::mk())
          deltaHead->set(catalog->attribute(FUNCTOR, "ECA"), Val_Str::mk("SEND"));
        else if ((*rule)[catalog->attribute(RULE, "DELETE")] == Val_Int32::mk(1))
          deltaHead->set(catalog->attribute(FUNCTOR, "ECA"), Val_Str::mk("DELETE"));
        else
          deltaHead->set(catalog->attribute(FUNCTOR, "ECA"), Val_Str::mk("ADD"));
        deltaHead->set(catalog->attribute(FUNCTOR, "POSITION"), Val_UInt32::mk(0));

        deltaRule->freeze();
        deltaHead->freeze();
        if (!catalog->table(RULE)->insert(deltaRule))
          throw Exception("Rewrite View: Can't insert delta rule. " + rule->toString());
        if (!catalog->table(FUNCTOR)->insert(deltaHead))
          throw Exception("Rewrite View: Can't insert delta head. " + rule->toString());

        // Create delta rule event 
        TuplePtr deltaEvent = (*iter1)->clone(FUNCTOR, true);
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
          if (*iter1 != *iter2) {
            TuplePtr deltaProbe = (*iter2)->clone(FUNCTOR, true);
            deltaProbe->set(catalog->attribute(FUNCTOR, "ECA"), Val_Str::mk("PROBE"));
            deltaProbe->set(catalog->attribute(FUNCTOR, "RID"), (*deltaRule)[TUPLE_ID]);
            deltaProbe->set(catalog->attribute(FUNCTOR, "POSITION"), Val_UInt32::mk(position++));
            deltaProbe->freeze();
            if (!catalog->table(FUNCTOR)->insert(deltaProbe))
              throw Exception("Rewrite View: Can't insert probe. " + rule->toString());
          }
        }
        
        // Copy over all assignments and selection predicates from old rule.
        key.clear();
        key.push_back(catalog->attribute(SELECT, "RID"));
        CommonTable::Iterator Iter;
        Iter = catalog->table(SELECT)->lookup(CommonTable::theKey(CommonTable::KEY2),
                                              key, rule); 
        while (!Iter->done()) {
          TuplePtr select = Iter->next()->clone(SELECT, true);
          select->set(catalog->attribute(SELECT, "RID"), (*deltaRule)[TUPLE_ID]);
          select->set(catalog->attribute(SELECT, "POSITION"), Val_UInt32::mk(position++));
          select->freeze();
          if (!catalog->table(SELECT)->insert(select))
            throw Exception("Rewrite View: Can't insert selection. " + rule->toString());
        }

        key.clear();
	key.push_back(catalog->attribute(ASSIGN, "RID"));
	Iter = catalog->table(ASSIGN)->lookup(CommonTable::theKey(CommonTable::KEY2),
                                              key, rule); 

        while (!Iter->done()) {
          TuplePtr assign = Iter->next()->clone(ASSIGN, true);
          assign->set(catalog->attribute(ASSIGN, "RID"), (*deltaRule)[TUPLE_ID]);
          assign->set(catalog->attribute(ASSIGN, "POSITION"), Val_UInt32::mk(position++));
          assign->freeze();
          if (!catalog->table(ASSIGN)->insert(assign))
            throw Exception("Rewrite View: Can't insert assignment. " + rule->toString());
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
      head->append(Val_Str::mk(triggerName)); // Event name
      head->append(Val_Null::mk());           // Reference table (event -> none)
      head->append(Val_Str::mk("SEND"));      // ECA action type
      head->append(Val_List::mk(schema));     // Attributes
      head->append(Val_UInt32::mk(0));        // Position
      head->append(Val_Null::mk());           // Access method
      head->freeze();
      functorTbl->insert(head);
  
      /** Create the trigger rule event tuple, assigning it to the origianl rule */
      event->append((*rule)[TUPLE_ID]);        // The original rule identifier
      event->append(Val_Str::mk(triggerName)); // Event name
      event->append(Val_Null::mk());           // Reference table (event -> none)
      event->append(Val_Str::mk("RECV"));      // ECA event type
      event->append(Val_List::mk(schema));     // Attributes
      event->append(Val_UInt32::mk(1));        // Position
      event->append(Val_Null::mk());           // Access method
      event->freeze();
      functorTbl->insert(event);

    }
  }
}
