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

typedef void(Pel_VM::*Op_fn_t)(u_int32_t);

struct JumpTableEnt_t {
  char *opcode;
  int	arity;
  Op_fn_t fn;
};

const char *Pel_VM::err_msgs[] = {
  "Success",
  "Type conversion",
  "Invalid Errno",
  "Unknown Error"
};

#include "pel_opcode_defns.gen.h"

//
// Create the VM
//
Pel_VM::Pel_VM() : st() {
  reset();
}

//
// Reset the VM
//
void Pel_VM::reset() {
  while (!st.empty()) {
    st.pop();
  }
}

//
// Return a result
//
TupleFieldRef Pel_VM::result_val() const {
  return st.top();
}

// 
// Make a tuple out of the top elements on the stack and return it
// 
TupleRef Pel_VM::result_tuple() {
  if (!result) {
    result = Tuple::mk();
  }
  return result;
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
u_int64_t Pel_VM::pop_force_unsigned() 
{
  TupleFieldRef t = st.top(); st.pop();
  switch( t->get_type() ) {
  case TupleField::INT32:
    return t->as_i32();
  case TupleField::UINT32:
    return t->as_ui32();
  case TupleField::INT64:
    return t->as_i64();
  case TupleField::UINT64:
    return t->as_ui64();
  default:
    error = PE_TYPE_CONVERSION;
    return 0;
  }
}

//
// Type conversion to an unsigned number with no checkng
//
int64_t Pel_VM::pop_signed() 
{
  TupleFieldRef t = st.top(); st.pop();
  switch( t->get_type() ) {
  case TupleField::INT32:
    return t->as_i32();
  case TupleField::UINT32:
    return t->as_ui32();
  case TupleField::INT64:
    return t->as_i64();
  case TupleField::UINT64:
    return t->as_ui64();
  default:
    error = PE_TYPE_CONVERSION;
    return 0;
  }
}

//
// Pull a string off the stack
//  
str Pel_VM::pop_string() 
{
  TupleFieldRef t = st.top(); st.pop();
  if (t->get_type() == TupleField::STRING) {
    return t->as_s();
  } else {
    error = PE_TYPE_CONVERSION;
    return str("");
  }
}

//
// Pull a double off the stack
//  
double Pel_VM::pop_double() 
{
  TupleFieldRef t = st.top(); st.pop();
  if (t->get_type() == TupleField::DOUBLE) {
    return t->as_d();
  } else {
    error = PE_TYPE_CONVERSION;
    return 0.0;
  }
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
DEF_OP(DROP) { st.pop(); }
DEF_OP(SWAP) { 
  TupleFieldRef t1 = st.top(); st.pop();
  TupleFieldRef t2 = st.top(); st.pop();
  st.push(t1); 
  st.push(t2);
}
DEF_OP(DUP) { st.push(st.top()); }
DEF_OP(PUSH_CONST) { 
  int ndx = (inst >> 16);
  if (ndx > prg->const_pool.size() ) { error = PE_BAD_CONSTANT; return; }
  st.push(prg->const_pool[ndx]);
}
DEF_OP(PUSH_FIELD) { 
  int ndx = (inst >> 16);
  if (ndx > operand->size() ) { error = PE_BAD_FIELD; return; }
  st.push((*operand)[ndx]);
}
DEF_OP(POP) {
  if (!result) { result = Tuple::mk(); }
  result->append(st.top());
  st.pop();
}

//
// Boolean operations
//
DEF_OP(NOT) {
  u_int64_t v = pop_force_unsigned();
  st.push(New refcounted<TupleField>(!v));
}
DEF_OP(AND) {
  u_int64_t v1 = pop_force_unsigned();
  u_int64_t v2 = pop_force_unsigned();
  st.push(New refcounted<TupleField>(v1 && v2));
}
DEF_OP(OR) {
  u_int64_t v1 = pop_force_unsigned();
  u_int64_t v2 = pop_force_unsigned();
  st.push(New refcounted<TupleField>(v1 || v2));
}

//
// Integer-only arithmetic operations (mostly bitwise)
//
DEF_OP(LSR) {
  u_int64_t v1 = pop_force_unsigned();
  u_int64_t v2 = pop_force_unsigned();
  st.push(New refcounted<TupleField>(v2 >> v1));
}
DEF_OP(ASR) {
  u_int64_t v1 = pop_force_unsigned();
  int64_t v2 = pop_signed();
  st.push(New refcounted<TupleField>(v2 >> v1));
}
DEF_OP(LSL) {
  u_int64_t v1 = pop_force_unsigned();
  u_int64_t v2 = pop_force_unsigned();
  st.push(New refcounted<TupleField>(v2 << v1));
}
DEF_OP(BIT_AND) {
  st.push(New refcounted<TupleField>(pop_force_unsigned() & pop_force_unsigned()));
}
DEF_OP(BIT_OR) {
  st.push(New refcounted<TupleField>(pop_force_unsigned() | pop_force_unsigned()));
}
DEF_OP(BIT_XOR) {
  st.push(New refcounted<TupleField>(pop_force_unsigned() ^ pop_force_unsigned()));
}
DEF_OP(BIT_NOT) {
  st.push(New refcounted<TupleField>(~pop_force_unsigned()));
}
DEF_OP(MOD) {
  int64_t v1 = pop_signed();
  int64_t v2 = pop_signed();
  st.push(New refcounted<TupleField>(v2 % v1));
}

//
// String operations.  Note that the '>' and '<' are reversed: think
// about operand order on the stack!
//
DEF_OP(STR_LT) { 
  st.push(New refcounted<TupleField>(pop_string() > pop_string())); 
}
DEF_OP(STR_LTE) { 
  st.push(New refcounted<TupleField>(pop_string() >= pop_string())); 
}
DEF_OP(STR_GT) { 
  st.push(New refcounted<TupleField>(pop_string() < pop_string())); 
}
DEF_OP(STR_GTE) { 
  st.push(New refcounted<TupleField>(pop_string() <= pop_string())); 
}
DEF_OP(STR_EQ) { 
  st.push(New refcounted<TupleField>(pop_string() == pop_string())); 
}
DEF_OP(STR_CAT) { 
  str s1 = pop_string();
  str s2 = pop_string();
  str r = strbuf() << s2 << s1;
  st.push(New refcounted<TupleField>(r));
}
DEF_OP(STR_LEN) {
  st.push(New refcounted<TupleField>(pop_string().len()));
}
DEF_OP(STR_UPPER) {
  // There _has_ to be a better way of doing this...
  char tmp_str[2];
  tmp_str[1] = '\0';
  strbuf sb;
  str s = pop_string();
  for(int i=0; i < s.len(); i++) {
    tmp_str[0] = toupper(s[i]);
    sb.cat(tmp_str);
  }
  st.push(New refcounted<TupleField>(sb));
}
DEF_OP(STR_LOWER) {
  // There _has_ to be a better way of doing this...
  char tmp_str[2];
  tmp_str[1] = '\0';
  strbuf sb;
  str s = pop_string();
  for(int i=0; i < s.len(); i++) {
    tmp_str[0] = toupper(s[i]);
    sb.cat(tmp_str);
  }
  st.push(New refcounted<TupleField>(sb));
}
DEF_OP(STR_SUBSTR) {
  int len = pop_force_unsigned();
  int pos = pop_force_unsigned();
  str s = pop_string();
  st.push(New refcounted<TupleField>(substr(s,pos,len)));
}
DEF_OP(STR_MATCH) {
  // XXX This is slow!!! For better results, memoize each regexp in a
  // hash map and study each one. 
  rxx re(pop_string());
  re.match(pop_string());
  st.push(New refcounted<TupleField>(re.success()));
}

//
// Integer arithmetic operations
//
DEF_OP(INT_NEG) {
  st.push(New refcounted<TupleField>(-pop_signed()));
}
DEF_OP(INT_PLUS) {
  st.push(New refcounted<TupleField>(pop_signed()-pop_signed()));
}
DEF_OP(INT_MINUS) {
  // Be careful of undefined evaluation order in C++!
  int64_t v1 = pop_signed();
  int64_t v2 = pop_signed();
  st.push(New refcounted<TupleField>(v2-v1));
}
DEF_OP(INT_MUL) {
  st.push(New refcounted<TupleField>(pop_signed()*pop_signed()));
}
DEF_OP(INT_DIV) {
  // Be careful of undefined evaluation order in C++!
  int64_t v1 = pop_signed();
  int64_t v2 = pop_signed();
  st.push(New refcounted<TupleField>(v2 / v1));
}
DEF_OP(INT_ABS) {
  st.push(New refcounted<TupleField>(llabs(pop_signed())));
}
DEF_OP(INT_EQ) {
  st.push(New refcounted<TupleField>(pop_signed() == pop_signed()));
}
DEF_OP(INT_LT) { 
  st.push(New refcounted<TupleField>(pop_signed() > pop_signed())); 
}
DEF_OP(INT_LTE) { 
  st.push(New refcounted<TupleField>(pop_signed() >= pop_signed())); 
}
DEF_OP(INT_GT) { 
  st.push(New refcounted<TupleField>(pop_signed() < pop_signed())); 
}
DEF_OP(INT_GTE) { 
  st.push(New refcounted<TupleField>(pop_signed() <= pop_signed())); 
}


//
// Floating-point arithmetic operations
//
DEF_OP(DBL_NEG) {
  st.push(New refcounted<TupleField>(-pop_double()));
}
DEF_OP(DBL_PLUS) {
  st.push(New refcounted<TupleField>(pop_double()-pop_double()));
}
DEF_OP(DBL_MINUS) {
  // Be careful of undefined evaluation order in C++!
  double v1 = pop_double();
  double v2 = pop_double();
  st.push(New refcounted<TupleField>(v2-v1));
}
DEF_OP(DBL_MUL) {
  st.push(New refcounted<TupleField>(pop_double()*pop_double()));
}
DEF_OP(DBL_DIV) {
  // Be careful of undefined evaluation order in C++!
  double v1 = pop_double();
  double v2 = pop_double();
  st.push(New refcounted<TupleField>(v2 / v1));
}
DEF_OP(DBL_EQ) {
  st.push(New refcounted<TupleField>(pop_double() == pop_double()));
}
DEF_OP(DBL_LT) { 
  st.push(New refcounted<TupleField>(pop_double() > pop_double())); 
}
DEF_OP(DBL_LTE) { 
  st.push(New refcounted<TupleField>(pop_double() >= pop_double())); 
}
DEF_OP(DBL_GT) { 
  st.push(New refcounted<TupleField>(pop_double() < pop_double())); 
}
DEF_OP(DBL_GTE) { 
  st.push(New refcounted<TupleField>(pop_double() <= pop_double())); 
}
DEF_OP(DBL_FLOOR) {
  st.push(New refcounted<TupleField>(floor(pop_double())));
}
DEF_OP(DBL_CEIL) {
  st.push(New refcounted<TupleField>(ceil(pop_double())));
}

//
// Explicit Type conversions
//
DEF_OP(CONV_I32) {
  TupleFieldRef t = st.top(); st.pop();
  int32_t i;
  switch( t->get_type() ) {
  case TupleField::INT32:
    i = t->as_i32();
  case TupleField::UINT32:
    i = t->as_ui32();
  case TupleField::INT64:
    i = t->as_i64();
  case TupleField::UINT64:
    i = t->as_ui64();
  case TupleField::STRING:
    i = lrint(atof(t->as_s()));
  case TupleField::DOUBLE:
    i = lrint(t->as_d());
  default:
    error = PE_TYPE_CONVERSION;
    i = 0;
  }
  st.push(New refcounted<TupleField>(i));
}
DEF_OP(CONV_U32) {
  TupleFieldRef t = st.top(); st.pop();
  uint32_t i;
  switch( t->get_type() ) {
  case TupleField::INT32:
    i = t->as_i32();
  case TupleField::UINT32:
    i = t->as_ui32();
  case TupleField::INT64:
    i = t->as_i64();
  case TupleField::UINT64:
    i = t->as_ui64();
  case TupleField::STRING:
    i = lrint(atof(t->as_s()));
  case TupleField::DOUBLE:
    i = lrint(t->as_d());
  default:
    error = PE_TYPE_CONVERSION;
    i = 0;
  }
  st.push(New refcounted<TupleField>(i));
}
DEF_OP(CONV_I64) {
  TupleFieldRef t = st.top(); st.pop();
  int64_t i;
  switch( t->get_type() ) {
  case TupleField::INT32:
    i = t->as_i32();
  case TupleField::UINT32:
    i = t->as_ui32();
  case TupleField::INT64:
    i = t->as_i64();
  case TupleField::UINT64:
    i = t->as_ui64();
  case TupleField::STRING:
    i = llrint(atof(t->as_s()));
  case TupleField::DOUBLE:
    i = llrint(t->as_d());
  default:
    error = PE_TYPE_CONVERSION;
    i = 0;
  }
  st.push(New refcounted<TupleField>(i));
}
DEF_OP(CONV_U64) {
  TupleFieldRef t = st.top(); st.pop();
  uint64_t i;
  switch( t->get_type() ) {
  case TupleField::INT32:
    i = t->as_i32();
  case TupleField::UINT32:
    i = t->as_ui32();
  case TupleField::INT64:
    i = t->as_i64();
  case TupleField::UINT64:
    i = t->as_ui64();
  case TupleField::STRING:
    i = llrint(atof(t->as_s()));
  case TupleField::DOUBLE:
    i = llrint(t->as_d());
  default:
    error = PE_TYPE_CONVERSION;
    i = 0;
  }
  st.push(New refcounted<TupleField>(i));
}
DEF_OP(CONV_STR) {
  TupleFieldRef t = st.top(); st.pop();
  st.push(New refcounted<TupleField>(t->toString()));
}
DEF_OP(CONV_DBL) {
  TupleFieldRef t = st.top(); st.pop();
  double i;
  switch( t->get_type() ) {
  case TupleField::INT32:
    i = t->as_i32();
  case TupleField::UINT32:
    i = t->as_ui32();
  case TupleField::INT64:
    i = t->as_i64();
  case TupleField::UINT64:
    i = t->as_ui64();
  case TupleField::STRING:
    i = atof(t->as_s());
  case TupleField::DOUBLE:
    i = t->as_d();
  default:
    error = PE_TYPE_CONVERSION;
    i = 0;
  }
  st.push(New refcounted<TupleField>(i));
}

/*
 * End of file 
 */
