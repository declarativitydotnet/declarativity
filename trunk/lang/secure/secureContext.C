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

#include<iostream>
#include<utility>
#include "secureContext.h"
#include "plumber.h"
#include "systemTable.h"
#include "tableManager.h"
#include "value.h"
#include "val_str.h"
#include "val_null.h"
#include "val_int64.h"
#include "val_list.h"
#include "val_tuple.h"
namespace compile {
  namespace secure{
    using namespace opr;
    DEFINE_ELEMENT_INITS_NS(Context, "SecureContext", compile::secure)
      uint32_t Context::ruleCounter = 0;

      Context::Context(string name)
	: compile::Context(name) { }

    Context::Context(TuplePtr args)
      : compile::Context((*args)[2]->toString()) { }

    void 
    Context::initLocSpecMap(CommonTable::ManagerPtr catalog, TuplePtr rule){
      ValuePtr ruleId = (*rule)[TUPLE_ID];
      SetPtr locSpecSet(new Set());
      SetPtr strongLocSpecSet(new Set());
      SetPtr locationSet(new Set());
      CommonTablePtr functorTbl = catalog->table(FUNCTOR);
      CommonTablePtr newTbl = catalog->table(NEW);
      CommonTable::Iterator funcIter;
      ValuePtr eventLocSpec;

      uint32_t refPosPos = catalog->attribute(REF, "LOCSPECFIELD");
      uint32_t refTypePos = catalog->attribute(REF, "REFTYPE");
      
      for (funcIter = functorTbl->lookup(CommonTable::theKey(CommonTable::KEY2), 
                                         CommonTable::theKey(CommonTable::KEY3), rule);
           !funcIter->done() && !eventLocSpec; ) {
        TuplePtr functor = funcIter->next();
        if ((*functor)[TUPLE_ID] != (*rule)[catalog->attribute(RULE, "HEAD_FID")]) {
	  ListPtr attributes = Val_List::cast((*functor)[catalog->attribute(FUNCTOR, "ATTRIBUTES")]);
          if ((*functor)[catalog->attribute(FUNCTOR, "TID")] != Val_Null::mk()) {
	    CommonTablePtr refTbl = catalog->table(REF);
	    CommonTable::Iterator refIter;
	      
	    for (refIter = refTbl->lookup(CommonTable::theKey(CommonTable::KEY5), CommonTable::theKey(CommonTable::KEY4), functor);
		 !refIter->done(); ) {
	      TuplePtr ref = refIter->next();
	      uint32_t refPosVal = Val_Int64::cast((*ref)[refPosPos]);
	      TuplePtr refLocSpecTuple = Val_Tuple::cast(attributes->at(refPosVal));
	      ValuePtr locSpecVarName = (*refLocSpecTuple)[CONTENTPOS];
	      locSpecSet->insert(locSpecVarName);
	      uint32_t refType = Val_Int64::cast((*ref)[refTypePos]);
	      if(refType == STRONGLINK || refType == STRONGSAYS){
		strongLocSpecSet->insert(locSpecVarName);
	      }
	    }
          } else{
	    eventLocSpec = attributes->front();
	  }
	  TuplePtr locTuple = Val_Tuple::cast(attributes->front());
	  //check if this is a new functor
	  CommonTable::Iterator newIter = newTbl->lookup(CommonTable::theKey(CommonTable::KEY2), CommonTable::theKey(CommonTable::KEY3), functor);
	  if(!newIter->done()){
	    TuplePtr newTerm = newIter->next();
	    locTuple = Val_Tuple::cast((*newTerm)[catalog->attribute(NEW, "EVENTLOC")]);
	  }

	  if(locSpecSet->member((*locTuple)[CONTENTPOS]) == 0){
	    locationSet->insert((*locTuple)[CONTENTPOS]);
	  }
	}
      }

      if(!eventLocSpec){
	SetPtr possibleViewLocation = locationSet->difference(locSpecSet);
	if(possibleViewLocation->size() == 0){
	  std::cout<<"locationSet"<<locationSet->toString()<<std::endl<<"locSpecSet"<<locSpecSet->toString()<<std::endl<<"differenceSet"<<possibleViewLocation->toString();
	  throw compile::secure::Exception("No event in rule" + rule->toString());
	}
	else{
	  // pick up the first non locSpec member as the eventLocSpec
	  TuplePtr eventLocSpecTuple = Tuple::mk(LOC);
	  eventLocSpecTuple->append(*possibleViewLocation->begin());
	  eventLocSpec = Val_Tuple::mk(eventLocSpecTuple);
	}
      }

      compile::Context::ruleLocSpecMap->insert(std::pair<ValuePtr, LocSpecInfo*>(ruleId, new LocSpecInfo(eventLocSpec, locSpecSet, strongLocSpecSet))); // insert values into the event loc spec
    }

