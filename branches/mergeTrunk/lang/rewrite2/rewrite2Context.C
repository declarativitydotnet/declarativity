/*
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: Pre-eca rewrite stage that does the following tasks:
 * 1> converts locSpec based accesses to transformed location based accesses in form of LOCSPECTABLE and versionTable/strongVersionTable
 * 2> changes the materialized "table" into materialized "tableversion" with corresponding modifications in the keys
 * Input: code with constructor and container creation rules. 
 * Assumption: constructors have local rhs, heads have location as locspec
 * Placement: before eca stage
 *
 */

#include<iostream>
#include<vector>
#include "rewrite2Context.h"
#include "plumber.h"
#include "systemTable.h"
#include "value.h"
#include "val_str.h"
#include "val_null.h"
#include "val_int64.h"
#include "val_list.h"
#include "set.h"
#include "val_tuple.h"

namespace compile {
  namespace rewrite2{
    using namespace opr;

    DEFINE_ELEMENT_INITS_NS(Context, "Rewrite2Context", compile::rewrite2)

    Context::Context(string name)
    : compile::Context(name) { }

    Context::Context(TuplePtr args)
    : compile::Context((*args)[2]->toString()) { }



    void
    rule1(CommonTable::ManagerPtr catalog, TuplePtr rule)
    {
      CommonTablePtr functorTbl = catalog->table(FUNCTOR);
      CommonTablePtr assignTbl  = catalog->table(ASSIGN);
      CommonTablePtr selectTbl  = catalog->table(SELECT);
  
      CommonTable::Key functorKey;
      CommonTable::Key assignKey;
      CommonTable::Key selectKey;

      functorKey.push_back(catalog->attribute(FUNCTOR, "RID"));
      functorKey.push_back(catalog->attribute(FUNCTOR, "POSITION"));

      assignKey.push_back(catalog->attribute(ASSIGN, "RID"));
      assignKey.push_back(catalog->attribute(ASSIGN, "POSITION"));

      selectKey.push_back(catalog->attribute(SELECT, "RID"));
      selectKey.push_back(catalog->attribute(SELECT, "POSITION"));

      ostringstream oss;
      if ((*rule)[catalog->attribute(RULE, "NAME")] != Val_Null::mk())
        oss << (*rule)[catalog->attribute(RULE, "NAME")]->toString() << " ";
      oss << ((Val_Int64::cast((*rule)[catalog->attribute(RULE, "DELETE")]) == 1)?"delete ":"") ;

      CommonTable::Iterator iter;
      for (int pos = 0; true; pos++) {
        TuplePtr lookup = Tuple::mk();
        lookup->append((*rule)[TUPLE_ID]);
        lookup->append(Val_Int64::mk(pos));
        lookup->freeze();
        TuplePtr lookup2 = Tuple::mk();
        lookup2->append((*rule)[TUPLE_ID]);
        lookup2->append(Val_Int64::mk(pos));
        lookup2->freeze();

        iter = functorTbl->lookup(CommonTable::theKey(CommonTable::KEY01), functorKey, lookup);
        if (iter->done())
          iter = functorTbl->lookup(CommonTable::theKey(CommonTable::KEY01), functorKey, lookup2);
        if (!iter->done()) {
          if (pos > 1) oss << ",\n\t";

          TuplePtr functor = iter->next();
          ListPtr  schema  = Val_List::cast((*functor)[catalog->attribute(FUNCTOR, "ATTRIBUTES")]);
          oss << (*functor)[catalog->attribute(FUNCTOR, "NAME")]->toString() 
	      << ((Val_Int64::cast((*functor)[catalog->attribute(FUNCTOR, "NEW")]) == 1)?"*":"")
	      <<"(";
          for (ValPtrList::const_iterator siter = schema->begin(); 
               siter != schema->end(); ) {
            TuplePtr arg = Val_Tuple::cast(*siter);
            oss << namestracker::exprString(arg);
            siter++;
            if (siter != schema->end()) oss << ", ";
          }
          oss << ")";
          if (pos == 0) {
            oss << " :-\n\t";
          }
          continue;
        }
        iter = assignTbl->lookup(CommonTable::theKey(CommonTable::KEY01), assignKey, lookup);
        if (iter->done())
          iter = assignTbl->lookup(CommonTable::theKey(CommonTable::KEY01), assignKey, lookup2);
        if (!iter->done()) {
          if (pos > 1) oss << ",\n\t";
          TuplePtr assign = iter->next();
          oss << namestracker::exprString(Val_Tuple::cast((*assign)[catalog->attribute(ASSIGN, "VAR")])); 
          oss << " := ";
          oss << namestracker::exprString(Val_Tuple::cast((*assign)[catalog->attribute(ASSIGN, "VALUE")])); 
          continue;
        }

        iter = selectTbl->lookup(CommonTable::theKey(CommonTable::KEY01), selectKey, lookup);
        if (iter->done())
          iter = selectTbl->lookup(CommonTable::theKey(CommonTable::KEY01), selectKey, lookup2);
        if (!iter->done()) {
          if (pos > 1) oss << ",\n\t";
          TuplePtr select = iter->next();
          oss << namestracker::exprString(Val_Tuple::cast((*select)[catalog->attribute(SELECT, "BOOL")])); 

          continue;
        }

        oss << ".\n";
        TELL_OUTPUT << oss.str() << std::endl << std::endl;
        return;  // Done.
      }
    }
   
