/*
 * @(#)$Id: pel_vm.C 1416 2007-10-02 06:35:28Z prince $
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: PEL (P2 Expression Language) virtual machine
 *
 */

#include "pel_vm.h"
#include "pel_opcode_decls.gen.h"
#include <iterator>    // for back_inserter
#include <locale>
#include <string>
#include <algorithm>
#include <cctype>      // old <ctype.h>

#include <stdlib.h>
#include <math.h>
#include <boost/regex.hpp>

#include <openssl/sha.h>


#include "plumber.h"
#include "compileUtil.h"
#include "secureUtil.h"
#include "secureUtil1.h"
#include "val_int64.h"
#include "val_str.h"
#include "val_double.h"
#include "val_null.h"
#include "val_tuple.h"
#include "val_opaque.h"
#include "val_time.h"
#include "val_id.h"
#include "val_set.h"
#include "val_vector.h"
#include "val_matrix.h"
#include "val_gaussian_factor.h"
#include "val_table_factor.h"
#include "oper.h"
#include "loop.h"
#include "set.h"
#include "fdbuf.h"
#include "xdrbuf.h"
#include "systemTable.h"



using namespace opr;
typedef Val_Gaussian_Factor::canonical_gaussian canonical_gaussian;

typedef void(Pel_VM::*Op_fn_t)(u_int32_t);

struct JumpTableEnt_t {
  char *opcode;
  int	arity;
  Op_fn_t fn;
};

const char *Pel_VM::err_msgs[] = {
  "ER:Success",
  "ER:Out-of-range constant reference",
  "ER:Out-of-range field reference",
  "ER:Operator not supported for type",
  "ER:Stack underflow",
  "ER:Bad type conversion",
  "ER:Bad opcode",
  "ER:Divide by zero",
  "ER:Bad string operation",
  "ER:Invalid Errno",
  "ER:Bad list field",
  "ER:List underflow",
  "ER:Unknown Error"
};

#include "pel_opcode_defns.gen.h"

//
// Create the VM
//
Pel_VM::Pel_VM() : _st() {
  reset();
}

//
// Create the VM with a preset stack
//
Pel_VM::Pel_VM(std::deque< ValuePtr > staque) : _st(staque) {
}

//
// Reset the VM
//
void
Pel_VM::reset() 
{
  while (!_st.empty()) {
    stackPop();
  }
}


void
Pel_VM::stop()
{
  error = Pel_VM::PE_STOP;
}

//
// Return some value. 
//
ValuePtr Pel_VM::result_val()
{
  if (_st.empty()) {
    stackPush(Val_Null::mk());
  }
  return stackTop();
}

// 
// Make a tuple out of the top elements on the stack and return it.  If
// no result has been popped, return NULL.
// 
TuplePtr Pel_VM::result_tuple() 
{
  return result;
}

// 
// Reset the result tuple
// 
void Pel_VM::reset_result_tuple() 
{
  result.reset();
}

//
// Convert an error message into a string
//
const char *Pel_VM::strerror(Pel_VM::Error e) {
  if (e < 0 | e > PE_UNKNOWN ) {
    e = PE_INVALID_ERRNO;
  }
  return err_msgs[e];
}

//
// Pure extraction of a ValuePtr
//
ValuePtr Pel_VM::pop() 
{
  ValuePtr t = stackTop(); stackPop();
  return t;
}

//
// Type conversion to an unsigned number with no checkng
//
int64_t Pel_VM::pop_signed() 
{
  ValuePtr t = stackTop(); stackPop();
  return Val_Int64::cast(t);
}

//
// Pull a string off the stack
//  
string Pel_VM::pop_string() 
{
  ValuePtr t = stackTop();
  stackPop();
  return Val_Str::cast(t);
}

//
// Pull a time off the stack
//  
boost::posix_time::ptime Pel_VM::pop_time() 
{
  ValuePtr t = stackTop(); stackPop();
  return Val_Time::cast(t);
}

//
// Pull a time_duration off the stack
//  
boost::posix_time::time_duration Pel_VM::pop_time_duration() 
{
  ValuePtr t = stackTop(); stackPop();
  return Val_Time_Duration::cast(t);
}

//
// Pull an ID off the stack
//  
IDPtr Pel_VM::pop_ID() 
{
  ValuePtr t = stackTop(); stackPop();
  return Val_ID::cast(t);
}

//
// Pull a double off the stack
//  
double Pel_VM::pop_double() 
{
  ValuePtr t = stackTop(); stackPop();
  return Val_Double::cast(t);
}

//
// Actually execute a program
//
Pel_VM::Error Pel_VM::execute(const Pel_Program &prog, const TuplePtr data)
{
  prg = &prog;
  error = PE_SUCCESS;
  pc = 0;
  result.reset();
  operand = data;
  
  for (pc = 0;
       pc < prg->ops.size();
       pc++) {
    error = execute_instruction(prg->ops[pc], operand);
    if (error == PE_STOP) {
      // Requested from the program
      return PE_SUCCESS;
    }
    if (error != PE_SUCCESS) {
      return error;
    }
  }
  return error;
}

void Pel_VM::dumpStack(string message)
{
  
  // Dump the stack
  for (std::deque<ValuePtr>::reverse_iterator i = _st.rbegin();
       i != _st.rend();
       i++) {
    TELL_WARN << "Stack entry[" << message << "]: " << (*i)->toString() << "\n";
  }
}

//
// Execute a single instruction
//
Pel_VM::Error Pel_VM::execute_instruction( u_int32_t inst, TuplePtr data)
{
  u_int32_t op = inst & 0xFFFF;
  if (op > NUM_OPCODES) {
    error = PE_BAD_OPCODE;
  } else if (_st.size() < (uint) jump_table[op].arity) {
    error = PE_STACK_UNDERFLOW;
  } else {
    try {
      // This is a somewhat esoteric bit of C++.  Believe it or not,
      // jump_table[op].fn is a pointer to a member function.
      // Consequently, "this->*" dereferences it with respect to the
      // "this" (i.e., the VM we're in), meaning that we can invoke it
      // as a member. 
      (this->*(jump_table[op].fn))(inst);
    } catch (opr::Oper::OperException oe) {
      TELL_ERROR << "Pel_VM caught an operator exception: '"
                 << oe.description()
                 << "\n";
      error = PE_OPER_UNSUP;
      return error;
    } catch (Value::TypeError te) {
      TELL_ERROR << "Pel_VM casting failed: '"
	         << "on operation " << jump_table[op].opcode << ":"
                 << te.what()
                 << "\n";
      TELL_ERROR<<data->toString()<<"\n";
      error = PE_TYPE_CONVERSION;
      return error;
    }
  }
  return error;
}

/***********************************************************************
 *
 * String handling functions...
 */

struct ToUpper
{
  ToUpper(std::locale const& l) : loc(l) {;}
  char operator() (char c) const  { return std::toupper(c,loc); }
private:
  std::locale const& loc;
};

struct ToLower
{
  ToLower(std::locale const& l) : loc(l) {;}
  char operator() (char c) const  { return std::tolower(c,loc); }
private:
  std::locale const& loc;
};


