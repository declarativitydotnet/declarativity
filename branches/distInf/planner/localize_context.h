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

#ifndef __LOCAL_CONTEXT_H__
#define __LOCAL_CONTEXT_H__

#include <vector>
#include <map>
#include <set>
#include "value.h"
#include "tuple.h"
#include "ol_context.h"
#include "parser_util.h"
#include "tableStore.h"


class Localize_Context
{

public:
  
  void rewrite(OL_Context* ctxt, TableStore* tableStore);
  string toString();
  std::vector<OL_Context::Rule*> getRules() { return _localizedRules; }
  
private:
  void rewriteRule(OL_Context::Rule* rule, TableStore* tableStore);
  OL_Context::Rule* addSendRule(OL_Context::Rule* nextRule,
				std::list<Parse_Term*> newTerms,
				string newFunctorName,
				Parse_Functor* functor,
				string loc, 
				boost::posix_time::time_duration minLifetime,
				std::vector<string> fieldNames,
				TableStore* tableStore);  
  void add_rule(OL_Context::Rule* rule);
  std::vector<OL_Context::Rule* > _localizedRules;
};

typedef boost::shared_ptr<Localize_Context> Local_ContextPtr;

#endif
