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

#ifndef __REWRITE1_CONTEXT_H__
#define __REWRITE1_CONTEXT_H__

#include <iostream>
#include <set>
#include "element.h"
#include "elementRegistry.h"
#include "compileContext.h"
#include "val_str.h"
#include "val_int64.h"
#include "set.h"

namespace compile {
  namespace rewrite1 {
    const string STAGEVARPREFIX = "RW1_";
    const string STAGERULEPREFIX = "rw1_";
    const string CREATELOCSPECFN = "f_createLocSpec";
    const string CREATEVERFN = "f_createVersion";
    const string ISSAYSFN = "f_isSays";
    const uint32_t ISSAYSFNARGS = 1; // tuple name and uniqueId, return serialized buffer
    const string SERIALIZEFN = "f_serialize";
    const uint32_t SERIALIZEFNARGS = 2; // tuple name and uniqueId, return serialized buffer
    const string DESERIALIZEFN = "f_deserialize";
    const uint32_t DESERIALIZEFNARGS = 2; // tuple name and serialized buffer; returns dummy useless variable
    const string TIMESTAMPFN = "f_now";
    const string ISLOCSPECFN = "f_isLocSpec";
    const string SENDTUPLE = "sendtuple";
    const string ROOTVERSUFFIX = "Ver";
    const string ROOTPROCESSSUFFIX = "Process";
    const string HEADSAYSSUFFIX = "NewSays";
    const string DUMMY = "dummy";
    const uint32_t DUMMYARGS = 1;
    const uint32_t LOCSPECFNARGS = 0;
    const uint32_t ISLOCSPECFNARGS = 1;
    const uint32_t UNIQUEIDPOS = 2 ; // in TnewV tuples
    extern uint32_t ruleCounter;
    extern bool needRecvTuple;

    class Exception : public compile::Exception {
    public:
      Exception(string msg) : compile::Exception(msg) {};
    };

    class NewHeadState{
    public:
      string newRuleHead;
      string newRuleBase;
      std::set<uint32_t> posSet; // set of new location specifiers positions
      uint32_t numVars;
      bool newRule;
      bool finalize;
      NewHeadState(string _newRuleBase, uint32_t  _numVars, bool _newRule){
	numVars = _numVars;
	newRuleBase = _newRuleBase;
	finalize = false;
	newRule = _newRule;
      }
      
      void addPos(uint32_t pos){
	if(finalize)throw compile::rewrite1::Exception("Can't insert positions in a finalized NewHeadState object");
	posSet.insert(pos);
      }
      
      void freeze(){
	assert(!finalize);
	finalize = true;
	std::set<uint32_t>::iterator posIter;
	ostringstream oss1;
	if(posSet.size() > 0){
	  for( posIter = posSet.begin(); posIter != posSet.end(); posIter++ ) {
	    oss1 << "_" <<(*posIter); 
	  }  
	  newRuleHead = newRuleBase + oss1.str();
	}
	else{
	  newRuleHead = newRuleBase;
	}
      }
    };

    struct ltNewHeadState
    {
      bool operator()(const NewHeadState *s1, const NewHeadState *s2) const
      {
	assert(s1->finalize && s2->finalize);
	return s1->newRuleHead < s2->newRuleHead;
      }
    };

    typedef std::set<NewHeadState*, ltNewHeadState> HeadStateSet;

    class Context : public compile::Context {
    public:
      Context(string name); 
      Context(TuplePtr args); 
  
      virtual ~Context() {
	HeadStateSet::iterator iter = headState.begin();
	for(; iter != headState.end(); iter++){
	  delete (*iter);
	}
      }; 
  
      const char *class_name() const { return "rewrite1::Context";}
  
      DECLARE_PUBLIC_ELEMENT_INITS

    private:
      HeadStateSet headState;
      SetPtr materializedTable;
      SetPtr materializedSaysTable;
      /* Process the current rule in the program */
      void rule(CommonTable::ManagerPtr catalog, TuplePtr rule);

      TuplePtr createLocSpec(ValuePtr ruleId, TuplePtr locSpec, uint32_t pos);

      TuplePtr program(CommonTable::ManagerPtr catalog, TuplePtr program);
      
      void pass2(CommonTable::ManagerPtr catalog, TuplePtr program);      

      void pass3(CommonTable::ManagerPtr catalog, TuplePtr program);

      TuplePtr materializeNewTupleVersionEvent(uint32_t &fictVar, CommonTable::ManagerPtr, ValuePtr programID, NewHeadState *state);
      TuplePtr materializeLocSpecTuple(uint32_t &fictVar, CommonTable::ManagerPtr, ValuePtr programID, TuplePtr newTuple, NewHeadState *state);
      TuplePtr materializeNewTupleVersion(uint32_t &fictVar, CommonTable::ManagerPtr, ValuePtr programID, TuplePtr newTuple, NewHeadState *state, bool says);
      TuplePtr materializeProcessTuple(uint32_t &fictVar, CommonTable::ManagerPtr, ValuePtr programID, NewHeadState *state);
      TuplePtr materializeDeleteProcessTuple(uint32_t &fictVar, CommonTable::ManagerPtr, ValuePtr programID, TuplePtr newTuple, NewHeadState *state);
      TuplePtr materializeSendTuple(uint32_t &fictVar, CommonTable::ManagerPtr, ValuePtr programID, TuplePtr newTuple, NewHeadState *state);
      TuplePtr materializeRecvTuple(CommonTable::ManagerPtr, ValuePtr programID);
	
      TuplePtr generateFunctor(CommonTable::ManagerPtr catalog, uint32_t &, string , ValuePtr , uint32_t);

      DECLARE_PRIVATE_ELEMENT_INITS
    };
  }
}

#endif /* __REWRITE1_CONTEXT_H__ */
