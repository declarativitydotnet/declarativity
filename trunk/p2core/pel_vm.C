/*
 * @(#)$Id$
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
#include <iterator>    // for back_inserter
#include <locale>
#include <string>
#include <algorithm>
#include <cctype>      // old <ctype.h>

#include <stdlib.h>
#include <math.h>
#include <boost/regex.hpp>

#include <openssl/sha.h>


#include "val_int32.h"
#include "val_uint32.h"
#include "val_int64.h"
#include "val_uint64.h"
#include "val_str.h"
#include "val_double.h"
#include "val_null.h"
#include "val_tuple.h"
#include "val_time.h"
#include "val_id.h"
#include "oper.h"
#include "loop.h"

using namespace opr;

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
void Pel_VM::reset() 
{
  while (!_st.empty()) {
    stackPop();
  }
}

void Pel_VM::stop()
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
uint64_t Pel_VM::pop_unsigned() 
{
  ValuePtr t = stackTop(); stackPop();
  try {
    return Val_UInt64::cast(t);
  } catch (Value::TypeError) {
    error = PE_TYPE_CONVERSION;
    return 0;
  }
}

//
// Type conversion to an unsigned number with no checkng
//
int64_t Pel_VM::pop_signed() 
{
  ValuePtr t = stackTop(); stackPop();
  try {
    return Val_Int64::cast(t);
  } catch (Value::TypeError) {
    error = PE_TYPE_CONVERSION;
    return 0;
  }
}

//
// Pull a string off the stack
//  
string Pel_VM::pop_string() 
{
  ValuePtr t = stackTop();
  stackPop();
  try {
    return Val_Str::cast(t);
  } catch (Value::TypeError) {
    error = PE_TYPE_CONVERSION;
    return "";
  }
}

//
// Pull a time off the stack
//  
boost::posix_time::ptime Pel_VM::pop_time() 
{
  ValuePtr t = stackTop(); stackPop();
  try {
    return Val_Time::cast(t);
  } catch (Value::TypeError) {
    error = PE_TYPE_CONVERSION;
    boost::posix_time::ptime t;
    return t;
  }
}

//
// Pull a time_duration off the stack
//  
boost::posix_time::time_duration Pel_VM::pop_time_duration() 
{
  ValuePtr t = stackTop(); stackPop();
  try {
    return Val_Time_Duration::cast(t);
  } catch (Value::TypeError) {
    error = PE_TYPE_CONVERSION;
    boost::posix_time::time_duration t;
    return t;
  }
}

//
// Pull an ID off the stack
//  
IDPtr Pel_VM::pop_ID() 
{
  ValuePtr t = stackTop(); stackPop();
  try {
    return Val_ID::cast(t);
  } catch (Value::TypeError) {
    error = PE_TYPE_CONVERSION;
    return ID::mk();
  }
}

//
// Pull a double off the stack
//  
double Pel_VM::pop_double() 
{
  ValuePtr t = stackTop(); stackPop();
  try { 
    return Val_Double::cast(t);
  } catch (Value::TypeError) {
    error = PE_TYPE_CONVERSION;
    return 0.0;
  }
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
    warn << "Stack entry[" << message << "]: " << (*i)->toString() << "\n";
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
    // This is a somewhat esoteric bit of C++.  Believe it or not,
    // jump_table[op].fn is a pointer to a member function.
    // Consequently, "this->*" dereferences it with respect to the
    // "this" (i.e., the VM we're in), meaning that we can invoke it
    // as a member. 
    (this->*(jump_table[op].fn))(inst);
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
  uint stackPosition = pop_unsigned();
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
  int64_t ifVal = pop_unsigned();
  if (ifVal) {
    stackPush(thenVal);
  } else {
    stackPush(elseVal);
  }
}
DEF_OP(IFPOP) {
  ValuePtr thenVal = stackTop(); stackPop();
  int64_t ifVal = pop_unsigned();
  if (ifVal) {
    if (!result) {
      result = Tuple::mk();
    }
    result->append(thenVal);
  }
}
DEF_OP(IFSTOP) {
  int64_t ifVal = pop_unsigned();
  if (ifVal) {
    stop();
    //    warn << "IF stop of " << ifVal << ".  Stopping!!!\n";
  } else {
    //    warn << "IF stop of " << ifVal << ".  Not stopping\n";
  }
}
DEF_OP(DUMPSTACK) {
  string s1 = pop_string();
  dumpStack(s1);
}
DEF_OP(IFPOP_TUPLE) {
  int64_t ifVal = pop_unsigned();
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
  unsigned field = pop_unsigned();
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

/**
 * List operations
 *
 */

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