    void
    Context::rule(CommonTable::ManagerPtr catalog, TuplePtr rule)
    {
      initLocSpecMap(catalog, rule);
      uint32_t newVariable = 1;
      CommonTablePtr functorTbl = catalog->table(FUNCTOR);
      CommonTablePtr saysTbl = catalog->table(SAYS);
      CommonTablePtr newTbl = catalog->table(NEW);
      CommonTablePtr tableTbl = catalog->table(TABLE);
      CommonTablePtr refTbl = catalog->table(REF);
      CommonTablePtr assignTbl = catalog->table(ASSIGN);
      CommonTablePtr ruleTbl = catalog->table(RULE);
      CommonTablePtr selectTbl = catalog->table(SELECT);

      ValuePtr ruleId = (*rule)[TUPLE_ID];
      TuplePtr eventLocSpec = Val_Tuple::cast(((compile::Context::ruleLocSpecMap->find(ruleId))->second)->location);
      assert(eventLocSpec);
      //      uint32_t pos = Val_Int64::cast((*rule)[catalog->attribute(RULE, "TERM_COUNT")]);
      TuplePtr head;
      for (CommonTable::Iterator headIter = functorTbl->lookup(CommonTable::theKey(CommonTable::KEY5), 
							       CommonTable::theKey(CommonTable::KEY2), rule);
           !headIter->done(); ) {
	assert(!head);
        head = headIter->next();
	CommonTable::Iterator saysIter = saysTbl->lookup(CommonTable::theKey(CommonTable::KEY2), 
							 CommonTable::theKey(CommonTable::KEY3), head);
	// check if the head is materialized or not?
	  CommonTable::Key nameKey;
	  nameKey.push_back(catalog->attribute(FUNCTOR, "NAME"));
	  CommonTable::Iterator tIter = tableTbl->lookup(nameKey, CommonTable::theKey(CommonTable::KEY3), head);
	  CommonTable::Iterator rIter = refTbl->lookup(nameKey, CommonTable::theKey(CommonTable::KEY4), head);
	  TuplePtr headSays;
	  TuplePtr headSaysType;
	  TuplePtr keyTypeVar;

	  ListPtr headSaysAttr;
	  ListPtr headAttr;
	  TuplePtr headProof;
	  string headName = "";
	  bool compoundHead = !rIter->done();
	  assert(!compoundHead || !tIter->done());
	  std::list<TupleList*> newRuleComparators;
	  headAttr = Val_List::cast((*head)[catalog->attribute(FUNCTOR, "ATTRIBUTES")]);
	  headName = Val_Str::cast((*head)[catalog->attribute(FUNCTOR, "NAME")]);

	  TupleList *listAssign = NULL;

	  if(!saysIter->done()){
	   headSays = saysIter->next();
	   headSaysAttr = Val_List::cast((*headSays)[catalog->attribute(SAYS, "ATTRIBUTES")]);
	   uint32_t headpos = Val_Int64::cast((*rule)[catalog->attribute(RULE, "TERM_COUNT")]);
	   ostringstream var6;
	   var6 << STAGEVARPREFIX << newVariable++;
	   headProof = Tuple::mk(VAR);
	   headProof->append(Val_Str::mk(var6.str()));
	   headProof->freeze(); // this is the variable in which the proof/key of the head says is inserted. This is passed to the normalizeGenerate so that we know 
	   // which variable should contain the proof, should there be a functor on rhs that matches the head

	   ostringstream var7;
	   var7 << STAGEVARPREFIX << newVariable++;
	   headSaysType = Tuple::mk(VAR);
	   headSaysType->append(Val_Str::mk(var7.str()));
	   headSaysType->freeze(); // this is the variable which is set to the appropriate operation type (CREATESAYS/COPYSAYS). Passed to the normalizeGenerate so that 
	   // we know for compound tuples, which variable should be set to the CREATESAYS/COPYSAYS

	   ostringstream v1;
	   v1 << STAGEVARPREFIX << newVariable++;
	   keyTypeVar = Tuple::mk(VAR);
	   keyTypeVar->append(Val_Str::mk(v1.str()));
	   keyTypeVar->freeze();

	   listAssign = new TupleList();
	   // create a new field if its not already present
	   TupleList* newTermsGen = normalizeGenerate(catalog, head, rule, eventLocSpec, newVariable, headpos, 
						      !tIter->done(), headProof, headSaysType, keyTypeVar, listAssign); 
	   // also if compoundHead is false, remove the new field

	   if(!compoundHead){
	     //	     headProof = Val_Tuple::cast(headAttr->back());
	   }
	   else{
	    TuplePtr assignTp  = Tuple::mk(ASSIGN, true);
	    assignTp->append((*rule)[TUPLE_ID]);
	    TuplePtr varCopy = headSaysType->clone();
	    varCopy->freeze();
	    assignTp->append(Val_Tuple::mk(varCopy));               // Assignment variable
	    TuplePtr val = Tuple::mk(VAL);
	    val->append(Val_Int64::mk(CREATESAYS));                  // copy
	    val->freeze();
	    assignTp->append(Val_Tuple::mk(val));                  // create
	    assignTp->append(Val_Int64::mk(headpos++));         // Position
	    newTermsGen->push_back(assignTp);
	   }

   	   newRuleComparators.push_back(newTermsGen);
	  }

	  CommonTable::Iterator funcIter;
	  ListPtr functorSaysAttr;
	  ListPtr functorAttr;

	  for (funcIter = functorTbl->lookup(CommonTable::theKey(CommonTable::KEY2), 
					     CommonTable::theKey(CommonTable::KEY3), rule);
	       !funcIter->done(); ) {
	    TuplePtr functor = funcIter->next();
	    if ((*functor)[TUPLE_ID] != (*rule)[catalog->attribute(RULE, "HEAD_FID")]) {
	      functorAttr =  Val_List::cast((*functor)[catalog->attribute(FUNCTOR, "ATTRIBUTES")]);
	      uint32_t funcpos = Val_Int64::cast((*rule)[catalog->attribute(RULE, "TERM_COUNT")]);
	      CommonTable::Iterator funcSaysIter = saysTbl->lookup(CommonTable::theKey(CommonTable::KEY2), 
								   CommonTable::theKey(CommonTable::KEY3), functor);
	      if(!funcSaysIter->done()){
		TuplePtr functorSays = funcSaysIter->next();
		string funcName = Val_Str::cast((*functor)[catalog->attribute(FUNCTOR, "NAME")]);

		functorSaysAttr = Val_List::cast((*functorSays)[catalog->attribute(SAYS, "ATTRIBUTES")]);
		uint32_t termCount = functorAttr->size(); // term count excluding the sys attr
		normalizeVerify(catalog, functor, functorSays, newVariable); //change the functorname and add terms from the says
		functorAttr = Val_List::cast((*functor)[catalog->attribute(FUNCTOR, "ATTRIBUTES")]); // the attributes including the said attrs

		if(headSays && headName == funcName){
		  // if head is also says for the same table

		  // maybe these should be select terms
		  TupleList *comparisonTerms = generateAssignTerms(headAttr->begin(),
								   functorAttr->begin(), 
								   termCount, 
								   funcpos, 
								   ruleId);
		  
		  generateAlgebraLT(headSaysAttr->begin(), 
				    functorSaysAttr->begin(), 
				    funcpos, 
				    ruleId, 
				    comparisonTerms);
		  TuplePtr assign  = Tuple::mk(ASSIGN, true);
		  assign->append(ruleId);   // Should be rule identifier
		  TuplePtr lhs = headProof->clone();
		  lhs->freeze();
		  assign->append(Val_Tuple::mk(lhs));
		  TuplePtr rhs = (Val_Tuple::cast(functorAttr->at(PROOFPOS)))->clone();
		  rhs->freeze();
		  assign->append(Val_Tuple::mk(rhs));
		  assign->append(Val_Int64::mk(funcpos++));        // Position
		  comparisonTerms->push_back(assign); 
		  if(compoundHead){
		    TuplePtr assignTp  = Tuple::mk(ASSIGN, true);
		    assignTp->append((*rule)[TUPLE_ID]);
		    TuplePtr varCopy = headSaysType->clone();
		    varCopy->freeze();
		    assignTp->append(Val_Tuple::mk(varCopy));               // Assignment variable
		    TuplePtr val = Tuple::mk(VAL);
		    val->append(Val_Int64::mk(COPYSAYS));                  // copy
		    val->freeze();
		    assignTp->append(Val_Tuple::mk(val));
		    assignTp->append(Val_Int64::mk(funcpos++));         // Position
		    comparisonTerms->push_back(assignTp);

		    TuplePtr assignTp1  = Tuple::mk(ASSIGN, true);
		    assignTp1->append((*rule)[TUPLE_ID]);
		    TuplePtr varCopy1 = keyTypeVar->clone();
		    varCopy1->freeze();
		    assignTp1->append(Val_Tuple::mk(varCopy1));               // Assignment variable
		    TuplePtr null = Tuple::mk(VAL);
		    null->append(Val_Null::mk());                  // copy
		    null->freeze();
		    assignTp1->append(Val_Tuple::mk(null));
		    assignTp1->append(Val_Int64::mk(funcpos++));         // Position
		    comparisonTerms->push_back(assignTp1);

		  }
		  newRuleComparators.push_back(comparisonTerms);

		}
		saysTbl->remove(functorSays);
	      }
	    }
	  } // end funcIter
	
	if(headSays)
	  {
	    saysTbl->remove(headSays);

	    // copy this rule into a new rule and insert the new rule into the list as well 
	    // as modify the existing rule
	    int size = newRuleComparators.size();
	    uint32_t originalTermCount = Val_Int64::cast((*rule)[catalog->attribute(RULE, "TERM_COUNT")]);

	    for(int i = 0; i < size; i++){
	      // if i < size - 1, then create a copy for next stage
	      // else use the current copy

	      TupleList *newTermsUse = newRuleComparators.front();
	      uint32_t assignTermPos = originalTermCount + newTermsUse->size();
	      uint32_t newTermsCount = originalTermCount + newTermsUse->size() + (listAssign!= NULL?listAssign->size():0);
	      newRuleComparators.pop_front();
	      
	      
	      ValuePtr newRuleId;
	      if(i < size - 1){
		ostringstream n;
		n << STAGERULEPREFIX << ruleCounter++;
		TuplePtr saysRule = rule->clone(RULE, true);
		newRuleId = (*saysRule)[TUPLE_ID];

		LocSpecInfo* locSpecInfo = new LocSpecInfo(*compile::Context::ruleLocSpecMap->find(ruleId)->second);
		compile::Context::ruleLocSpecMap->insert(std::pair<ValuePtr, LocSpecInfo*>(newRuleId, locSpecInfo)); // insert values into the event loc spec
		saysRule->set(catalog->attribute(RULE, "NAME"), Val_Str::mk(n.str()));
		TuplePtr newHead = head->clone(FUNCTOR, true);
		ListPtr attr = List::mk();
		for (ValPtrList::const_iterator iter = headAttr->begin();
		     iter!= headAttr->end(); iter++) {
		  attr->append(*iter);
		}
		newHead->set(catalog->attribute(FUNCTOR, "ATTRIBUTES"), Val_List::mk(attr));
		newHead->set(catalog->attribute(FUNCTOR, "RID"), newRuleId);
		newHead->freeze();
		functorTbl->insert(newHead);
		saysRule->set(catalog->attribute(RULE, "HEAD_FID"), (*newHead)[TUPLE_ID]);
		saysRule->set(catalog->attribute(RULE, "TERM_COUNT"), Val_Int64::mk(newTermsCount));
		saysRule->freeze();
		ruleTbl->insert(saysRule);

		CommonTable::Iterator Iter;
		Iter = newTbl->lookup(CommonTable::theKey(CommonTable::KEY2), CommonTable::theKey(CommonTable::KEY3), head);
		if(!Iter->done()){
		  TuplePtr newTerm = Iter->next()->clone(NEW, true);
		  newTerm->set(catalog->attribute(NEW, "FID"), (*newHead)[TUPLE_ID]);
		  newTerm->freeze();
		  newTbl->insert(newTerm);
		}


		//copy functors
		CommonTable::Iterator funcIter;
		CommonTable::Key key;
		
		for (funcIter = functorTbl->lookup(CommonTable::theKey(CommonTable::KEY2), 
						   CommonTable::theKey(CommonTable::KEY3), rule);
		     !funcIter->done(); ) {
		  TuplePtr functor = funcIter->next();
		  if ((*functor)[TUPLE_ID] != (*rule)[catalog->attribute(RULE, "HEAD_FID")]) {
		    Iter = newTbl->lookup(CommonTable::theKey(CommonTable::KEY2), CommonTable::theKey(CommonTable::KEY3), functor);
		    ListPtr funcAttr = Val_List::cast((*functor)[catalog->attribute(FUNCTOR, "ATTRIBUTES")]);
		    functor = functor->clone(FUNCTOR, true);
		    ListPtr attr = List::mk();
		    for (ValPtrList::const_iterator iter = funcAttr->begin();
			 iter!= funcAttr->end(); iter++) {
		      attr->append(*iter);
		    }
		    functor->set(catalog->attribute(FUNCTOR, "ATTRIBUTES"), Val_List::mk(attr));

		    functor->set(catalog->attribute(FUNCTOR, "RID"), newRuleId);
		    functor->freeze();
		    functorTbl->insert(functor);
		    if(!Iter->done()){
		      TuplePtr newTerm = Iter->next()->clone(NEW, true);
		      newTerm->set(catalog->attribute(NEW, "FID"), (*functor)[TUPLE_ID]);
		      newTerm->freeze();
		      newTbl->insert(newTerm);
		    }
		  }
		}

		// copy assign
		key.push_back(catalog->attribute(ASSIGN, "RID"));
		
		Iter = catalog->table(ASSIGN)->lookup(CommonTable::theKey(CommonTable::KEY2), key, rule); 
		while (!Iter->done()) {
		  TuplePtr assign = Iter->next()->clone(ASSIGN, true);
		  assign->set(catalog->attribute(ASSIGN, "RID"), newRuleId);
		  assign->freeze();
		  if (!catalog->table(ASSIGN)->insert(assign)){
		    throw Exception("Secure Rewrite: Can't insert assignment. " + rule->toString());
		  }
		}

		key.clear();
		
		key.push_back(catalog->attribute(SELECT, "RID"));
		Iter = catalog->table(SELECT)->lookup(CommonTable::theKey(CommonTable::KEY2), key, rule); 
		while (!Iter->done()) {
		  TuplePtr select = Iter->next()->clone(SELECT, true);
		  select->set(catalog->attribute(SELECT, "RID"), newRuleId);
		  select->freeze();
		  if (!catalog->table(SELECT)->insert(select)){
		    throw Exception("Secure rewrite: Can't insert selection. " + rule->toString());
		  }
		}


	      }
	      else{
		newRuleId = (*rule)[TUPLE_ID];
		TuplePtr saysRule = rule->clone();
		saysRule->set(catalog->attribute(RULE, "TERM_COUNT"), Val_Int64::mk(newTermsCount));
		saysRule->freeze();
		ruleTbl->insert(saysRule);
	      }

	      if(listAssign != NULL){
		for (TupleList::iterator it = listAssign->begin(); 
		     it != listAssign->end(); it++) {
		  TuplePtr term = *it;
		  string tname = Val_Str::cast((*term)[TNAME]);
		  if(tname == FUNCTOR){
		    TuplePtr functor = term->clone(FUNCTOR, true);
		    functor->set(catalog->attribute(FUNCTOR, "RID"), newRuleId);
		    functor->set(catalog->attribute(FUNCTOR, "POSITION"), Val_Int64::mk(assignTermPos++));
		    functor->freeze();
		    functorTbl->insert(functor);
		  }
		  else if(tname == ASSIGN){
		    TuplePtr assign = term->clone(ASSIGN, true);
		    assign->set(catalog->attribute(ASSIGN, "RID"),  newRuleId);
		    assign->set(catalog->attribute(ASSIGN, "POSITION"), Val_Int64::mk(assignTermPos++));
		    assign->freeze();
		    assignTbl->insert(assign);
		  }
		  else if(tname == SELECT){
		    TuplePtr select = term->clone(SELECT, true);
		    select->set(catalog->attribute(SELECT, "RID"), newRuleId);
		    select->set(catalog->attribute(SELECT, "POSITION"), Val_Int64::mk(assignTermPos++));
		    select->freeze();
		    selectTbl->insert(select);
		  }
		  else{
		    throw Exception("Secure rewrite: Invalid term type" + tname);
		  }
		}
	      }

	      if(newTermsUse != NULL){
		for (TupleList::iterator it = newTermsUse->begin(); 
		     it != newTermsUse->end(); it++) {
		  TuplePtr term = *it;
		  string tname = Val_Str::cast((*term)[TNAME]);
		  if(tname == FUNCTOR){
		    term->set(catalog->attribute(FUNCTOR, "RID"), newRuleId);
		    term->freeze();
		    functorTbl->insert(term);
		  }
		  else if(tname == ASSIGN){
		    term->set(catalog->attribute(ASSIGN, "RID"),  newRuleId);
		    term->freeze();
		    assignTbl->insert(term);
		  }
		  else if(tname == SELECT){
		    term->set(catalog->attribute(SELECT, "RID"), newRuleId);
		    term->freeze();
		    selectTbl->insert(term);
		  }
		  else{
		    throw Exception("Secure rewrite: Invalid term type" + tname);
		  }
		}
		delete newTermsUse;
	      }

	    }
	    if(listAssign != NULL){
	      delete listAssign;
	    }

	  }
      }

    }

