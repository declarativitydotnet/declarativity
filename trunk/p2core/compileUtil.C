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
#include "list.h"
#include "systemTable.h"
#include "val_tuple.h"
#include "val_list.h"
#include "val_str.h"
#include "val_int64.h"
#include "val_null.h"

namespace compile {

  namespace namestracker {
    using namespace opr;

    ListPtr 
    getMask(const ValuePtr v)
    {
      ListPtr mask = List::mk();
      //      std::cout<<"Mask for " << v->toString()<<std::endl;
      calculateMaskRecur(mask, v);
      //      std::cout<<" is " << mask->toString()<<std::endl;
      return mask;
    }

    void calculateMaskRecur(ListPtr &mask, const ValuePtr v)
    {
      TuplePtr t = Val_Tuple::cast(v);
      string type = (*t)[0]->toString();
      if(type == LOC || type == VAR)
      {
	mask->append(Val_Tuple::mk(t));
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
	unsigned numArgs = Val_Int64::cast((*t)[3]);
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
	count = 0;
	countEnd = (mask->size() - oldPos + 1u);
	for(ValPtrList::const_iterator iter = mask->begin();
	    iter != mask->end() && !found && count < countEnd ; iter++, count++) {
	  ListPtr maskPart = Val_List::cast(*iter);
	  found |= (position(maskPart, *outer) >= 0);
	}
	if (found)
	{
	  maskedList->append(*outer);
	}
      }
      
      return maskedList;
    }


    string 
    exprString(TuplePtr expr)
    {
      ostringstream oss;
      string type = (*expr)[TNAME]->toString();
      if (type == VAR || type == VAL || type == MAT || type == VEC) {
        ValuePtr val = (*expr)[2];
        if (type == VAL && val->typeCode() == Value::STR) {
          oss << "\"" << val->toString() << "\""; 
        }
        else oss << val->toString();
      }
      else if (type == LOC) {
        oss << "@" << (*expr)[2]->toString(); 
      }
      else if (type == NEWLOCSPEC) {
        oss << "&" << (*expr)[2]->toString(); 
      }
      else if (type == AGG) {
        oss << (*expr)[3]->toString() << "< "  
             << ((*expr)[2] == Val_Null::mk() ? "*" : exprString(Val_Tuple::cast((*expr)[2])))
             << " >"; 
      }
      else if (type == BOOL) {
        if ((*expr)[4] != Val_Null::mk()) {
          // Binary boolean expression
          oss << exprString(Val_Tuple::cast((*expr)[3])); 
          oss << " " << (*expr)[2]->toString() << " ";
          oss << exprString(Val_Tuple::cast((*expr)[4])); 
        }
        else {
          // Uniary boolean expression
          oss << (*expr)[2]->toString() << " ";
          oss << exprString(Val_Tuple::cast((*expr)[3])); 
        } 
      }
      else if (type == MATH) {
        oss << exprString(Val_Tuple::cast((*expr)[3])); 
        oss << " " << (*expr)[2]->toString() << " ";
        oss << exprString(Val_Tuple::cast((*expr)[4])); 
      }
      else if (type == FUNCTION) {
        unsigned args = Val_Int64::cast((*expr)[3]);
        oss << (*expr)[2]->toString() << "("; // Function name
        for (unsigned int i = 0; i < args; i++) {
          oss << exprString(Val_Tuple::cast((*expr)[4 + i])); 
          if (i + 1 != args) oss << ", ";
        }
        oss << ")";
      }
      return oss.str();
    }

    bool
    isTheta(ValuePtr boolv)
    {
      TuplePtr tp = Val_Tuple::cast(boolv);
      if ((*tp)[TNAME]->toString() == BOOL) {
        ValuePtr lhs = (*tp)[3];
        ValuePtr rhs = (*tp)[4];

        if (rhs != Val_Null::mk()) {
          if ((*tp)[2]->toString() == "==") {
            return false;
          } 
          else if ((*Val_Tuple::cast(lhs))[0]->toString() == VAR &&
                   (*Val_Tuple::cast(rhs))[0]->toString() == VAL) {
            return true;
          }
          else if ((*Val_Tuple::cast(rhs))[0]->toString() == VAR &&
                   (*Val_Tuple::cast(lhs))[0]->toString() == VAL) {
            return true;
          }
        }
      }
      return false;
    }