    void
    rule2(CommonTable::ManagerPtr catalog, TuplePtr rule)
    {
      CommonTablePtr functorTbl = catalog->table(FUNCTOR);
      CommonTablePtr assignTbl  = catalog->table(ASSIGN);
      CommonTablePtr selectTbl  = catalog->table(SELECT);
  
      CommonTable::Key functorKey;
      CommonTable::Key assignKey;
      CommonTable::Key selectKey;

      functorKey.push_back(catalog->attribute(FUNCTOR, "RID"));
      functorKey.push_back(catalog->attribute(FUNCTOR, "POSITION"));

      assignKey.push_back(catalog->attribute(ASSIGN, "RID"));
      assignKey.push_back(catalog->attribute(ASSIGN, "POSITION"));

      selectKey.push_back(catalog->attribute(SELECT, "RID"));
      selectKey.push_back(catalog->attribute(SELECT, "POSITION"));

      ostringstream oss;
      if ((*rule)[catalog->attribute(RULE, "NAME")] != Val_Null::mk())
        oss << (*rule)[catalog->attribute(RULE, "NAME")]->toString() << " ";
      oss << rule->toString() << "\n";
      CommonTable::Iterator iter;
      for (uint32_t pos = 0; true; pos++) {
        TuplePtr lookup = Tuple::mk();
        lookup->append((*rule)[TUPLE_ID]);
        lookup->append(Val_Int64::mk(pos));
        lookup->freeze();
        TuplePtr lookup2 = Tuple::mk();
        lookup2->append((*rule)[TUPLE_ID]);
        lookup2->append(Val_Int64::mk(pos));
        lookup2->freeze();

        iter = functorTbl->lookup(CommonTable::theKey(CommonTable::KEY01), functorKey, lookup);
        if (iter->done())
          iter = functorTbl->lookup(CommonTable::theKey(CommonTable::KEY01), functorKey, lookup2);
        if (!iter->done()) {
          if (pos > 1) oss << ",\n\t";

          TuplePtr functor = iter->next();
          oss << functor->toString() << "\n";
          if (pos == 0) {
            oss << " :-\n\t";
          }
          continue;
        }
        iter = assignTbl->lookup(CommonTable::theKey(CommonTable::KEY01), assignKey, lookup);
        if (iter->done())
          iter = assignTbl->lookup(CommonTable::theKey(CommonTable::KEY01), assignKey, lookup2);
        if (!iter->done()) {
          if (pos > 1) oss << ",\n\t";
          TuplePtr assign = iter->next();
          oss << assign->toString();
          continue;
        }

        iter = selectTbl->lookup(CommonTable::theKey(CommonTable::KEY01), selectKey, lookup);
        if (iter->done())
          iter = selectTbl->lookup(CommonTable::theKey(CommonTable::KEY01), selectKey, lookup2);
        if (!iter->done()) {
          if (pos > 1) oss << ",\n\t";
          TuplePtr select = iter->next();
	  oss << select->toString();
          continue;
        }

        oss << ".\n";
        TELL_OUTPUT << oss.str() << std::endl << std::endl;
	rule1(catalog, rule);
	return;  // Done.
      }
      
    }   