      /**
       * modifies the rule which has the head :- table(), says<> into non says rhs
       */
      void Context::normalizeVerify(CommonTable::ManagerPtr catalog, TuplePtr &functor, TuplePtr says, uint32_t &newVariable){
	functor = functor->clone();
	string name = Val_Str::cast((*functor)[catalog->attribute(FUNCTOR, "NAME")]);

	functor->set(catalog->attribute(FUNCTOR, "NAME"), Val_Str::mk(name + SAYSSUFFIX));

	CommonTable::Key nameKey;
	nameKey.push_back(catalog->attribute(FUNCTOR, "NAME"));
	CommonTablePtr tableTbl = catalog->table(TABLE);
	CommonTable::Iterator tIter = 
	  tableTbl->lookup(nameKey, CommonTable::theKey(CommonTable::KEY3), functor);
	if (!tIter->done()) 
	  functor->set(catalog->attribute(FUNCTOR, "TID"), (*tIter->next())[TUPLE_ID]);
	else {
	  functor->set(catalog->attribute(FUNCTOR, "TID"), Val_Null::mk());
	}

	ListPtr attr = Val_List::cast((*functor)[catalog->attribute(FUNCTOR, "ATTRIBUTES")]);

	ListPtr newAttr = List::mk();
	ValPtrList::const_iterator iter = attr->begin(); 
	newAttr->append(*iter);
	iter++;
	
	ListPtr saysAttr = Val_List::cast((*says)[catalog->attribute(SAYS, "ATTRIBUTES")]);
	for (ValPtrList::const_iterator saysIter = saysAttr->begin(); 
	     saysIter != saysAttr->end(); saysIter++){
	  newAttr->append(*saysIter);
	}
	ostringstream var5;
	var5<< STAGEVARPREFIX << newVariable++ ;
	TuplePtr proof = Tuple::mk(VAR);
	proof->append(Val_Str::mk(var5.str()));
	proof->freeze();

	newAttr->append(Val_Tuple::mk(proof));
	for (; iter != attr->end(); iter++){
	  newAttr->append(*iter);
	}

	attr->prepend(Val_Tuple::mk(proof));

	functor->set(catalog->attribute(FUNCTOR, "ATTRIBUTES"), Val_List::mk(newAttr));
	functor->freeze();
	catalog->table(FUNCTOR)->insert(functor);

	verificationTables.insert(new VerificationTuple(name, newAttr->size()));

      }