    ValuePtr
    uniqueSchema(ValuePtr value)
    {
      ListPtr schema = List::mk();
      if (value->typeCode() == Value::LIST) {
        ListPtr list = Val_List::cast(value);
        int fictvar = 0;
        for (ValPtrList::const_iterator iter = list->begin();
             iter != list->end(); iter++) {
          ostringstream oss;
          oss << "$_" << fictvar++;
          if ((*Val_Tuple::cast(*iter))[0]->toString() == LOC) {
            TuplePtr var = Tuple::mk(LOC);
            var->append(Val_Str::mk(oss.str()));
            var->freeze();
            schema->append(Val_Tuple::mk(var));
          }
          else {
            TuplePtr var = Tuple::mk(VAR);
            var->append(Val_Str::mk(oss.str()));
            var->freeze();
            schema->append(Val_Tuple::mk(var));
          }
        }
      }
      return Val_List::mk(schema);
    }

    ValuePtr
    toVar(ValuePtr value)
    {
      if (value->typeCode() == Value::LIST) {
        ListPtr list = Val_List::cast(value);
        ListPtr varList = List::mk();
        for (ValPtrList::const_iterator iter = list->begin();
             iter != list->end(); iter++) {
          if ((*Val_Tuple::cast(*iter))[0]->toString() == LOC) {
            varList->append(*iter);
          }
          else {
            ValuePtr var = toVar(*iter);
            if (var) {
              varList->append(var);
            }
          }
        }
        return Val_List::mk(varList);
      }
      else {
        static int fictVar = 0;
        if (value == Val_Null::mk() ||
            (*Val_Tuple::cast(value))[0]->toString() == VAL) {
            ostringstream oss;
            oss << "$_" << fictVar++;
            TuplePtr var = Tuple::mk(VAR);
            var->append(Val_Str::mk(oss.str()));
            var->freeze();
            return Val_Tuple::mk(var);
        }
        else if ((*Val_Tuple::cast(value))[0]->toString() == VAR) {
            return value;
        } 
        else if ((*Val_Tuple::cast(value))[0]->toString() == LOC) {
            TuplePtr tp = Val_Tuple::cast(value);
            TuplePtr var = Tuple::mk(VAR);
            var->append((*tp)[2]);
            var->freeze();
            return Val_Tuple::mk(var);
        }
        else if ((*Val_Tuple::cast(value))[0]->toString() == AGG) {
          return toVar((*Val_Tuple::cast(value))[2]);
        }
      }
      return ValuePtr();
    }

    ListPtr flatten(const ListPtr args)
    {
      ListPtr schema = List::mk();
      for (ValPtrList::const_iterator i = args->begin(); 
           i != args->end(); i++) {
        TuplePtr arg = Val_Tuple::cast(*i);
        if ((*arg)[TNAME]->toString() == LOC) {
          schema->append(*i);
        }
        else if ((*arg)[TNAME]->toString() != AGG) {
          schema->append(toVar(*i));
        }
        else if ((*arg)[2] != Val_Null::mk()) {
          schema->append(toVar(*i));
        }
      }
      return schema;

    }

    ListPtr groupby(const ListPtr args)
    {
      ListPtr gb = List::mk();
      for (ValPtrList::const_iterator i = args->begin(); 
           i != args->end(); i++) {
        TuplePtr arg = Val_Tuple::cast(*i);
        if ((*arg)[TNAME]->toString() != AGG) {
          gb->append(toVar(*i));
        }
      }
      return gb;
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
        else if ((*attribute)[0]->toString() == AGG &&
                 (*attribute)[2] != Val_Null::mk()) {
          TuplePtr tp = Val_Tuple::cast((*attribute)[2]);
          if ((*tp)[0]->toString() == LOC) {
            return (*attribute)[2];
          }
        }
      }
      return ValuePtr();
    }

    int
    subset(const ListPtr schema1, const ListPtr schema2) 
    {
      for (ValPtrList::const_iterator i = schema1->begin(); 
           i != schema1->end(); i++) {
        TuplePtr tp = Val_Tuple::cast(*i);
        if ((*tp)[0]->toString() == VAL) {
          continue;
        }
        else if (position(schema2, toVar(*i)) < 0) {
          return false;
        }
      }
      return true;
    }