    void
    Context::rule(CommonTable::ManagerPtr catalog, TuplePtr rule)
    {
      //      rule2(catalog, rule);
      SetPtr locSpecSet;      
      SetPtr strongLocSpecSet;
      bool newRule = ((Val_Int64::cast((*rule)[catalog->attribute(RULE, "NEW")]) == 1)?true:false);
      CommonTablePtr functorTbl = catalog->table(FUNCTOR);
      CommonTable::Iterator funcIter;
      ValuePtr eventLocSpec;
      ValuePtr ruleId = (*rule)[TUPLE_ID];
      uint32_t termCount = Val_Int64::cast((*rule)[catalog->attribute(RULE, "TERM_COUNT")]);
      TuplePtr *termList = new TuplePtr[termCount];
      std::vector<TuplePtr> newTermList;
      uint32_t varSuffix = 0;
      LocSpecMap::iterator ruleLocSpecInfo = compile::Context::ruleLocSpecMap->find(ruleId);
      if(ruleLocSpecInfo == compile::Context::ruleLocSpecMap->end()){
	std::cout<<"WARNING: not locspec-izing rule " + rule->toString();
	return;
      }
      LocSpecInfo *locSpecInfo = ruleLocSpecInfo->second;
      eventLocSpec = locSpecInfo->location;
      locSpecSet = locSpecInfo->locSpecSet;
      strongLocSpecSet = locSpecInfo->strongLocSpecSet;
      compile::Context::ruleLocSpecMap->erase(ruleLocSpecInfo);
      delete locSpecInfo;

      for (funcIter = functorTbl->lookup(CommonTable::theKey(CommonTable::KEY2), 
                                         CommonTable::theKey(CommonTable::KEY3), rule);
           !funcIter->done(); ) {
        TuplePtr functor = funcIter->next();
	uint32_t functorPos = Val_Int64::cast((*functor)[catalog->attribute(FUNCTOR, "POSITION")]);
	termList[functorPos] = functor;
      }

      // copy assign terms
      CommonTablePtr assignTbl = catalog->table(ASSIGN);
      CommonTable::Iterator assignIter;

      for (assignIter = assignTbl->lookup(CommonTable::theKey(CommonTable::KEY2), 
                                         CommonTable::theKey(CommonTable::KEY3), rule);
           !assignIter->done(); ) {
        TuplePtr assign = assignIter->next();
	uint32_t assignPos = Val_Int64::cast((*assign)[catalog->attribute(ASSIGN, "POSITION")]);
	termList[assignPos] = assign;
      }

      // copy select terms
      CommonTablePtr selectTbl = catalog->table(SELECT);
      CommonTable::Iterator selectIter;

      for (selectIter = selectTbl->lookup(CommonTable::theKey(CommonTable::KEY2), 
                                         CommonTable::theKey(CommonTable::KEY3), rule);
           !selectIter->done(); ) {
        TuplePtr select = selectIter->next();
	uint32_t selectPos = Val_Int64::cast((*select)[catalog->attribute(SELECT, "POSITION")]);
	termList[selectPos] = select;
      }

      CommonTablePtr tableTbl = catalog->table(TABLE);
      CommonTable::Iterator tIter;
      uint32_t newpos = 0;

      // now start sequential processing
      for(uint32_t i = 0; i < termCount; i++){
	// if non-functor then simply push it in the new term list
	// ignore "new" functors and head functors of constructor rules which will be transformed to be local and "event"ed
	if(termList[i] == NULL){
	  std::cout<<"TermCount" <<termCount <<" i " << i<<std::endl;
	  throw compile::rewrite2::Exception("Null value in termList");
	}
	string tname = Val_Str::cast((*termList[i])[TNAME]);
	if(tname == ASSIGN){
	  if(newpos != i){
	    TuplePtr copyAssign = termList[i]->clone();
	    copyAssign->set(catalog->attribute(ASSIGN, "POSITION"), Val_Int64::mk(newpos++));
	    copyAssign->freeze();
	    assignTbl->insert(copyAssign);
	  }
	  else{
	    newpos++;
	  }
	  
	}
	else if(tname == SELECT){
	  if(newpos != i){
	    TuplePtr copySelect = termList[i]->clone();
	    copySelect->set(catalog->attribute(SELECT, "POSITION"), Val_Int64::mk(newpos++));
	    copySelect->freeze();
	    selectTbl->insert(copySelect);
	  }
	  else{
	    newpos++;
	  }


	}
	else if(tname == FUNCTOR){
	  // handle functors
	  // check if materialized or not?
	  CommonTable::Key nameKey;
	  nameKey.push_back(catalog->attribute(FUNCTOR, "NAME"));
	  tIter = tableTbl->lookup(nameKey, CommonTable::theKey(CommonTable::KEY3), termList[i]);
	  ValuePtr name = (*(termList[i]))[catalog->attribute(FUNCTOR, "NAME")];
	  if (!tIter->done() && (compile::Context::materializedTables->member(name) == 1)) {
	    // materialized and user created
	    ListPtr attributes = Val_List::cast((*termList[i])[catalog->attribute(FUNCTOR, "ATTRIBUTES")]);
	    TuplePtr tupleLocTuple = Val_Tuple::cast(attributes->front()); // tuple for the location var of the functor
	    TuplePtr tupleLocTupleClone = tupleLocTuple->clone(); 
	    ValuePtr tupleLoc = (*tupleLocTuple)[CONTENTPOS];
	    bool newFunctor = ((Val_Int64::cast((*termList[i])[catalog->attribute(FUNCTOR, "NEW")]) == 1)?true:false);
	    //if locSpec in locSpecSet, then add locSpec(@eventLocSpec, S, N, V), tableVersion(@N, V,...)
	    // otherwise: simply change the name to add version, and add the 0 version
	    if(!newFunctor){
	      if(locSpecSet->member(tupleLoc) == 1){
		if(!newRule || (i != 0)){
		  assert(i != 0);
		  ostringstream oss1;
		  oss1 << STAGEVARPREFIX << varSuffix++;
		  TuplePtr verLocTuple = Tuple::mk(LOC); // location of the version tuple
		  verLocTuple->append(Val_Str::mk(oss1.str()));
		  TuplePtr verLocTupleField = verLocTuple->clone(VAR); // its counterpart in the locSpecTable

		  TuplePtr version = makeVersionedTuple(varSuffix, termList[i], verLocTuple, catalog);
		  ListPtr verAttr = Val_List::cast((*version)[catalog->attribute(FUNCTOR, "ATTRIBUTES")]);
		  TuplePtr verTuple = (Val_Tuple::cast(verAttr->at(compile::VERPOS)))->clone();
		  tupleLocTupleClone->set(TNAME, Val_Str::mk(VAR)); // reset the LOC field of the location specifier
		  tupleLocTupleClone->freeze();

		  TuplePtr locSpecTuple = createLocSpecTuple(ruleId,
							     (Val_Tuple::cast(eventLocSpec))->clone(), // original functor
							     tupleLocTupleClone,  // location specifier
							     verLocTupleField, // location of the version tuple
							     verTuple, // version field
							     catalog);
		  locSpecTuple->set(catalog->attribute(FUNCTOR, "POSITION"), Val_Int64::mk(newpos++));
		  locSpecTuple->freeze();
		  if(!functorTbl->insert(locSpecTuple)){
		    throw compile::rewrite2::Exception("Failed to insert functor" + locSpecTuple->toString());
		  }

		  if(strongLocSpecSet->member(tupleLoc) == 1){
		    // add the link expander tuple
		    ostringstream oss2;
		    oss2 << STAGEVARPREFIX << varSuffix++;
		    TuplePtr linkExpanderSetTuple = Tuple::mk(VAR); // location of the version tuple
		    linkExpanderSetTuple->append(Val_Str::mk(oss2.str()));
		    linkExpanderSetTuple->freeze();

		    TuplePtr tupleLocTupleClone = tupleLocTuple->clone(VAR);
		    tupleLocTupleClone->freeze();

		    TuplePtr eventLocClone = (Val_Tuple::cast(eventLocSpec))->clone();
		    eventLocClone->freeze();

		    TuplePtr linkExpanderTuple = createLinkExpanderTuple(ruleId,
									 eventLocClone,
									 tupleLocTupleClone,  // location specifier
									 linkExpanderSetTuple,
									 catalog);
		    
		    linkExpanderTuple->set(catalog->attribute(FUNCTOR, "POSITION"), Val_Int64::mk(newpos++));
		    linkExpanderTuple->freeze();

		    if(!functorTbl->insert(linkExpanderTuple)){
		      throw compile::rewrite2::Exception("Failed to insert functor" + linkExpanderTuple->toString());
		    }
		    
		    TuplePtr linkExpanderSetTupleCopy = linkExpanderSetTuple->clone();
		    linkExpanderSetTupleCopy->freeze();

		    TuplePtr verTupleClone = verTuple->clone();
		    verTupleClone->freeze();

		    TuplePtr linkExpanderCheck = createLinkExpanderCheck(ruleId, 
									 linkExpanderSetTupleCopy,
									 verTupleClone);

		    linkExpanderCheck->set(catalog->attribute(SELECT, "POSITION"), Val_Int64::mk(newpos++));
		    linkExpanderCheck->freeze();

		    if(!selectTbl->insert(linkExpanderCheck)){
		      throw compile::rewrite2::Exception("Failed to insert select" + linkExpanderCheck->toString());
		    }
		    
		  }
		  version->set(catalog->attribute(FUNCTOR, "POSITION"), Val_Int64::mk(newpos++));
		  version->freeze();
		  functorTbl->insert(version);

		}
		else{
		  newpos++;// skip head with locSpec at location field in new Rules
		}

	      }
	      else{
		tupleLocTupleClone->freeze();
		TuplePtr version = makeVersionedTuple(varSuffix, termList[i], tupleLocTupleClone, catalog);
		version->set(catalog->attribute(FUNCTOR, "POSITION"), Val_Int64::mk(newpos++));
		ListPtr verAttr = Val_List::cast((*version)[catalog->attribute(FUNCTOR, "ATTRIBUTES")]);
		TuplePtr verVarTuple = Val_Tuple::cast(verAttr->at(compile::VERPOS));
		if(i != 0){
		  TuplePtr select = createCurVerSelect(ruleId, verVarTuple->clone());
		  select->set(catalog->attribute(SELECT, "POSITION"), Val_Int64::mk(newpos++));
		  select->freeze();
		  if(!selectTbl->insert(select)){
		    throw compile::rewrite2::Exception("Failed to insert select" + select->toString());
		  }
		}
		else{
		  TuplePtr assign = createCurVerAssign(ruleId, verVarTuple->clone());
		  assign->set(catalog->attribute(ASSIGN, "POSITION"), Val_Int64::mk(newpos++));
		  assign->freeze();
		  if(!assignTbl->insert(assign)){
		    throw compile::rewrite2::Exception("Failed to insert assign" + assign->toString());
		  }

		}

		version->freeze();
		functorTbl->insert(version);

	      }
	    }
	    else{
	      //new functor
	      newpos++;
	    }
	  }
	  else {
	    //event: simply copy
	    if(newpos != i){
	      TuplePtr copyFunctor = termList[i]->clone();
	      copyFunctor->set(catalog->attribute(FUNCTOR, "POSITION"), Val_Int64::mk(newpos++));
	      copyFunctor->freeze();
	      if(!functorTbl->insert(copyFunctor)){
		throw compile::rewrite2::Exception("Failed to insert functor" + copyFunctor->toString());
	      }
	    }
	    else{
	      newpos++;
	    }
	  }
	}
	else{
	  throw compile::rewrite2::Exception("Invalid term type in rule" + termList[i]->toString());
	}
      }
      delete []termList;
      if(newpos > termCount){
	TuplePtr ruleCopy = rule->clone();
	CommonTablePtr ruleTbl = catalog->table(RULE);	
	ruleCopy->set(catalog->attribute(RULE, "TERM_COUNT"), Val_Int64::mk(newpos));
	ruleCopy->freeze();
	if(!ruleTbl->insert(ruleCopy)){
	  throw compile::rewrite2::Exception("Failed to insert rule" + ruleCopy->toString());
	}
	
      }

      //      rule2(catalog, rule);
    }

