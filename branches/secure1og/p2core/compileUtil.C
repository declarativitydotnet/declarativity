/*
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: Utility classes for compile process
 *
 */

#include "compileUtil.h"
#include "plumber.h"
#include "commonTable.h"
#include "oper.h"
#include "systemTable.h"
#include "val_tuple.h"
#include "val_list.h"
#include "val_uint32.h"
#include "val_null.h"

namespace compile {

  namespace namestracker {
    using namespace opr;

    void 
    exprString(ostringstream *oss, TuplePtr expr)
    {
      string type = (*expr)[TNAME]->toString();
      if (type == VAR || type == VAL) {
        ValuePtr val = (*expr)[2];
        if (type == VAL && val->typeCode() == Value::STR)
          *oss << "\"" << val->toString() << "\""; 
        else
          *oss << val->toString();
      }
      else if (type == LOC) {
        *oss << "@" << (*expr)[2]->toString(); 
      }
      else if (type == AGG) {
        *oss << (*expr)[3]->toString() << "< "  << (*expr)[2]->toString() << " >"; 
      }
      else if (type == BOOL) {
        if ((*expr)[4] != Val_Null::mk()) {
          // Binary boolean expression
          exprString(oss, Val_Tuple::cast((*expr)[3])); 
          *oss << " " << (*expr)[2]->toString() << " ";
          exprString(oss, Val_Tuple::cast((*expr)[4])); 
        }
        else {
          // Uniary boolean expression
          *oss << (*expr)[2]->toString() << " ";
          exprString(oss, Val_Tuple::cast((*expr)[3])); 
        } 
      }
      else if (type == MATH) {
        exprString(oss, Val_Tuple::cast((*expr)[3])); 
        *oss << " " << (*expr)[2]->toString() << " ";
        exprString(oss, Val_Tuple::cast((*expr)[4])); 
      }
      else if (type == FUNCTION) {
        unsigned args = Val_UInt32::cast((*expr)[3]);
        *oss << (*expr)[2]->toString() << "("; // Function name
        for (unsigned int i = 0; i < args; i++) {
          exprString(oss, Val_Tuple::cast((*expr)[4 + i])); 
          if (i + 1 != args) *oss << ", ";
        }
        *oss << ")";
      }
    }

    ValuePtr
    toVar(ValuePtr var)
    {
      TuplePtr tp = Val_Tuple::cast(var);
      TuplePtr variable = Tuple::mk(VAR);
      variable->append((*tp)[2]);
      variable->freeze();
      return Val_Tuple::mk(variable);
    }

    int
    aggregation(const ListPtr args)
    {
      int index = 0;
      ValPtrList::const_iterator i;
      for (i = args->begin(); i != args->end(); i++, index++) {
        TuplePtr arg = Val_Tuple::cast(*i);
        if ((*arg)[0]->toString() == AGG) {
          return index;
        }
      }
      return -1;
    }

    ValuePtr 
    location(const ListPtr schema)
    {
      for (ValPtrList::const_iterator i = schema->begin(); i != schema->end(); i++) {
        TuplePtr attribute = Val_Tuple::cast(*i);
        if ((*attribute)[0]->toString() == LOC) {
          return (*attribute)[2];  // return the variable
        }
      }
      return ValuePtr();
    }

    int
    position(const ListPtr args, const ValuePtr variable)
    {
      ValuePtr var;
      if ((*Val_Tuple::cast(variable))[0]->toString() == VAR ||
          (*Val_Tuple::cast(variable))[0]->toString() == LOC) {
        var = (*Val_Tuple::cast(variable))[2];
      }
      else if ((*Val_Tuple::cast(variable))[0]->toString() == AGG) {
        var = (*Val_Tuple::cast(variable))[2];
        if (var != Val_Null::mk()) 
          var = (*Val_Tuple::cast(var))[2];
      }
      else {
        var = variable;
      }

      int index = 0;
      ValPtrList::const_iterator i;
      for (i = args->begin(); i != args->end(); i++, index++) {
        TuplePtr arg = Val_Tuple::cast(*i);
        if ((*arg)[0]->toString() != AGG && 
            (*arg)[0]->toString() != VAR && 
            (*arg)[0]->toString() != LOC) {
          TELL_ERROR << "ARGS: " << args->toString() << " VARIABLE: " 
                    << variable->toString() << std::endl;
          throw compile::Exception("FieldNamesTracker: Fields must be variables!");
        }
        if ((*arg)[0]->toString() == AGG)
          arg = Val_Tuple::cast((*arg)[2]);  // Get the variable
        if ((*arg)[2] == var)
          return index;
      }
      return -1;
    }
    