/***********************************************************************
 *
 * Opcode definitions follow
 * 
 * Since the jumptable contains the operation arity, we don't need to
 * check here that there are sufficient operands on the
 * stack. However, we do need to check their types at this stage. 
 * 
 * DO NOT be tempted to make some of these functions more concise by
 * collapsing "pop" statements into "push" arguments.  Remember that
 * C++ does not define the order of evaluation of function arguments,
 * and we HAVE seen the optimiser reorder these differently on
 * different version of the compiler. 
 *
 */

//
// Stack operations
//
DEF_OP(DROP) { stackPop(); }
DEF_OP(SWAP) { 
  ValuePtr t1 = stackTop(); stackPop();
  ValuePtr t2 = stackTop(); stackPop();
  stackPush(t1); 
  stackPush(t2);
}
DEF_OP(DUP) { 
  stackPush(stackTop()); 
}
DEF_OP(PUSH_CONST) { 
  uint ndx = (inst >> 16);
  if (ndx > prg->const_pool.size()) {
    error = PE_BAD_CONSTANT;
    return;
  }
  stackPush(prg->const_pool[ndx]);
}
DEF_OP(PUSH_FIELD) { 
  uint ndx = (inst >> 16);
  if (ndx > operand->size()) {
    error = PE_BAD_FIELD;
    return;
  }
  stackPush((*operand)[ndx]);
}
DEF_OP(POP) {
  if (!result) { result = Tuple::mk(); }
  ValuePtr top = stackTop();
  stackPop();
  if (top->typeCode() == Value::TUPLE) {
    // Freeze it before taking it out
    Val_Tuple::cast(top)->freeze();
  }
  result->append(top);
}
DEF_OP(POP_ALL) {
  if (!result) { result = Tuple::mk(); }
  while (_st.size() > 0) {
    ValuePtr top = pop();
    if (top->typeCode() == Value::TUPLE) {
      // Freeze it before taking it out
      Val_Tuple::cast(top)->freeze();
    }
    result->append(top);
  }
}
DEF_OP(PEEK) {
  int64_t stackPosition = pop_signed();
  if (stackPosition >= _st.size()) {
    error = PE_STACK_UNDERFLOW;
    return;
  }

  // Push that stack element
  stackPush(stackPeek(stackPosition));
}
DEF_OP(IFELSE) {
  ValuePtr elseVal = stackTop(); stackPop();
  ValuePtr thenVal = stackTop(); stackPop();
  int64_t ifVal = pop_signed();
  if (ifVal) {
    stackPush(thenVal);
  } else {
    stackPush(elseVal);
  }
}
DEF_OP(IFPOP) {
  ValuePtr thenVal = stackTop(); stackPop();
  int64_t ifVal = pop_signed();
  if (ifVal) {
    if (!result) {
      result = Tuple::mk();
    }
    result->append(thenVal);
  }
}
DEF_OP(IFSTOP) {
  int64_t ifVal = pop_signed();
  if (ifVal) {
    stop();
    //    TELL_WARN << "IF stop of " << ifVal << ".  Stopping!!!\n";
  } else {
    //    TELL_WARN << "IF stop of " << ifVal << ".  Not stopping\n";
  }
}
DEF_OP(DUMPSTACK) {
  string s1 = pop_string();
  dumpStack(s1);
}
DEF_OP(IFPOP_TUPLE) {
  int64_t ifVal = pop_signed();
  if (ifVal) {
    if (!result) {
      result = Tuple::mk();
    }
    for (uint i = 0;
         i < operand->size();
         i++) {
      result->append((*operand)[i]);
    }
  }
}
DEF_OP(T_MKTUPLE) {
  ValuePtr val = stackTop(); stackPop();
  TuplePtr t = Tuple::mk();
  t->append(val);
  ValuePtr tuple = Val_Tuple::mk(t);
  stackPush(tuple);
}

DEF_OP(T_APPEND) {
  ValuePtr value = stackTop(); stackPop();
  ValuePtr obj = stackTop();
  TuplePtr t = Val_Tuple::cast(obj);
  t->append(value);
}

DEF_OP(T_UNBOX) {
  ValuePtr tuple = stackTop(); stackPop();
  TuplePtr t = Val_Tuple::cast(tuple);
  // Now start pushing fields from the end forwards
  for (int i = t->size() - 1;
       i >= 0;
       i--) {
    stackPush((*t)[i]);
  }
}
DEF_OP(T_UNBOXPOP) {
  ValuePtr tuple = stackTop(); stackPop();
  TuplePtr t = Val_Tuple::cast(tuple);
  // Now start popping fields from front out
  if (!result) {
    result = Tuple::mk();
  }
  for (uint32_t i = 0;
       i < t->size();
       i++) {
    result->append((*t)[i]);
  }
}
DEF_OP(T_FIELD) {
  unsigned field = pop_signed();
  ValuePtr tuple = stackTop(); stackPop();
  TuplePtr theTuple = Val_Tuple::cast(tuple);
  ValuePtr value = (*theTuple)[field];
  if (value == NULL) {
    stackPush(Val_Null::mk());
  } else {
    stackPush(value);
  }
}
DEF_OP(T_SWALLOW) { 
  ValuePtr swallowed = Val_Tuple::mk(operand);
  stackPush(swallowed);
}

DEF_OP(TYPEOF) { 
  ValuePtr value = stackTop(); stackPop();
  ValuePtr typeName = Val_Str::mk(string(value->typeName()));

  stackPush(typeName);
}


/**
 * HASH OPERATIONS
 */
DEF_OP(H_SHA1) { 
  ValuePtr vp = stackTop(); stackPop();
  std::string svalue = vp->toString();
  unsigned char digest[SHA_DIGEST_LENGTH];	// 20 byte array
  SHA1(reinterpret_cast<const unsigned char*>(svalue.c_str()), 
       svalue.size(), &digest[0]);

  IDPtr hashID = ID::mk(reinterpret_cast<uint32_t*>(digest));
  stackPush(Val_ID::mk(hashID));
}

DEF_OP(T_IDGEN) { 
  stackPush(Val_Int64::mk(Plumber::catalog()->uniqueIdentifier()));
}

/* MAX and MIN of two value types. */
DEF_OP(MAX) { 
  ValuePtr v1 = stackTop(); stackPop();
  ValuePtr v2 = stackTop(); stackPop();
  stackPush((v1 <= v2 ? v2 : v1));
}

DEF_OP(MIN) { 
  ValuePtr v1 = stackTop(); stackPop();
  ValuePtr v2 = stackTop(); stackPop();
  stackPush((v1 <= v2 ? v1 : v2));
}


/**
 * Named Functions
 */
DEF_OP(FUNC0) { 
  ValuePtr vp = stackTop(); stackPop();

  // Find the function

  // Run the function
}

DEF_OP(INITMASK) { 
   ValuePtr listVal = stackTop(); stackPop();
   ListPtr list = List::mk();
   list->append(listVal);
   ValuePtr mask = Val_List::mk(list);
   stackPush(mask);
}

