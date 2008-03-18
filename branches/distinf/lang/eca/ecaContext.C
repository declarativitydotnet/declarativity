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
#include "val_list.h"
#include "val_tuple.h"
#include "oper.h"
#include "val_int64.h"

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
      TuplePtr alternate;
      std::deque<TuplePtr> probes; 
      for (funcIter = functorTbl->lookup(CommonTable::theKey(CommonTable::KEY2), 
                                         CommonTable::theKey(CommonTable::KEY3), rule);
           !funcIter->done(); ) {
        TuplePtr functor = funcIter->next();
        if ((*functor)[TUPLE_ID] != (*rule)[catalog->attribute(RULE, "HEAD_FID")]) {
          if ((*functor)[catalog->attribute(FUNCTOR, "TID")] == Val_Null::mk()) {
            if (event) {
              std::cerr << "EVENT1: " << event->toString() << std::endl;
              std::cerr << "EVENT2: " << functor->toString() << std::endl;
              throw compile::eca::Exception("More than one event in rule: " + rule->toString());
            }
            event = functor;
          } else probes.push_back(functor);

          if ((*functor)[catalog->attribute(FUNCTOR, "ECA")] != Val_Null::mk() &&
              (*functor)[catalog->attribute(FUNCTOR, "ECA")]->toString() != "PROBE") {
            if (alternate) {
              std::cerr << "ALTERNATE EVENT1: " << alternate->toString() << std::endl;
              std::cerr << "ALTERNATE EVENT2: " << functor->toString() << std::endl;
              throw compile::eca::Exception("More than one event in rule: " + rule->toString());
            }
            alternate = functor;
          }
        }
      }

      if (!event && alternate) {
        event = alternate;
        for (std::deque<TuplePtr>::iterator iter = probes.begin();
             iter != probes.end(); iter++) {
          if ((*iter).get() == event.get()) {
            probes.erase(iter);
            break;
          }
        }
      }
      else if (!event) {
        if (probes.size() == 1) {
          event = probes.front();
          probes.pop_front();
        }
        else {
          string rname = (*rule)[catalog->attribute(RULE, "NAME")]->toString();
          throw compile::eca::Exception("RULE " + rname + ": can't handle materialize views with more than 1 body predicate!");
        }
      }

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
          /* Set the ECA type of the periodic event to be insert. */
          ecaEvent->set(functorEcaPos, Val_Str::mk("INSERT"));
      }
      else if ((*event)[catalog->attribute(FUNCTOR,"TID")] != Val_Null::mk() &&
               (*event)[catalog->attribute(FUNCTOR, "ECA")] == Val_Null::mk()) {
          ecaEvent->set(functorEcaPos, Val_Str::mk("DELTA_INSERT"));
      }
      else if ((*event)[catalog->attribute(FUNCTOR,"ECA")] == Val_Null::mk()) {
          /* All other event types are receive (like network packet).
             The reason being that we can not express table side effect
             events in OverLog. */
          ecaEvent->set(functorEcaPos, Val_Str::mk("RECV"));
      }
      /* Make sure the event appears in position 1 of the rule. */
      eventPos = Val_Int64::cast((*ecaEvent)[functorPosPos]);
      ecaEvent->set(functorPosPos, Val_Int64::mk(1));
      ecaEvent->freeze();
      functorTbl->insert(ecaEvent); // Commit the updated event functor
      
      headEca(catalog, rule, ecaHead);

      /* Ensure the head appears in position 0. */
      ecaHead->set(functorPosPos, Val_Int64::mk(0));
      ecaHead->freeze();
      functorTbl->remove(ecaHead);
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
	  uint32_t oldPos = Val_Int64::cast((*tp)[catalog->attribute(FUNCTOR, "POSITION")]);
	  if(oldPos <= eventPos){
	      tp->set(functorPosPos, Val_Int64::mk(oldPos+1));
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
	  uint32_t oldPos = Val_Int64::cast((*assign)[catalog->attribute(ASSIGN, "POSITION")]);
	  if(oldPos <= eventPos){
	    assign->set(catalog->attribute(ASSIGN, "POSITION"), Val_Int64::mk(oldPos+1));
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
	  uint32_t oldPos = Val_Int64::cast((*select)[catalog->attribute(SELECT, "POSITION")]);
	  if(oldPos <= eventPos){
	    select->set(catalog->attribute(SELECT, "POSITION"), Val_Int64::mk(oldPos + 1));
            select->freeze();
            if (!catalog->table(SELECT)->insert(select)){
	      throw Exception("Rewrite View: Can't insert selection. " + select->toString());
            }
	  }
          position++;
      }
    } 

    void
    Context::headEca(CommonTable::ManagerPtr catalog, TuplePtr rule, TuplePtr head) 
    {
      if ((*head)[catalog->attribute(FUNCTOR, "TID")] != Val_Null::mk()) {
        if (Val_Int64::cast((*rule)[catalog->attribute(RULE, "DELETE")])) {
          head->set(catalog->attribute(FUNCTOR, "ECA"), Val_Str::mk("DELETE"));
        }
        else {
          head->set(catalog->attribute(FUNCTOR, "ECA"), Val_Str::mk("ADD"));
        }
      }
      else {
          head->set(catalog->attribute(FUNCTOR, "ECA"), Val_Str::mk("SEND"));
      }
    }
  
  }
}