    ListPtr 
    merge(const ListPtr outer, const ListPtr inner)
    {
      ListPtr join = List::mk();
      for (ValPtrList::const_iterator i = outer->begin(); 
           i != outer->end(); i++) {
        join->append(*i);
      }
      for (ValPtrList::const_iterator i = inner->begin(); 
           i != inner->end(); i++) {
        if (position(outer, *i) < 0) join->append(*i);
      }
      return join;
    }

    ListPtr 
    getMask(const ValuePtr v)
    {
      ListPtr mask = List::mk();
      calculateMaskRecur(mask, v);
      return mask;
    }

    void calculateMaskRecur(ListPtr &mask, const ValuePtr v)
    {
      TuplePtr t = Val_Tuple::cast(v);
      string type = (*t)[0]->toString();
      if(type == LOC || type == VAR)
      {
	mask->append((*t)[2]);
      }
      else if(type == AGG)
      {
	calculateMaskRecur(mask, (*t)[2]);
      }
      else if(type == BOOL)
      {
	calculateMaskRecur(mask, (*t)[3]);
	calculateMaskRecur(mask, (*t)[4]);
      }
      else if(type == RANGE)
      {
	calculateMaskRecur(mask, (*t)[3]);
	calculateMaskRecur(mask, (*t)[4]);
      }
      else if(type == MATH)
      {
	calculateMaskRecur(mask, (*t)[3]);
	calculateMaskRecur(mask, (*t)[4]);
      }
      else if(type == FUNCTION)
      {
	unsigned numArgs = Val_UInt32::cast((*t)[3]);
	for(unsigned i = 0; i < numArgs; i++)
	{
	  calculateMaskRecur(mask, (*t)[4 + i]);
	}
      }
      else if(type == VEC || type == MAT)
      {
	assert(0);
      }

    }

    ListPtr applyMask(ListPtr original, ListPtr mask, unsigned oldPos){
      ListPtr maskedList = List::mk();

      bool found = false;
      unsigned count = 0, countEnd = 0;
      for(ValPtrList::const_iterator outer = original->begin();
           outer != original->end(); outer++) {

	found = false;
	ValPtrList::const_iterator end = mask->end();
	count = 0;
	countEnd = (mask->size() - oldPos + 1u);
	for(ValPtrList::const_iterator iter = mask->begin();
	    iter != end, !found, count < countEnd ; iter++) {
	  ListPtr maskPart = Val_List::cast(*iter);
	  for (ValPtrList::const_iterator inner = maskPart->begin();
	       inner != maskPart->end(), !found; inner++) {
	    found |= (position(maskPart, *outer) >= 0);
	  }
	  
	}
	if (found)
	{
	  maskedList->append(*outer);
	}
      }
      
      return maskedList;
    }

    void
    joinKeys(const ListPtr outer, const ListPtr inner,
          CommonTable::Key& joinKey, CommonTable::Key& indexKey, CommonTable::Key& baseKey) 
    {
      unsigned innerPos = 1;	// Skip the table name, not refered to by arguments.
      for (ValPtrList::const_iterator iter = inner->begin();
           iter != inner->end(); iter++, innerPos++) {
        int pos = position(outer, *iter);
        if (pos < 0) {
          baseKey.push_back(innerPos);
        } 
        else {
          joinKey.push_back(pos+1);  // Positions are offset by 1 due to tablename.
          indexKey.push_back(innerPos);
        }
      }
    }
  } // END NAMESTRACKER