DEF_OP(GETMASK) { 
  ValuePtr v = stackTop(); stackPop();
  ValuePtr mask = Val_List::mk(compile::namestracker::getMask(v));
  stackPush(mask);
}

DEF_OP(COMBINEMASK) { 
  ValuePtr first = stackTop(); stackPop();
  ValuePtr second = stackTop(); stackPop();
  ListPtr list = Val_List::cast(first);
  if (!list) {
    error = PE_BAD_LIST_FIELD;
    return;
  }

  list->append(second);
  stackPush(first);
}

DEF_OP(MASK) { 
  ValuePtr tempSchema = stackTop(); stackPop();
  ValuePtr maskSchema = stackTop(); stackPop();
  ValuePtr oldPos = stackTop(); stackPop();
  ListPtr maskList = Val_List::cast(maskSchema);
  ListPtr tempList = Val_List::cast(tempSchema);
  if (!maskList || !tempList) {
    error = PE_BAD_LIST_FIELD;
    return;
  }
  ListPtr maskedSchema = 
    compile::namestracker::applyMask(tempList, maskList, Val_Int64::cast(oldPos));
  stackPush(Val_List::mk(maskedSchema));
}

DEF_OP(A_TO_VAR) { 
   ValuePtr attr = stackTop(); stackPop();
   stackPush(compile::namestracker::toVar(attr));
}

DEF_OP(A_UNIQUE_SCHEMA) { 
   ValuePtr attr = stackTop(); stackPop();
   stackPush(compile::namestracker::uniqueSchema(attr));
}

DEF_OP(ISTHETA) { 
   ValuePtr boolv = stackTop(); stackPop();
   stackPush(Val_Int64::mk(compile::namestracker::isTheta(boolv)));
}

/**
 * List operations
 *
 */
DEF_OP(L_CREATEKEY) { 
   ValuePtr first = stackTop(); stackPop();
   ListPtr schema = Val_List::cast(first);

   uint pos = 1;
   ListPtr key = List::mk();
   for (ValPtrList::const_iterator iter = schema->begin(); 
        iter != schema->end(); iter++) {
     key->append(Val_Int64::mk(pos++));
   }
   stackPush(Val_List::mk(key));
}

DEF_OP(L_FACT) { 
   ValuePtr first  = stackTop(); stackPop();
   ValuePtr second = stackTop(); stackPop();
  
   TuplePtr fact  = Tuple::mk(first->toString(), false);
   ListPtr values = Val_List::cast(second);

   for (ValPtrList::const_iterator iter = values->begin(); 
        iter != values->end(); iter++) {
     ValuePtr val = *iter;
     if (val->typeCode() == Value::TUPLE) {
       val = (*Val_Tuple::cast(val))[2]; // unbox the datatype
     }
     fact->append(val);
   } 
   fact->freeze();
   stackPush(Val_Tuple::mk(fact));
}

DEF_OP(L_INDEXMATCH) { 
   ValuePtr val1 = stackTop(); stackPop();
   ValuePtr val2 = stackTop(); stackPop();
   ValuePtr val3 = stackTop(); stackPop();
   ListPtr outer = Val_List::cast(val1);
   ListPtr inner = Val_List::cast(val2);
   ListPtr index = Val_List::cast(val3);

   CommonTable::Key joinKey;
   CommonTable::Key indexKey;
   CommonTable::Key baseKey;

   /* We need to ensure that the index is contained by the 
      indexKey returned by joinKeys(...). This basically means
      we can use the index. */
   compile::namestracker::joinKeys(outer, inner, joinKey, indexKey, baseKey); 
   bool match = true;
   for (ValPtrList::const_iterator kiter = index->begin(); 
        match && kiter != index->end(); kiter++) {
     int64_t pos = Val_Int64::cast(*kiter);
     CommonTable::Key::const_iterator iiter = indexKey.begin();
     for ( ; iiter != indexKey.end(); iiter++) {
       if (*iiter == pos) {
         break;
       }
     }
     match = iiter != indexKey.end();
   }
   
   stackPush(Val_Int64::mk(match));
}

DEF_OP(L_VARIABLES) { 
   ValuePtr expr = stackTop(); stackPop();
   stackPush(Val_List::mk(compile::namestracker::variables(expr)));
}

DEF_OP(L_CASTASSIGN) { 
   ValuePtr val1 = stackTop(); stackPop();
   ValuePtr val2 = stackTop(); stackPop();
   ValuePtr val3 = stackTop(); stackPop();
   ListPtr outer = Val_List::cast(val1);
   ListPtr inner = Val_List::cast(val2);
   TuplePtr select = Val_Tuple::cast(val3);

   ValuePtr value = compile::namestracker::castassign(outer,inner,select);
   stackPush(value);
}

DEF_OP(L_ADORNMENT) { 
   ValuePtr first  = stackTop(); stackPop();
   ValuePtr second = stackTop(); stackPop();

   ListPtr bound = List::mk();
   if (first != Val_Null::mk()) {
     bound = Val_List::cast(first);
   }
   ListPtr schema = Val_List::cast(second);

   stackPush(Val_List::mk(compile::namestracker::adornment(bound, schema)));
}

DEF_OP(L_PROJECT) { 
   ValuePtr first  = stackTop(); stackPop();
   ValuePtr second = stackTop(); stackPop();
  
   ListPtr positions = Val_List::cast(first);
   ListPtr schema    = Val_List::cast(second);

   stackPush(Val_List::mk(compile::namestracker::project(positions, schema)));
}

DEF_OP(L_SORTATTR) { 
   ValuePtr val1   = stackTop(); stackPop();
   ValuePtr oorder = stackTop(); stackPop();
   ValuePtr val3   = stackTop(); stackPop();
   ValuePtr iorder = stackTop(); stackPop();
   ListPtr outer   = Val_List::cast(val1);
   ListPtr inner   = Val_List::cast(val3);
   
   stackPush(compile::namestracker::sortAttr(outer, oorder, inner, iorder));
}

DEF_OP(L_PREFIX) { 
   ValuePtr val1 = stackTop(); stackPop();
   ValuePtr val2 = stackTop(); stackPop();

   if (val1 == Val_Null::mk() || val2 == Val_Null::mk()) {
     stackPush(Val_Int64::mk(0));
   }
   else {
     ListPtr list1 = Val_List::cast(val1);
     ListPtr list2 = Val_List::cast(val2);
     stackPush(Val_Int64::mk(compile::namestracker::prefix(list1, list2)));
   }
}

DEF_OP(L_EQUIVALENT) { 
   ValuePtr val1 = stackTop(); stackPop();
   ValuePtr val2 = stackTop(); stackPop();
   ListPtr plan1 = Val_List::cast(val1);
   ListPtr plan2 = Val_List::cast(val2);

   stackPush(Val_Int64::mk(compile::namestracker::equivalent(plan1, plan2)));
}

DEF_OP(L_MERGE) { 
   ValuePtr val1 = stackTop(); stackPop();
   ValuePtr val2 = stackTop(); stackPop();
   ListPtr list1 = Val_List::cast(val1);
   ListPtr list2 = Val_List::cast(val2);
   
   stackPush(Val_List::mk(compile::namestracker::merge(list1, list2)));
}