      // return a list of terms that needs to be added to the rule on converting the 
      // securelog term f into overlog.
      // Also converts f into the appropriate overlog form
      TupleList* Context::normalizeGenerate(CommonTable::ManagerPtr catalog, TuplePtr& head, TuplePtr& rule, 
					    TuplePtr loc, uint32_t& newVariable, uint32_t& pos, 
					    bool _compound, TuplePtr keyProofVar, TuplePtr headSaysType, 
					    TuplePtr keyTypeVar, TupleList* listAssign)
      {
	TupleList *newTerms = new TupleList();
	
	CommonTablePtr newTbl = catalog->table(NEW);
	CommonTablePtr saysTbl = catalog->table(SAYS);
	CommonTablePtr ruleTbl = catalog->table(RULE);
	CommonTablePtr assignTbl = catalog->table(ASSIGN);
	CommonTablePtr functorTbl = catalog->table(FUNCTOR);

	CommonTable::Iterator saysIter = saysTbl->lookup(CommonTable::theKey(CommonTable::KEY2), 
							 CommonTable::theKey(CommonTable::KEY3), head);

	TuplePtr says = saysIter->next();

	ListPtr saysAttr = Val_List::cast((*says)[catalog->attribute(SAYS, "ATTRIBUTES")]);

	ValuePtr ruleId = (*rule)[TUPLE_ID];

	// generate variables for key
// 	ostringstream v;
// 	v << STAGEVARPREFIX << newVariable++;
// 	TuplePtr keyVar = Tuple::mk(VAR);
// 	keyVar->append(Val_Str::mk(v.str()));
// 	keyVar->freeze();

	// generate variable for key type
// 	ostringstream v1;
// 	v1 << STAGEVARPREFIX << newVariable++;
// 	TuplePtr keyTypeVar = Tuple::mk(VAR);
// 	keyTypeVar->append(Val_Str::mk(v1.str()));
// 	keyTypeVar->freeze();

	// if new, then add the says params and encryption key to the hint field
	// else generate terms for creating the buffer and encrypting the buffer using the key
	if(_compound){
	  // assume that the new field is already there: ideally in a non-constructor rule, you must replace the lhs to have the new field
	  // but we work assuming that new field is already there
	  CommonTable::Iterator iter = newTbl->lookup(CommonTable::theKey(CommonTable::KEY2), CommonTable::theKey(CommonTable::KEY3), head);
	  TuplePtr newPtr;
	  TuplePtr sysHint;
	  if(iter->done()){
	    // create a new tuple for this new functor
	    head = head->clone();
	    head->set(catalog->attribute(FUNCTOR, "NEW"), Val_Int64::mk(1)); // make new
	    newPtr = Tuple::mk(NEW, true);
	    newPtr->append((*head)[TUPLE_ID]); // FID

	    TuplePtr myloc1 = loc->clone();
	    myloc1->freeze();
	    newPtr->append(Val_Tuple::mk(myloc1)); // LOC
	    ostringstream oss;
	    oss << STAGEVARPREFIX << newVariable++;
	    TuplePtr var = Tuple::mk(VAR);
	    var->append(Val_Str::mk(oss.str()));
	    var->freeze();
	    newPtr->append(Val_Tuple::mk(var)); // OPAQUE

	    ostringstream oss1;
	    oss1 << STAGEVARPREFIX << newVariable++;
	    sysHint = Tuple::mk(VAR);
	    sysHint->append(Val_Str::mk(oss1.str()));
	    sysHint->freeze();
	    newPtr->append(Val_Tuple::mk(sysHint)); // HINTVAR
	    
	    TuplePtr val = Tuple::mk(VAL);
	    val->append(Val_Null::mk());
	    val->freeze();
	    
	    TuplePtr assignTp  = Tuple::mk(ASSIGN, true);
	    assignTp->append(ruleId);
	    TuplePtr varCopy = var->clone();
	    varCopy->freeze();
	    assignTp->append(Val_Tuple::mk(varCopy));               // Assignment variable
	    assignTp->append(Val_Tuple::mk(val));                  // Assignemnt value
	    assignTp->append(Val_Int64::mk(pos++));         // Position

	    assignTp->freeze();
	    assignTbl->insert(assignTp);
	    //	    newTerms->push_back(assignTp);  

	    head->freeze();
	    functorTbl->insert(head);
	    newPtr->freeze();
	    newTbl->insert(newPtr);
	  }
	  else{
	    newPtr = iter->next()->clone();
	    ostringstream var3;
	    var3 << STAGEVARPREFIX << newVariable++;
	    sysHint = Tuple::mk(VAR);
	    sysHint->append(Val_Str::mk(var3.str()));
	    sysHint->freeze();
	    newPtr->set(catalog->attribute(NEW, "SYSTEMINFO"), Val_Tuple::mk(sysHint));
	    newPtr->freeze();
	    newTbl->insert(newPtr);
	    
	  }

	  uint32_t offsetPos = 0;

	  // now add terms that assign to this list
	  
	  TuplePtr f_init1 = Tuple::mk(FUNCTION);
	  f_init1->append(Val_Str::mk(CONSLIST));
	  f_init1->append(Val_Int64::mk(CONSLISTARGS));
	  f_init1->append(Val_Tuple::mk(headSaysType));
	  TuplePtr null = Tuple::mk(VAL);
	  null->append(Val_Null::mk());
	  null->freeze();
	  f_init1->append(Val_Tuple::mk(null));
	  f_init1->freeze();

	  TuplePtr f_init2 = Tuple::mk(FUNCTION);
	  f_init2->append(Val_Str::mk(APPENDFUNC));
	  f_init2->append(Val_Int64::mk(APPENDFUNCARGS));
	  f_init2->append(Val_Tuple::mk(keyTypeVar));
	  f_init2->append(Val_Tuple::mk(f_init1));
	  f_init2->freeze();

	  TuplePtr f_init = Tuple::mk(FUNCTION);
	  f_init->append(Val_Str::mk(APPENDFUNC));
	  f_init->append(Val_Int64::mk(APPENDFUNCARGS));
	  f_init->append(Val_Tuple::mk(keyProofVar));
	  f_init->append(Val_Tuple::mk(f_init2));
	  f_init->freeze();

	  ostringstream var4;
	  var4 << STAGEVARPREFIX << newVariable++;
	  TuplePtr l1 = Tuple::mk(VAR);
	  l1->append(Val_Str::mk(var4.str()));
	  l1->freeze();

	  // generate l1 = f_initList(key)
	  TuplePtr assignTp1  = Tuple::mk(ASSIGN, true);
	  assignTp1->append(ruleId);   // Should be rule identifier
	  assignTp1->append(Val_Tuple::mk(l1));
	  assignTp1->append(Val_Tuple::mk(f_init));
	  assignTp1->append(Val_Int64::mk(offsetPos++));        // Position

	  //	  assignTp1->freeze();
	  //	  assignTbl->insert(assignTp1);
	  //	  newTerms->push_back(assignTp1);  
	  listAssign->push_back(assignTp1);
	  
	  // now generate l2 = f_append(l1, P)
	  TuplePtr prevList = l1;
	  uint32_t maxCount = 4;
	  uint32_t counter = 0;
	  for(ValPtrList::const_iterator saysIter = saysAttr->begin(); // initially at P
	      saysIter != saysAttr->end() && counter < maxCount; saysIter++, counter++){
	    TuplePtr f_append = Tuple::mk(FUNCTION);
	    f_append->append(Val_Str::mk(APPENDFUNC));
	    f_append->append(Val_Int64::mk(APPENDFUNCARGS));
	    f_append->append(*saysIter); // add P, R, k, V
	    f_append->append(Val_Tuple::mk(prevList));
	    f_append->freeze();

	    TuplePtr l2;
	    if(counter < maxCount -1){
	      ostringstream var5;
	      var5 << STAGEVARPREFIX << newVariable++;
	      l2 = Tuple::mk(VAR);
	      l2->append(Val_Str::mk(var5.str()));
	      l2->freeze();
	    }
	    else{
	      l2 = sysHint;
	    }
	    
	    TuplePtr assignTp2  = Tuple::mk(ASSIGN, true);
	    assignTp2->append(ruleId);   // Should be rule identifier
	    assignTp2->append(Val_Tuple::mk(l2));
	    assignTp2->append(Val_Tuple::mk(f_append));
	    assignTp2->append(Val_Int64::mk(offsetPos++));        // Position

	    //	    assignTp2->freeze();
	    //	    assignTbl->insert(assignTp2);
	    //	    newTerms->push_back(assignTp2);  
	    listAssign->push_back(assignTp2);

	    prevList = l2; // update prevList

	  }

	  rule = rule->clone();
	  rule->set(catalog->attribute(RULE, "TERM_COUNT"), Val_Int64::mk(pos));
	  rule->freeze();
	  ruleTbl->insert(rule);
	  
	}

	// create encryption hint
	TuplePtr     encHintTp = Tuple::mk(FUNCTOR, true);
        encHintTp->append(ruleId);
	encHintTp->append(Val_Int64::mk(0)); // NOTIN?
	encHintTp->append(Val_Str::mk(ENCHINT));   // Functor name
  
	// Fill in table reference if functor is materialized
	CommonTable::Key nameKey;
	nameKey.push_back(catalog->attribute(FUNCTOR, "NAME"));
	CommonTablePtr tableTbl = catalog->table(TABLE);
	CommonTable::Iterator tIter = 
	  tableTbl->lookup(nameKey, CommonTable::theKey(CommonTable::KEY3), encHintTp);
	if (!tIter->done()) 
	  encHintTp->append((*tIter->next())[TUPLE_ID]);
	else {
	  assert(0);
	  encHintTp->append(Val_Null::mk());
	}
  
	encHintTp->append(Val_Null::mk());          // The ECA flag
	encHintTp->append(Val_Null::mk());          // The attributes field
	encHintTp->append(Val_Int64::mk(pos++));          // The position field
	encHintTp->append(Val_Null::mk());          // The access method
	encHintTp->append(Val_Int64::mk(0)); // The new field

	// Now take care of the functor arguments and variable dependencies
	ListPtr hintattributes = List::mk();
	TuplePtr myloc = loc->clone();
	myloc->freeze();
	hintattributes->append(Val_Tuple::mk(myloc));
	// generate variables for P, R, k, V, Id
	for(uint32_t i = 0; i < NUMSECUREFIELDS + 1; i++){
	  ostringstream v;
	  v << STAGEVARPREFIX << newVariable++;
	  TuplePtr var = Tuple::mk(VAR);
	  var->append(Val_Str::mk(v.str()));
	  var->freeze();
	  hintattributes->append(Val_Tuple::mk(var));
	}

	encHintTp->set(catalog->attribute(FUNCTOR, "ATTRIBUTES"), Val_List::mk(hintattributes));
	newTerms->push_back(encHintTp);

	TuplePtr     genKeyTp = Tuple::mk(FUNCTOR, true);
        genKeyTp->append(ruleId);
	genKeyTp->append(Val_Int64::mk(0)); // NOTIN?
	genKeyTp->append(Val_Str::mk(GENTABLE));   // Functor name
  
	// Fill in table reference if functor is materialized
	nameKey.clear();
	nameKey.push_back(catalog->attribute(FUNCTOR, "NAME"));
	tIter = tableTbl->lookup(nameKey, CommonTable::theKey(CommonTable::KEY3), genKeyTp);
	if (!tIter->done()) 
	  genKeyTp->append((*tIter->next())[TUPLE_ID]);
	else {
	  assert(0);
	  genKeyTp->append(Val_Null::mk());
	}
  
	genKeyTp->append(Val_Null::mk());          // The ECA flag
	genKeyTp->append(Val_Null::mk());          // The attributes field
	genKeyTp->append(Val_Int64::mk(pos++));          // The position field
	genKeyTp->append(Val_Null::mk());          // The access method
	genKeyTp->append(Val_Int64::mk(0)); // The new field

	// Now take care of the functor arguments and variable dependencies
	ListPtr genKeyAttributes = List::mk();
	myloc = loc->clone();
	myloc->freeze();
	genKeyAttributes->append(Val_Tuple::mk(myloc));
	TuplePtr hintId = (Val_Tuple::cast(hintattributes->back()))->clone();
	hintId->freeze();
	genKeyAttributes->append(Val_Tuple::mk(hintId));
	TuplePtr keyTypeVarClone = keyTypeVar->clone();
	keyTypeVarClone->freeze();
	genKeyAttributes->append(Val_Tuple::mk(keyTypeVarClone));
	TuplePtr keyVarClone = keyProofVar->clone();
	keyVarClone->freeze();
	genKeyAttributes->append(Val_Tuple::mk(keyVarClone));
	
	genKeyTp->set(catalog->attribute(FUNCTOR, "ATTRIBUTES"), Val_List::mk(genKeyAttributes));
	newTerms->push_back(genKeyTp);

	ValPtrList::const_iterator rhsIter = hintattributes->begin();
	rhsIter++;//for location field

	// add constraints between saysParams and rhsSaysParams
	generateAlgebraLT(saysAttr->begin(), rhsIter, pos, ruleId, newTerms);

	if(!_compound){
	  // use good old scheme
	  // also delete any new tuples and remove that NEW tag from head if present
	  
	  CommonTable::Iterator iter = newTbl->lookup(CommonTable::theKey(CommonTable::KEY2), CommonTable::theKey(CommonTable::KEY3), head);
	  if(!iter->done()){
	    TuplePtr newPtr = iter->next();
	    newTbl->remove(newPtr);
	    head = head->clone();
	    head->set(catalog->attribute(FUNCTOR, "NEW"), Val_Int64::mk(0));
	    head->freeze();
	    functorTbl->insert(head);
	  }
	  
	  //create buffer creation logic
	  
	  ostringstream var3;
	  var3 << STAGEVARPREFIX << newVariable++;
	  TuplePtr buf = Tuple::mk(VAR);
	  buf->append(Val_Str::mk(var3.str()));
	  buf->freeze();
	  
	  // check if the concatenate operation is needed or not
	  // remember that the loc specifier is excluded but the table name must be included
	  TuplePtr table = Tuple::mk(VAL);
	  string tablename = Val_Str::cast((*head)[catalog->attribute(FUNCTOR, "NAME")]);
	  table->append(Val_Str::mk(tablename));
	  table->freeze();

	  ValuePtr cur = Val_Tuple::mk(table);

	  ListPtr headAttr = Val_List::cast((*head)[catalog->attribute(FUNCTOR, "ATTRIBUTES")]);

	  ValuePtr append = Val_Str::mk("|||");
	  // since loc spec is not included
	  if(headAttr->size() > 1)
	    {
	      ValPtrList::const_iterator iter = headAttr->begin();
	      //exclude location specifier
	      iter++; 
	      for (; iter != headAttr->end(); iter++){
		TuplePtr newcur = Tuple::mk(MATH);
		newcur->append(append);
		newcur->append(cur);
		TuplePtr iterTuple = (Val_Tuple::cast(*iter))->clone();
		iterTuple->freeze();
		newcur->append(Val_Tuple::mk(iterTuple));
		newcur->freeze();
		
		cur = Val_Tuple::mk(newcur);
	      }
	    }
	 
	  TuplePtr assignTp  = Tuple::mk(ASSIGN, true);
	  assignTp->append(ruleId);   // Should be rule identifier
	  assignTp->append(Val_Tuple::mk(buf));
	  assignTp->append(cur);
	  assignTp->append(Val_Int64::mk(pos++));        // Position
	  newTerms->push_back(assignTp);  
	  
	  //create generation logic
// 	  ostringstream var4;
// 	  var4 << STAGEVARPREFIX << newVariable++;
// 	  TuplePtr hashVar = Tuple::mk(VAR);
// 	  hashVar->append(Val_Str::mk(var4.str()));
// 	  hashVar->freeze();
	  
	  ostringstream var5;
	  var5 << STAGEVARPREFIX << newVariable++;
	  TuplePtr proof = Tuple::mk(VAR);
	  proof->append(Val_Str::mk(var5.str()));
	  proof->freeze();

// 	  TuplePtr f_hash = Tuple::mk(FUNCTION);
// 	  f_hash->append(Val_Str::mk(HASHFUNC));
// 	  f_hash->append(Val_Int64::mk(HASHFUNCARGS));
// 	  f_hash->append(Val_Tuple::mk(buf));
// 	  f_hash->freeze();
	  
// 	  TuplePtr assHash  = Tuple::mk(ASSIGN, true);
// 	  assHash->append(ruleId);   // Should be rule identifier
// 	  assHash->append(Val_Tuple::mk(hashVar));
// 	  assHash->append(Val_Tuple::mk(f_hash));
// 	  assHash->append(Val_Int64::mk(pos++));        // Position
// 	  newTerms->push_back(assHash);  

	  TuplePtr f_gen = Tuple::mk(FUNCTION);
	  f_gen->append(Val_Str::mk(GENFUNC));
	  f_gen->append(Val_Int64::mk(GENFUNCARGS));
	  f_gen->append(Val_Tuple::mk(buf)); // hashVar
	  f_gen->append(Val_Tuple::mk(keyTypeVar));
	  f_gen->append(Val_Tuple::mk(keyProofVar));
	  f_gen->freeze();
	  
	  TuplePtr assGen  = Tuple::mk(ASSIGN, true);
	  assGen->append(ruleId);   // Should be rule identifier
	  assGen->append(Val_Tuple::mk(proof));
	  assGen->append(Val_Tuple::mk(f_gen));
	  assGen->append(Val_Int64::mk(pos++));        // Position
	  newTerms->push_back(assGen);  
	  
	  // now modify the says tuple
	  
	  ListPtr newHeadAttr = List::mk();
	  ValPtrList::const_iterator headIter = headAttr->begin(); 
	  newHeadAttr->append(*headIter);
	  headIter++;
	  
	  for (ValPtrList::const_iterator iter = saysAttr->begin(); 
	       iter != saysAttr->end(); iter++){
	    newHeadAttr->append(*iter);
	  }
	  newHeadAttr->append(Val_Tuple::mk(proof));
	  for (; headIter != headAttr->end(); headIter++){
	    newHeadAttr->append(*headIter);
	  }

	  head = head->clone();
	  head->set(catalog->attribute(FUNCTOR, "NAME"), Val_Str::mk(tablename + MAKESAYS));	  
	  head->set(catalog->attribute(FUNCTOR, "ATTRIBUTES"), Val_List::mk(newHeadAttr));	  

	  CommonTable::Key nameKey;
	  nameKey.push_back(catalog->attribute(FUNCTOR, "NAME"));
	  CommonTablePtr tableTbl = catalog->table(TABLE);
	  CommonTable::Iterator tIter = 
	    tableTbl->lookup(nameKey, CommonTable::theKey(CommonTable::KEY3), head);
	  if (!tIter->done()) 
	    head->set(catalog->attribute(FUNCTOR, "TID"), (*tIter->next())[TUPLE_ID]);
	  else {
	    head->set(catalog->attribute(FUNCTOR, "TID"), Val_Null::mk());
	  }

	  head->freeze();
	  functorTbl->insert(head);
	}
	
	return newTerms;
      }

