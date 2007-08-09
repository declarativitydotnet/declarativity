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

#include "localContext.h"
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
  namespace local {
  using namespace opr;

    DEFINE_ELEMENT_INITS_NS(Context, "LocalContext", compile::local)

    Context::Context(string name)
    : compile::Context(name) { }

    Context::Context(TuplePtr args)
    : compile::Context((*args)[2]->toString()) { }

    int 
    Context::initialize()
    {
      CommonTable::ManagerPtr catalog = Plumber::catalog();

      CommonTablePtr programTbl = catalog->table(PROGRAM);  
      CommonTable::Iterator iter = programTbl->scan();
      while (!iter->done()) {
        compile::Context::program(catalog, iter->next());
      }

      return 0;
    }
  
    void
    Context::rule(CommonTable::ManagerPtr catalog, TuplePtr rule)
    {
      CommonTablePtr functorTbl = catalog->table(FUNCTOR);
      CommonTablePtr ruleTbl    = catalog->table(RULE);
      CommonTable::Iterator funcIter;

      /** Separate the event and probe functor terms */
      TuplePtr event;
      TuplePtr head;
      for (funcIter = functorTbl->lookup(CommonTable::theKey(CommonTable::KEY2), 
                                         CommonTable::theKey(CommonTable::KEY3), rule);
           !funcIter->done(); ) {
        TuplePtr functor = funcIter->next();
        if ((*functor)[catalog->attribute(FUNCTOR, "POSITION")] == Val_UInt32::mk(1)) {
          if (event) throw compile::local::Exception("LOCAL CONTEXT: More than one event in rule" + rule->toString() + " \n prev event" + event->toString() + " \n current event " + functor->toString() + "\n");
          event = functor;
        } 
        else if ((*functor)[catalog->attribute(FUNCTOR, "POSITION")] == Val_UInt32::mk(0)) {
          if (head) throw compile::local::Exception("LOCAL CONTEXT: More than one head in rule");
          head = functor;
        }
      }
      
      if(!event)
      {
	std::cout<<rule->toString();
      }
      TuplePtr triggerEvent = event->clone();
      ListPtr  headSchema   = Val_List::cast((*head)[catalog->attribute(FUNCTOR, "ATTRIBUTES")]);
      ListPtr  eventSchema  = Val_List::cast((*event)[catalog->attribute(FUNCTOR, "ATTRIBUTES")]);
      string   eventType    = (*triggerEvent)[catalog->attribute(FUNCTOR, "ECA")]->toString();
      string   eventName    = (*triggerEvent)[catalog->attribute(FUNCTOR, "NAME")]->toString();

      if (eventType != "RECV" &&
          (namestracker::location(headSchema) != namestracker::location(eventSchema) ||
           Val_UInt32::cast((*rule)[catalog->attribute(RULE, "TERM_COUNT")]) > 2)) {
        CommonTable::Key indexKey;
        indexKey.push_back(catalog->attribute(GLOBAL_EVENT, "NAME"));
        indexKey.push_back(catalog->attribute(GLOBAL_EVENT, "TYPE"));

        TuplePtr lookup = Tuple::mk();
        lookup->append(Val_Str::mk(eventName));
        lookup->append(Val_Str::mk(eventType));
        lookup->freeze();
        CommonTable::Iterator eIter = 
          catalog->table(GLOBAL_EVENT)->lookup(CommonTable::theKey(CommonTable::KEY01),
                                               indexKey, lookup);
        TuplePtr globalTrigger = eIter->next();

        if (globalTrigger) {
          triggerEvent->set(catalog->attribute(FUNCTOR, "NAME"), 
                            (*globalTrigger)[catalog->attribute(GLOBAL_EVENT, "TRIGGER")]);
          triggerEvent->set(catalog->attribute(FUNCTOR, "ECA"), Val_Str::mk("RECV")); 
          triggerEvent->freeze();
          functorTbl->insert(triggerEvent);
        }
        else {
          string globalEventName = eventName;
          string::size_type pos = eventName.find_last_of("::");
          string   name_space   = "";
          if (pos != string::npos) {
            name_space = eventName.substr(0, pos+1);
            eventName  = eventName.substr(pos+1, eventName.size());
          }
          if (eventName == "periodic") return; // Don't deal with this yet.

          ValuePtr newRid = Val_UInt32::mk(catalog->uniqueIdentifier());
          string triggerName = name_space + "trigger_" + eventType + "_" + eventName;
          int functorRidPos  = catalog->attribute(FUNCTOR, "RID");
          int functorPosPos  = catalog->attribute(FUNCTOR, "POSITION");
    
          /** Setup the trigger rule tuple */
          TuplePtr triggerRule  = Tuple::mk(RULE);
          TuplePtr triggerHead  = Tuple::mk(FUNCTOR, true);
          triggerRule->append(newRid);                    // Supply the predefined rule identifier.
          triggerRule->append((*rule)[catalog->attribute(RULE, "PID")]); // Same program identifier.
          triggerRule->append(Val_Str::mk(string("triggerRule_") + eventName)); // Rule name
          triggerRule->append((*triggerHead)[TUPLE_ID]);                        // Head tuple identifier
          triggerRule->append(Val_Null::mk());                           // P2DL text for this rule
          triggerRule->append(Val_UInt32::mk(0));                        // Not delete rule
          triggerRule->append(Val_UInt32::mk(2));                        // Contains head + event
          triggerRule->freeze();
          ruleTbl->insert(triggerRule);
    
          /** Point the old event tuple to the new trigger rule tuple */
          triggerEvent->set(functorRidPos, (*triggerRule)[TUPLE_ID]); // Point event to new rule
          triggerEvent->set(functorPosPos, Val_UInt32::mk(1));        // Event Position
          functorTbl->insert(triggerEvent);
      
          /** Create the trigger rule head tuple */
          triggerHead->append((*triggerRule)[TUPLE_ID]);  // The "trigger" rule identifier
          triggerHead->append(Val_Str::mk(triggerName));  // Event name
          triggerHead->append(Val_Null::mk());            // Reference table (event -> none)
          triggerHead->append(Val_Str::mk("SEND"));       // ECA action type
          triggerHead->append(Val_List::mk(eventSchema)); // Attributes
          triggerHead->append(Val_UInt32::mk(0));         // Position
          triggerHead->append(Val_Null::mk());            // Acess method
          triggerHead->freeze();
          functorTbl->insert(triggerHead);
      
          /** Create the trigger rule event tuple, assigning it to the origianl rule */
          TuplePtr event = Tuple::mk(FUNCTOR, true);
          event->append((*rule)[TUPLE_ID]);          // The original rule identifier
          event->append(Val_Str::mk(triggerName));   // Event name
          event->append(Val_Null::mk());             // Reference table (event -> none)
          event->append(Val_Str::mk("RECV"));        // ECA event type
          event->append(Val_List::mk(eventSchema));  // Attributes
          event->append(Val_UInt32::mk(1));          // Position
          event->append(Val_Null::mk());             // Acess method
          event->freeze();
          functorTbl->insert(event);

          /** Add the new global event trigger to the table for future registrations */
          globalTrigger = Tuple::mk(GLOBAL_EVENT, true);
          globalTrigger->append(Val_Str::mk(globalEventName)); 
          globalTrigger->append(Val_Str::mk(eventType)); 
          globalTrigger->append(Val_Str::mk(triggerName)); 
          globalTrigger->freeze();
          catalog->table(GLOBAL_EVENT)->insert(globalTrigger);
        }
      }
    }
  }
}