DEF_OP(L_ASSIGNSCHEMA) { 
   ValuePtr first = stackTop(); stackPop();
   ValuePtr var = stackTop(); stackPop();
   ListPtr schema = Val_List::cast(first);
   
   stackPush(Val_List::mk(compile::namestracker::assignSchema(schema, var)));
}

DEF_OP(L_POS_ATTR) { 
   ValuePtr var     = stackTop(); stackPop();
   ValuePtr listVal = stackTop(); stackPop();
   ListPtr  list    = Val_List::cast(listVal);
   
   stackPush(Val_Int64::mk(compile::namestracker::position(list, var)));
}

DEF_OP(T_MK_TYPE) { 
   ValuePtr type = stackTop(); stackPop();
   ValuePtr val  = stackTop(); stackPop();

   TuplePtr tp = Tuple::mk(type->toString());
   tp->append(val);
   tp->freeze();
   stackPush(Val_Tuple::mk(tp));
}

DEF_OP(T_MK_BOOL) { 
   ValuePtr type = stackTop(); stackPop();
   ValuePtr val1  = stackTop(); stackPop();
   ValuePtr val2  = stackTop(); stackPop();

   TuplePtr tp = Tuple::mk(BOOL);
   tp->append(type);
   tp->append(val1);
   tp->append(val2);
   tp->freeze();
   stackPush(Val_Tuple::mk(tp));
}

DEF_OP(L_GET_ATTR) { 
   ValuePtr attr    = stackTop(); stackPop();
   ValuePtr listVal = stackTop(); stackPop();
   ListPtr  list    = Val_List::cast(listVal);
   
   if (attr->typeCode() == Value::INT64) {
     int position = Val_Int64::cast(attr);
     for (ValPtrList::const_iterator iter = list->begin();
          iter != list->end(); iter++) {
       if (!position--) {
         stackPush(*iter);
         return;
       } 
     }
   }
   else if (attr->typeCode() == Value::STR) {
     string type = Val_Str::cast(attr);
     if (type == LOC) {
       ValuePtr location = compile::namestracker::location(list);
       if (!location) {
         TELL_ERROR << "NO LOCATION VARIABLE IN SCHEMA: " << list->toString() << std::endl;
         error = PE_BAD_LIST_FIELD;
       }
       else {
         TuplePtr loc = Tuple::mk(LOC);
         loc->append(compile::namestracker::location(list));
         loc->freeze();
         stackPush(Val_Tuple::mk(loc));
       }
       return;
     }
     for (ValPtrList::const_iterator iter = list->begin();
          iter != list->end(); iter++) {
       if ((*Val_Tuple::cast(*iter))[0]->toString() == type) {
         stackPush(*iter);
         return;
       } 
     } 
   }
   stackPush(Val_Null::mk());
}

DEF_OP(L_FLATTEN) { 
   ValuePtr listVal = stackTop(); stackPop();
   ListPtr  list    = Val_List::cast(listVal);
   stackPush(Val_List::mk(compile::namestracker::flatten(list)));
}

DEF_OP(L_GROUPBY_ATTR) { 
   ValuePtr listVal = stackTop(); stackPop();
   ListPtr  list    = Val_List::cast(listVal);
   stackPush(Val_List::mk(compile::namestracker::groupby(list)));
}

DEF_OP(L_AGG_ATTR) { 
   ValuePtr listVal = stackTop(); stackPop();
   ListPtr  list    = Val_List::cast(listVal);
   
   stackPush(Val_Int64::mk(compile::namestracker::aggregation(list)));
}

DEF_OP(L_CONCAT) {
   ValuePtr val1 = stackTop(); stackPop();
   ValuePtr val2 = stackTop(); stackPop();
   ListPtr list1 = Val_List::cast(val1);
   ListPtr list2 = Val_List::cast(val2);
   
   stackPush(Val_List::mk(list1->concat(list2)));
}

DEF_OP(L_APPEND) {
   ValuePtr value = stackTop(); stackPop();
   ValuePtr listVal = stackTop(); stackPop();
   ListPtr list = Val_List::cast(listVal);
   
   if(list->size() == 0) {
      stackPush(Val_List::mk(ListPtr(new List(value))));
   } else {
      list->append(value);
      stackPush(Val_List::mk(list));
   }
}

DEF_OP(L_SUBSET) {
   ValuePtr first = stackTop(); stackPop();
   ValuePtr second = stackTop(); stackPop();

   if (first->typeCode() == Value::LIST &&
       second->typeCode() == Value::LIST)
   {
     ListPtr s1 = Val_List::cast(first);
     ListPtr s2 = Val_List::cast(second);
     stackPush(Val_Int64::mk(compile::namestracker::subset(s1, s2)));
     return;
   }
   error = PE_BAD_LIST_FIELD;
   return;
}


DEF_OP(L_MEMBER) {
   ValuePtr value = stackTop(); stackPop();
   ValuePtr listVal = stackTop(); stackPop();

   int isMember = 0;
   if(listVal->typeCode() == Value::LIST){
    isMember = (Val_List::cast(listVal))->member(value);
   }
   else{
     isMember = (Val_Set::cast(listVal))->member(value);
   }
   stackPush(Val_Int64::mk(isMember));
}

DEF_OP(L_INTERSECT) {
   ValuePtr val1 = stackTop(); stackPop();
   ValuePtr val2 = stackTop(); stackPop();
   ListPtr l1 = Val_List::cast(val1);
   ListPtr l2 = Val_List::cast(val2);
   
   stackPush(Val_List::mk(l1->intersect(l2)));
}

DEF_OP(L_MULTISET_INTERSECT) {
   ValuePtr val1 = stackTop(); stackPop();
   ValuePtr val2 = stackTop(); stackPop();
   ListPtr l1 = Val_List::cast(val1);
   ListPtr l2 = Val_List::cast(val2);
   
   stackPush(Val_List::mk(l1->multiset_intersect(l2)));
}

// Vector operations
DEF_OP(V_INITVEC) {
  uint64_t sz = (uint64_t) pop_signed();
  ValuePtr vector = Val_Vector::mk2(sz);
  stackPush(vector);
}

DEF_OP(V_GETOFFSET) {
   int64_t offset = pop_signed();
   ValuePtr val1 = stackTop(); stackPop();
   VectorPtr v1 = Val_Vector::cast(val1);
   stackPush((*v1)[offset]);
}


DEF_OP(V_SETOFFSET) {
   ValuePtr val1 = stackTop(); stackPop();
   int64_t offset = pop_signed();
   ValuePtr val2 = stackTop(); stackPop();
   VectorPtr v2 = Val_Vector::cast(val2);
   (*v2)[offset] = val1;
   stackPush(Val_Vector::mk(v2));
}

DEF_OP(V_COMPAREVEC) {
  ValuePtr val1 = stackTop(); stackPop();
  ValuePtr val2 = stackTop(); stackPop();
  VectorPtr v2 = Val_Vector::cast(val2);
  Val_Vector vec2(v2);
  stackPush(Val_Int64::mk(vec2.compareTo(val1)));
}

