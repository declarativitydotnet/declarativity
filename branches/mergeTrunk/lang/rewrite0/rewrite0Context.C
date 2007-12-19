/*
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: This stage must be inserted before the eca stage as it modifies the head functors
 * 1> changes the name of new functors on rhs of constructor rules to include the pos of the newly constructed 
 *	parent tuples. i.e. changes the "tuplenew" to "tuplenew4" if the new loc spec appears in 4th field
 * 2> Also, changes the head functors to new functors in "new" rules if they are not tagged new already: All
 * new rules must have new lhs because version need to be created for the lhs terms
 * 3> localizes the new rules' rhs
 * Assumption: constructor rules are purely local
 * So, the only interesting scenarios in this case are rules that create container tuples 
 * (i.e. parent tuples) but are not constructor
 * PS: it seems that in presence of new tuples, this localization is not really needed, so we are skipping this for the moment
 */

#include<iostream>
#include "rewrite0Context.h"
#include "plumber.h"
#include "systemTable.h"
#include "value.h"
#include "val_str.h"
#include "val_null.h"
#include "val_list.h"
#include "set.h"
#include "val_tuple.h"
#include "val_int64.h"

namespace compile {
  namespace rewrite0{
    using namespace opr;
    
    DEFINE_ELEMENT_INITS_NS(Context, "Rewrite0Context", compile::rewrite0)
      
      Context::Context(string name)
	: compile::Context(name) { }
    
    Context::Context(TuplePtr args)
      : compile::Context((*args)[2]->toString()) { }

    /**
     * convert a new functor into conventional serialized form
     */
    void Context::serializeNewFunctor(CommonTable::ManagerPtr catalog, TuplePtr& functor){
      CommonTablePtr newTbl = catalog->table(NEW);
      CommonTablePtr functorTbl = catalog->table(FUNCTOR);
      assert(Val_Int64::cast((*functor)[catalog->attribute(FUNCTOR, "NEW")]) == 1);
      CommonTable::Iterator iter = newTbl->lookup(CommonTable::theKey(CommonTable::KEY2), CommonTable::theKey(CommonTable::KEY3), functor);
      if(!iter->done()){
	TuplePtr newPtr = iter->next();
	functor = functor->clone();
	string newname = Val_Str::cast((*functor)[catalog->attribute(FUNCTOR, "NAME")]) + compile::NEWSUFFIX;
	functor->set(catalog->attribute(FUNCTOR, "NAME"), Val_Str::mk(newname));
	functor->set(catalog->attribute(FUNCTOR, "TID"), Val_Null::mk());
	ListPtr funcAttr  = Val_List::cast((*functor)[catalog->attribute(FUNCTOR, "ATTRIBUTES")]);
	TuplePtr locAttr = (Val_Tuple::cast(funcAttr->front()))->clone();
	funcAttr->pop_front();
	locAttr->set(TNAME, Val_Str::mk(VAR));//change from loc to var
	locAttr->freeze();
	funcAttr->prepend(Val_Tuple::mk(locAttr));
	funcAttr->prepend((*newPtr)[catalog->attribute(NEW, "SYSTEMINFO")]);
	funcAttr->prepend((*newPtr)[catalog->attribute(NEW, "OPAQUE")]);
	funcAttr->prepend((*newPtr)[catalog->attribute(NEW, "EVENTLOC")]);
	newTbl->remove(newPtr);
	functor->set(catalog->attribute(FUNCTOR, "ATTRIBUTES"), Val_List::mk(funcAttr));
	functor->freeze();
	functorTbl->insert(functor);
      }
      else{
	std::cout<<"serializeNewFunctor called on functor: " <<functor->toString()<<std::endl;
	assert(0);
      }
    }