DEF_OP(L_MEMBER) {
   ValuePtr value = stackTop(); stackPop();
   ValuePtr listVal = stackTop();
   ListPtr list = Val_List::cast(listVal);
   
   int isMember = list->member(value);
   stackPush(Val_Int32::mk(isMember));
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


//
// Boolean operations
//
DEF_OP(NOT) {
  u_int64_t v = pop_unsigned();
  //  warn << "NOT of " << v << " is " << !v << "\n";
  stackPush(Val_Int32::mk(!v));
}
DEF_OP(AND) {
  u_int64_t v2 = pop_unsigned();
  u_int64_t v1 = pop_unsigned();
  stackPush(Val_Int32::mk(v1 && v2));
}
DEF_OP(OR) {
  u_int64_t v2 = pop_unsigned();
  u_int64_t v1 = pop_unsigned();
  stackPush(Val_Int32::mk(v1 || v2));
}
DEF_OP(RAND) {
  int32_t r = random();
  stackPush(Val_Int32::mk(r));
}
DEF_OP(COIN) {
  double r = double(random()) / double(RAND_MAX);
  double p = pop_double();
  stackPush(Val_Int32::mk(r <= p));
}

DEF_OP(INITLIST) {
  ValuePtr second = stackTop(); stackPop();
  ValuePtr first = stackTop(); stackPop();
  TuplePtr newTuple = Tuple::mk();
  newTuple->append(first);
  newTuple->append(second);
  newTuple->freeze();
  stackPush(Val_Tuple::mk(newTuple));
}

DEF_OP(CONSLIST) {
  ValuePtr second = stackTop(); stackPop();
  ValuePtr first = stackTop(); stackPop();
  
  TuplePtr firstTuple, secondTuple;
  // make each argument a tuple
  if (string(first->typeName()) == "tuple") {
    firstTuple = Val_Tuple::cast(first);
  } else {
    firstTuple = Tuple::mk();
    firstTuple->append(first);
  }
  if (string(second->typeName()) == "tuple") {
    secondTuple = Val_Tuple::cast(second);
  } else {
    secondTuple = Tuple::mk();
    secondTuple->append(second);
  }

  // combine the two
  TuplePtr combinedTuple = Tuple::mk();
  combinedTuple->concat(firstTuple);
  combinedTuple->concat(secondTuple);
  combinedTuple->freeze();
  stackPush(Val_Tuple::mk(combinedTuple));
}

DEF_OP(INLIST) {
  ValuePtr second = stackTop(); stackPop();
  ValuePtr first = stackTop(); stackPop();

  TuplePtr curTuple= Val_Tuple::cast(first);

  bool flag = false;
  for (unsigned k = 0; k < curTuple->size(); k++) {
    if ((*curTuple)[k]->compareTo(second) == 0) {
      flag = true;
    }
  }
  if (flag == true) {
    stackPush(Val_Int32::mk(1));
  } else {
    stackPush(Val_Int32::mk(0));
  } 
}

DEF_OP(REMOVELAST) {
  ValuePtr first = stackTop(); stackPop();

  TuplePtr curTuple= Val_Tuple::cast(first);

  TuplePtr newTuple = Tuple::mk();
  for (unsigned k = 0; k < curTuple->size()-1; k++) {
    newTuple->append(Val_Str::mk((*curTuple)[k]->toString()));
  }
  newTuple->freeze();
  stackPush(Val_Tuple::mk(newTuple));
}

DEF_OP(LAST) {
  ValuePtr first = stackTop(); stackPop();
  TuplePtr curTuple= Val_Tuple::cast(first);
  warn << "Get Last: " << curTuple->toString() << "\n";
  stackPush(Val_Str::mk((*curTuple)[curTuple->size()-1]->toString()));
}

DEF_OP(SIZE) {
  ValuePtr first = stackTop(); stackPop();
  TuplePtr curTuple= Val_Tuple::cast(first);
  stackPush(Val_Int32::mk(curTuple->size()));
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
  try {
    ValuePtr neg = Val_Int64::mk(-1);
    stackPush(pop() * neg);
  } catch (opr::Oper::OperException e) {
    error = PE_OPER_UNSUP;
  }
}
DEF_OP(PLUS) {
  ValuePtr v2 = pop();
  ValuePtr v1 = pop();
  try {
    ValuePtr r = v1 + v2;
    stackPush(r);
  } catch (opr::Oper::OperException e) {
    error = PE_OPER_UNSUP;
  }
}
DEF_OP(MINUS) {
  // Be careful of undefined evaluation order in C++!
  ValuePtr v1 = pop();
  ValuePtr v2 = pop();
  try {
    stackPush(v2 - v1);
  } catch (opr::Oper::OperException e) {
    error = PE_OPER_UNSUP;
  }
}
DEF_OP(MINUSMINUS) {
  ValuePtr v1 = pop();
  try {
    stackPush(--v1);
  } catch (opr::Oper::OperException e) {
    error = PE_OPER_UNSUP;
  }
}
DEF_OP(PLUSPLUS) {
  ValuePtr v1 = pop();
  try {
    stackPush(++v1);
  } catch (opr::Oper::OperException e) {
    error = PE_OPER_UNSUP;
  }
}
DEF_OP(MUL) {
  ValuePtr v2 = pop();
  ValuePtr v1 = pop();
  try {
    stackPush(v1 * v2);
  } catch (opr::Oper::OperException e) {
    error = PE_OPER_UNSUP;
  }
}
DEF_OP(DIV) {
  // Be careful of undefined evaluation order in C++!
  ValuePtr v1 = pop();
  ValuePtr v2 = pop();
  if (v1 != Val_UInt64::mk(0)) { 
    try {
      stackPush((v2 / v1));
    } catch (opr::Oper::OperException e) {
      error = PE_OPER_UNSUP;
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
  if (v1 != Val_UInt64::mk(0)) { 
    try {
      stackPush((v2 % v1));
    } catch (opr::Oper::OperException e) {
      error = PE_OPER_UNSUP;
    }
  } else if (error == PE_SUCCESS) {
    error = PE_DIVIDE_BY_ZERO;
  }
}


//
// Comparison operators
//
DEF_OP(EQ) {
  ValuePtr v1 = pop();
  ValuePtr v2 = pop();
  stackPush(Val_Int32::mk(v2 == v1));
}
DEF_OP(LT) { 
  ValuePtr v1 = pop();
  ValuePtr v2 = pop();
  stackPush(Val_Int32::mk(v2 > v1)); 
}
DEF_OP(LTE) { 
  ValuePtr v1 = pop();
  ValuePtr v2 = pop();
  stackPush(Val_Int32::mk(v2 >= v1)); 
}
DEF_OP(GT) { 
  ValuePtr v1 = pop();
  ValuePtr v2 = pop();
  stackPush(Val_Int32::mk(v2 < v1)); 
}
DEF_OP(GTE) { 
  ValuePtr v1 = pop();
  ValuePtr v2 = pop();
  stackPush(Val_Int32::mk(v2 <= v1)); 
}

//
// IN Operator
//
DEF_OP(INOO) {
  ValuePtr to   = pop();
  ValuePtr from = pop();
  ValuePtr key  = pop();
  stackPush(Val_Int32::mk(inOO(key, from, to)));
}
DEF_OP(INOC) {
  ValuePtr to   = pop();
  ValuePtr from = pop();
  ValuePtr key  = pop();
  stackPush(Val_Int32::mk(inOC(key, from, to)));
}
DEF_OP(INCO) {
  ValuePtr to   = pop();
  ValuePtr from = pop();
  ValuePtr key  = pop();
  stackPush(Val_Int32::mk(inCO(key, from, to)));
}
DEF_OP(INCC) {
  ValuePtr to   = pop();
  ValuePtr from = pop();
  ValuePtr key  = pop();
  stackPush(Val_Int32::mk(inCC(key, from, to)));
}

//
// Time operations.  Note that the '>' and '<' are reversed: think
// about operand order on the stack!
//
DEF_OP(TIME_LT) { 
  boost::posix_time::ptime s1 = pop_time();
  boost::posix_time::ptime s2 = pop_time();
  stackPush(Val_Int32::mk(s2 < s1));
}
DEF_OP(TIME_LTE) { 
  boost::posix_time::ptime s1 = pop_time();
  boost::posix_time::ptime s2 = pop_time();
  stackPush(Val_Int32::mk(s2 <= s1));
}
DEF_OP(TIME_GT) { 
  boost::posix_time::ptime s1 = pop_time();
  boost::posix_time::ptime s2 = pop_time();
  stackPush(Val_Int32::mk(s2 > s1));
}
DEF_OP(TIME_GTE) { 
  boost::posix_time::ptime s1 = pop_time();
  boost::posix_time::ptime s2 = pop_time();
  stackPush(Val_Int32::mk(s2 >= s1));
}
DEF_OP(TIME_EQ) { 
  boost::posix_time::ptime s1 = pop_time();
  boost::posix_time::ptime s2 = pop_time();
  stackPush(Val_Int32::mk(s2 == s1));
}
DEF_OP(TIME_NOW) { 
  boost::posix_time::ptime t;
  getTime(t);
  stackPush(Val_Time::mk(t));
}

//
// Time_Duration operations.  Note that the '>' and '<' are reversed: think
// about operand order on the stack!
//
DEF_OP(TIME_DURATION_LT) { 
  boost::posix_time::time_duration s1 = pop_time_duration();
  boost::posix_time::time_duration s2 = pop_time_duration();
  stackPush(Val_Int32::mk(s2 < s1));
}
DEF_OP(TIME_DURATION_LTE) { 
  boost::posix_time::time_duration s1 = pop_time_duration();
  boost::posix_time::time_duration s2 = pop_time_duration();
  stackPush(Val_Int32::mk(s2 <= s1));
}
DEF_OP(TIME_DURATION_GT) { 
  boost::posix_time::time_duration s1 = pop_time_duration();
  boost::posix_time::time_duration s2 = pop_time_duration();
  stackPush(Val_Int32::mk(s2 > s1));
}
DEF_OP(TIME_DURATION_GTE) { 
  boost::posix_time::time_duration s1 = pop_time_duration();
  boost::posix_time::time_duration s2 = pop_time_duration();
  stackPush(Val_Int32::mk(s2 >= s1));
}
DEF_OP(TIME_DURATION_EQ) { 
  boost::posix_time::time_duration s1 = pop_time_duration();
  boost::posix_time::time_duration s2 = pop_time_duration();
  stackPush(Val_Int32::mk(s2 == s1));
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
// ID operations.  Note that the '>' and '<' are reversed: think
// about operand order on the stack!
//
DEF_OP(ID_LT) { 
  IDPtr s1 = pop_ID();
  IDPtr s2 = pop_ID();
  stackPush(Val_Int32::mk(s2->compareTo(s1) < 0));
}
DEF_OP(ID_LTE) { 
  IDPtr s1 = pop_ID();
  IDPtr s2 = pop_ID();
  stackPush(Val_Int32::mk(s2->compareTo(s1) <= 0));
}
DEF_OP(ID_GT) { 
  IDPtr s1 = pop_ID();
  IDPtr s2 = pop_ID();
  stackPush(Val_Int32::mk(s2->compareTo(s1) > 0));
}
DEF_OP(ID_GTE) { 
  IDPtr s1 = pop_ID();
  IDPtr s2 = pop_ID();
  stackPush(Val_Int32::mk(s2->compareTo(s1) >= 0));
}
DEF_OP(ID_EQ) { 
  IDPtr s1 = pop_ID();
  IDPtr s2 = pop_ID();
  stackPush(Val_Int32::mk(s1->equals(s2)));
}
DEF_OP(ID_PLUS) {
  stackPush(Val_ID::mk(pop_ID()->add(pop_ID())));
}
DEF_OP(ID_MINUSMINUS) {
  stackPush(Val_ID::mk(ID::ONE->distance(pop_ID())));
}
DEF_OP(ID_LSL) {
  uint32_t shift = pop_unsigned();
  IDPtr id = pop_ID();
  //warn << "Left shift " << shift << " " << id->toString() << " " << id->shift(shift)->toString() << "\n";
  stackPush(Val_ID::mk(id->shift(shift)));
}
DEF_OP(ID_DIST) {
  // Be careful of undefined evaluation order in C++!
  IDPtr v1 = pop_ID();
  IDPtr v2 = pop_ID();
  //warn << "Distance(" << v2->toString() << " to " << v1->toString() << "=" << v2->distance(v1)->toString() << "\n";
  stackPush(Val_ID::mk(v2->distance(v1)));
}
DEF_OP(ID_BTWOO) {
  IDPtr to = pop_ID();
  IDPtr from = pop_ID();
  IDPtr key = pop_ID();
  //  warn << key->toString() << "(" << from->toString() << "," << to->toString() << ") :" << key->betweenOO(from, to) << "\n";
  stackPush(Val_Int32::mk(key->betweenOO(from, to)));
}
DEF_OP(ID_BTWOC) {
  IDPtr to = pop_ID();
  IDPtr from = pop_ID();
  IDPtr key = pop_ID();
  //  warn << key->toString() << "(" << from->toString() << "," << to->toString() << "] :" << key->betweenOC(from, to) << "\n";
  stackPush(Val_Int32::mk(key->betweenOC(from, to)));
}
DEF_OP(ID_BTWCO) {
  IDPtr to = pop_ID();
  IDPtr from = pop_ID();
  IDPtr key = pop_ID();
  stackPush(Val_Int32::mk(key->betweenCO(from, to)));
}
DEF_OP(ID_BTWCC) {
  IDPtr to = pop_ID();
  IDPtr from = pop_ID();
  IDPtr key = pop_ID();
  stackPush(Val_Int32::mk(key->betweenCC(from, to)));
}

//
// String operations.  Note that the '>' and '<' are reversed: think
// about operand order on the stack!
//
DEF_OP(STR_LT) { 
  string s1 = pop_string();
  string s2 = pop_string();
  stackPush(Val_Int32::mk(s2 < s1));
}
DEF_OP(STR_LTE) { 
  string s1 = pop_string();
  string s2 = pop_string();
  stackPush(Val_Int32::mk(s2 <= s1));
}
DEF_OP(STR_GT) { 
  string s1 = pop_string();
  string s2 = pop_string();
  stackPush(Val_Int32::mk(s2 > s1));
}
DEF_OP(STR_GTE) { 
  string s1 = pop_string();
  string s2 = pop_string();
  stackPush(Val_Int32::mk(s2 >= s1));
}
DEF_OP(STR_EQ) { 
  string s1 = pop_string();
  string s2 = pop_string();
  //  warn << s1 << "==?" << s2 << " is " << (s1==s2) << "\n";
  stackPush(Val_Int32::mk(s2 == s1));
}
DEF_OP(STR_CAT) { 
  string s1 = pop_string();
  string s2 = pop_string();
  string r = s2 + s1;
  stackPush(Val_Str::mk(r));
}
DEF_OP(STR_LEN) {
  stackPush(Val_UInt32::mk(pop_string().length()));
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
DEF_OP(STR_SUBSTR) {
  uint len = pop_unsigned();
  uint pos = pop_unsigned();
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
    stackPush(Val_Int32::mk(true));
  } else {
    stackPush(Val_Int32::mk(false));
  }
}
DEF_OP(STR_CONV) {
  ValuePtr t = stackTop(); stackPop();
  stackPush(Val_Str::mk(t->toString()));
}

//
// Integer arithmetic operations
//
DEF_OP(INT_NEG) {
  stackPush(Val_Int64::mk(-pop_signed()));
}
DEF_OP(INT_PLUS) {
  stackPush(Val_Int64::mk(pop_signed()+pop_signed()));
}
DEF_OP(INT_MINUS) {
  // Be careful of undefined evaluation order in C++!
  int64_t v1 = pop_signed();
  int64_t v2 = pop_signed();
  stackPush(Val_Int64::mk(v2 - v1));
}
DEF_OP(INT_MUL) {
  stackPush(Val_Int64::mk(pop_signed()*pop_signed()));
}
DEF_OP(INT_DIV) {
  // Be careful of undefined evaluation order in C++!
  int64_t v1 = pop_signed();
  int64_t v2 = pop_signed();
  if (v1) { 
    stackPush(Val_Int64::mk(v2 / v1));
  } else if (error == PE_SUCCESS) {
    error = PE_DIVIDE_BY_ZERO;
  }
}
DEF_OP(INT_ABS) {
  stackPush(Val_Int64::mk(llabs(pop_signed())));
}
DEF_OP(INT_EQ) {
  stackPush(Val_Int32::mk(pop_signed() == pop_signed()));
}
DEF_OP(INT_LT) { 
  stackPush(Val_Int32::mk(pop_signed() > pop_signed())); 
}
DEF_OP(INT_LTE) { 
  stackPush(Val_Int32::mk(pop_signed() >= pop_signed())); 
}
DEF_OP(INT_GT) { 
  stackPush(Val_Int32::mk(pop_signed() < pop_signed())); 
}
DEF_OP(INT_GTE) { 
  stackPush(Val_Int32::mk(pop_signed() <= pop_signed())); 
}


//
// Floating-point arithmetic operations
//
DEF_OP(DBL_NEG) {
  stackPush(Val_Double::mk(-pop_double()));
}
DEF_OP(DBL_PLUS) {
  stackPush(Val_Double::mk(pop_double()+pop_double()));
}
DEF_OP(DBL_MINUS) {
  // Be careful of undefined evaluation order in C++!
  double v1 = pop_double();
  double v2 = pop_double();
  stackPush(Val_Double::mk(v2 - v1));
}
DEF_OP(DBL_MUL) {
  stackPush(Val_Double::mk(pop_double()*pop_double()));
}
DEF_OP(DBL_DIV) {
  // Be careful of undefined evaluation order in C++!
  double v1 = pop_double();
  double v2 = pop_double();
  stackPush(Val_Double::mk(v2 / v1));
}
DEF_OP(DBL_EQ) {
  stackPush(Val_Int32::mk(pop_double() == pop_double()));
}
DEF_OP(DBL_LT) { 
  stackPush(Val_Int32::mk(pop_double() > pop_double())); 
}
DEF_OP(DBL_LTE) { 
  stackPush(Val_Int32::mk(pop_double() >= pop_double())); 
}
DEF_OP(DBL_GT) { 
  stackPush(Val_Int32::mk(pop_double() < pop_double())); 
}
DEF_OP(DBL_GTE) { 
  stackPush(Val_Int32::mk(pop_double() <= pop_double())); 
}
DEF_OP(DBL_FLOOR) {
  stackPush(Val_Double::mk(floor(pop_double())));
}
DEF_OP(DBL_CEIL) {
  stackPush(Val_Double::mk(ceil(pop_double())));
}

//
// Explicit Type conversions
//
DEF_OP(CONV_I32) {
  stackPush(Val_Int32::mk(pop_signed()));
}
DEF_OP(CONV_U32) {
  stackPush(Val_UInt32::mk(pop_unsigned()));
}  
DEF_OP(CONV_I64) {
  stackPush(Val_Int64::mk(pop_signed()));
}
DEF_OP(CONV_U64) {
  stackPush(Val_UInt64::mk(pop_unsigned()));
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


/*
 * End of file 
 */
