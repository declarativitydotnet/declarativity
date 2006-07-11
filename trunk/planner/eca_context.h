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
#include "localize_context.h"
#include "parser_util.h"
#include "tableStore.h"

class Parse_Event 
{
public:
  virtual ~Parse_Event() {};

  enum Event {INSERT, DELETE, RECV};
  
  Parse_Event(Parse_Functor *pf, Event e)
    : _pf(pf), _event(e) {};
  
  virtual string toString();  
  
  Parse_Functor* _pf;
  Event _event;
};


// struct for action
class Parse_Action 
{
public:
  virtual ~Parse_Action() {};

  enum Action {SEND, ADD, DELETE};

  Parse_Action(Parse_Functor *pf, Action a)
    : _pf(pf), _action(a) {};
  
  virtual string toString();
  
  Parse_Functor* _pf;
  Action _action;
};

class ECA_Rule {
public:
  ECA_Rule(string r) 
    : _ruleID(r) 
  { _event = NULL; _action = NULL; _aggTerm = NULL; };
  
  string toString();
  string toRuleString();
  string getEventName() { return _event->_pf->fn->name; }
  
  string _ruleID; 
  Parse_Event* _event;
  Parse_Action* _action;   
  std::vector<Parse_Functor*> _probeTerms;
  std::vector<Parse_Term*> _selectAssignTerms;
  Parse_AggTerm* _aggTerm;
};


class ECA_Context
{
public:
  void rewrite(Localize_Context* lctxt, TableStore* tableStore);
  std::vector<ECA_Rule*> getRules() { return _ecaRules; }
  string toString();
  
private:
  std::vector<ECA_Rule* > _ecaRules;
  void add_rule(ECA_Rule* eca_rule);
  void rewriteViewRule(OL_Context::Rule*, TableStore*);
  void rewriteEventRule(OL_Context::Rule*, TableStore*);
  std::vector<OL_Context::Rule*> localizeRewrite(OL_Context::Rule*, TableStore*);
};

typedef boost::shared_ptr<ECA_Context> ECA_ContextPtr;

#endif