// Matrix operations
DEF_OP(M_INITMAT) {
  uint64_t sz2 = (uint64_t) pop_signed();
  uint64_t sz1 = (uint64_t) pop_signed();
  ValuePtr matrix = Val_Matrix::mk2(sz1, sz2);
  stackPush(matrix);
}

DEF_OP(M_INITMATZERO) {
  uint64_t sz2 = pop_signed();
  uint64_t sz1 = pop_signed();

  MatrixPtr mp(new ublas::matrix<ValuePtr>(sz1, sz2));
  ValuePtr va = Val_Double::mk(0.0);

  for (uint64_t i = 0; i < sz1; i++)
      for (uint64_t j = 0; j < sz2; j++)
        (*mp)(i,j) = va;
  stackPush(Val_Matrix::mk(mp));
}

DEF_OP(M_GETOFFSET) {
   int64_t offset1 = pop_signed();
   int64_t offset2 = pop_signed();
   ValuePtr val1 = stackTop(); stackPop();
   MatrixPtr m1 = Val_Matrix::cast(val1);
   stackPush((*m1)(offset1,offset2));
}


DEF_OP(M_SETOFFSET) {
   ValuePtr val1 = stackTop(); stackPop();
   int64_t offset1 = pop_signed();
   int64_t offset2 = pop_signed();
   ValuePtr val2 = stackTop(); stackPop();
   MatrixPtr mat = Val_Matrix::cast(val2);
   (*mat)(offset1,offset2) = val1;
   stackPush(Val_Matrix::mk(mat));
}

DEF_OP(M_COMPAREMAT) {
  ValuePtr val1 = stackTop(); stackPop();
  ValuePtr val2 = stackTop(); stackPop();
  MatrixPtr m2 = Val_Matrix::cast(val2);
  Val_Matrix mat2(m2);
  stackPush(Val_Int64::mk(mat2.compareTo(val1)));
}

DEF_OP(M_TRANSPOSE) {
  ValuePtr val1 = stackTop(); stackPop();
  MatrixPtr m1 = Val_Matrix::cast(val1);
  Val_Matrix mat1(m1);
  stackPush(mat1.transpose());
}


/* Factor operations */

DEF_OP(FACTOR_REGISTERVAR) {
  //pushing order(name, type, size)

  string name = pop_string();
  string type = pop_string();
  std::size_t size = pop_signed();

  if(type == "V") Val_Factor::registerVectorVariable(name, size);
  else if(type == "F") Val_Factor::registerFiniteVariable(name, size);
  else assert(false);
  stackPush(Val_Int64::mk(1));
}

DEF_OP(COMBINE) {
  ValuePtr val1 = stackTop(); stackPop();
  ValuePtr val2 = stackTop(); stackPop();
  stackPush(dynamic_cast<Val_Factor*>(val1.get())->multiply(val2));
}

DEF_OP(COMBINE_ALL) {
  ValuePtr val1 = stackTop(); stackPop();
  ListPtr list = Val_List::cast(val1);
  
  assert(list->size()!=0);
  ValuePtr product = list->front();
  ValPtrList::const_iterator iter = list->begin()++, end = list->end();
  while(iter != end) 
    product = dynamic_cast<Val_Factor*>(product.get())->multiply(*(iter++));
  stackPush(product);
}

DEF_OP(COLLAPSE) {
  using namespace std;
  ValuePtr val1 = stackTop(); stackPop();
  ValuePtr val2 = stackTop(); stackPop();
  cout << "COLLAPSE: " << val1->toString() << ',' << val2->toString() << endl;
  ListPtr varlist = Val_List::cast(val2);
  stackPush(dynamic_cast<Val_Factor*>(val1.get())->marginal(varlist));
}

DEF_OP(FACTOR_CREATE_CANONICAL_FACTOR) {
  /* popping order varlist, mat, vec */
  ListPtr varlist = Val_List::cast(stackTop()); stackPop();
  MatrixPtr lambdamat = Val_Matrix::cast(stackTop()); stackPop();
  VectorPtr etavec = Val_Vector::cast(stackTop()); stackPop();

  stackPush(Val_Gaussian_Factor::mk(varlist, lambdamat, etavec));
}

DEF_OP(FACTOR_DEFAULT_CANONICAL_FACTOR) {
  stackPush(Val_Gaussian_Factor::mk());
}

DEF_OP(CREATE_TABLE_FACTOR) {
  ListPtr var_list = Val_List::cast(stackTop()); stackPop();
  ListPtr assignments = Val_List::cast(stackTop()); stackPop();
  stackPush(Val_Table_Factor::mk(var_list, assignments));
}

DEF_OP(DEFAULT_TABLE_FACTOR) {
  stackPush(Val_Table_Factor::mk());
}
	    
DEF_OP(FACTOR_GAUSS_MEAN) {
  ValuePtr val = stackTop(); stackPop();
  stackPush(dynamic_cast<Val_Gaussian_Factor*>(val.get())->mean());
}

DEF_OP(FACTOR_GAUSS_COV) {
  ValuePtr val = stackTop(); stackPop();
  stackPush(dynamic_cast<Val_Gaussian_Factor*>(val.get())->covariance());
}


//
// Boolean operations
//
DEF_OP(NOT) {
  u_int64_t v = pop_signed();
  //  TELL_WARN << "NOT of " << v << " is " << !v << "\n";
  stackPush(Val_Int64::mk(!v));
}
DEF_OP(AND) {
  u_int64_t v2 = pop_signed();
  u_int64_t v1 = pop_signed();
  stackPush(Val_Int64::mk(v1 && v2));
}
DEF_OP(OR) {
  u_int64_t v2 = pop_signed();
  u_int64_t v1 = pop_signed();
  stackPush(Val_Int64::mk(v1 || v2));
}
DEF_OP(RAND) {
  int64_t r = random();
  stackPush(Val_Int64::mk(r));
}
DEF_OP(COIN) {
  double r = double(random()) / double(RAND_MAX);
  double p = pop_double();
  stackPush(Val_Int64::mk(r <= p));
}

DEF_OP(L_INIT) {
  stackPush(Val_List::mk(List::mk()));
}

DEF_OP(EMPTY) {
  stackPush(Val_Set::mk(Set::mk()));
}


DEF_OP(SERIALIZE) {
  ValuePtr first = stackTop(); stackPop();
  ValuePtr second = stackTop(); stackPop();
  stackPush(compile::secure::processGen(first, second));
}
DEF_OP(DESERIALIZE) {
  ValuePtr first = stackTop(); stackPop();
  ValuePtr second = stackTop(); stackPop();
  stackPush(compile::secure::processExtract(first, second));
}
DEF_OP(CREATEVER) {
  stackPush(compile::secure::generateVersion());
}
DEF_OP(CREATELOCSPEC) {
  stackPush(compile::secure::generateLocSpec());
}
DEF_OP(IS_LOCSPEC) {
  ValuePtr first = stackTop(); stackPop();
  stackPush(Val_Int64::mk(compile::secure::isLocSpec(first)?1:0));
}
DEF_OP(GET_CERT) {
  ValuePtr first = stackTop(); stackPop();
  stackPush(compile::secure::getCert(first));
}
DEF_OP(LOADKEYFILE) {
  ValuePtr first = stackTop(); stackPop();
  stackPush(compile::secure::loadFile(first));
}
DEF_OP(IS_SAYS) {
  ValuePtr first = stackTop(); stackPop();
  stackPush(Val_Int64::mk(compile::secure::isSaysHint(first)));
}


