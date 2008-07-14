// -*- c-basic-offset: 2; related-file-name: "ol_context.C" -*-
/*
 * @(#)$Id$
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: Rewritten context from Overlog to ECA
 *
 */

#ifndef __ECA_CONTEXT_H__
#define __ECA_CONTEXT_H__

#include <vector>
#include <map>
#include <set>
#include "value.h"
#include "tuple.h"
#include "ol_context.h"
#include "parser_util.h"
#include "tableStore.h"
#include "localize_context.h"

class Parse_Event 
{
public:
  virtual ~Parse_Event() {};

  enum Event {
    INSERT,
    DELETE,
    RECV,
    REFRESH
  };
  

  Parse_Event(Parse_Functor *pf, Event e);
  

  virtual string
  toString();  
  
  
  Parse_Functor* _pf;
  

  Event _event;
};


// struct for action
class Parse_Action 
{
public:
  virtual ~Parse_Action() {};

  enum Action {
    SEND,                       // Generate a new event, local or remote
    ADD,                        // Add to the database
    DELETE,                     // Remove from the database
    DROP                        // Do nothing. This is a no-op rule
  };

  Parse_Action(Parse_Functor *pf, Action a)
    : _pf(pf), _action(a) {};
  
  virtual string toString();
  
  Parse_Functor* _pf;

  Action _action;
};


/** An individual ECA rule structure */
class ECA_Rule {
public:
  /** Create a new empty rule */
  ECA_Rule(string r);
  

  /** Generate a string representation of the rule */
  string
  toString();


  string
  toRuleString();


  /** What's the name of my event tuple? */
  string
  getEventName();
  

  string _ruleID; 

  /** The event of this ECA rule */
  Parse_Event* _event;


  /** The action of this rule */
  Parse_Action* _action;   


  /** The join conditions of this rule */
  std::vector<Parse_Functor*> _probeTerms;


  /** The selection/assginment conditions of this rule */
  std::vector<Parse_Term*> _selectAssignTerms;


  /** Is this an aggwrap rule? */
  bool _aggWrap;
};




class ECA_Context
{
public:
  /** Rewrite all Localized OverLog rules in the localize context into
      ECA rules in the current ECA context, using the given table store
      for state access */
  void
  rewrite(Localize_Context* lctxt, TableStore* tableStore);


  std::vector<ECA_Rule*>
  getRules() { return _ecaRules; }


  string toString();
  

private:
  std::vector<ECA_Rule* > _ecaRules;


  void
  add_rule(ECA_Rule* eca_rule);
  

  void
  rewriteViewRule(OL_Context::Rule*, TableStore*);
  

  /** Rewrites a localized, single-event OverLog rule into ECA */
  void
  rewriteEventRule(OL_Context::Rule*, TableStore*);
  

  void
  rewriteAggregateView(OL_Context::Rule* rule, 
                       TableStore *tableStore);


  /** Generate an action for a rule given its old version (from the
      localized OverLog), its new version so far (i.e., the right hand
      side ECA rule), the location specifier of the body and the table
      store */
  void
  generateActionHead(OL_Context::Rule* rule,
                     string bodyLoc,
                     TableStore* tableStore,
                     ECA_Rule* eca_rule);


  /** Create a new ECA rule that does nothing. This is used for watched
      events that have no actual ECA rules listening for them.  Without
      an event listener, no watches can be implemented further down in
      the planner */
  void
  watchStubRule(string watchedTupleName);
};

typedef boost::shared_ptr<ECA_Context> ECA_ContextPtr;

#endif
