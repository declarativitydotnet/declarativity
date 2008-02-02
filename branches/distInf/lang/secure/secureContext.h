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

#ifndef __SECURE_CONTEXT_H__
#define __SECURE_CONTEXT_H__

#include <iostream>
#include "element.h"
#include "elementRegistry.h"
#include "compileContext.h"
#include "val_str.h"

namespace compile {
  namespace secure {
    const string MAX = "f_max";
    const string CONCAT = "f_concat";
    const string MOD = "f_mod";
    const uint32_t MODARGS = 1;
    const string VERTABLE = "verKey"; 
    const string GENTABLE = "genKey"; 
    const string HASHFUNC = "f_sha1"; 
    const uint32_t HASHFUNCARGS = 1; 
    const string VERFUNC = "f_verify"; 
    const uint32_t VERFUNCARGS = 6; 
    const string GENFUNC = "f_gen"; 
    const uint32_t GENFUNCARGS = 3; 
    const string ENCHINT = "encHint"; 
    const string CONSLIST = "f_cons"; 
    const uint32_t CONSLISTARGS = 2; 
    const string APPENDFUNC = "f_append"; 
    const uint32_t APPENDFUNCARGS = 2; 
    const string STAGEVARPREFIX = "SV_";
    const string STAGERULEPREFIX = "sr_"; 
    const string SAYSSUFFIX = "Says"; 
    const string MAKESAYS = "makeSays"; 
    const uint32_t NUMSECUREFIELDS = 4;
    const uint32_t PROOFPOS = 6; // pos of proof field in the says tuple
    class Exception : public compile::Exception {
    public:
      Exception(string msg) : compile::Exception(msg) {};
    };

    typedef std::deque<TuplePtr> TupleList;

    class VerificationTuple{
    public:
      string name;
      uint32_t numVars;
      VerificationTuple(string _name, uint32_t  _numVars){
	numVars = _numVars;
	name = _name;
      }
    };

    struct ltVerificationTuple
    {
      bool operator()(const VerificationTuple *s1, const VerificationTuple *s2) const
      {
	return s1->name < s2->name;
      }
    };

    typedef std::set<VerificationTuple*, ltVerificationTuple> VerificationTupleSet;


    class Context : public compile::Context {
    public:
      static uint32_t ruleCounter;
      VerificationTupleSet verificationTables;
      Context(string name); 
      Context(TuplePtr args); 
  
      virtual ~Context() { }; 
  
      const char *class_name() const { return "secure::Context";}

  
      DECLARE_PUBLIC_ELEMENT_INITS

    private:
      /* Process the current rule in the program */
      void rule(CommonTable::ManagerPtr catalog, TuplePtr rule);

      TuplePtr program(CommonTable::ManagerPtr catalog, TuplePtr rule);

      /**
       * modifies the rule which has the head :- table(), says<> into non says rhs
       */
      void normalizeVerify(CommonTable::ManagerPtr catalog, TuplePtr &functor, TuplePtr says, uint32_t &newVar);

      // return a list of terms that needs to be added to the rule on converting the 
      // securelog term f into overlog.
      // Also converts f into the appropriate overlog form
      TupleList* normalizeGenerate(CommonTable::ManagerPtr catalog, TuplePtr &head, TuplePtr &rule, 
				   TuplePtr loc, uint32_t& newVariable, uint32_t& pos, bool _new, 
				   TuplePtr keyProofVar, TuplePtr headSaysType, TuplePtr keyTypeVar,
				   TupleList* listAssign = NULL);

      TupleList* generateAlgebraLT(ValPtrList::const_iterator start1, 
					   ValPtrList::const_iterator start2, 
					   uint32_t &pos,
					   ValuePtr ruleId,
					   TupleList *newTerms = NULL);

      TupleList* generateAssignTerms(ValPtrList::const_iterator start1, 
				     ValPtrList::const_iterator start2, 
				     uint32_t count, 
				     uint32_t &pos, 
				     ValuePtr ruleId, 
				     TupleList *t = NULL);
      
      TupleList* generateSelectTerms(ValPtrList::const_iterator start1, 
				     ValPtrList::const_iterator start2, 
				     uint32_t count, 
				     ValuePtr o, 
				     uint32_t &pos, 
				     ValuePtr ruleId, 
				     TupleList *t = NULL);
      
      TuplePtr generateFunctor(CommonTable::ManagerPtr catalog, uint32_t &, string , ValuePtr , uint32_t);


      void generateVerificationRules(CommonTable::ManagerPtr catalog, TuplePtr program);			   

      void generateMaterialize(CommonTable::ManagerPtr catalog);

      void initLocSpecMap(CommonTable::ManagerPtr catalog, TuplePtr rule);

      DECLARE_PRIVATE_ELEMENT_INITS
    };
  }
}

#endif /* __SECURE_CONTEXT_H__ */