    TuplePtr Context::makeVersionedTuple(uint32_t &varSuffix, TuplePtr functor, TuplePtr location, CommonTable::ManagerPtr catalog){
      string name  = Val_Str::cast((*functor)[catalog->attribute(FUNCTOR, "NAME")]);

      TuplePtr functorTp = functor->clone();
      ListPtr attributes = Val_List::cast((*functor)[catalog->attribute(FUNCTOR, "ATTRIBUTES")]);
      attributes->pop_front();

      functorTp->set(catalog->attribute(FUNCTOR, "NAME"), Val_Str::mk(name + compile::VERSIONSUFFIX));
  
      //make version variable and push it
      ostringstream oss;
      oss << STAGEVARPREFIX << varSuffix++;
      TuplePtr verTuple = Tuple::mk(VAR);
      verTuple->append(Val_Str::mk(oss.str()));
      verTuple->freeze();
      attributes->prepend(Val_Tuple::mk(verTuple));

      // if location == NULL, create locationVariable and push it else push location variable
      location->freeze();
      attributes->prepend(Val_Tuple::mk(location));
      functorTp->set(catalog->attribute(FUNCTOR, "ATTRIBUTES"), Val_List::mk(attributes));

      return functorTp;

    }

    TuplePtr Context::createLocSpecTuple(ValuePtr ruleId, TuplePtr location, TuplePtr locSpec, TuplePtr refLocation, 
				TuplePtr ver, CommonTable::ManagerPtr catalog){
      TuplePtr     functorTp = Tuple::mk(FUNCTOR, true);
      functorTp->append(ruleId);
      functorTp->append(Val_Int64::mk(0)); // NOTIN?
      functorTp->append(Val_Str::mk(compile::LOCSPECTABLE));   // Functor name
  
      // Fill in table reference if functor is materialized
      CommonTable::Key nameKey;
      nameKey.push_back(catalog->attribute(FUNCTOR, "NAME"));
      CommonTablePtr tableTbl = catalog->table(TABLE);
      CommonTable::Iterator tIter = 
        tableTbl->lookup(nameKey, CommonTable::theKey(CommonTable::KEY3), functorTp);
      if (!tIter->done()) 
        functorTp->append((*tIter->next())[TUPLE_ID]);
      else {
	assert(0);
      }
  
      functorTp->append(Val_Null::mk());          // The ECA flag
      functorTp->append(Val_Null::mk());          // The attributes field
      functorTp->append(Val_Null::mk());          // The position field
      functorTp->append(Val_Null::mk());          // The access method
      functorTp->append(Val_Int64::mk(0)); // The new field

      // Now take care of the functor arguments and variable dependencies
      ListPtr attributes = List::mk();
      attributes->append(Val_Tuple::mk(location));
      attributes->append(Val_Tuple::mk(locSpec));
      attributes->append(Val_Tuple::mk(refLocation));
      attributes->append(Val_Tuple::mk(ver));
      functorTp->set(catalog->attribute(FUNCTOR, "ATTRIBUTES"), Val_List::mk(attributes));

      return functorTp;

    }


