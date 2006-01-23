// -*- c-basic-offset: 2; related-file-name: "ol_context.C" -*-
/*
 * @(#)$Id$
 *
 * Copyright (c) 2005 Intel Corporation. All rights reserved.
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Rewrite environment for ECA rules
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
#include "catalog.h"

// struct for an event term
class Parse_Event 
{
public:
  virtual ~Parse_Event() {};

  enum Event {UPDATE, AGGUPDATE, RECV, PERIODIC};
  
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
  void eca_rewrite(OL_Context* ctxt, Catalog* catalog);
  string toString();
  void add_rule(ECA_Rule* eca_rule);

  std::vector<ECA_Rule* > _ecaRules;
  
private:
  void activateLocalizedRule(OL_Context::Rule*, Catalog*);
  std::vector<OL_Context::Rule*> localizeRule(OL_Context::Rule*, Catalog*);
};

typedef boost::shared_ptr<ECA_Context> ECA_ContextPtr;

#endif