DEF_OP(L_CONS) {
  ValuePtr first = stackTop(); stackPop();
  ValuePtr second = stackTop(); stackPop();
  
  ListPtr list = List::mk();

  if (first->typeCode() != Value::NULLV) {
    if (first->typeCode() == Value::LIST) {
      list = list->concat(Val_List::cast(first));
    }
    else {
      list->append(first);
    }
  }

  if (second->typeCode() != Value::NULLV) {
    if (second->typeCode() == Value::LIST) {
      list = list->concat(Val_List::cast(second));
    }
    else {
      list->append(second);
    }
  }
  
  stackPush(Val_List::mk(list));
}

DEF_OP(L_CDR) {
  ValuePtr first = stackTop(); stackPop();
  ListPtr list = Val_List::cast(first);

  if (!list) {
    error = PE_BAD_LIST_FIELD;
    return;
  }
  else if (list->size() == 0) {
    error = PE_LIST_UNDERFLOW;
    return;
  }

  list = list->clone();
  list->pop_front();
  stackPush(Val_List::mk(list));
}

DEF_OP(L_CAR) {
  ValuePtr first = stackTop(); stackPop();
  ListPtr list = Val_List::cast(first);

  if (!list) {
    error = PE_BAD_LIST_FIELD;
    return;
  }
  else if (list->size() == 0) {
    error = PE_LIST_UNDERFLOW;
    return;
  }

  stackPush(list->front());
}

DEF_OP(L_CONTAINS) {
  ValuePtr second = stackTop(); stackPop();
  ValuePtr first = stackTop(); stackPop();

  ListPtr list = Val_List::cast(first);
  if (!list) {
    TELL_ERROR << "PEL CONTAINS: " 
               << second->toString() << " in " 
               << first->toString() << std::endl;
    error = PE_BAD_LIST_FIELD;
    return;
  }
  stackPush(Val_Int64::mk(list->member(second)));
}

DEF_OP(GEN) {
  ValuePtr serializedMsg_1 = stackTop(); stackPop();
  int64_t type_2 = Val_Int64::cast(stackTop()); stackPop();
  ValuePtr key_3 = stackTop(); stackPop();
  ValuePtr res = compile::secure::SecurityAlgorithms::generate(serializedMsg_1, type_2, key_3);
  
  stackPush(res);
}


DEF_OP(INITSET) {
  ValuePtr v = stackTop(); stackPop();
  SetPtr setPtr = Set::mk();
  setPtr->insert(v);
  stackPush(Val_Set::mk(setPtr));
}

DEF_OP(SETMOD) {
  ValuePtr setVPtr = stackTop(); stackPop();
  if(setVPtr->typeCode() == Value::SET){
    SetPtr setPtr = Val_Set::cast(setVPtr);
    stackPush(Val_Int64::mk(setPtr->size()));
  }
  else{
    stackPush(Val_Int64::mk(1));
  }
}


DEF_OP(VERIFY) {
  ValuePtr msg_1 = stackTop(); stackPop();
  ValuePtr proof_2 = stackTop(); stackPop();
  ValuePtr P_3 = stackTop(); stackPop();
  ValuePtr R_4 = stackTop(); stackPop();
  int64_t K_5 = Val_Int64::cast(stackTop()); stackPop();
  ValuePtr V_6 = stackTop(); stackPop();
  ListPtr proofList;
  SetPtr P, R, V;
  if(proof_2->typeCode() != Value::LIST){
    proofList = List::mk();
    proofList->append(proof_2);
  }
  else{
    proofList = Val_List::cast(proof_2);
  }

  if(P_3->typeCode() != Value::SET){
    P = Set::mk();
    P->insert(P_3);
  }
  else{
    P = Val_Set::cast(P_3);
  }
  if(R_4->typeCode() != Value::SET){
    R = Set::mk();
    R->insert(R_4);
  }
  else{
    R = Val_Set::cast(R_4);
  }

  if(V_6->typeCode() != Value::SET){
    V = Set::mk();
    V->insert(V_6);
  }
  else{
    V = Val_Set::cast(V_6);
  }

  compile::secure::Primitive primitive(P, R, K_5, V);
  stackPush(Val_Int64::mk(compile::secure::SecurityAlgorithms::verify(msg_1, proofList, &primitive)));
}

DEF_OP(L_REMOVELAST) {
  ValuePtr first = stackTop(); stackPop();
  ListPtr list = Val_List::cast(first);

  if (!list) {
    error = PE_BAD_LIST_FIELD;
    return;
  }
  else if (list->size() == 0) {
    error = PE_LIST_UNDERFLOW;
    return;
  }

  list->pop_back();
  stackPush(Val_List::mk(list));
}

DEF_OP(L_LAST) {
  ValuePtr first = stackTop(); stackPop();
  ListPtr list = Val_List::cast(first);

  if (!list) {
    error = PE_BAD_LIST_FIELD;
    return;
  }
  else if (list->size() == 0) {
    error = PE_LIST_UNDERFLOW;
    return;
  }

  stackPush(list->back());
}

DEF_OP(L_SIZE) {
  ValuePtr first = stackTop(); stackPop();
  ListPtr list = Val_List::cast(first);

  if (!list) {
    error = PE_BAD_LIST_FIELD;
    return;
  }

  stackPush(Val_Int64::mk(list->size()));
}

//
// Integer-only arithmetic operations (mostly bitwise)
//

DEF_OP(ASR) {
  ValuePtr shiftCount = pop();
  ValuePtr numberToShift = pop();
  stackPush((numberToShift >> shiftCount));
}
DEF_OP(ASL) {
  ValuePtr v1 = pop();
  ValuePtr v2 = pop();
  stackPush((v2 << v1));
}
DEF_OP(BIT_AND) {
  ValuePtr v1 = pop();
  ValuePtr v2 = pop();
  stackPush((v2 & v1));
}
DEF_OP(BIT_OR) {
  ValuePtr v1 = pop();
  ValuePtr v2 = pop();
  stackPush((v2 | v1));
}
DEF_OP(APPEND) {
  ValuePtr v1 = pop();
  ValuePtr v2 = pop();
  /*  Fdbuf* fin = new Fdbuf(0);
  XDR xdr;
  xdrfdbuf_create(&xdr, fin, false, XDR_ENCODE);
  ValuePtr va;
  v1->xdr_marshal(&xdr);
  v2->xdr_marshal(&xdr);
  ValuePtr res = Val_Opaque::mk(FdbufPtr(fin));*/
  string resStr = v1->toString() + v2->toString();
  ValuePtr res = Val_Str::mk(resStr);
  //  std::cout<<"Append of "<< v1->toString() << " and " << v2->toString() << " produced " << res->toString()<<"\n";
  stackPush(res);
}
DEF_OP(BIT_XOR) {
  ValuePtr v1 = pop();
  ValuePtr v2 = pop();
  stackPush((v2 ^ v1));
}
DEF_OP(BIT_NOT) {
  stackPush((~pop()));
}