    int
    position(const ListPtr args, const ValuePtr variable)
    {
      ValuePtr var = variable;
      if (variable == Val_Null::mk()) {
        return -1;
      }
      else if (variable->typeCode() == Value::TUPLE) {
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
      }

      int index = 0;
      ValPtrList::const_iterator i;
      for (i = args->begin(); i != args->end(); i++, index++) {
        TuplePtr arg = Val_Tuple::cast(*i);
        if ((*arg)[0]->toString() != AGG && 
            (*arg)[0]->toString() != VAR && 
            (*arg)[0]->toString() != LOC) {
          continue;
        }
        if ((*arg)[0]->toString() == AGG)
          arg = Val_Tuple::cast((*arg)[2]);  // Get the variable
        if (arg != NULL && (*arg)[2] == var)
          return index;
      }
      return -1;
    }

    bool
    filter(const ListPtr schema, const ValuePtr boolv) 
    {
      ListPtr vars = variables(boolv);
      for (ValPtrList::const_iterator iter = vars->begin();
           iter != vars->end(); iter++) {
        if (position(schema, toVar(*iter)) < 0) {
          return false;
        }
      }
      return true;
    }

    ListPtr 
    variables(const ValuePtr value)
    {
      ListPtr vars  = List::mk();
      if (value == Val_Null::mk()) {
        return vars;
      }

      TuplePtr expr = Val_Tuple::cast(value);
      if ((*expr)[TNAME]->toString() == BOOL ||
          (*expr)[TNAME]->toString() == MATH) {
        ValuePtr lhs = (*expr)[3];
        ValuePtr rhs = (*expr)[4];
        vars->append(variables(lhs));
        vars->append(variables(rhs));
      }
      else if ((*expr)[TNAME]->toString() == VAR) {
        vars->append(value);
      }
      else if ((*expr)[TNAME]->toString() == LOC) {
        vars->append(value);
      }
      else if ((*expr)[TNAME]->toString() == AGG) {
        vars->append(variables((*expr)[2]));
      }
      else if ((*expr)[TNAME]->toString() == FUNCTION) {
        int args = Val_Int64::cast((*expr)[3]);
        while (args > 0) {
          vars->append(variables((*expr)[3+args--]));
        }
      }
      return vars;
    }

    ValuePtr 
    sortAttr(const ListPtr outer, const ValuePtr outerOrder,
             const ListPtr inner, const ValuePtr innerOrder)
    {
      if (outerOrder->size() > 0 && outerOrder == innerOrder) {
        return outerOrder;
      }

      ListPtr join = List::mk();
      for (ValPtrList::const_iterator i = inner->begin(); 
           i != inner->end(); i++) {
        if (position(outer, *i) >= 0) join->append(*i);
      }


      if (outerOrder->size() > 0 && position(join, outerOrder) > 0) {
        return outerOrder;
      }
      else if (innerOrder->size() > 0 && position(join, innerOrder) > 0) {
        return innerOrder;
      }
      else {
        for (ValPtrList::const_iterator i = join->begin(); 
             i != join->end(); i++) {
          if ((*Val_Tuple::cast(*i))[TNAME]->toString() == VAR) {
            return *i;
          }
        }
      }

      return Val_Null::mk();
    }

