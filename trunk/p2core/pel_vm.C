/*
 * @(#)$Id$
 *
 * Copyright (c) 2005 Intel Corporation. All rights reserved.
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: PEL (P2 Expression Language) virtual machine
 *
 */

#include "pel_vm.h"
#include <stdlib.h>
#include <math.h>
#include <rxx.h>

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

using namespace opr;

typedef void(Pel_VM::*Op_fn_t)(u_int32_t);

struct JumpTableEnt_t {
  char *opcode;
  int	arity;
  Op_fn_t fn;
};

const char *Pel_VM::err_msgs[] = {
  "Success",
  "Out-of-range constant reference",
  "Out-of-range field reference",
  "Operator not supported for type",
  "Stack underflow",
  "Bad type conversion",
  "Bad opcode",
  "Divide by zero",
  "Invalid Errno",
  "Unknown Error"
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
Pel_VM::Pel_VM(std::deque< ValueRef > staque) : _st(staque) {
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
ValueRef Pel_VM::result_val()
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
  result = NULL;
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
// Type conversion to an unsigned number with no checkng
//
ValueRef Pel_VM::pop() 
{
  ValueRef t = stackTop(); stackPop();
  return t;
}

//
// Type conversion to an unsigned number with no checkng
//
uint64_t Pel_VM::pop_unsigned() 
{
  ValueRef t = stackTop(); stackPop();
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
  ValueRef t = stackTop(); stackPop();
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
str Pel_VM::pop_string() 
{
  ValueRef t = stackTop(); stackPop();
  try {
    return Val_Str::cast(t);
  } catch (Value::TypeError) {
    error = PE_TYPE_CONVERSION;
    return str("");
  }
}

//
// Pull a time off the stack
//  
struct timespec Pel_VM::pop_time() 
{
  ValueRef t = stackTop(); stackPop();
  try {
    return Val_Time::cast(t);
  } catch (Value::TypeError) {
    error = PE_TYPE_CONVERSION;
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = 0;
    return t;
  }
}

//
// Pull an ID off the stack
//  
IDRef Pel_VM::pop_ID() 
{
  ValueRef t = stackTop(); stackPop();
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
  ValueRef t = stackTop(); stackPop();
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
Pel_VM::Error Pel_VM::execute(const Pel_Program &prog, const TupleRef data)
{
  prg = &prog;
  error = PE_SUCCESS;
  pc = 0;
  result = NULL;
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

void Pel_VM::dumpStack(str message)
{
  
  // Dump the stack
  for (std::deque<ValueRef>::reverse_iterator i = _st.rbegin();
       i != _st.rend();
       i++) {
    warn << "Stack entry[" << message << "]: " << (*i)->toString() << "\n";
  }
}

//
// Execute a single instruction
//
Pel_VM::Error Pel_VM::execute_instruction( u_int32_t inst, TupleRef data)
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
 * Opcode definitions follow
 * 
 * Since the jumptable contains the operation arity, we don't need to
 * check here that there are sufficient operands on the
 * stack. However, we do need to check their types at this stage. 
 *
 */

//
// Stack operations
//
DEF_OP(DROP) { stackPop(); }
DEF_OP(SWAP) { 
  ValueRef t1 = stackTop(); stackPop();
  ValueRef t2 = stackTop(); stackPop();
  stackPush(t1); 
  stackPush(t2);
}
DEF_OP(DUP) { 
  stackPush(stackTop()); 
}
DEF_OP(PUSH_CONST) { 
  uint ndx = (inst >> 16);
  if (ndx > prg->const_pool.size() ) { error = PE_BAD_CONSTANT; return; }
  stackPush(prg->const_pool[ndx]);
}
DEF_OP(PUSH_FIELD) { 
  uint ndx = (inst >> 16);
  if (ndx > operand->size() ) { error = PE_BAD_FIELD; return; }
  stackPush((*operand)[ndx]);
}
DEF_OP(POP) {
  if (!result) { result = Tuple::mk(); }
  ValueRef top = stackTop(); stackPop();
  if (top->typeCode() == Value::TUPLE) {
    // Freeze it before taking it out
    Val_Tuple::cast(top)->freeze();
  }
  result->append(top);
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
  ValueRef elseVal = stackTop(); stackPop();
  ValueRef thenVal = stackTop(); stackPop();
  int64_t ifVal = pop_unsigned();
  if (ifVal) {
    stackPush(thenVal);
  } else {
    stackPush(elseVal);
  }
}
DEF_OP(IFPOP) {
  ValueRef thenVal = stackTop(); stackPop();
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
  str s1 = pop_string();
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
  ValueRef val = stackTop(); stackPop();
  TupleRef t = Tuple::mk();
  t->append(val);
  ValueRef tuple = Val_Tuple::mk(t);
  stackPush(tuple);
}
DEF_OP(T_APPEND) {
  ValueRef value = stackTop(); stackPop();
  ValueRef tuple = stackTop();
  TupleRef t = Val_Tuple::cast(tuple);
  t->append(value);
}
DEF_OP(T_UNBOX) {
  ValueRef tuple = stackTop(); stackPop();
  TupleRef t = Val_Tuple::cast(tuple);
  // Now start pushing fields from the end forwards
  for (int i = t->size() - 1;
       i >= 0;
       i--) {
    stackPush((*t)[i]);
  }
}
DEF_OP(T_UNBOXPOP) {
  ValueRef tuple = stackTop(); stackPop();
  TupleRef t = Val_Tuple::cast(tuple);
  // Now start popping fields from front out
  if (!result) {
    result = Tuple::mk();
  }
  for (size_t i = 0;
       i < t->size();
       i++) {
    result->append((*t)[i]);
  }
}
DEF_OP(T_FIELD) {
  unsigned field = pop_unsigned();
  ValueRef tuple = stackTop(); stackPop();
  TupleRef theTuple = Val_Tuple::cast(tuple);
  ValuePtr value = (*theTuple)[field];
  if (value == NULL) {
    stackPush(Val_Null::mk());
  } else {
    stackPush(value);
  }
}
DEF_OP(T_SWALLOW) { 
  ValueRef swallowed = Val_Tuple::mk(operand);
  stackPush(swallowed);
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
  u_int64_t v1 = pop_unsigned();
  u_int64_t v2 = pop_unsigned();
  stackPush(Val_Int32::mk(v1 && v2));
}
DEF_OP(OR) {
  u_int64_t v1 = pop_unsigned();
  u_int64_t v2 = pop_unsigned();
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
  ValueRef second = stackTop(); stackPop();
  ValueRef first = stackTop(); stackPop();
  TupleRef newTuple = Tuple::mk();
  newTuple->append(first);
  newTuple->append(second);
  newTuple->freeze();
  stackPush(Val_Tuple::mk(newTuple));
}

DEF_OP(CONSLIST) {
  ValueRef second = stackTop(); stackPop();
  ValueRef first = stackTop(); stackPop();
  
  TuplePtr firstTuple, secondTuple;
  // make each argument a tuple
  if (str(first->typeName()) == "tuple") {
    firstTuple = Val_Tuple::cast(first);
  } else {
    firstTuple = Tuple::mk();
    firstTuple->append(first);
  }
  if (str(second->typeName()) == "tuple") {
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

//
// Integer-only arithmetic operations (mostly bitwise)
//
DEF_OP(ASR) {
  ValueRef v1 = pop();
  ValueRef v2 = pop();
  stackPush((v2 >> v1));
}
DEF_OP(ASL) {
  ValueRef v1 = pop();
  ValueRef v2 = pop();
  stackPush((v2 << v1));
}
DEF_OP(BIT_AND) {
  stackPush((pop() & pop()));
}
DEF_OP(BIT_OR) {
  stackPush((pop() | pop()));
}
DEF_OP(BIT_XOR) {
  stackPush((pop() ^ pop()));
}
DEF_OP(BIT_NOT) {
  stackPush((~pop()));
}

//
// arithmetic operations
//
DEF_OP(NEG) {
  try {
    ValueRef neg = Val_Int32::mk(-1);
    stackPush((neg * pop()));
  } catch (opr::Oper::Exception *e) {
    error = PE_OPER_UNSUP;
  }
}
DEF_OP(PLUS) {
  try {
    stackPush((pop()+pop()));
  } catch (opr::Oper::Exception *e) {
    error = PE_OPER_UNSUP;
  }
}
DEF_OP(MINUS) {
  // Be careful of undefined evaluation order in C++!
  ValueRef v1 = pop();
  ValueRef v2 = pop();
  try {
    stackPush((v2-v1));
  } catch (opr::Oper::Exception *e) {
    error = PE_OPER_UNSUP;
  }
}
DEF_OP(MINUSMINUS) {
  ValueRef v1 = pop();
  try {
    stackPush(--v1);
  } catch (opr::Oper::Exception *e) {
    error = PE_OPER_UNSUP;
  }
}
DEF_OP(PLUSPLUS) {
  ValueRef v1 = pop();
  try {
    stackPush(++v1);
  } catch (opr::Oper::Exception *e) {
    error = PE_OPER_UNSUP;
  }
}
DEF_OP(MUL) {
  try {
    stackPush((pop()*pop()));
  } catch (opr::Oper::Exception *e) {
    error = PE_OPER_UNSUP;
  }
}
DEF_OP(DIV) {
  // Be careful of undefined evaluation order in C++!
  ValueRef v1 = pop();
  ValueRef v2 = pop();
  if (v1 != Val_UInt64::mk(0)) { 
    try {
      stackPush((v2 / v1));
    } catch (opr::Oper::Exception *e) {
      error = PE_OPER_UNSUP;
    }
  } else if (error == PE_SUCCESS) {
    error = PE_DIVIDE_BY_ZERO;
  }
}
DEF_OP(MOD) {
  // Be careful of undefined evaluation order in C++!
  ValueRef v1 = pop();
  ValueRef v2 = pop();
  if (v1 != Val_UInt64::mk(0)) { 
    try {
      stackPush((v2 % v1));
    } catch (opr::Oper::Exception *e) {
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
  stackPush(Val_Int32::mk(pop() == pop()));
}
DEF_OP(LT) { 
  stackPush(Val_Int32::mk(pop() > pop())); 
}
DEF_OP(LTE) { 
  stackPush(Val_Int32::mk(pop() >= pop())); 
}
DEF_OP(GT) { 
  stackPush(Val_Int32::mk(pop() < pop())); 
}
DEF_OP(GTE) { 
  stackPush(Val_Int32::mk(pop() <= pop())); 
}

//
// IN Operator
//
DEF_OP(INOO) {
  ValueRef to   = pop();
  ValueRef from = pop();
  ValueRef key  = pop();
  stackPush(Val_Int32::mk(inOO(key, from, to)));
}
DEF_OP(INOC) {
  ValueRef to   = pop();
  ValueRef from = pop();
  ValueRef key  = pop();
  stackPush(Val_Int32::mk(inOC(key, from, to)));
}
DEF_OP(INCO) {
  ValueRef to   = pop();
  ValueRef from = pop();
  ValueRef key  = pop();
  stackPush(Val_Int32::mk(inCO(key, from, to)));
}
DEF_OP(INCC) {
  ValueRef to   = pop();
  ValueRef from = pop();
  ValueRef key  = pop();
  stackPush(Val_Int32::mk(inCC(key, from, to)));
}

//
// Time operations.  Note that the '>' and '<' are reversed: think
// about operand order on the stack!
//
DEF_OP(TIME_LT) { 
  struct timespec s1 = pop_time();
  struct timespec s2 = pop_time();
  stackPush(Val_Int32::mk(s2 < s1));
}
DEF_OP(TIME_LTE) { 
  struct timespec s1 = pop_time();
  struct timespec s2 = pop_time();
  stackPush(Val_Int32::mk(s2 <= s1));
}
DEF_OP(TIME_GT) { 
  struct timespec s1 = pop_time();
  struct timespec s2 = pop_time();
  stackPush(Val_Int32::mk(s2 > s1));
}
DEF_OP(TIME_GTE) { 
  struct timespec s1 = pop_time();
  struct timespec s2 = pop_time();
  stackPush(Val_Int32::mk(s2 >= s1));
}
DEF_OP(TIME_EQ) { 
  struct timespec s1 = pop_time();
  struct timespec s2 = pop_time();
  stackPush(Val_Int32::mk(s2 == s1));
}
DEF_OP(TIME_NOW) { 
  struct timespec t;
  clock_gettime(CLOCK_REALTIME, &t);
  stackPush(Val_Time::mk(t));
}
DEF_OP(TIME_PLUS) {
  stackPush(Val_Time::mk(pop_time()+pop_time()));
}
DEF_OP(TIME_MINUS) {
  // Be careful of undefined evaluation order in C++!
  struct timespec v1 = pop_time();
  struct timespec v2 = pop_time();
  stackPush(Val_Time::mk(v2-v1));
}

//
// ID operations.  Note that the '>' and '<' are reversed: think
// about operand order on the stack!
//
DEF_OP(ID_LT) { 
  IDRef s1 = pop_ID();
  IDRef s2 = pop_ID();
  stackPush(Val_Int32::mk(s2->compareTo(s1) < 0));
}
DEF_OP(ID_LTE) { 
  IDRef s1 = pop_ID();
  IDRef s2 = pop_ID();
  stackPush(Val_Int32::mk(s2->compareTo(s1) <= 0));
}
DEF_OP(ID_GT) { 
  IDRef s1 = pop_ID();
  IDRef s2 = pop_ID();
  stackPush(Val_Int32::mk(s2->compareTo(s1) > 0));
}
DEF_OP(ID_GTE) { 
  IDRef s1 = pop_ID();
  IDRef s2 = pop_ID();
  stackPush(Val_Int32::mk(s2->compareTo(s1) >= 0));
}
DEF_OP(ID_EQ) { 
  IDRef s1 = pop_ID();
  IDRef s2 = pop_ID();
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
  IDRef id = pop_ID();
  //warn << "Left shift " << shift << " " << id->toString() << " " << id->shift(shift)->toString() << "\n";
  stackPush(Val_ID::mk(id->shift(shift)));
}
DEF_OP(ID_DIST) {
  // Be careful of undefined evaluation order in C++!
  IDRef v1 = pop_ID();
  IDRef v2 = pop_ID();
  //warn << "Distance(" << v2->toString() << " to " << v1->toString() << "=" << v2->distance(v1)->toString() << "\n";
  stackPush(Val_ID::mk(v2->distance(v1)));
}
DEF_OP(ID_BTWOO) {
  IDRef to = pop_ID();
  IDRef from = pop_ID();
  IDRef key = pop_ID();
  //  warn << key->toString() << "(" << from->toString() << "," << to->toString() << ") :" << key->betweenOO(from, to) << "\n";
  stackPush(Val_Int32::mk(key->betweenOO(from, to)));
}
DEF_OP(ID_BTWOC) {
  IDRef to = pop_ID();
  IDRef from = pop_ID();
  IDRef key = pop_ID();
  //  warn << key->toString() << "(" << from->toString() << "," << to->toString() << "] :" << key->betweenOC(from, to) << "\n";
  stackPush(Val_Int32::mk(key->betweenOC(from, to)));
}
DEF_OP(ID_BTWCO) {
  IDRef to = pop_ID();
  IDRef from = pop_ID();
  IDRef key = pop_ID();
  stackPush(Val_Int32::mk(key->betweenCO(from, to)));
}
DEF_OP(ID_BTWCC) {
  IDRef to = pop_ID();
  IDRef from = pop_ID();
  IDRef key = pop_ID();
  stackPush(Val_Int32::mk(key->betweenCC(from, to)));
}

//
// String operations.  Note that the '>' and '<' are reversed: think
// about operand order on the stack!
//
DEF_OP(STR_LT) { 
  str s1 = pop_string();
  str s2 = pop_string();
  stackPush(Val_Int32::mk(s2 < s1));
}
DEF_OP(STR_LTE) { 
  str s1 = pop_string();
  str s2 = pop_string();
  stackPush(Val_Int32::mk(s2 <= s1));
}
DEF_OP(STR_GT) { 
  str s1 = pop_string();
  str s2 = pop_string();
  stackPush(Val_Int32::mk(s2 > s1));
}
DEF_OP(STR_GTE) { 
  str s1 = pop_string();
  str s2 = pop_string();
  stackPush(Val_Int32::mk(s2 >= s1));
}
DEF_OP(STR_EQ) { 
  str s1 = pop_string();
  str s2 = pop_string();
  //  warn << s1 << "==?" << s2 << " is " << (s1==s2) << "\n";
  stackPush(Val_Int32::mk(s2 == s1));
}
DEF_OP(STR_CAT) { 
  str s1 = pop_string();
  str s2 = pop_string();
  str r = strbuf() << s2 << s1;
  stackPush(Val_Str::mk(r));
}
DEF_OP(STR_LEN) {
  stackPush(Val_UInt32::mk(pop_string().len()));
}
DEF_OP(STR_UPPER) {
  // There _has_ to be a better way of doing this...
  char tmp_str[2];
  tmp_str[1] = '\0';
  strbuf sb;
  str s = pop_string();
  for(uint i=0; i < s.len(); i++) {
    tmp_str[0] = toupper(s[i]);
    sb.cat(tmp_str);
  }
  stackPush(Val_Str::mk(sb));
}
DEF_OP(STR_LOWER) {
  // There _has_ to be a better way of doing this...
  char tmp_str[2];
  tmp_str[1] = '\0';
  strbuf sb;
  str s = pop_string();
  for(uint i=0; i < s.len(); i++) {
    tmp_str[0] = tolower(s[i]);
    sb.cat(tmp_str);
  }
  stackPush(Val_Str::mk(sb));
}
DEF_OP(STR_SUBSTR) {
  int len = pop_unsigned();
  int pos = pop_unsigned();
  str s = pop_string();
  stackPush(Val_Str::mk(substr(s,pos,len)));
}
DEF_OP(STR_MATCH) {
  // XXX This is slow!!! For better results, memoize each regexp in a
  // hash map and study each one. 
  rxx re(pop_string());
  re.match(pop_string());
  stackPush(Val_Int32::mk(re.success()));
}
DEF_OP(STR_CONV) {
  ValueRef t = stackTop(); stackPop();
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
  stackPush(Val_Int64::mk(v2-v1));
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
  stackPush(Val_Double::mk(v2-v1));
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
  ValueRef top = stackTop();
  stackPop();
  stackPush(Val_Time::mk(Val_Time::cast(top)));
}
DEF_OP(CONV_ID) {
  ValueRef top = stackTop();
  stackPop();
  stackPush(Val_ID::mk(Val_ID::cast(top)));
}

//
// Hashing - could be a lot faster.  Room for improvement here. 
//
DEF_OP(HASH) {
  uint32_t h = (hash_t)(stackTop()->toTypeString());
  stackPop();
  stackPush(Val_UInt32::mk(h));
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
