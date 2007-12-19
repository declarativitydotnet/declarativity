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

#include "util.h"
#include "oper.h"
#include "compileContext.h"
#include "systemTable.h"
#include "val_tuple.h"
#include "val_list.h"
#include "val_uint32.h"

namespace compile {

  namespace namestracker {
    using namespace opr;

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

    int
    position(const ListPtr args, const ValuePtr variable)
    {
      ValuePtr var = (*Val_Tuple::cast(variable))[2];
      if ((*Val_Tuple::cast(variable))[0]->toString() == AGG)
        var = (*Val_Tuple::cast(var))[2];
      int index = 0;
      ValPtrList::const_iterator i;
      for (i = args->begin(); i != args->end(); i++, index++) {
        TuplePtr arg = Val_Tuple::cast(*i);
        if ((*arg)[0]->toString() != AGG && (*arg)[0]->toString() != VAR) {
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

    void
    joinKeys(const ListPtr outer, const ListPtr inner,
          CommonTable::Key& joinKey, CommonTable::Key& indexKey, CommonTable::Key& baseKey) 
    {
      unsigned innerPos = 1;	// Skip the table name, not refered to by arguments
      for (ValPtrList::const_iterator iter = inner->begin();
           iter != inner->end(); iter++, innerPos++) {
        int pos = position(outer, *iter);
        if (pos < 0) {
          baseKey.push_back(pos);
        } 
        else {
          joinKey.push_back(pos);
          indexKey.push_back(innerPos);
        }
      }
    }
  } // END NAMESTRACKER

  namespace pel {
    string func(ListPtr schema, TuplePtr funcTp) 
    {
      ostringstream pel;

      unsigned args = Val_UInt32::cast((*funcTp)[3]);
      while (args > 0) {
         pel << gen(schema, Val_Tuple::cast((*funcTp)[4+args--])) << " ";
      } 
      pel << (*funcTp)[2]->toString() << " ";

      return pel.str();
    }

    string sets(ListPtr schema, TuplePtr funcTp) 
    {
      ostringstream pel;

      unsigned args = Val_UInt32::cast((*funcTp)[3]);
      while (args > 0) {
         pel << gen(schema, Val_Tuple::cast((*funcTp)[4+args--])) << " ";
      } 
      pel << (*funcTp)[2]->toString() << " ";

      return pel.str();
    }

    string value(ListPtr schema, TuplePtr valTp) {
      return (*valTp)[2]->toString() + " ";
    }

    string variable(ListPtr schema, TuplePtr varTp) {
      ostringstream var;
      var << "$" << namestracker::position(schema, (*varTp)[2]) << " ";
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
          << "not ifstop popall ";
      
      return pel.str();
    }
    
    string assign(ListPtr schema, TuplePtr assignTp)
    {
      ostringstream pel;
      int varPosition = namestracker::position(schema, (*assignTp)[4]);
      if (varPosition < 0)
        throw compile::Exception("Assignment variable " + 
                                 (*assignTp)[4]->toString() +
                                 " does not exist in tuple schema: " +
                                 schema->toString() + "!");
      for (unsigned k = 0; k < schema->size(); k++) {
        if (k == (unsigned) varPosition) 
          pel << gen(schema, Val_Tuple::cast((*assignTp)[5])) << "pop ";
        else pel << "$" << k << " pop ";
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
      string type = (*exprTp)[1]->toString();

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
      else if (type == SET) {
        return sets(schema, exprTp); 
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