    TupleList* Context::generateAlgebraLT(ValPtrList::const_iterator start1, 
					 ValPtrList::const_iterator start2, 
					 uint32_t &pos,
					 ValuePtr ruleId,
					 TupleList *newTerms){
      TupleList *t;
      if(newTerms == NULL){
	t = new TupleList();
      }
      else{
	t = newTerms;
      }
      ValuePtr lte = Val_Str::mk("<=");
      ValuePtr minus = Val_Str::mk("-");
      
	//      is 1 < 2
	// first generate the lte terms
      generateSelectTerms(start1, 
			  start2, 
			  NUMSECUREFIELDS, 
			  lte, 
			  pos, 
			  ruleId, 
			  t);
      
      // generate f_mod(P1)
      TuplePtr modFunc1 = Tuple::mk(FUNCTION);
      modFunc1->append(Val_Str::mk(MOD));
      modFunc1->append(Val_Int64::mk(MODARGS));
      TuplePtr p1 = (Val_Tuple::cast(*start1))->clone();
      p1->freeze();
      modFunc1->append(Val_Tuple::mk(p1));
      
      // generate f_mod(P2)
      TuplePtr modFunc2 = Tuple::mk(FUNCTION);
      modFunc2->append(Val_Str::mk(MOD));
      modFunc2->append(Val_Int64::mk(MODARGS));
      TuplePtr p2 = (Val_Tuple::cast(*start2))->clone();
      p2->freeze();
      modFunc2->append(Val_Tuple::mk(p2));
      
      // generate f_mod(P1) - f_mod(P2)
      TuplePtr mathLHS = Tuple::mk(MATH);
      mathLHS->append(minus);
      mathLHS->append(Val_Tuple::mk(modFunc1));
      mathLHS->append(Val_Tuple::mk(modFunc2));
      mathLHS->freeze();
      
      // generate k1-k2
      TuplePtr mathRHS = Tuple::mk(MATH);
      mathRHS->append(minus);
      
      start1++; // for P
      start1++; // for R
      TuplePtr k1 = (Val_Tuple::cast(*start1))->clone();
      k1->freeze();
      
      start2++; start2++;// for P and R
      TuplePtr k2 = (Val_Tuple::cast(*start2))->clone();
      k2->freeze();
      
      mathRHS->append(Val_Tuple::mk(k1));
      mathRHS->append(Val_Tuple::mk(k2));
      mathRHS->freeze();
      
      // now generate select
      TuplePtr selectBool = Tuple::mk(BOOL);
      selectBool->append(lte);
      selectBool->append(Val_Tuple::mk(mathLHS));
      selectBool->append(Val_Tuple::mk(mathRHS));
      selectBool->freeze();

      TuplePtr selectTp  = Tuple::mk(SELECT, true);
      selectTp->append(ruleId);   // Should be rule identifier
      selectTp->append(Val_Tuple::mk(selectBool));              // Boolean expression
      selectTp->append(Val_Int64::mk(pos++));        // Position
      selectTp->append(Val_Null::mk());        // Access method
      t->push_back(selectTp); 
      
      return t;
    }
    
