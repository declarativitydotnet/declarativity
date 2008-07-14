/*
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: Deals with the container creator rules: i.e. rules with new on lhs and zero or more new vars.
 * This stage processes only the lhs of such rules
 * Pass 1:
 * 1> creates root container tuple for each constructor 
 * 2> calculates the list of locSpec and version creation rules that need to be created at the end of fix-point * 
 * Pass 2:
 * 3> Creates the rules that trigger the new tuples for each new locspec using the new tuples on lhs (which can potentially have multiple new loc specs)
 * Pass 3:
 * 4> Creates the locSpec and version creation rules to install the locSpec and version tuples at the end of fix-point 
 * 5> Creates the rules to trigger the processing function which performs the sending/encryption/serialization at the end of the fix-point
 * Input: code with constructor and container creation rules. 
 * Assumptions: that the head functors for the constructor tuples have beed converted to "new" types if they were not "new" to start with
 * Placement: before the eca stage and after the rewrite0 stage
 *
 */

#include<iostream>
#include "rewrite1Context.h"
#include "plumber.h"
#include "systemTable.h"
#include "tableManager.h"
#include "value.h"
#include "val_str.h"
#include "val_null.h"
#include "val_int64.h"
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

    /*
     * Pass 1:
     * 1> creates root container tuple for each constructor 
     * 2> calculates the list of locSpec and version creation rules that need to be created at the end of fix-point
     */
    void
    Context::rule(CommonTable::ManagerPtr catalog, TuplePtr rule)
    {
      // do stuff corresponding to pass1 right here
      CommonTablePtr functorTbl = catalog->table(FUNCTOR);
      CommonTable::Iterator headIter;
      TuplePtr head;
      for (headIter = functorTbl->lookup(CommonTable::theKey(CommonTable::KEY5), 
                                         CommonTable::theKey(CommonTable::KEY2), rule);
           !headIter->done(); ) {
	assert(!head);
        head = headIter->next()->clone();
	// do anything at all if this rule has a new head functor
	if(Val_Int64::cast((*head)[catalog->attribute(FUNCTOR, "NEW")]) == 1){
	  string basename = Val_Str::cast((*head)[catalog->attribute(FUNCTOR, "NAME")]);
	  ListPtr attributes = Val_List::cast((*head)[catalog->attribute(FUNCTOR, "ATTRIBUTES")]);
	  uint32_t size = attributes->size();
	  bool newRule = Val_Int64::cast((*rule)[catalog->attribute(RULE, "NEW")]) == 1;
	  NewHeadState *state = new NewHeadState(basename, size, newRule);
	  
	  uint32_t count = 1; //hack to ensure that the pos field correctly reflects the position of the new loc spec
	  // i.e. it excludes the fields because of the external "new" wrapper: location and opaque
	  uint32_t ruleSize = Val_Int64::cast((*rule)[catalog->attribute(RULE, "TERM_COUNT")]);
	  ListPtr copyAttributes = List::mk();
	  ValPtrList::const_iterator iter = attributes->begin();
	  copyAttributes->append(*iter);
	  iter++; // for location
	  copyAttributes->append(*iter);
	  iter++; // for opaque
	  copyAttributes->append(*iter);
	  iter++; // for hint
	  for (; iter != attributes->end(); iter++, count++) {
	    TuplePtr exprTuple = Val_Tuple::cast(*iter);
	    if(Val_Str::cast((*exprTuple)[TNAME]) == NEWLOCSPEC){
	      state->addPos(count);
	      TuplePtr exprTupleClone1 = exprTuple->clone(VAR);
	      TuplePtr exprTupleClone2 = exprTuple->clone(VAR);
	      exprTupleClone1->freeze();
	      exprTupleClone2->freeze();
	      copyAttributes->append(Val_Tuple::mk(exprTupleClone1));
	      TuplePtr locSpecAssign = createLocSpec((*rule)[catalog->attribute(RULE, "RID")], exprTupleClone2, ruleSize);
	      ruleSize++;
	      locSpecAssign->freeze();
	      CommonTablePtr assignTbl = catalog->table(ASSIGN);
	      assignTbl->insert(locSpecAssign);
	    }
	    else{
	      copyAttributes->append(*iter);
	    }
	  }

	  state->freeze();
	  head->set(catalog->attribute(FUNCTOR, "NAME"), Val_Str::mk(state->newRuleHead));
	  head->set(catalog->attribute(FUNCTOR, "TID"), Val_Null::mk());
	  headState.insert(state);
	  if(state->posSet.size() > 0){
	    TuplePtr ruleCopy = rule->clone();
	    ruleCopy->set(catalog->attribute(RULE, "TERM_COUNT"), Val_Int64::mk(ruleSize));
	    ruleCopy->freeze();
	    CommonTablePtr ruleTbl = catalog->table(RULE);
	    ruleTbl->insert(ruleCopy);
	    rule = ruleCopy;
	    head->set(catalog->attribute(FUNCTOR, "ATTRIBUTES"), Val_List::mk(copyAttributes));	    
	    head->freeze();
	    functorTbl->insert(head);
	  }
	}
      }
    }

    /*
     * Pass 2:
     * 3> Creates the rules that trigger the new tuples for each new locspec using the new 
     * tuples on lhs (which can potentially have multiple new loc specs)
     * 3.1> Also generates the newT :- newTk rule which reduces the number of rules we need to add 
     */
    void
    Context::pass2(CommonTable::ManagerPtr catalog, TuplePtr program)
    {
      uint32_t fictVar = 0;
      CommonTablePtr ruleTbl = catalog->table(RULE);
      CommonTablePtr functorTbl = catalog->table(FUNCTOR);
      ValuePtr programID = (*program)[catalog->attribute(PROGRAM, "PID")];
      HeadStateSet::iterator iter = headState.begin();
      for(; iter != headState.end(); iter++){
	NewHeadState* state = *iter;
	if(state->posSet.size() > 1){
	  //generate the 
	  std::set<uint32_t>::iterator posIter = state->posSet.begin();
	  int count = 0;
	  int maxCount = state->posSet.size() + 1;
	  for(;count < maxCount ; posIter++, count++){
	    string lhsname;
	    if(count < maxCount - 1){
	      ostringstream oss1;
	      oss1 << state->newRuleBase <<"_"<<(*posIter);
	      lhsname = oss1.str();
	    }
	    else{
	      lhsname = state->newRuleBase;
	    }
	    ostringstream oss2;
	    oss2 << STAGERULEPREFIX <<ruleCounter++;
	    string rulename = oss2.str();

	    TuplePtr ruleTp = Tuple::mk(RULE, true);
	    ruleTp->append(programID);
	    ruleTp->append(Val_Str::mk(rulename));

	    ruleTp->append(Val_Null::mk());             // Add the "head" functor identifer.
	    ruleTp->append(Val_Null::mk());                  // The P2DL desc. of this rule
	    ruleTp->append(Val_Int64::mk(false));         // Delete rule?
	    ruleTp->append(Val_Int64::mk(2)); // Term count?
	    ruleTp->append(Val_Int64::mk(0));

	    TuplePtr head = Context::generateFunctor(catalog, fictVar, lhsname, (*ruleTp)[TUPLE_ID], state->numVars);
	    head->set(catalog->attribute(FUNCTOR, "POSITION"), Val_Int64::mk(0));
	    head->freeze();
	    functorTbl->insert(head);
	    ruleTp->set(catalog->attribute(RULE, "HEAD_FID"), (*head)[TUPLE_ID]);
	    ruleTp->freeze();
	    ruleTbl->insert(ruleTp);	               // Add rule to rule table.
	    
	    TuplePtr rhs = head->clone(FUNCTOR, true);
	    rhs->set(catalog->attribute(FUNCTOR, "POSITION"), Val_Int64::mk(1));	    
	    rhs->set(catalog->attribute(FUNCTOR, "NAME"), Val_Str::mk(state->newRuleHead));
	    rhs->set(catalog->attribute(FUNCTOR, "TID"), Val_Null::mk());
	    rhs->freeze();
	    functorTbl->insert(rhs);
	  }
	}
	else if(state->posSet.size() == 1){
	    string lhsname = state->newRuleBase;
	    ostringstream oss2;
	    oss2 << STAGERULEPREFIX <<ruleCounter++;
	    string rulename = oss2.str();

	    TuplePtr ruleTp = Tuple::mk(RULE, true);
	    ruleTp->append(programID);
	    ruleTp->append(Val_Str::mk(rulename));

	    ruleTp->append(Val_Null::mk());             // Add the "head" functor identifer.
	    ruleTp->append(Val_Null::mk());                  // The P2DL desc. of this rule
	    ruleTp->append(Val_Int64::mk(false));         // Delete rule?
	    ruleTp->append(Val_Int64::mk(2)); // Term count?
	    ruleTp->append(Val_Int64::mk(0));

	    TuplePtr head = Context::generateFunctor(catalog, fictVar, lhsname, (*ruleTp)[TUPLE_ID], state->numVars);
	    head->set(catalog->attribute(FUNCTOR, "POSITION"), Val_Int64::mk(0));
	    head->freeze();
	    functorTbl->insert(head);
	    ruleTp->set(catalog->attribute(RULE, "HEAD_FID"), (*head)[TUPLE_ID]);
	    ruleTp->freeze();
	    ruleTbl->insert(ruleTp);	               // Add rule to rule table.
	    
	    TuplePtr rhs = head->clone(FUNCTOR, true);
	    rhs->set(catalog->attribute(FUNCTOR, "POSITION"), Val_Int64::mk(1));	    
	    rhs->set(catalog->attribute(FUNCTOR, "NAME"), Val_Str::mk(state->newRuleHead));
	    rhs->set(catalog->attribute(FUNCTOR, "TID"), Val_Null::mk());
	    rhs->freeze();
	    functorTbl->insert(rhs);
	}
      }
    }

    void
    Context::pass3(CommonTable::ManagerPtr catalog, TuplePtr program)
    {
      SetPtr done(new Set());
      CommonTablePtr ruleTbl = catalog->table(RULE);
      CommonTablePtr functorTbl = catalog->table(FUNCTOR);
      ValuePtr programID = (*program)[catalog->attribute(PROGRAM, "PID")];
      HeadStateSet::iterator iter = headState.begin();
      for(; iter != headState.end(); iter++){
	NewHeadState* state = *iter;
	uint32_t fictVar = 0;
	if(done->member(Val_Str::mk(state->newRuleBase)) == 0){
	  done->insert(Val_Str::mk(state->newRuleBase));
	  if(state->newRule){
	    // first do everything assuming that its a non-says link
	    //do it the hacky way
	    bool says = true;
	    // first create the newTVersion :- newT RULE
	    TuplePtr newTupleVersion = materializeNewTupleVersionEvent(fictVar, catalog, programID, state);
	    // next create the locSpec tuple using the newTVersion
	    materializeLocSpecTuple(fictVar, catalog, programID, newTupleVersion->clone(FUNCTOR, true), state);
	    // and create the TVersion tuple using the newTVersion
	    materializeNewTupleVersion(fictVar, catalog, programID, newTupleVersion->clone(FUNCTOR, true), state, says);
	    size_t lastnewpos = state->newRuleBase.rfind(compile::NEWSUFFIX, state->newRuleBase.size());
	    if(lastnewpos == string::npos){
	      throw compile::rewrite1::Exception("Invalid new tuple:" + state->newRuleBase);
	    }
	    string scopedName = state->newRuleBase.substr(0, lastnewpos);
	    assert(scopedName + compile::NEWSUFFIX == state->newRuleBase);
	    catalog->createIndex(scopedName, HASH_INDEX, CommonTable::theKey(CommonTable::KEY2));

	    // now do assuming that its a says link
	    says = false;
	    materializeNewTupleVersion(fictVar, catalog, programID, newTupleVersion->clone(FUNCTOR, true), state, says);

	  }
	  else{
	    TuplePtr processTuple = materializeProcessTuple(fictVar, catalog, programID, state);
	    //	  materializeDeleteProcessTuple(fictVar, catalog, programID, processTuple->clone(FUNCTOR, true), state);
	    materializeSendTuple(fictVar, catalog, programID, processTuple->clone(FUNCTOR, true), state);
	    needRecvTuple = true;
	  }
	}
      }
    }

    TuplePtr Context::createLocSpec(ValuePtr ruleId, TuplePtr locSpec, uint32_t pos){
      TuplePtr fntp = Tuple::mk(FUNCTION);
      
      fntp->append(Val_Str::mk(CREATELOCSPECFN));
      fntp->append(Val_Int64::mk(LOCSPECFNARGS));
      fntp->freeze();

      TuplePtr assignTp  = Tuple::mk(ASSIGN, true);
      assignTp->append(ruleId);
      assert(Val_Str::cast((*locSpec)[TNAME]) == VAR);
      assignTp->append(Val_Tuple::mk(locSpec));               // Assignment variable
      assignTp->append(Val_Tuple::mk(fntp));                  // Assignemnt value
      assignTp->append(Val_Int64::mk(pos));         // Position
      return assignTp;
    }
    

    TuplePtr Context::materializeNewTupleVersionEvent(uint32_t &fictVar, 
						      CommonTable::ManagerPtr catalog, 
						      ValuePtr programID, 
						      NewHeadState *state){
      CommonTablePtr ruleTbl = catalog->table(RULE);
      CommonTablePtr functorTbl = catalog->table(FUNCTOR);
      CommonTablePtr assignTbl = catalog->table(ASSIGN);
      CommonTablePtr selectTbl = catalog->table(SELECT);
      uint32_t pos = 0;
      const uint32_t termCount = 5;
      string rhsname = state->newRuleBase;
      string lhsname = state->newRuleBase + ROOTVERSUFFIX;
      ostringstream oss2;
      oss2 << STAGERULEPREFIX <<ruleCounter++;
      string rulename = oss2.str();

      TuplePtr ruleTp = Tuple::mk(RULE, true);
      ruleTp->append(programID);
      ruleTp->append(Val_Str::mk(rulename));
      
      ruleTp->append(Val_Null::mk());             // Add the "head" functor identifer.
      ruleTp->append(Val_Null::mk());                  // The P2DL desc. of this rule
      ruleTp->append(Val_Int64::mk(false));         // Delete rule?
      ruleTp->append(Val_Int64::mk(termCount)); // Term count?
      ruleTp->append(Val_Int64::mk(0)); // new
      
      ValuePtr ruleId = (*ruleTp)[TUPLE_ID];
      TuplePtr head = Context::generateFunctor(catalog, fictVar, lhsname, ruleId, state->numVars + 1);
      head->set(catalog->attribute(FUNCTOR, "POSITION"), Val_Int64::mk(pos++));
      head->freeze();
      functorTbl->insert(head);
      ruleTp->set(catalog->attribute(RULE, "HEAD_FID"), (*head)[TUPLE_ID]);
      ruleTp->freeze();
      ruleTbl->insert(ruleTp);	               // Add rule to rule table.
	    
      TuplePtr rhs = head->clone(FUNCTOR, true);
      rhs->set(catalog->attribute(FUNCTOR, "POSITION"), Val_Int64::mk(pos++));	    
      rhs->set(catalog->attribute(FUNCTOR, "NAME"), Val_Str::mk(state->newRuleBase));
      rhs->set(catalog->attribute(FUNCTOR, "TID"), Val_Null::mk());
      
      ListPtr lhsargs = Val_List::cast((*head)[catalog->attribute(FUNCTOR, "ATTRIBUTES")]);
      ListPtr attributes = List::mk();
      int count = 0;
      int size = lhsargs->size();
      for (ValPtrList::const_iterator iter = lhsargs->begin();
           count < (size - 1); iter++, count++) {
        attributes->append(*iter);
      }

      rhs->set(catalog->attribute(FUNCTOR, "ATTRIBUTES"), Val_List::mk(attributes));
      rhs->freeze();
      functorTbl->insert(rhs);

      // now make the assign: var := f_isLocSpec(RW3)
      TuplePtr assignIsLocSpec = Tuple::mk(ASSIGN, true);
      assignIsLocSpec->append(ruleId);
      ostringstream oss1;
      oss1 << STAGEVARPREFIX <<fictVar++;
      TuplePtr tempIsLocSpecVar = Tuple::mk(VAR);
      tempIsLocSpecVar->append(Val_Str::mk(oss1.str()));
      tempIsLocSpecVar->freeze();
      
      TuplePtr isLocSpecFn = Tuple::mk(FUNCTION);
      isLocSpecFn->append(Val_Str::mk(ISLOCSPECFN)); // fn name
      isLocSpecFn->append(Val_Int64::mk(ISLOCSPECFNARGS)); // num of args
      TuplePtr  locSpecVar = Val_Tuple::cast(attributes->at(compile::LOCSPECPOS));
      isLocSpecFn->append(Val_Tuple::mk(locSpecVar->clone())); // args
      isLocSpecFn->freeze();

      assignIsLocSpec->append(Val_Tuple::mk(tempIsLocSpecVar));
      assignIsLocSpec->append(Val_Tuple::mk(isLocSpecFn));
      assignIsLocSpec->append(Val_Int64::mk(pos++));
      assignIsLocSpec->freeze();
      assignTbl->insert(assignIsLocSpec);

      // now the select statement (IsLocSpec  == 1
      /*      TuplePtr expr1 = Tuple::mk(BOOL);
      expr1->append(Val_Str::mk("=="));
      TuplePtr destLoc = Val_Tuple::cast(attributes->at(compile::LOCSPECPOS));
      expr1->append(Val_Tuple::mk(destLoc->clone(VAR)));
      TuplePtr myLoc = Val_Tuple::cast(attributes->front());
      expr1->append(Val_Tuple::mk(myLoc->clone(VAR)));
      expr1->freeze();
      
      TuplePtr expr2 = Tuple::mk(BOOL);
      expr2->append(Val_Str::mk("or"));
      expr2->append(Val_Tuple::mk(tempIsLocSpecVar->clone()));
      expr2->append(Val_Tuple::mk(expr1));
      expr2->freeze();
      
      TuplePtr expr3 = Tuple::mk(BOOL);
      expr3->append(Val_Str::mk("=="));
      TuplePtr val = Tuple::mk(VAL);
      val->append(Val_Int64::mk(1));
      expr3->append(Val_Tuple::mk(expr2));
      expr3->append(Val_Tuple::mk(val));
      expr3->freeze();*/

      TuplePtr expr3 = Tuple::mk(BOOL);
      expr3->append(Val_Str::mk("=="));
      TuplePtr val = Tuple::mk(VAL);
      val->append(Val_Int64::mk(1));
      val->freeze();
      TuplePtr isLocSpecVar = tempIsLocSpecVar->clone();
      isLocSpecVar->freeze();
      expr3->append(Val_Tuple::mk(isLocSpecVar));
      expr3->append(Val_Tuple::mk(val));
      
      TuplePtr       selectTp  = Tuple::mk(SELECT, true);
      selectTp->append(ruleId);   // Should be rule identifier
      selectTp->append(Val_Tuple::mk(expr3));              // Boolean expression
      selectTp->append(Val_Int64::mk(pos++));        // Position
      selectTp->append(Val_Null::mk());        // Access method
      selectTp->freeze();
      selectTbl->insert(selectTp); 

      // finally the version creating statement

      TuplePtr assignVer = Tuple::mk(ASSIGN, true);
      assignVer->append(ruleId);

      TuplePtr  verVar = Val_Tuple::cast(lhsargs->back());
      assignVer->append(Val_Tuple::mk(verVar->clone())); 

      TuplePtr createVerFn = Tuple::mk(FUNCTION);
      createVerFn->append(Val_Str::mk(CREATEVERFN)); // fn name
      createVerFn->append(Val_Int64::mk(0)); // num of args
      createVerFn->freeze();
      
      assignVer->append(Val_Tuple::mk(createVerFn));
      assignVer->append(Val_Int64::mk(pos++));
      assignVer->freeze();
      assignTbl->insert(assignVer);

      return head;
    }

    TuplePtr Context::materializeLocSpecTuple(uint32_t &fictVar, 
					      CommonTable::ManagerPtr catalog, 
					      ValuePtr programID, 
					      TuplePtr newTuple, 
					      NewHeadState *state){
      CommonTablePtr ruleTbl = catalog->table(RULE);
      CommonTablePtr functorTbl = catalog->table(FUNCTOR);
      uint32_t pos = 0;
      const uint32_t termCount = 2;

      ostringstream oss2;
      oss2 << STAGERULEPREFIX <<ruleCounter++;
      string rulename = oss2.str();

      TuplePtr ruleTp = Tuple::mk(RULE, true);
      ruleTp->append(programID);
      ruleTp->append(Val_Str::mk(rulename));
      
      ruleTp->append(Val_Null::mk());             // Add the "head" functor identifer.
      ruleTp->append(Val_Null::mk());                  // The P2DL desc. of this rule
      ruleTp->append(Val_Int64::mk(false));         // Delete rule?
      ruleTp->append(Val_Int64::mk(termCount)); // Term count?
      ruleTp->append(Val_Int64::mk(0)); // new
      
      ValuePtr ruleId = (*ruleTp)[TUPLE_ID];

      TuplePtr head = Tuple::mk(FUNCTOR, true);
      head->append(ruleId);
      head->append(Val_Int64::mk(0)); // NOTIN?

      head->append(Val_Str::mk(compile::LOCSPECTABLE));   // Functor name
  
      // Fill in table reference if functor is materialized
      CommonTable::Key nameKey;
      nameKey.push_back(catalog->attribute(FUNCTOR, "NAME"));
      CommonTablePtr tableTbl = catalog->table(TABLE);
      CommonTable::Iterator tIter = 
        tableTbl->lookup(nameKey, CommonTable::theKey(CommonTable::KEY3), head);
      if (!tIter->done()) 
        head->append((*tIter->next())[TUPLE_ID]);
      else {
	assert(0);
        head->append(Val_Null::mk());
      }
  
      head->append(Val_Null::mk());          // The ECA flag

      // Now take care of the functor arguments and variable dependencies
      ListPtr attributes = List::mk();
      ListPtr verAttr = Val_List::cast((*newTuple)[catalog->attribute(FUNCTOR, "ATTRIBUTES")]);
      TuplePtr loc = (Val_Tuple::cast(verAttr->front()))->clone();
      loc->freeze();
      attributes->append(Val_Tuple::mk(loc));

      TuplePtr locSpec = (Val_Tuple::cast(verAttr->at(compile::LOCSPECPOS)))->clone();
      locSpec->freeze();
      attributes->append(Val_Tuple::mk(locSpec));

      TuplePtr newloc = (Val_Tuple::cast(verAttr->front()))->clone(VAR);
      newloc->freeze();
      attributes->append(Val_Tuple::mk(newloc));

      TuplePtr ver = Val_Tuple::cast(verAttr->back());
      attributes->append(Val_Tuple::mk(ver->clone()));

      head->append(Val_List::mk(attributes));// the attribute field
      head->append(Val_Int64::mk(pos++));          // The position field
      head->append(Val_Null::mk());          // The access method
      head->append(Val_Int64::mk(0));        // The new field

      head->freeze();
      functorTbl->insert(head);
      ruleTp->set(catalog->attribute(RULE, "HEAD_FID"), (*head)[TUPLE_ID]);
      ruleTp->freeze();
      ruleTbl->insert(ruleTp);	               // Add rule to rule table.
      
      newTuple->set(catalog->attribute(FUNCTOR, "RID"), ruleId);
      newTuple->set(catalog->attribute(FUNCTOR, "POSITION"), Val_Int64::mk(pos++));
      ListPtr tempargs = Val_List::cast((*newTuple)[catalog->attribute(FUNCTOR, "ATTRIBUTES")]);
      ListPtr newattributes = List::mk();
      for (ValPtrList::const_iterator iter = tempargs->begin();
           iter != tempargs->end(); iter++) {
	newattributes->append(*iter);
      }
      
      newTuple->set(catalog->attribute(FUNCTOR, "ATTRIBUTES"), Val_List::mk(newattributes));

      newTuple->freeze();
      functorTbl->insert(newTuple);
      return head;
    }

    TuplePtr Context::materializeNewTupleVersion(uint32_t &fictVar, 
						 CommonTable::ManagerPtr catalog, 
						 ValuePtr programID, 
						 TuplePtr newTuple, 
						 NewHeadState *state, 
						 bool says){
      CommonTablePtr ruleTbl = catalog->table(RULE);
      CommonTablePtr functorTbl = catalog->table(FUNCTOR);
      CommonTablePtr selectTbl = catalog->table(SELECT);
      uint32_t pos = 0;
      const uint32_t termCount = 3; // 1 head, 1 ver-event and 1 for check
      string headSuffix = (says?compile::rewrite1::HEADSAYSSUFFIX:compile::VERSIONSUFFIX);
      
      // find out the name of the version tuple using the new tuple name: replace new by version
      string originalRuleBase = state->newRuleBase;
      size_t lastnewpos = originalRuleBase.rfind(compile::NEWSUFFIX, originalRuleBase.size());
      if(lastnewpos == string::npos){
	throw compile::rewrite1::Exception("Invalid new tuple:" + state->newRuleBase);
      }
      
      string headname = originalRuleBase.replace(lastnewpos, compile::NEWSUFFIX.length(), headSuffix, 0, headSuffix.length());
      if(says){

	if(materializedSaysTable->member(Val_Str::mk(headname)) == 0){
	  // need to materialize the lhs
	  Table2::Key _keys;
	  _keys.push_back(UNIQUEIDPOS); // since the count for keys start from 0
	  Table2::mk(*catalog,headname,CommonTable::NO_EXPIRATION,CommonTable::NO_SIZE,_keys,ListPtr(),programID);
	  materializedSaysTable->insert(Val_Str::mk(headname));
	}
      }

      ostringstream oss2;
      oss2 << STAGERULEPREFIX <<ruleCounter++;
      string rulename = oss2.str();

      TuplePtr ruleTp = Tuple::mk(RULE, true);
      ruleTp->append(programID);
      ruleTp->append(Val_Str::mk(rulename));
      
      ruleTp->append(Val_Null::mk());             // Add the "head" functor identifer.
      ruleTp->append(Val_Null::mk());                  // The P2DL desc. of this rule
      ruleTp->append(Val_Int64::mk(false));         // Delete rule?
      ruleTp->append(Val_Int64::mk(termCount)); // Term count?
      ruleTp->append(Val_Int64::mk(0)); // new
      
      ValuePtr ruleId = (*ruleTp)[TUPLE_ID];

      TuplePtr head = Tuple::mk(FUNCTOR, true);
      head->append(ruleId);
      head->append(Val_Int64::mk(0)); // NOTIN?
      head->append(Val_Str::mk(headname));   // Functor name
  
      // Fill in table reference if functor is materialized
      CommonTable::Key nameKey;
      nameKey.push_back(catalog->attribute(FUNCTOR, "NAME"));
      CommonTablePtr tableTbl = catalog->table(TABLE);
      CommonTable::Iterator tIter = 
        tableTbl->lookup(nameKey, CommonTable::theKey(CommonTable::KEY3), head);
      if (!tIter->done()) 
        head->append((*tIter->next())[TUPLE_ID]);
      else {
	assert(0);
      }
  
      head->append(Val_Null::mk());          // The ECA flag

      // Now take care of the functor arguments and variable dependencies
      ListPtr attributes = List::mk();
      ListPtr verAttr = Val_List::cast((*newTuple)[catalog->attribute(FUNCTOR, "ATTRIBUTES")]);
      TuplePtr loc = (Val_Tuple::cast(verAttr->front()))->clone();
      loc->freeze();
      attributes->append(Val_Tuple::mk(loc));
      TuplePtr ver = (Val_Tuple::cast(verAttr->back()))->clone();
      ver->freeze();
      attributes->append(Val_Tuple::mk(ver));
      ValPtrList::const_iterator iter = verAttr->begin();
      int count = 0;
      iter++; iter++; 
      TuplePtr hintVar = (Val_Tuple::cast(*iter))->clone();
      hintVar->freeze();
      iter = verAttr->begin();
      iter++; count++;// for loc
      if(!says){
	iter++; count++;// for opaque
	iter++; count++;// for hint
	iter++; count++;// for destLocSpec
      }
      // exclude the last field as it has already been included
      int size = verAttr->size();
      for (;count < (size - 1); iter++, count++) {
	TuplePtr exprTuple = Val_Tuple::cast(*iter);
	attributes->append(Val_Tuple::mk(exprTuple->clone()));
      }

      head->append(Val_List::mk(attributes));// the attribute field
      head->append(Val_Int64::mk(pos++));          // The position field
      head->append(Val_Null::mk());          // The access method
      head->append(Val_Int64::mk(0));        // The new field

      head->freeze();
      functorTbl->insert(head);
      ruleTp->set(catalog->attribute(RULE, "HEAD_FID"), (*head)[TUPLE_ID]);
      ruleTp->freeze();
      ruleTbl->insert(ruleTp);	               // Add rule to rule table.
      
      newTuple->set(catalog->attribute(FUNCTOR, "RID"), ruleId);
      newTuple->set(catalog->attribute(FUNCTOR, "POSITION"), Val_Int64::mk(pos++));
      ListPtr tempargs = Val_List::cast((*newTuple)[catalog->attribute(FUNCTOR, "ATTRIBUTES")]);
      ListPtr newattributes = List::mk();
      for (ValPtrList::const_iterator iter = tempargs->begin();
           iter != tempargs->end(); iter++) {
	newattributes->append(*iter);
      }
      
      newTuple->set(catalog->attribute(FUNCTOR, "ATTRIBUTES"), Val_List::mk(newattributes));

      newTuple->freeze();
      functorTbl->insert(newTuple);

      TuplePtr expr3 = Tuple::mk(BOOL);
      expr3->append(Val_Str::mk("=="));
      TuplePtr val = Tuple::mk(VAL);
      val->append(Val_Int64::mk(says?1:0));
      val->freeze();

      TuplePtr f_isSays = Tuple::mk(FUNCTION);
      f_isSays->append(Val_Str::mk(ISSAYSFN));
      f_isSays->append(Val_Int64::mk(ISSAYSFNARGS));
      f_isSays->append(Val_Tuple::mk(hintVar));
      f_isSays->freeze();
      expr3->append(Val_Tuple::mk(val));
      expr3->append(Val_Tuple::mk(f_isSays));
      expr3->freeze();

      TuplePtr       selectTp  = Tuple::mk(SELECT, true);
      selectTp->append(ruleId);   // Should be rule identifier
      selectTp->append(Val_Tuple::mk(expr3));              // Boolean expression
      selectTp->append(Val_Int64::mk(pos++));        // Position
      selectTp->append(Val_Null::mk());        // Access method
      selectTp->freeze();
      selectTbl->insert(selectTp); 

      return head;
    }

    TuplePtr Context::materializeProcessTuple(uint32_t &fictVar, 
					      CommonTable::ManagerPtr catalog, 
					      ValuePtr programID, 
					      NewHeadState *state)
    {
      CommonTablePtr ruleTbl = catalog->table(RULE);
      CommonTablePtr functorTbl = catalog->table(FUNCTOR);
      CommonTablePtr assignTbl = catalog->table(ASSIGN);
      CommonTablePtr selectTbl = catalog->table(SELECT);
      uint32_t pos = 0;
      const uint32_t termCount = 4; // functor, select and assign
      string rhsname = state->newRuleBase;
      string lhsname = state->newRuleBase + ROOTPROCESSSUFFIX;

      if(materializedTable->member(Val_Str::mk(lhsname)) == 0){
	// need to materialize the lhs
	Table2::Key _keys;
	_keys.push_back(UNIQUEIDPOS); // since the count for keys start from 0
	Table2::mk(*catalog,lhsname,CommonTable::NO_EXPIRATION, CommonTable::NO_SIZE, _keys, ListPtr(), programID);
	materializedTable->insert(Val_Str::mk(lhsname));
      }

      ostringstream oss2;
      oss2 << STAGERULEPREFIX <<ruleCounter++;
      string rulename = oss2.str();

      TuplePtr ruleTp = Tuple::mk(RULE, true);
      ruleTp->append(programID);
      ruleTp->append(Val_Str::mk(rulename));
      
      ruleTp->append(Val_Null::mk());             // Add the "head" functor identifer.
      ruleTp->append(Val_Null::mk());                  // The P2DL desc. of this rule
      ruleTp->append(Val_Int64::mk(false));         // Delete rule?
      ruleTp->append(Val_Int64::mk(termCount)); // Term count?
      ruleTp->append(Val_Int64::mk(0)); // new
      
      ValuePtr ruleId = (*ruleTp)[TUPLE_ID];
      TuplePtr head = Context::generateFunctor(catalog, fictVar, lhsname, ruleId, state->numVars + 1); // an additional term for timestamp
      head->set(catalog->attribute(FUNCTOR, "POSITION"), Val_Int64::mk(pos++));
      head->freeze();
      functorTbl->insert(head);
      ruleTp->set(catalog->attribute(RULE, "HEAD_FID"), (*head)[TUPLE_ID]);
      ruleTp->freeze();
      ruleTbl->insert(ruleTp);	               // Add rule to rule table.
	    
      TuplePtr rhs = head->clone(FUNCTOR, true);
      rhs->set(catalog->attribute(FUNCTOR, "POSITION"), Val_Int64::mk(pos++));	    
      rhs->set(catalog->attribute(FUNCTOR, "NAME"), Val_Str::mk(rhsname));
      
      CommonTable::Key nameKey;
      nameKey.push_back(catalog->attribute(FUNCTOR, "NAME"));
      CommonTablePtr tableTbl = catalog->table(TABLE);
      CommonTable::Iterator tIter = 
        tableTbl->lookup(nameKey, CommonTable::theKey(CommonTable::KEY3), rhs);
      if (!tIter->done()) 
	rhs->set(catalog->attribute(FUNCTOR, "TID"), (*tIter->next())[TUPLE_ID]);
      else {
	rhs->set(catalog->attribute(FUNCTOR, "TID"), Val_Null::mk());
      }  

      ListPtr lhsargs = Val_List::cast((*head)[catalog->attribute(FUNCTOR, "ATTRIBUTES")]);
      ListPtr attributes = List::mk();
      TuplePtr uniqueId;
      uint32_t curpos = 1;
      for (ValPtrList::const_iterator iter = lhsargs->begin();
           iter != lhsargs->end(); iter++) {
	if(curpos != UNIQUEIDPOS){
	  attributes->append(*iter);
	}
	else{
	  uniqueId = Val_Tuple::cast(*iter);
	}
	curpos++;
      }
      
      rhs->set(catalog->attribute(FUNCTOR, "ATTRIBUTES"), Val_List::mk(attributes));
      rhs->freeze();
      functorTbl->insert(rhs);

      // now make the select f_isLocSpec(RW3) == 0
      // first make the f_isLocSpec fn
      TuplePtr isLocSpecFn = Tuple::mk(FUNCTION);
      isLocSpecFn->append(Val_Str::mk(ISLOCSPECFN)); // fn name
      isLocSpecFn->append(Val_Int64::mk(ISLOCSPECFNARGS)); // num of args
      TuplePtr  locSpecVar = Val_Tuple::cast(attributes->at(compile::LOCSPECPOS));
      isLocSpecFn->append(Val_Tuple::mk(locSpecVar->clone())); // args
      isLocSpecFn->freeze();

      //next make the 0
      
      TuplePtr zeroVal = Tuple::mk(VAL);
      zeroVal->append(Val_Int64::mk(0));
      zeroVal->freeze();

      // now the select statement (IsLocSpec || Me == DestLocSpec) == 1
      TuplePtr expr1 = Tuple::mk(BOOL);
      expr1->append(Val_Str::mk("=="));
      expr1->append(Val_Tuple::mk(isLocSpecFn));
      expr1->append(Val_Tuple::mk(zeroVal));
      expr1->freeze();

      TuplePtr       selectTp  = Tuple::mk(SELECT, true);
      selectTp->append(ruleId);   // Should be rule identifier
      selectTp->append(Val_Tuple::mk(expr1));              // Boolean expression
      selectTp->append(Val_Int64::mk(pos++));        // Position
      selectTp->append(Val_Null::mk());        // Access method
      selectTp->freeze();
      selectTbl->insert(selectTp); 
      

      // now make the assign: uniqueId := f_timeStamp()
      TuplePtr assignTimeStamp = Tuple::mk(ASSIGN, true);
      assignTimeStamp->append(ruleId);
      
      TuplePtr timeStampFn = Tuple::mk(FUNCTION);
      timeStampFn->append(Val_Str::mk(TIMESTAMPFN)); // fn name
      timeStampFn->append(Val_Int64::mk(0)); // num of args
      timeStampFn->freeze();

      assignTimeStamp->append(Val_Tuple::mk(uniqueId->clone()));
      assignTimeStamp->append(Val_Tuple::mk(timeStampFn));
      assignTimeStamp->append(Val_Int64::mk(pos++));
      assignTimeStamp->freeze();
      assignTbl->insert(assignTimeStamp);

      return head;

    }

    TuplePtr Context::materializeDeleteProcessTuple(uint32_t &fictVar, 
						    CommonTable::ManagerPtr catalog, 
						    ValuePtr programID, 
						    TuplePtr newTuple, 
						    NewHeadState *state){
      CommonTablePtr ruleTbl = catalog->table(RULE);
      CommonTablePtr functorTbl = catalog->table(FUNCTOR);
      uint32_t pos = 0;
      const uint32_t termCount = 2; // functor, select and assign
      
      ostringstream oss2;
      oss2 << STAGERULEPREFIX <<ruleCounter++;
      string rulename = oss2.str();

      TuplePtr ruleTp = Tuple::mk(RULE, true);
      ValuePtr ruleId = (*ruleTp)[TUPLE_ID];
      newTuple->set(catalog->attribute(FUNCTOR, "RID"), ruleId);
      ListPtr tempargs =  Val_List::cast((*newTuple)[catalog->attribute(FUNCTOR, "ATTRIBUTES")]);

      TuplePtr head = newTuple->clone(FUNCTOR, true);
      head->set(catalog->attribute(FUNCTOR, "POSITION"), Val_Int64::mk(pos++));      
      ListPtr headattributes = List::mk();
      for (ValPtrList::const_iterator iter = tempargs->begin();
           iter != tempargs->end(); iter++) {
	headattributes->append(*iter);
      }
      
      head->set(catalog->attribute(FUNCTOR, "ATTRIBUTES"), Val_List::mk(headattributes));
      head->freeze();
      TuplePtr rhs = newTuple;
      rhs->set(catalog->attribute(FUNCTOR, "POSITION"), Val_Int64::mk(pos++));      
      ListPtr rhsattributes = List::mk();
      for (ValPtrList::const_iterator iter = tempargs->begin();
           iter != tempargs->end(); iter++) {
	rhsattributes->append(*iter);
      }
      
      rhs->set(catalog->attribute(FUNCTOR, "ATTRIBUTES"), Val_List::mk(rhsattributes));
      rhs->freeze();
      
      ruleTp->append(programID);
      ruleTp->append(Val_Str::mk(rulename));
      ruleTp->append( (*head)[TUPLE_ID]);             // Add the "head" functor identifer.
      ruleTp->append(Val_Null::mk());                  // The P2DL desc. of this rule
      ruleTp->append(Val_Int64::mk(true));         // Delete rule?
      ruleTp->append(Val_Int64::mk(termCount)); // Term count?
      ruleTp->append(Val_Int64::mk(0)); // new
      ruleTp->freeze();

      ruleTbl->insert(ruleTp);
      functorTbl->insert(head);
      functorTbl->insert(rhs);
      return head;
      
    }

    TuplePtr Context::materializeSendTuple(uint32_t &fictVar, 
					   CommonTable::ManagerPtr catalog, 
					   ValuePtr programID, 
					   TuplePtr newTuple, 
					   NewHeadState *state){

      CommonTablePtr ruleTbl = catalog->table(RULE);
      CommonTablePtr functorTbl = catalog->table(FUNCTOR);
      CommonTablePtr assignTbl = catalog->table(ASSIGN);
      uint32_t pos = 0;
      const uint32_t termCount = 4; // functor and two assigns

      string lhsname = SENDTUPLE;
      ListPtr tempargs = Val_List::cast((*newTuple)[catalog->attribute(FUNCTOR, "ATTRIBUTES")]);

      ostringstream oss2;
      oss2 << STAGERULEPREFIX <<ruleCounter++;
      string rulename = oss2.str();

      TuplePtr ruleTp = Tuple::mk(RULE, true);
      ruleTp->append(programID);
      ruleTp->append(Val_Str::mk(rulename));
      
      ruleTp->append(Val_Null::mk());             // Add the "head" functor identifer.
      ruleTp->append(Val_Null::mk());                  // The P2DL desc. of this rule
      ruleTp->append(Val_Int64::mk(false));         // Delete rule?
      ruleTp->append(Val_Int64::mk(termCount)); // Term count?
      ruleTp->append(Val_Int64::mk(0)); // new
      
      ValuePtr ruleId = (*ruleTp)[TUPLE_ID];
      TuplePtr head = Context::generateFunctor(catalog, fictVar, lhsname, ruleId, 2);
      head->set(catalog->attribute(FUNCTOR, "POSITION"), Val_Int64::mk(pos++));
      ListPtr headargs = Val_List::cast((*head)[catalog->attribute(FUNCTOR, "ATTRIBUTES")]);
      TuplePtr curHeadDest = (Val_Tuple::cast(headargs->front()))->clone();
      headargs->pop_front();
      curHeadDest->set(TNAME, Val_Str::mk(VAR));
      curHeadDest->freeze();
      headargs->prepend(Val_Tuple::mk(curHeadDest));
      TuplePtr finalDest = (Val_Tuple::cast(tempargs->at(compile::LOCSPECPOS+1)))->clone(LOC); // +1 because of the unique id field
      finalDest->freeze();
      headargs->prepend(Val_Tuple::mk(finalDest));
      head->freeze();
      functorTbl->insert(head);
      ruleTp->set(catalog->attribute(RULE, "HEAD_FID"), (*head)[TUPLE_ID]);
      ruleTp->freeze();
      ruleTbl->insert(ruleTp);	               // Add rule to rule table.
	    
      TuplePtr rhs = newTuple;
      rhs->set(catalog->attribute(FUNCTOR, "POSITION"), Val_Int64::mk(pos++));
      rhs->set(catalog->attribute(FUNCTOR, "RID"), ruleId);

      //copy attributes
      ListPtr attributes = List::mk();
      for (ValPtrList::const_iterator iter = tempargs->begin();
           iter != tempargs->end(); iter++) {
	attributes->append(*iter);
      }
      rhs->set(catalog->attribute(FUNCTOR, "ATTRIBUTES"), Val_List::mk(attributes));
      rhs->freeze();
      functorTbl->insert(rhs);

      // now make the assign: buf := f_serialize("tablename", uniqueid)
      TuplePtr assignTblName = Tuple::mk(ASSIGN, true);
      assignTblName->append(ruleId);


      TuplePtr var = curHeadDest->clone();
      var->freeze();
      
      string originalBaseName = state->newRuleBase;
      size_t lastnewpos = originalBaseName.rfind(compile::NEWSUFFIX, originalBaseName.size());
      if(lastnewpos == string::npos){
	throw compile::rewrite1::Exception("Invalid new tuple:" + state->newRuleBase);
      }
      string headname = originalBaseName.replace(lastnewpos, compile::NEWSUFFIX.length(), compile::VERSIONSUFFIX, 0, compile::VERSIONSUFFIX.length());

      assignTblName->append(Val_Tuple::mk(var));
      TuplePtr val = Tuple::mk(VAL);
      val->append(Val_Str::mk(headname));
      val->freeze();
      assignTblName->append(Val_Tuple::mk(val));
      assignTblName->append(Val_Int64::mk(pos++));
      assignTblName->freeze();
      assignTbl->insert(assignTblName);

      TuplePtr assignSerialized = Tuple::mk(ASSIGN, true);
      assignSerialized->append(ruleId);
      
      TuplePtr serializeFn = Tuple::mk(FUNCTION);
      serializeFn->append(Val_Str::mk(SERIALIZEFN)); // fn name
      serializeFn->append(Val_Int64::mk(SERIALIZEFNARGS)); // num of args

      serializeFn->append(Val_Tuple::mk(var->clone())); // num of args
      TuplePtr uniqueId = Val_Tuple::cast(tempargs->at(UNIQUEIDPOS));
      serializeFn->append(Val_Tuple::mk(uniqueId->clone())); // num of args
      serializeFn->freeze();

      TuplePtr bufVar = (Val_Tuple::cast(headargs->back()))->clone();
      bufVar->freeze();
      assignSerialized->append(Val_Tuple::mk(bufVar));
      assignSerialized->append(Val_Tuple::mk(serializeFn));
      assignSerialized->append(Val_Int64::mk(pos++));
      assignSerialized->freeze();
      assignTbl->insert(assignSerialized);
      return head;
    }

    TuplePtr Context::materializeRecvTuple(CommonTable::ManagerPtr catalog, ValuePtr programID){

      CommonTablePtr ruleTbl = catalog->table(RULE);
      CommonTablePtr functorTbl = catalog->table(FUNCTOR);
      CommonTablePtr assignTbl = catalog->table(ASSIGN);
      uint32_t pos = 0;
      const uint32_t termCount = 3; // functor and assign
      uint32_t fictVar = 0;

      string lhsname = DUMMY;
      string rhsname = SENDTUPLE;

      ostringstream oss2;
      oss2 << STAGERULEPREFIX <<ruleCounter++;
      string rulename = oss2.str();

      TuplePtr ruleTp = Tuple::mk(RULE, true);
      ruleTp->append(programID);
      ruleTp->append(Val_Str::mk(rulename));
      
      ruleTp->append(Val_Null::mk());             // Add the "head" functor identifer.
      ruleTp->append(Val_Null::mk());                  // The P2DL desc. of this rule
      ruleTp->append(Val_Int64::mk(false));         // Delete rule?
      ruleTp->append(Val_Int64::mk(termCount)); // Term count?
      ruleTp->append(Val_Int64::mk(0)); // new
      
      ValuePtr ruleId = (*ruleTp)[TUPLE_ID];
      TuplePtr head = Context::generateFunctor(catalog, fictVar, lhsname, ruleId, DUMMYARGS-1);
      head->set(catalog->attribute(FUNCTOR, "POSITION"), Val_Int64::mk(pos++));
      ListPtr headargs = Val_List::cast((*head)[catalog->attribute(FUNCTOR, "ATTRIBUTES")]);

      ruleTp->set(catalog->attribute(RULE, "HEAD_FID"), (*head)[TUPLE_ID]);
      ruleTp->freeze();
      ruleTbl->insert(ruleTp);	               // Add rule to rule table.
	    
      TuplePtr rhs = Context::generateFunctor(catalog, fictVar, rhsname, ruleId, DESERIALIZEFNARGS + 1); //+1 for location
      rhs->set(catalog->attribute(FUNCTOR, "POSITION"), Val_Int64::mk(pos++));

      rhs->freeze();
      functorTbl->insert(rhs);

      ListPtr attributes = Val_List::cast((*rhs)[catalog->attribute(FUNCTOR, "ATTRIBUTES")]);
      TuplePtr rhsLoc = Val_Tuple::cast(attributes->front());
      headargs->prepend(Val_Tuple::mk(rhsLoc->clone()));
      head->freeze();
      functorTbl->insert(head);

      // now make the assign: uniqueId := f_timeStamp()
      TuplePtr assignDeserialized = Tuple::mk(ASSIGN, true);
      assignDeserialized->append(ruleId);
      
      TuplePtr deserializeFn = Tuple::mk(FUNCTION);
      deserializeFn->append(Val_Str::mk(DESERIALIZEFN)); // fn name
      deserializeFn->append(Val_Int64::mk(DESERIALIZEFNARGS)); // num of args
      
      ValPtrList::const_iterator iter = attributes->begin();
      for (iter++; iter != attributes->end(); iter++) {
	deserializeFn->append(*iter); 
      }
      deserializeFn->freeze();

      // make a dummy variable
      ostringstream oss;
      oss << STAGEVARPREFIX << fictVar++;
      TuplePtr var = Tuple::mk(VAR);
      var->append(Val_Str::mk(oss.str()));

      assignDeserialized->append(Val_Tuple::mk(var));
      assignDeserialized->append(Val_Tuple::mk(deserializeFn));
      assignDeserialized->append(Val_Int64::mk(pos++));
      assignDeserialized->freeze();
      assignTbl->insert(assignDeserialized);

      return head;
    }

    TuplePtr 
    Context::program(CommonTable::ManagerPtr catalog, TuplePtr program) 
    {
      SetPtr tmp(new Set());
      materializedTable = tmp;

      SetPtr tmp1(new Set());
      materializedSaysTable = tmp1;

      //add materialization for LOCSPECTABLE
      string scopedName = compile::LOCSPECTABLE;
      Table2::Key _keys;
      _keys.push_back(1); // location
      _keys.push_back(2); // loc spec
      _keys.push_back(3); // ver location
      _keys.push_back(4); // ver

      try{
	Table2::mk(*catalog,scopedName,CommonTable::NO_EXPIRATION, CommonTable::NO_SIZE, _keys, ListPtr(), (*program)[catalog->attribute(PROGRAM, "PID")]);
	catalog->createIndex(scopedName, HASH_INDEX, CommonTable::theKey(CommonTable::KEY2));
	TELL_ERROR<<" Successfully created table "<< scopedName
	            << "\n";
	}
      catch(TableManager::Exception e){
	TELL_ERROR << "generateMaterialize caught a table creation exception: '"
		   << e.toString()
		   << "\n";
	
      }
      
      scopedName = compile::LINKEXPANDERTABLE;
      
      _keys.clear();
      _keys.push_back(2); // loc spec
      try{
	Table2::mk(*catalog,scopedName,CommonTable::NO_EXPIRATION, CommonTable::NO_SIZE, _keys, ListPtr(), (*program)[catalog->attribute(PROGRAM, "PID")]);
	TELL_ERROR<<" Successfully created table"<< scopedName
		  << "\n";
      }
      catch(TableManager::Exception e){
	TELL_ERROR << "generateMaterialize caught a table creation exception: '"
		   << e.toString()
		   << "\n";
	
      }
      
      TuplePtr newProgram = this->compile::Context::program(catalog, program);
      
      pass2(catalog, program);
      pass3(catalog, program);
      ValuePtr programID = (*program)[catalog->attribute(PROGRAM, "PID")];
      if(needRecvTuple){
	materializeRecvTuple(catalog, programID);
      }

      return newProgram;

    }

    TuplePtr
    Context::generateFunctor(CommonTable::ManagerPtr catalog, uint32_t &fictVar, string name, ValuePtr ruleId, uint32_t numVars){

      TuplePtr     functorTp = Tuple::mk(FUNCTOR, true);
      if (ruleId) {
        functorTp->append(ruleId);
      }
      else {
        functorTp->append(Val_Null::mk());
      }
      functorTp->append(Val_Int64::mk(0)); // NOTIN?
      functorTp->append(Val_Str::mk(name));   // Functor name

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
      //      functorTp->append(Val_Null::mk());
  
  
      functorTp->append(Val_Null::mk());          // The ECA flag

      // Now take care of the functor arguments and variable dependencies
      ListPtr attributes = List::mk();
      if(numVars > 0){
	ostringstream oss;
	oss << STAGEVARPREFIX << fictVar++;
	TuplePtr var = Tuple::mk(LOC);
	var->append(Val_Str::mk(oss.str()));
	attributes->append(Val_Tuple::mk(var));
      }

      for (uint32_t i = 1; i < numVars; i++){
	TuplePtr var = Tuple::mk(VAR);
	ostringstream oss;
	oss << STAGEVARPREFIX << fictVar++;
	var->append(Val_Str::mk(oss.str()));
	var->freeze();
	attributes->append(Val_Tuple::mk(var));
      }

      functorTp->append(Val_List::mk(attributes));// the attribute field
      functorTp->append(Val_Null::mk());          // The position field
      functorTp->append(Val_Null::mk());          // The access method
      functorTp->append(Val_Int64::mk(0));        // The new field
      return functorTp;
      
    }

  }
}