  namespace pel {
    string func(ListPtr schema, TuplePtr funcTp) 
    {
      ostringstream pel;

      CommonTablePtr functionTbl = Plumber::catalog()->table(FUNCTION);
      CommonTable::Key indexKey;
      indexKey.push_back(Plumber::catalog()->attribute(FUNCTION, "NAME"));
      indexKey.push_back(Plumber::catalog()->attribute(FUNCTION, "NUMARGS"));
      CommonTable::Iterator Iter = functionTbl->lookup(CommonTable::theKey(CommonTable::KEY23),
                                                       indexKey, funcTp);
      if (Iter->done()) {
        throw compile::Exception("Unknown function definition! " + funcTp->toString());
      }
      string name = (*Iter->next())[Plumber::catalog()->attribute(FUNCTION, "PEL")]->toString();
        
      unsigned args = Val_UInt32::cast((*funcTp)[3]);
      while (args > 0) {
         pel << gen(schema, Val_Tuple::cast((*funcTp)[3+args--])) << " ";
      } 
      pel << name << " ";

      return pel.str();
    }

    string value(ListPtr schema, TuplePtr valTp) {
      ValuePtr val = (*valTp)[2];
      
      return val->typeCode() == Value::STR 
               ?  ("\\\"" + val->toString() + "\\\" ") : (val->toString() + " ");
    }

    string variable(ListPtr schema, TuplePtr varTp) {
      ostringstream var;
      unsigned pos = namestracker::position(schema, Val_Tuple::mk(varTp));
      if (pos < 0 || pos >= schema->size()) {
        throw compile::Exception("Unknown variable " + varTp->toString() + 
                                 " in schema " + schema->toString() + ".");
      }
      var << "$" << (pos+1) << " ";
      return var.str();
    }
    
    
    string boolean(ListPtr schema, TuplePtr boolTp) 
    {
      ostringstream pel;  
    
      string oper = (*boolTp)[2]->toString();

      pel << gen(schema, Val_Tuple::cast((*boolTp)[3]));
      pel << gen(schema, Val_Tuple::cast((*boolTp)[4]));
      if (oper != "in") {
        pel << oper << " "; 
      }
      return pel.str();
    }

    string range(ListPtr schema, TuplePtr rangeTp) 
    {
      ostringstream pel;  

      pel << gen(schema, Val_Tuple::cast((*rangeTp)[3]));
      pel << gen(schema, Val_Tuple::cast((*rangeTp)[4]));
      pel << (*rangeTp)[2]->toString() << " ";
    
      return pel.str();
    }
    
    string select(ListPtr schema, TuplePtr selectTp)
    {
      ostringstream pel;
      pel << gen(schema, Val_Tuple::cast((*selectTp)[4])) 
          << "not ifstop swallow unboxPop ";
      
      return pel.str();
    }
    
    string assign(ListPtr schema, TuplePtr assignTp)
    {
      ostringstream pel;
      int varPosition = namestracker::position(schema, (*assignTp)[4]);
      if (varPosition < 0) {
        varPosition = schema->size();
        schema->append((*assignTp)[4]);      
      }

      for (unsigned k = 0; k < schema->size(); k++) {
        if (k == (unsigned) varPosition) 
          pel << gen(schema, Val_Tuple::cast((*assignTp)[5])) << "pop ";
        else pel << "$" << (k+1) << " pop ";
      }
      return pel.str();
    }

    string math(ListPtr schema, TuplePtr mathTp)
    {
      ostringstream  pel;  
      
      pel << gen(schema, Val_Tuple::cast((*mathTp)[3]));
      pel << gen(schema, Val_Tuple::cast((*mathTp)[4]));
      pel << (*mathTp)[2]->toString() << " ";

      return pel.str();
    }

    string gen(ListPtr schema, TuplePtr exprTp) {
      string type = (*exprTp)[0]->toString();

      if (type == FUNCTION) {
        return func(schema, exprTp); 
      }
      else if (type == VAL) {
        return value(schema, exprTp); 
      }
      else if (type == VAR) {
        return variable(schema, exprTp); 
      }
      else if (type == MATH) {
        return math(schema, exprTp); 
      }
      else if (type == BOOL) {
        return boolean(schema, exprTp); 
      }
      else if (type == RANGE) {
        return range(schema, exprTp); 
      }
      else if (type == SELECT) {
        return select(schema, exprTp); 
      }
      else if (type == ASSIGN) {
        return assign(schema, exprTp); 
      }
      else {
        throw compile::Exception("pel::gen: Unknown expresion type! " + type);
      }
    }
  } // END PEL

} // END COMPILE
