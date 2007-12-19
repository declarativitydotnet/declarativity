/*
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: All rewrites extend this abstract class. This class
 * provides some basic functionality common to a rewrite. Its primary
 * duty is to deal with the dataflow tuples that trigger the actual
 * rewrite. A rewrite tuple is a programEvent tuple containing the
 * attributes listed in the PROGRAM table. Upon receipt of a 
 * programEvent in simple_action, we call the program function which
 * will iterate over all rule tuples in the program, calling the
 * rule function for each one. It is assumed that the rewrite extending
 * this class will override the rule function in order to do its rewrite
 * of the rule. Nothing prohibits a rewrite from overriding the program
 * function as well. See the parse rewrite for such an example.
 */

#ifndef __COMPILE_CONTEXT_H__
#define __COMPILE_CONTEXT_H__

#include <deque>
#include <map>
#include "compileUtil.h"
#include "commonTable.h"
#include "element.h"
#include "value.h"
#include "tuple.h"
#include "table2.h"
#include "list.h"
#include "set.h"

namespace compile {
  const string VERSIONSUFFIX = "";
  const string LOCSPECTABLE = "locSpecTable";
  const string LINKEXPANDERTABLE = "linkExpanderTable";
  const string NEWSUFFIX = "New";
  const string STRONGSUFFIX = "Strong";
  const uint32_t NEWFIELDS = 3; //location and opaque and hint: number of fields from new before anything from original tuple (such as its name starts)
  const uint32_t OPAQUEPOS = 2;  // in new tuples  

  const uint32_t HINTPOS = 3;  // in new tuples  

  /*FUNNY STUFF according to rewrite0*/
  const uint32_t LOCSPECPOS = NEWFIELDS + 1; // pos of loc spec field in new tuples

  const uint32_t LOCSPECLOCSPECPOS = 2; // pos of loc spec field in locspec tuples
  const uint32_t VERLOCSPECPOS = LOCSPECLOCSPECPOS + 1; // pos of loc version field in locspec tuples
  const uint32_t CERTLOCSPECPOS = VERLOCSPECPOS + 1; // pos of loc version field in locspec tuples
  const uint32_t VERCERTLOCSPECPOS = CERTLOCSPECPOS + 1; // pos of loc version field in locspec tuples

  // position of version field in version tuple: 1 is the 1st pos
  const uint32_t VERPOS = 2; // pos of version field in version tuple: 1 is the 1st pos
  const uint32_t VERDATAPOS = VERPOS + 1; // pos of data fields in version tuple 
  const uint32_t VERCERTPOS = VERPOS + 1; // pos of cert field in strong version tuples
  const uint32_t STRONGVERDATAPOS = VERCERTPOS + 1; // pos of data fields in strong versin tuples

  const uint32_t STRONGPOS = 1; // pos of the boolean in hint list indicating whether this new tuple needs should be converted to a strong version or a weak one
  const uint32_t VERSIONIFYPOS = STRONGPOS + 1; // pos of the boolean in hint list indicating whether this new graph needs to be converted into proper version form or not: not if copied, yes otherwise
  const uint32_t OPERATIONTYPEPOS = VERSIONIFYPOS + 1; // pos of the field in hint list indicating if the says has to create a new proof or should use the proof present in the next field
  const bool MAKESAYS = 0;
  const bool SAYSUSINGPROOF = 1;
  const bool CREATEVERSION = 2;

  const uint32_t PROOFPOS = OPERATIONTYPEPOS + 1; // pos of the field in hint list containing the proof/key field for this new tuple

  class LocSpecInfo{
  public:
    ValuePtr location;
    SetPtr locSpecSet;
    SetPtr strongLocSpecSet;
    LocSpecInfo(ValuePtr l, SetPtr s, SetPtr strongSet){
      location = l;
      locSpecSet = s;
      strongLocSpecSet = strongSet;
    }

    LocSpecInfo(const LocSpecInfo& l ){
      location = l.location;
      locSpecSet = l.locSpecSet;
      strongLocSpecSet = l.strongLocSpecSet;
    }
    
    ~LocSpecInfo(){}

  };

  struct ltLocSpecMap
  {
    bool operator()(const ValuePtr s1, const ValuePtr s2) const
      {
	return s1->compareTo(s2) < 0;
      }
  };
  
  typedef std::map<ValuePtr, LocSpecInfo*, ltLocSpecMap> LocSpecMap;

  class Context : public Element {
  public:
    static LocSpecMap* ruleLocSpecMap;
    
    static SetPtr materializedTables;
  
    Context(string name); 

    virtual ~Context() {};

    virtual const char *class_name() const = 0;
    const char *processing() const { return "a/a"; }
    const char *flow_code()  const { return "x/x"; }

    TuplePtr simple_action(TuplePtr p);

  protected:
    virtual int initialize();

    /** Performs rewrite stage on the given program. */ 
    virtual TuplePtr program(CommonTable::ManagerPtr catalog, TuplePtr program);
    /** Performs rewrite stage on the given rule. */
    virtual void rule(CommonTable::ManagerPtr catalog, TuplePtr rule);

    /** Stops program compilation and inserts an error into the source node's error table. */
    void error(CommonTable::ManagerPtr catalog, TuplePtr program, int code, string desc); 
  };
};

#endif /* __COMPILE_CONTEXT_H__ */