    void
    Context::rule(CommonTable::ManagerPtr catalog, TuplePtr rule)
    {
      // do stuff corresponding to pass1 right here
      CommonTablePtr functorTbl = catalog->table(FUNCTOR);

      if(Val_Int64::cast((*rule)[catalog->attribute(RULE, "NEW")]) == 1){
	CommonTablePtr ruleTbl = catalog->table(RULE);
	CommonTablePtr assignTbl = catalog->table(ASSIGN);
	CommonTable::Iterator headIter;
	CommonTable::Iterator funcIter;
	TuplePtr headLocAttr;
	ListPtr headAttributes;
	ValuePtr eventLocSpec;
	TuplePtr headFunctor; 
	uint32_t fictVar = 0;
	bool headNew = false;
	for (headIter = functorTbl->lookup(CommonTable::theKey(CommonTable::KEY5), 
					   CommonTable::theKey(CommonTable::KEY2), rule);
	     !headIter->done(); ) {
	  if(headFunctor){
	    throw compile::rewrite0::Exception("More than one head functors in rule" + rule->toString());
	  }
	  headFunctor = headIter->next()->clone();
	  headAttributes = Val_List::cast((*headFunctor)[catalog->attribute(FUNCTOR, "ATTRIBUTES")]);
	  headLocAttr = Val_Tuple::cast(headAttributes->front());
	  headNew = Val_Int64::cast((*headFunctor)[catalog->attribute(FUNCTOR, "NEW")]) != 0;
	  // do anything at all if this rule does not already have a new head Functor
	  if(!headNew){
	    headLocAttr = headLocAttr->clone();
	    headLocAttr->set(TNAME, Val_Str::mk(VAR));//change from loc to var

	    string newname = Val_Str::cast((*headFunctor)[catalog->attribute(FUNCTOR, "NAME")]) + compile::NEWSUFFIX;
	    headFunctor->set(catalog->attribute(FUNCTOR, "NAME"), Val_Str::mk(newname));
	    headFunctor->set(catalog->attribute(FUNCTOR, "TID"), Val_Null::mk());

	    headFunctor->set(catalog->attribute(FUNCTOR, "NEW"), Val_Int64::mk(1)); // make new
	    headAttributes->pop_front();
	    headLocAttr->freeze();
	    headAttributes->prepend(Val_Tuple::mk(headLocAttr));
	    ostringstream oss;
	    oss << STAGEVARPREFIX << fictVar++;
	    TuplePtr var = Tuple::mk(VAR);
	    var->append(Val_Str::mk(oss.str()));
	    var->freeze();
	    headAttributes->prepend(Val_Tuple::mk(var));

	    ostringstream oss1;
	    oss1 << STAGEVARPREFIX << fictVar++;

	    TuplePtr opaqueTuple = Tuple::mk(VAR);
	    opaqueTuple->append(Val_Str::mk(oss1.str()));
	    opaqueTuple->freeze();
	    headAttributes->prepend(Val_Tuple::mk(opaqueTuple));
	    
	    TuplePtr val = Tuple::mk(VAL);
	    val->append(Val_Null::mk());
	    val->freeze();
	    TuplePtr val1 = val->clone();
	    val1->freeze();
	    
	    rule = rule->clone();
	    ValuePtr ruleId = (*rule)[TUPLE_ID];
	    uint32_t ruleSize = Val_Int64::cast((*rule)[catalog->attribute(RULE, "TERM_COUNT")]);

	    TuplePtr assignTp  = Tuple::mk(ASSIGN, true);
	    assignTp->append(ruleId);
	    TuplePtr varCopy = var->clone();
	    varCopy->freeze();
	    assignTp->append(Val_Tuple::mk(varCopy));               // Assignment variable
	    assignTp->append(Val_Tuple::mk(val));                  // Assignemnt value
	    assignTp->append(Val_Int64::mk(ruleSize++));         // Position
	    assignTp->freeze();
	    assignTbl->insert(assignTp);

	    TuplePtr assignTp1  = Tuple::mk(ASSIGN, true);
	    assignTp1->append(ruleId);
	    TuplePtr varCopy1 = opaqueTuple->clone();
	    varCopy1->freeze();
	    assignTp1->append(Val_Tuple::mk(varCopy1));               // Assignment variable
	    assignTp1->append(Val_Tuple::mk(val1));                  // Assignemnt value
	    assignTp1->append(Val_Int64::mk(ruleSize++));         // Position
	    assignTp1->freeze();
	    assignTbl->insert(assignTp1);

	    rule->set(catalog->attribute(RULE, "TERM_COUNT"), Val_Int64::mk(ruleSize));
	    rule->freeze();
	    ruleTbl->insert(rule);

	  }
	  else{
	    serializeNewFunctor(catalog, headFunctor);
	  }
	}
	
	for (funcIter = functorTbl->lookup(CommonTable::theKey(CommonTable::KEY2), 
					   CommonTable::theKey(CommonTable::KEY3), rule);
	     !funcIter->done(); ) {
	  TuplePtr functor = funcIter->next();
	  if ((*functor)[TUPLE_ID] != (*rule)[catalog->attribute(RULE, "HEAD_FID")]) {
	    if(Val_Int64::cast((*functor)[catalog->attribute(FUNCTOR, "NEW")]) == 1){
	      serializeNewFunctor(catalog, functor);
	      functor = functor->clone();
	      ListPtr attributes = Val_List::cast((*functor)[catalog->attribute(FUNCTOR, "ATTRIBUTES")]);
	      eventLocSpec = attributes->front();
	      // find location of headLocAttr in attributes
	      uint32_t pos = 1;
	      for (ValPtrList::const_iterator iter = attributes->begin();
		   iter != attributes->end(); iter++, pos++) {
		TuplePtr attr = Val_Tuple::cast(*iter);
		if(((*attr)[TUPLE_ID])->compareTo((*headLocAttr)[TUPLE_ID]) == 0){
		  break;
		}
	      }
	      assert(pos <= attributes->size());
	      ostringstream oss1;
	      oss1 << Val_Str::cast((*functor)[catalog->attribute(FUNCTOR, "NAME")]) << "_"<<(pos - NEWFIELDS);
	      string newname = oss1.str();
	      
	      functor->set(catalog->attribute(FUNCTOR, "TID"), Val_Null::mk());
	      functor->set(catalog->attribute(FUNCTOR, "NAME"), Val_Str::mk(newname));
	      functor->freeze();
	      functorTbl->insert(functor);	    
	    }
	  }
	}
	if(!eventLocSpec){
	  throw compile::rewrite0::Exception("No new functor found in new rule:" + rule->toString());
	}
	if(!headNew){
	  headAttributes->prepend(eventLocSpec);
	  headFunctor->set(catalog->attribute(FUNCTOR, "ATTRIBUTES"), Val_List::mk(headAttributes));
	  headFunctor->freeze();
	  functorTbl->insert(headFunctor);
	}
      }
      else{
	for (CommonTable::Iterator funcIter = functorTbl->lookup(CommonTable::theKey(CommonTable::KEY2), 
					   CommonTable::theKey(CommonTable::KEY3), rule);
	     !funcIter->done(); ) {
	  TuplePtr functor = funcIter->next();
	  if(Val_Int64::cast((*functor)[catalog->attribute(FUNCTOR, "NEW")]) == 1){
	    serializeNewFunctor(catalog, functor);
	  }	 
	}
      }
    }


  }
}