      TupleList* Context::generateAssignTerms(ValPtrList::const_iterator start1, 
					      ValPtrList::const_iterator start2, 
					      uint32_t count, 
					      uint32_t &pos, 
					      ValuePtr ruleId, 
					      TupleList *t){
	TupleList *newTerms;
	if(t == NULL){
	  newTerms = new TupleList();
	}
	else{
	  newTerms = t;
	}
	//skip location specifier terms
	uint32_t counter = 0;
	for (;counter < count; start1++, start2++, counter++){
	  if((*start1)->compareTo(*start2) != 0){
 	    TuplePtr assign  = Tuple::mk(ASSIGN, true);
	    assign->append(ruleId);   // Should be rule identifier
	    TuplePtr lhs = (Val_Tuple::cast(*start1))->clone();
	    lhs->freeze();
	    assign->append(Val_Tuple::mk(lhs));
	    TuplePtr rhs = (Val_Tuple::cast(*start2))->clone();
	    rhs->freeze();
	    assign->append(Val_Tuple::mk(rhs));
	    assign->append(Val_Int64::mk(pos++));        // Position
	    newTerms->push_back(assign); 
	  }
	}

	return newTerms;
      }

    TupleList* Context::generateSelectTerms(ValPtrList::const_iterator start1, 
					    ValPtrList::const_iterator start2, 
					    uint32_t count, 
					    ValuePtr o, 
					    uint32_t &pos, 
					    ValuePtr ruleId, 
					    TupleList *t){
      TupleList *newTerms;
      if(t == NULL){
	newTerms = new TupleList();
      }
      else{
	newTerms = t;
      }
      //skip location specifier terms
      uint32_t counter = 0;
      for (;counter < count; start1++, start2++, counter++){
	if((*start1)->compareTo(*start2) != 0){
	  TuplePtr selectBool = Tuple::mk(BOOL);
	  selectBool->append(o);
	  TuplePtr lhs = (Val_Tuple::cast(*start1))->clone();
	  lhs->freeze();
	  selectBool->append(Val_Tuple::mk(lhs));
	  TuplePtr rhs = (Val_Tuple::cast(*start2))->clone();
	  rhs->freeze();

	  selectBool->append(Val_Tuple::mk(rhs));
	  selectBool->freeze();

	  TuplePtr selectTp  = Tuple::mk(SELECT, true);
	  selectTp->append(ruleId);   // Should be rule identifier
	  selectTp->append(Val_Tuple::mk(selectBool));              // Boolean expression
	  selectTp->append(Val_Int64::mk(pos++));        // Position
	  selectTp->append(Val_Null::mk());        // Access method
	  newTerms->push_back(selectTp); 
	}
      }
      return newTerms;
    }

    
    TuplePtr 
    Context::program(CommonTable::ManagerPtr catalog, TuplePtr program) 
    {

      // generate materialize for genKey and encHint and decKey
      //      std::cout<<"exitting secure stage"<<std::endl;
      generateMaterialize(catalog);
      TuplePtr programRes = this->compile::Context::program(catalog, program);
      generateVerificationRules(catalog, programRes);
      return programRes;
    }