    TuplePtr Context::createLinkExpanderTuple(ValuePtr ruleId, TuplePtr location, TuplePtr locSpec, TuplePtr linkExpanderSet, CommonTable::ManagerPtr catalog){
      TuplePtr     functorTp = Tuple::mk(FUNCTOR, true);
      functorTp->append(ruleId);
      functorTp->append(Val_Int64::mk(0)); // NOTIN?
      functorTp->append(Val_Str::mk(compile::LINKEXPANDERTABLE));   // Functor name
  
      // Fill in table reference if functor is materialized
      CommonTable::Key nameKey;
      nameKey.push_back(catalog->attribute(FUNCTOR, "NAME"));
      CommonTablePtr tableTbl = catalog->table(TABLE);
      CommonTable::Iterator tIter = 
        tableTbl->lookup(nameKey, CommonTable::theKey(CommonTable::KEY3), functorTp);
      if (!tIter->done()) 
        functorTp->append((*tIter->next())[TUPLE_ID]);
      else {
	assert(0);
      }
  
      functorTp->append(Val_Null::mk());          // The ECA flag
      functorTp->append(Val_Null::mk());          // The attributes field
      functorTp->append(Val_Null::mk());          // The position field
      functorTp->append(Val_Null::mk());          // The access method
      functorTp->append(Val_Int64::mk(0)); // The new field

      // Now take care of the functor arguments and variable dependencies
      ListPtr attributes = List::mk();
      attributes->append(Val_Tuple::mk(location));
      attributes->append(Val_Tuple::mk(locSpec));
      attributes->append(Val_Tuple::mk(linkExpanderSet));
      functorTp->set(catalog->attribute(FUNCTOR, "ATTRIBUTES"), Val_List::mk(attributes));

      return functorTp;

    }