//
// arithmetic operations
//
DEF_OP(NEG) {
  stackPush(-pop());
}
DEF_OP(PLUS) {
  ValuePtr v2 = pop();
  ValuePtr v1 = pop();
  ValuePtr r1 = v1 + v2;
  stackPush(r1);
}
DEF_OP(MINUS) {
  // Be careful of undefined evaluation order in C++!
  ValuePtr v1 = pop();
  ValuePtr v2 = pop();
  stackPush(v2 - v1);
}
DEF_OP(MINUSMINUS) {
  ValuePtr v1 = pop();
  stackPush(--v1);
}
DEF_OP(PLUSPLUS) {
  ValuePtr v1 = pop();
  stackPush(++v1);
}
DEF_OP(MUL) {
  ValuePtr v2 = pop();
  ValuePtr v1 = pop();
  stackPush(v1 * v2);
}
DEF_OP(DIV) {
  // Be careful of undefined evaluation order in C++!
  ValuePtr v1 = pop();
  ValuePtr v2 = pop();
  if (v1 != Val_Int64::mk(0)) { 
    try {
      stackPush((v2 / v1));
    } catch (opr::Oper::DivisionByZeroException e) {
      error = PE_DIVIDE_BY_ZERO;
    }
  } else if (error == PE_SUCCESS) {
    error = PE_DIVIDE_BY_ZERO;
  }
}
DEF_OP(MOD) {
  // Be careful of undefined evaluation order in C++!
  ValuePtr v1 = pop();
  ValuePtr v2 = pop();
  if (v1 != Val_Int64::mk(0)) { 
    stackPush((v2 % v1));
  } else if (error == PE_SUCCESS) {
    error = PE_DIVIDE_BY_ZERO;
  }
}


//
// Comparison operators
//
DEF_OP(TOTALCOMP) { 
  ValuePtr v1 = pop();
  ValuePtr v2 = pop();
  stackPush(Val_Int64::mk(v1->compareTo(v2)));
}
DEF_OP(EQ) {
  ValuePtr v1 = pop();
  ValuePtr v2 = pop();
  stackPush(Val_Int64::mk(v2 == v1));
}
DEF_OP(LT) { 
  ValuePtr v1 = pop();
  ValuePtr v2 = pop();
  stackPush(Val_Int64::mk(v2 > v1)); 
}
DEF_OP(LTE) { 
  ValuePtr v1 = pop();
  ValuePtr v2 = pop();
  stackPush(Val_Int64::mk(v2 >= v1)); 
}
DEF_OP(GT) { 
  ValuePtr v1 = pop();
  ValuePtr v2 = pop();
  stackPush(Val_Int64::mk(v2 < v1)); 
}
DEF_OP(GTE) { 
  ValuePtr v1 = pop();
  ValuePtr v2 = pop();
  stackPush(Val_Int64::mk(v2 <= v1)); 
}

//
// IN Operator
//
DEF_OP(INOO) {
  ValuePtr to   = pop();
  ValuePtr from = pop();
  ValuePtr key  = pop();
  stackPush(Val_Int64::mk(inOO(key, from, to)));
}
DEF_OP(INOC) {
  ValuePtr to   = pop();
  ValuePtr from = pop();
  ValuePtr key  = pop();
  stackPush(Val_Int64::mk(inOC(key, from, to)));
}
DEF_OP(INCO) {
  ValuePtr to   = pop();
  ValuePtr from = pop();
  ValuePtr key  = pop();
  stackPush(Val_Int64::mk(inCO(key, from, to)));
}
DEF_OP(INCC) {
  ValuePtr to   = pop();
  ValuePtr from = pop();
  ValuePtr key  = pop();
  stackPush(Val_Int64::mk(inCC(key, from, to)));
}

//
// Time operations.  Note that the '>' and '<' are reversed: think
// about operand order on the stack!
//
DEF_OP(TIME_LT) { 
  boost::posix_time::ptime s1 = pop_time();
  boost::posix_time::ptime s2 = pop_time();
  stackPush(Val_Int64::mk(s2 < s1));
}
DEF_OP(TIME_LTE) { 
  boost::posix_time::ptime s1 = pop_time();
  boost::posix_time::ptime s2 = pop_time();
  stackPush(Val_Int64::mk(s2 <= s1));
}
DEF_OP(TIME_GT) { 
  boost::posix_time::ptime s1 = pop_time();
  boost::posix_time::ptime s2 = pop_time();
  stackPush(Val_Int64::mk(s2 > s1));
}
DEF_OP(TIME_GTE) { 
  boost::posix_time::ptime s1 = pop_time();
  boost::posix_time::ptime s2 = pop_time();
  stackPush(Val_Int64::mk(s2 >= s1));
}
DEF_OP(TIME_EQ) { 
  boost::posix_time::ptime s1 = pop_time();
  boost::posix_time::ptime s2 = pop_time();
  stackPush(Val_Int64::mk(s2 == s1));
}
DEF_OP(TIME_NOW) { 
  boost::posix_time::ptime t;
  getTime(t);
  stackPush(Val_Time::mk(t));
}
DEF_OP(TIME_MINUS) {
   // Be careful of undefined evaluation order in C++!
   // Note that this returns a Time_Duration!
   boost::posix_time::ptime v1 = pop_time();
   boost::posix_time::ptime v2 = pop_time();
   stackPush(Val_Time_Duration::mk(v2 - v1));
 }

//
// Time_Duration operations.  Note that the '>' and '<' are reversed: think
// about operand order on the stack!
//
DEF_OP(TIME_DURATION_LT) { 
  boost::posix_time::time_duration s1 = pop_time_duration();
  boost::posix_time::time_duration s2 = pop_time_duration();
  stackPush(Val_Int64::mk(s2 < s1));
}
DEF_OP(TIME_DURATION_LTE) { 
  boost::posix_time::time_duration s1 = pop_time_duration();
  boost::posix_time::time_duration s2 = pop_time_duration();
  stackPush(Val_Int64::mk(s2 <= s1));
}
DEF_OP(TIME_DURATION_GT) { 
  boost::posix_time::time_duration s1 = pop_time_duration();
  boost::posix_time::time_duration s2 = pop_time_duration();
  stackPush(Val_Int64::mk(s2 > s1));
}
DEF_OP(TIME_DURATION_GTE) { 
  boost::posix_time::time_duration s1 = pop_time_duration();
  boost::posix_time::time_duration s2 = pop_time_duration();
  stackPush(Val_Int64::mk(s2 >= s1));
}
DEF_OP(TIME_DURATION_EQ) { 
  boost::posix_time::time_duration s1 = pop_time_duration();
  boost::posix_time::time_duration s2 = pop_time_duration();
  stackPush(Val_Int64::mk(s2 == s1));
}
DEF_OP(TIME_DURATION_PLUS) {
   stackPush(Val_Time_Duration::mk(pop_time_duration()+pop_time_duration()));
 }