    void Context::generateVerificationRules(CommonTable::ManagerPtr catalog, TuplePtr program){
      CommonTablePtr ruleTbl = catalog->table(RULE);
      CommonTablePtr assignTbl = catalog->table(ASSIGN);
      CommonTablePtr functorTbl = catalog->table(FUNCTOR);
      CommonTablePtr selectTbl = catalog->table(SELECT);

      ValuePtr programID = (*program)[TUPLE_ID];
      VerificationTupleSet::iterator vtiter = verificationTables.begin();

      for(; vtiter != verificationTables.end(); vtiter++){
	uint32_t pos = 0;
	uint32_t fictVar = 0;
	VerificationTuple *vt = (*vtiter);
    
	ostringstream oss;
	oss << STAGERULEPREFIX <<ruleCounter++;
	string rulename = oss.str();

	TuplePtr ruleTp = Tuple::mk(RULE, true);
	ruleTp->append(programID);
	ruleTp->append(Val_Str::mk(rulename));
	
	ruleTp->append(Val_Null::mk());             // Add the "head" functor identifer.
	ruleTp->append(Val_Null::mk());                  // The P2DL desc. of this rule
	ruleTp->append(Val_Int64::mk(false));         // Delete rule?
	ruleTp->append(Val_Int64::mk(2)); // Term count?
	ruleTp->append(Val_Int64::mk(0)); // NEW

	ValuePtr ruleId = (*ruleTp)[TUPLE_ID];
	TuplePtr head = generateFunctor(catalog, fictVar, vt->name + SAYSSUFFIX, ruleId, vt->numVars);
	head->set(catalog->attribute(FUNCTOR, "POSITION"), Val_Int64::mk(pos++));
	head->freeze();
	functorTbl->insert(head);

	TuplePtr rhs = head->clone(FUNCTOR, true);
	rhs->set(catalog->attribute(FUNCTOR, "NAME"), Val_Str::mk(vt->name+MAKESAYS));
	rhs->set(catalog->attribute(FUNCTOR, "POSITION"), Val_Int64::mk(pos++));
	rhs->set(catalog->attribute(FUNCTOR, "TID"), Val_Null::mk());

	ruleTp->set(catalog->attribute(RULE, "HEAD_FID"), (*head)[TUPLE_ID]);
	//create buffer creation logic
	
	ostringstream var3;
	var3 << STAGEVARPREFIX << fictVar++;
	TuplePtr buf = Tuple::mk(VAR);
	buf->append(Val_Str::mk(var3.str()));
	buf->freeze();
	
	// check if the concatenate operation is needed or not
	// remember that the loc specifier is excluded but the table name must be included
	TuplePtr table = Tuple::mk(VAL);
	table->append(Val_Str::mk(vt->name));
	table->freeze();

	ValuePtr cur = Val_Tuple::mk(table);
	  
	ValuePtr append = Val_Str::mk("|||");
	// since loc spec is not included
	ListPtr headAttr = Val_List::cast((*head)[catalog->attribute(FUNCTOR, "ATTRIBUTES")]);
	ValPtrList::const_iterator iter = headAttr->begin();
	//exclude location specifier
	iter++; 

	assert(headAttr->size() >= 6);
	
	TuplePtr P = (Val_Tuple::cast(*iter))->clone();
	P->freeze();
	iter++;

	TuplePtr R = (Val_Tuple::cast(*iter))->clone();
	R->freeze();
	iter++;

	TuplePtr k = (Val_Tuple::cast(*iter))->clone();
	k->freeze();
	iter++;

	TuplePtr V = (Val_Tuple::cast(*iter))->clone();
	V->freeze();
	iter++;

	TuplePtr proof = (Val_Tuple::cast(*iter))->clone();
	proof->freeze();
	iter++;
	  
	// since loc spec is not included
	if(headAttr->size() > 6) // for locspec, P, R, k, V and proof
	{
	  uint32_t maxCount = headAttr->size();
	  uint32_t count = 6;
	  
	  for (; count < maxCount; iter++, count++){
	    TuplePtr newcur = Tuple::mk(MATH);
	    newcur->append(append);
	    newcur->append(cur);
	    TuplePtr iterTuple = (Val_Tuple::cast(*iter))->clone();
	    iterTuple->freeze();
	    newcur->append(Val_Tuple::mk(iterTuple));
	    newcur->freeze();
	    
	    cur = Val_Tuple::mk(newcur);
	  }
	}

	TuplePtr assignTp  = Tuple::mk(ASSIGN, true);
	assignTp->append(ruleId);   // Should be rule identifier
	assignTp->append(Val_Tuple::mk(buf));
	assignTp->append(cur);
	assignTp->append(Val_Int64::mk(pos++));        // Position
	assignTp->freeze();
	assignTbl->insert(assignTp);

	TuplePtr f_ver = Tuple::mk(FUNCTION);
	f_ver->append(Val_Str::mk(VERFUNC));
	f_ver->append(Val_Int64::mk(VERFUNCARGS));
	f_ver->append(Val_Tuple::mk(buf));
	f_ver->append(Val_Tuple::mk(proof));
	f_ver->append(Val_Tuple::mk(P));
	f_ver->append(Val_Tuple::mk(R));
	f_ver->append(Val_Tuple::mk(k));
	f_ver->append(Val_Tuple::mk(V));
	f_ver->freeze();
	
	TuplePtr one = Tuple::mk(VAL);
	one->append(Val_Int64::mk(1));
	one->freeze();

	TuplePtr _boolExpr = Tuple::mk(BOOL);
	_boolExpr->append(Val_Str::mk("=="));
	_boolExpr->append(Val_Tuple::mk(one));
	_boolExpr->append(Val_Tuple::mk(f_ver));
	_boolExpr->freeze();

	TuplePtr selVer  = Tuple::mk(SELECT, true);
	selVer->append(ruleId);   // Should be rule identifier	
	selVer->append(Val_Tuple::mk(_boolExpr));        // Position
	selVer->append(Val_Int64::mk(pos++));        // Position
	selVer->append(Val_Null::mk());        // AM
	selVer->freeze();
	selectTbl->insert(selVer);

	ruleTp->set(catalog->attribute(RULE, "TERM_COUNT"), Val_Int64::mk(pos));
	ruleTp->freeze();
	ruleTbl->insert(ruleTp);

	ListPtr rhsAttr = List::mk();
	iter = headAttr->begin();
	for(; iter != headAttr->end(); iter++){
	  rhsAttr->append(*iter);
	}
	rhs->set(catalog->attribute(FUNCTOR, "ATTRIBUTES"), Val_List::mk(rhsAttr));
	rhs->freeze();
	functorTbl->insert(rhs);

	delete vt;

	if((*head)[catalog->attribute(FUNCTOR, "TID")] != Val_Null::mk()){
	  SetPtr locSpecSet(new Set());
	  SetPtr strongLocSpecSet(new Set());
	  ValuePtr eventLocSpec = rhsAttr->front();
	  ValuePtr ruleId = (*ruleTp)[TUPLE_ID];

	  compile::Context::ruleLocSpecMap->insert(std::pair<ValuePtr, LocSpecInfo*>(ruleId, new LocSpecInfo(eventLocSpec, locSpecSet, strongLocSpecSet))); // insert values into the event loc spec
	}
      }
      
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

    void Context::generateMaterialize(CommonTable::ManagerPtr catalog)
      {

	//generate materialize for encHint
	Table2::Key encHintKey;
	encHintKey.push_back(2); 
	encHintKey.push_back(3); 
	encHintKey.push_back(4); 
	encHintKey.push_back(5); 
	encHintKey.push_back(6); 

	try{
	  Table2::mk(*catalog,ENCHINT,CommonTable::NO_EXPIRATION, CommonTable::NO_SIZE, encHintKey, ListPtr(), Val_Null::mk());
	  TELL_ERROR<<" Successfully created table encHint"
	            << "\n";
	}
	catch(TableManager::Exception e){
	  TELL_ERROR << "generateMaterialize caught a table creation exception: '"
                 << e.toString()
                 << "\n";

	}
      
	//generate materialize for verTable

	Table2::Key vertableKey;
	vertableKey.push_back(2); 
	try{
	  Table2::mk(*catalog,VERTABLE,CommonTable::NO_EXPIRATION, CommonTable::NO_SIZE, vertableKey, ListPtr(), Val_Null::mk());
	TELL_ERROR<<" Successfully created table verTable"
	            << "\n";
	}
	catch(TableManager::Exception e){
	  TELL_ERROR << "generateMaterialize caught a table creation exception: '"
                 << e.toString()
                 << "\n";

	}

	//generate materialize for genTable
	Table2::Key gentableKey;
	gentableKey.push_back(2); 

	try{
	  Table2::mk(*catalog,GENTABLE,CommonTable::NO_EXPIRATION, CommonTable::NO_SIZE, gentableKey, ListPtr(), Val_Null::mk());
	TELL_ERROR<<" Successfully created table genTable"
	            << "\n";
	}
	catch(TableManager::Exception e){
	  TELL_ERROR << "generateMaterialize caught a table creation exception: '"
                 << e.toString()
                 << "\n";

	}

	CommonTablePtr functorTbl = catalog->table(FUNCTOR);
	CommonTable::Iterator funcIter;
	TuplePtr dummyTpl = Tuple::mk();
	CommonTablePtr tableTbl = catalog->table(TABLE);
	CommonTable::Iterator tIter;
	ValuePtr tid;
	uint32_t tidPos = catalog->attribute(FUNCTOR, "TID");

	// add materialization statements for ENCHINT
	dummyTpl->append(Val_Str::mk(ENCHINT));
	
	tIter = tableTbl->lookup(CommonTable::theKey(CommonTable::KEY0), CommonTable::theKey(CommonTable::KEY3), dummyTpl);
	if (!tIter->done()) 
	  tid = (*tIter->next())[TUPLE_ID];
	else {
	  assert(0);
	}

	for (funcIter = functorTbl->lookup(CommonTable::theKey(CommonTable::KEY0), 
					   CommonTable::theKey(CommonTable::KEY5), dummyTpl);
	     !funcIter->done(); ) {
	  TuplePtr functor = funcIter->next()->clone();
	  functor->set(tidPos, tid);
	  functor->freeze();
	  functorTbl->insert(functor);
	}

	dummyTpl->set(0, Val_Str::mk(VERTABLE));
	
	tIter = tableTbl->lookup(CommonTable::theKey(CommonTable::KEY0), CommonTable::theKey(CommonTable::KEY3), dummyTpl);
	if (!tIter->done()) 
	  tid = (*tIter->next())[TUPLE_ID];
	else {
	  assert(0);
	}

	for (funcIter = functorTbl->lookup(CommonTable::theKey(CommonTable::KEY0), 
					   CommonTable::theKey(CommonTable::KEY5), dummyTpl);
	     !funcIter->done(); ) {
	  TuplePtr functor = funcIter->next()->clone();
	  functor->set(tidPos, tid);
	  functor->freeze();
	  functorTbl->insert(functor);
	}


	dummyTpl->set(0, Val_Str::mk(GENTABLE));
	
	tIter = tableTbl->lookup(CommonTable::theKey(CommonTable::KEY0), CommonTable::theKey(CommonTable::KEY3), dummyTpl);
	if (!tIter->done()) 
	  tid = (*tIter->next())[TUPLE_ID];
	else {
	  assert(0);
	}

	for (funcIter = functorTbl->lookup(CommonTable::theKey(CommonTable::KEY0), 
					   CommonTable::theKey(CommonTable::KEY5), dummyTpl);
	     !funcIter->done(); ) {
	  TuplePtr functor = funcIter->next()->clone();
	  functor->set(tidPos, tid);
	  functor->freeze();
	  functorTbl->insert(functor);
	}
	
      }
 
  }
}