    TuplePtr Context::createLinkExpanderCheck(ValuePtr ruleId, TuplePtr linkExpanderSet, TuplePtr ver){
      ValuePtr gte = Val_Str::mk(">=");

      TuplePtr selectTp  = Tuple::mk(SELECT, true);
      selectTp->append(ruleId);   // Should be rule identifier
      
      TuplePtr boolExpr = Tuple::mk(BOOL);
      boolExpr->append(gte);
      boolExpr->append(Val_Tuple::mk(linkExpanderSet));

      TuplePtr certFn = Tuple::mk(FUNCTION);
      certFn->append(Val_Str::mk(CERTFN));
      certFn->append(Val_Int64::mk(1));
      certFn->append(Val_Tuple::mk(ver));
      certFn->freeze();

      boolExpr->append(Val_Tuple::mk(certFn));
      boolExpr->freeze();

      selectTp->append(Val_Tuple::mk(boolExpr));              // Boolean expression
      selectTp->append(Val_Null::mk());        // Pos
      selectTp->append(Val_Null::mk());        // Access method
      return selectTp;
    }

    
    TuplePtr Context::createCurVerAssign(ValuePtr ruleId, TuplePtr ver){
      TuplePtr val = Tuple::mk(VAL);
      val->append(Val_Int64::mk(CURVERSION));
      val->freeze();
      ver->freeze();

      TuplePtr       assignTp  = Tuple::mk(ASSIGN, true);
      assignTp->append(ruleId);
      assignTp->append(Val_Tuple::mk(ver));               // Assignment variable
      assignTp->append(Val_Tuple::mk(val));                  // Assignemnt value
      assignTp->append(Val_Null::mk());         // Position
      return assignTp;
    }

    TuplePtr Context::createCurVerSelect(ValuePtr ruleId, TuplePtr ver){
      ValuePtr eq = Val_Str::mk("==");
      TuplePtr val = Tuple::mk(VAL);
      val->append(Val_Int64::mk(CURVERSION));
      val->freeze();
      ver->freeze();

      TuplePtr selectTp  = Tuple::mk(SELECT, true);
      selectTp->append(ruleId);   // Should be rule identifier
      
      TuplePtr boolExpr = Tuple::mk(BOOL);
      boolExpr->append(eq);
      boolExpr->append(Val_Tuple::mk(ver));
      boolExpr->append(Val_Tuple::mk(val));
      boolExpr->freeze();

      selectTp->append(Val_Tuple::mk(boolExpr));              // Boolean expression
      selectTp->append(Val_Null::mk());        // Pos
      selectTp->append(Val_Null::mk());        // Access method
      return selectTp;
    }

 
    TuplePtr 
    Context::program(CommonTable::ManagerPtr catalog, TuplePtr program) 
    {

      TuplePtr newProgram = this->compile::Context::program(catalog, program);

      delete compile::Context::ruleLocSpecMap;
      return newProgram;

    }

  }
}