DEF_OP(TIME_DURATION_MINUS) {
   // Be careful of undefined evaluation order in C++!
   boost::posix_time::time_duration v1 = pop_time_duration();
   boost::posix_time::time_duration v2 = pop_time_duration();
   stackPush(Val_Time_Duration::mk(v2 - v1));
 }

//
// String operations.  Note that the '>' and '<' are reversed: think
// about operand order on the stack!
//
DEF_OP(STR_CAT) { 
  string s1 = pop_string();
  string s2 = pop_string();
  string r = s2 + s1;
  stackPush(Val_Str::mk(r));
}
DEF_OP(STR_LEN) {
  stackPush(Val_Int64::mk(pop_string().length()));
}
DEF_OP(STR_UPPER) {
  std::locale  loc_c("C");
  ToUpper      up(loc_c);
  string s = pop_string();
  string  result;

  std::transform(s.begin(), s.end(), std::back_inserter(result), up);
  stackPush(Val_Str::mk(result));
}
DEF_OP(STR_LOWER) {
  std::locale  loc_c("C");
  ToLower      down(loc_c);
  string s = pop_string();
  string  result;

  std::transform(s.begin(), s.end(), std::back_inserter(result), down);
  stackPush(Val_Str::mk(result));
}
DEF_OP(STR_FIND) {
  // This appears as f_strfind(theStringToLookUp, theStringToLookInto)
  string theStringToLookInto = pop_string();
  string theStringToLookUp = pop_string();
  std::string::size_type pos;
  int64_t result;
  pos = theStringToLookInto.find(theStringToLookUp);
  if (pos == std::string::npos) {
    // Didn't get it
    result = -1;
  } else {
    result = pos;
  }
  stackPush(Val_Int64::mk(result));
}
DEF_OP(STR_SUBSTR) {
  int64_t len = pop_signed();
  int64_t pos = pop_signed();
  string s = pop_string();

  // Sanity check parameters
  if ((pos < 0) ||
      (len <= 0) ||             // no reason to run substr with 0
                                // length!
      (pos + len > s.length())) {
    // Invalid substring request
    error = PE_BAD_STRING_OP;
    return;
  } else {
    stackPush(Val_Str::mk(s.substr(pos, len)));
  }
}
DEF_OP(STR_MATCH) {
  // XXX This is slow!!! For better results, memoize each regexp in a
  // hash map and study each one. 

  boost::regex re(pop_string());
  boost::smatch what;
  if (boost::regex_match(pop_string(),what,re)) {
    stackPush(Val_Int64::mk(true));
  } else {
    stackPush(Val_Int64::mk(false));
  }
}

DEF_OP(STR_REPLACE) {
  ValuePtr first  = stackTop(); stackPop();
  ValuePtr second = stackTop(); stackPop();
  ValuePtr third  = stackTop(); stackPop();

  string pattern = first->toString();
  string replace = second->toString();
  string value   = third->toString();

  for (string::size_type s = 0;
       (s = value.find(pattern, s)) != string::npos; s++)
          value.replace(s, pattern.length(), replace);
  stackPush(Val_Str::mk(value));
}

DEF_OP(STR_CONV) {
  ValuePtr t = stackTop(); stackPop();

  if (t->typeCode() == Value::TUPLE) {
    string expr = compile::namestracker::exprString(Val_Tuple::cast(t));
    stackPush(Val_Str::mk(expr));
  }
  else if (t->typeCode() == Value::LIST) {
    ListPtr list = Val_List::cast(t);
    ostringstream oss;
    for (ValPtrList::const_iterator iter = list->begin();
         iter != list->end(); iter++) {
      ValuePtr v = *iter;
      if (v->typeCode() == Value::TUPLE) {
        oss << compile::namestracker::exprString(Val_Tuple::cast(v));
      }
      else oss << v->toString();
    }
    stackPush(Val_Str::mk(oss.str()));
  }
  else stackPush(Val_Str::mk(t->toString()));
}

//
// Integer arithmetic operations
//
DEF_OP(INT_ABS) {
  stackPush(Val_Int64::mk(llabs(pop_signed())));
}


//
// Floating-point arithmetic operations
//
DEF_OP(DBL_FLOOR) {
  stackPush(Val_Double::mk(floor(pop_double())));
}
DEF_OP(DBL_CEIL) {
  stackPush(Val_Double::mk(ceil(pop_double())));
}


//
// Explicit Type conversions
//
DEF_OP(CONV_I64) {
  stackPush(Val_Int64::mk(pop_signed()));
}
DEF_OP(CONV_STR) {
  stackPush(Val_Str::mk(pop_string()));
}
DEF_OP(CONV_DBL) {
  stackPush(Val_Double::mk(pop_double()));
}
DEF_OP(CONV_TIME) {
  ValuePtr top = stackTop();
  stackPop();
  stackPush(Val_Time::mk(Val_Time::cast(top)));
}
DEF_OP(CONV_ID) {
  ValuePtr top = stackTop();
  stackPop();
  stackPush(Val_ID::mk(Val_ID::cast(top)));
}
DEF_OP(CONV_TIME_DURATION) {
  ValuePtr top = stackTop();
  stackPop();
  stackPush(Val_Time_Duration::mk(Val_Time_Duration::cast(top)));
}

//
// Extra hacks for Symphony...
//
DEF_OP(EXP) {
  stackPush(Val_Double::mk(exp(pop_double())));
}
DEF_OP(LN) {
  stackPush(Val_Double::mk(log(pop_double())));
}
DEF_OP(DRAND48) {
  stackPush(Val_Double::mk(drand48()));
}

// Extra hacks for System R
DEF_OP(O_STATUS) {
  ValuePtr second = stackTop(); stackPop();
  ValuePtr first  = stackTop(); stackPop();
}

DEF_OP(O_SELECT) {
  ValuePtr third  = stackTop(); stackPop();
  ValuePtr second = stackTop(); stackPop();
  ValuePtr first  = stackTop(); stackPop();
}

DEF_OP(O_RANGEAM) {
  ValuePtr second = stackTop(); stackPop();
  ValuePtr first = stackTop(); stackPop();
}

DEF_OP(O_FILTER) {
  ValuePtr first = stackTop(); stackPop();
  ValuePtr second = stackTop(); stackPop();
  ListPtr schema = Val_List::cast(first);

  bool f = compile::namestracker::filter(schema, second);
  stackPush(Val_Int64::mk(f));
}






REMOVABLE_INLINE ValuePtr
Pel_VM::stackTop()
{
  return _st.back();
}


REMOVABLE_INLINE ValuePtr
Pel_VM::stackPeek(unsigned p)
{
  return _st[_st.size() - 1 - p];
}


REMOVABLE_INLINE void
Pel_VM::stackPop()
{
  _st.pop_back();
}


REMOVABLE_INLINE void
Pel_VM::stackPush(ValuePtr v)
{
  _st.push_back(v);
}

/*
 * End of file 
 */