    bool equivalent(const ListPtr plan1, const ListPtr plan2)
    {
      for (ValPtrList::const_iterator i1 = plan1->begin(); 
           i1 != plan1->end(); i1++) {
        bool found = false;
        for (ValPtrList::const_iterator i2 = plan2->begin(); 
             !found && i2 != plan2->end(); i2++) {
          if (*i1 == *i2) found = true;
        }
        if (!found) return false;
      }
      return true;
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
        if (position(outer, *i) < 0 && (*Val_Tuple::cast(*i))[TNAME]->toString() != VAL) 
          join->append(*i);
      }
      return join;
    }

    bool 
    prefix(const ListPtr prefix, const ListPtr schema)
    {
      int pos = 0;
      for (ValPtrList::const_iterator i = prefix->begin(); 
           i != prefix->end(); i++) {
        int attrPos = position(schema, *i);
        if (attrPos < pos) return false;
        pos = attrPos;
      }
      return true;
    }

    ListPtr 
    project(const ListPtr positions, const ListPtr schema)
    {
      ListPtr projection = List::mk();
      for (ValPtrList::const_iterator iter = positions->begin();
           iter != positions->end(); iter++) {
        uint pos = Val_Int64::cast(*iter) - 1;
        if (pos < schema->size()) {
          projection->append(schema->at(pos));
        }
        else {
          TELL_ERROR << "NAMESTRACKER PROJECT: "
                     << positions->toString() << " on "
                     << schema->toString() << std::endl;
          return schema;
        }
      }
      return projection;
    }

    ListPtr 
    adornment(const ListPtr bound, const ListPtr schema)
    {
      ListPtr adorn = List::mk();
   
      int pos = 1; // Schema variables start at position 1.
      for (ValPtrList::const_iterator iter = schema->begin();
           iter != schema->end(); iter++, pos++) {
        TuplePtr arg = Val_Tuple::cast(*iter);
        if ((*arg)[TNAME]->toString() == VAR &&
            compile::namestracker::position(bound, *iter) >= 0) {
          adorn->append(Val_Int64::mk(pos));
        }
        else if ((*arg)[TNAME]->toString() != VAR &&
                 (*arg)[TNAME]->toString() != LOC) {
          adorn->append(Val_Int64::mk(pos));
        }
      }
      return adorn;
    }

    ListPtr 
    assignSchema(const ListPtr outer, const ValuePtr var)
    {
      ListPtr schema = List::mk();
      for (ValPtrList::const_iterator i = outer->begin(); 
           i != outer->end(); i++) {
        schema->append(*i);
      }
      if (position(outer, var) < 0) {
        TuplePtr tp = Val_Tuple::cast(var);
        schema->append(Val_Tuple::mk(tp->clone()));
      }
      return schema;
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

    ValuePtr
    castassign(const ListPtr outer, const ListPtr inner, const TuplePtr boolv)
    {
      if ((*boolv)[TNAME]->toString() == BOOL && (*boolv)[4] != Val_Null::mk()) {
        ValuePtr lhs = (*boolv)[3];
        ValuePtr rhs = (*boolv)[4];
        if (position(outer, lhs) >= 0 && 
            position(inner, rhs) >= 0) {
          ListPtr assign = List::mk();
          assign->append(rhs);
          assign->append(lhs);
          return Val_List::mk(assign);
        }
        else if (position(outer, rhs) >= 0 && 
                 position(inner, lhs) >= 0) {
          ListPtr assign = List::mk();
          assign->append(lhs);
          assign->append(rhs);
          return Val_List::mk(assign);
        }
      }
      return Val_Null::mk();
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
        
      unsigned args = Val_Int64::cast((*funcTp)[3]);
      while (args > 0) {
         pel << gen(schema, Val_Tuple::cast((*funcTp)[3+args--])) << " ";
      } 
      pel << name << " ";

      return pel.str();
    }

    string value(ListPtr schema, TuplePtr valTp) {
      ValuePtr val = (*valTp)[2];
      return (val->typeCode() == Value::STR) ? 
                "\\\"" + val->toString() + "\\\" " :
                val->toConfString() + " ";
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
      if (oper == "in") {
        TuplePtr tp1 = Val_Tuple::cast((*boolTp)[3]);
        TuplePtr tp2 = Val_Tuple::cast((*boolTp)[4]);
        if (((*tp1)[0]->toString() == VAR || (*tp1)[0]->toString() == VAL) && 
            (*tp2)[0]->toString() == VAR) {
          TuplePtr fn = Tuple::mk(FUNCTION);
          fn->append(Val_Str::mk("f_contains"));
          fn->append(Val_Int64::mk(2));
          fn->append((*boolTp)[3]);
          fn->append((*boolTp)[4]);
          fn->freeze();
          return func(schema, fn);
        }
      }

      pel << gen(schema, Val_Tuple::cast((*boolTp)[3]));
      if ((*boolTp)[4] != Val_Null::mk()) {
	// Binary boolean expression
	pel << gen(schema, Val_Tuple::cast((*boolTp)[4]));
      }
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
      else if (type == MAT) {
        return value(schema, exprTp); 
      }
      else if (type == VEC) {
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
        throw compile::Exception("pel::gen: Unknown expresion type! " + type + " expr = " + ((*exprTp)[2])->toString() );
      }
    }
  } // END PEL

} // END COMPILE
