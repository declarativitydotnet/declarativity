// -*- c-basic-offset: 2; related-file-name: "oper.h" -*-
/*
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: P2's concrete type system.  The base type for values.
 *
 */

#include "oper.h"
#include "val_null.h"
#include "val_str.h"
#include "val_int32.h"
#include "val_uint32.h"
#include "val_int64.h"
#include "val_uint64.h"
#include "val_double.h"
#include "val_opaque.h"
#include "val_tuple.h"
#include "val_time.h"
#include "val_id.h"
#include "val_ip_addr.h"

namespace opr {

  /**
   * The initialization of the operator table for operator functions.
   * The entries are filled in based on the base type of the corresponding
   * operands. The TypeName of the operands are used to index this table.
   * Each entry for each possible combination of operand types holds the
   * location of the BASE type operator function. The base type operator
   * function is a public static member variable defined as oper_ in each P2
   * concrete type.  
   *
   * CHEAT SHEET: type of each column entry.
   * NULLV,  STR,    INT32,  
   * UINT32, INT64,  UINT64, 
   * DOUBLE, OPAQUE, TUPLE,  
   * TIME,   ID,     IP_ADDR
   */
  const Oper** Oper::oper_table_[Value::TYPES][Value::TYPES] = {
    /* NULLV */
    {&Val_Null::oper_, &Val_Null::oper_, &Val_Null::oper_, 
     &Val_Null::oper_, &Val_Null::oper_, &Val_Null::oper_, 
     &Val_Null::oper_, &Val_Null::oper_, &Val_Null::oper_, 
     &Val_Null::oper_, &Val_Null::oper_, &Val_Null::oper_},
    /* STR */
    {&Val_Str::oper_, &Val_Str::oper_, &Val_Str::oper_, 
     &Val_Str::oper_, &Val_Str::oper_, &Val_Str::oper_, 
     &Val_Str::oper_, &Val_Str::oper_, &Val_Str::oper_, 
     &Val_Str::oper_, &Val_Str::oper_, &Val_Str::oper_},
    /* INT32 */
    {&Val_Int32::oper_, &Val_Int32::oper_, &Val_Int32::oper_, 
     &Val_Int32::oper_, &Val_Int32::oper_, &Val_Int32::oper_, 
     &Val_Int32::oper_, &Val_Int32::oper_, &Val_Int32::oper_, 
     &Val_Int32::oper_, &Val_Int32::oper_, &Val_Int32::oper_},
    /* UINT32 */
    {&Val_UInt32::oper_, &Val_UInt32::oper_, &Val_UInt32::oper_, 
     &Val_UInt32::oper_, &Val_UInt32::oper_, &Val_UInt32::oper_, 
     &Val_UInt32::oper_, &Val_UInt32::oper_, &Val_UInt32::oper_, 
     &Val_UInt32::oper_, &Val_UInt32::oper_, &Val_UInt32::oper_},
    /* INT64 */
    {&Val_Int64::oper_, &Val_Int64::oper_, &Val_Int64::oper_, 
     &Val_Int64::oper_, &Val_Int64::oper_, &Val_Int64::oper_, 
     &Val_Int64::oper_, &Val_Int64::oper_, &Val_Int64::oper_, 
     &Val_Int64::oper_, &Val_Int64::oper_, &Val_Int64::oper_},
    /* INT64 */
    {&Val_UInt64::oper_, &Val_UInt64::oper_, &Val_UInt64::oper_, 
     &Val_UInt64::oper_, &Val_UInt64::oper_, &Val_UInt64::oper_, 
     &Val_UInt64::oper_, &Val_UInt64::oper_, &Val_UInt64::oper_, 
     &Val_UInt64::oper_, &Val_UInt64::oper_, &Val_UInt64::oper_},
    /* Double */
    {&Val_Double::oper_, &Val_Double::oper_, &Val_Double::oper_, 
     &Val_Double::oper_, &Val_Double::oper_, &Val_Double::oper_, 
     &Val_Double::oper_, &Val_Double::oper_, &Val_Double::oper_, 
     &Val_Double::oper_, &Val_Double::oper_, &Val_Double::oper_},
    /* Opaque */
    {&Val_Opaque::oper_, &Val_Opaque::oper_, &Val_Opaque::oper_, 
     &Val_Opaque::oper_, &Val_Opaque::oper_, &Val_Opaque::oper_, 
     &Val_Opaque::oper_, &Val_Opaque::oper_, &Val_Opaque::oper_, 
     &Val_Opaque::oper_, &Val_Opaque::oper_, &Val_Opaque::oper_},
    /* Tuple */
    {&Val_Tuple::oper_, &Val_Tuple::oper_, &Val_Tuple::oper_, 
     &Val_Tuple::oper_, &Val_Tuple::oper_, &Val_Tuple::oper_, 
     &Val_Tuple::oper_, &Val_Tuple::oper_, &Val_Tuple::oper_, 
     &Val_Tuple::oper_, &Val_Tuple::oper_, &Val_Tuple::oper_},
    /* Time */
    {&Val_Time::oper_, &Val_Time::oper_, &Val_Time::oper_, 
     &Val_Time::oper_, &Val_Time::oper_, &Val_Time::oper_, 
     &Val_Time::oper_, &Val_Time::oper_, &Val_Time::oper_, 
     &Val_Time::oper_, &Val_Time::oper_, &Val_Time::oper_},
    /* ID */
    {&Val_ID::oper_, &Val_ID::oper_, &Val_ID::oper_, 
     &Val_ID::oper_, &Val_ID::oper_, &Val_ID::oper_, 
     &Val_ID::oper_, &Val_ID::oper_, &Val_ID::oper_, 
     &Val_ID::oper_, &Val_ID::oper_, &Val_ID::oper_},
    /* IP_ADDR */
    {&Val_IP_ADDR::oper_, &Val_IP_ADDR::oper_, &Val_IP_ADDR::oper_, 
     &Val_IP_ADDR::oper_, &Val_IP_ADDR::oper_, &Val_IP_ADDR::oper_, 
     &Val_IP_ADDR::oper_, &Val_IP_ADDR::oper_, &Val_IP_ADDR::oper_, 
     &Val_IP_ADDR::oper_, &Val_IP_ADDR::oper_, &Val_IP_ADDR::oper_}
    
  };
  // const Oper** Oper::oper_table_[Value::TYPES][Value::TYPES] = {
  //   /* NULLV */
  //   {&Val_Null::oper_,   &Val_Null::oper_,   &Val_Int32::oper_, 
  //    &Val_UInt32::oper_, &Val_Int64::oper_,  &Val_UInt64::oper_, 
  //    &Val_Double::oper_, &Val_Null::oper_,   &Val_Null::oper_,   
  //    &Val_Time::oper_,   &Val_Null::oper_, &Val_IP_ADDR::oper_},
  //   /* STR */
  //   {&Val_Null::oper_, &Val_Str::oper_,    &Val_Str::oper_,    
  //    &Val_Str::oper_,  &Val_Str::oper_,    &Val_Str::oper_,  
  //    &Val_Str::oper_,  &Val_Opaque::oper_, &Val_Tuple::oper_, 
  //    &Val_Str::oper_,  &Val_Str::oper_,  &Val_IP_ADDR::oper_},
  //   /* INT32 */
  //   {&Val_Null::oper_,   &Val_Str::oper_,    &Val_Int32::oper_,  
  //    &Val_UInt32::oper_, &Val_Int64::oper_,  &Val_UInt64::oper_, 
  //    &Val_Double::oper_, &Val_Opaque::oper_, &Val_Tuple::oper_,  
  //    &Val_Time::oper_,   &Val_ID::oper_,  &Val_IP_ADDR::oper_},
  //   /* UINT32 */
  //   {&Val_Null::oper_,   &Val_Str::oper_,    &Val_UInt32::oper_, 
  //    &Val_UInt32::oper_, &Val_Int64::oper_,  &Val_UInt64::oper_, 
  //    &Val_Double::oper_, &Val_Opaque::oper_, &Val_Tuple::oper_,  
  //    &Val_Time::oper_,   &Val_ID::oper_, &Val_IP_ADDR::oper_},
  //   /* INT64 */
  //   {&Val_Null::oper_,   &Val_Str::oper_,    &Val_Int64::oper_,  
  //    &Val_Int64::oper_,  &Val_Int64::oper_,  &Val_UInt64::oper_, 
  //    &Val_Double::oper_, &Val_Opaque::oper_, &Val_Tuple::oper_, 
  //    &Val_Time::oper_,   &Val_ID::oper_, &Val_IP_ADDR::oper_},
  //   /* UINT64 */
  //   {&Val_Null::oper_,   &Val_Str::oper_,    &Val_UInt64::oper_,  
  //    &Val_UInt64::oper_, &Val_UInt64::oper_, &Val_UInt64::oper_, 
  //    &Val_Double::oper_, &Val_Opaque::oper_, &Val_Tuple::oper_,  
  //    &Val_Time::oper_,   &Val_ID::oper_, &Val_IP_ADDR::oper_},
  //   /* DOUBLE */
  //   {&Val_Null::oper_,   &Val_Str::oper_,    &Val_Double::oper_, 
  //    &Val_Double::oper_, &Val_Double::oper_, &Val_Double::oper_, 
  //    &Val_Double::oper_, &Val_Opaque::oper_, &Val_Tuple::oper_,  
  //    &Val_Time::oper_,   &Val_ID::oper_, &Val_IP_ADDR::oper_},
  //   /* OPAQUE */
  //   {&Val_Opaque::oper_, &Val_Opaque::oper_, &Val_Opaque::oper_, 
  //    &Val_Opaque::oper_, &Val_Opaque::oper_, &Val_Opaque::oper_, 
  //    &Val_Opaque::oper_, &Val_Opaque::oper_, &Val_Opaque::oper_, 
  //    &Val_Opaque::oper_, &Val_Opaque::oper_, &Val_IP_ADDR::oper_},
  //   /* TUPLE */
  //   {&Val_Tuple::oper_, &Val_Tuple::oper_, &Val_Tuple::oper_, 
  //    &Val_Tuple::oper_, &Val_Tuple::oper_, &Val_Tuple::oper_, 
  //    &Val_Tuple::oper_, &Val_Tuple::oper_, &Val_Tuple::oper_, 
  //    &Val_Tuple::oper_, &Val_Tuple::oper_, &Val_IP_ADDR::oper_},
  //   /* TIME */
  //   {&Val_Null::oper_, &Val_Str::oper_,    &Val_Time::oper_,   
  //    &Val_Time::oper_, &Val_Time::oper_,   &Val_Time::oper_, 
  //    &Val_Time::oper_, &Val_Opaque::oper_, &Val_Tuple::oper_, 
  //    &Val_Time::oper_, &Val_Str::oper_, &Val_IP_ADDR::oper_},
  //   /* ID */
  //   {&Val_Null::oper_, &Val_Str::oper_,    &Val_ID::oper_,     
  //    &Val_ID::oper_,   &Val_ID::oper_,     &Val_ID::oper_,   
  //    &Val_ID::oper_,   &Val_Opaque::oper_, &Val_Tuple::oper_, 
  //    &Val_Str::oper_,  &Val_ID::oper_, &Val_IP_ADDR::oper_},
  //   /* IP_ADDR */
  //   {&Val_IP_ADDR::oper_, &Val_IP_ADDR::oper_, &Val_IP_ADDR::oper_,
  //    &Val_IP_ADDR::oper_, &Val_IP_ADDR::oper_, &Val_IP_ADDR::oper_,
  //    &Val_IP_ADDR::oper_, &Val_IP_ADDR::oper_, &Val_IP_ADDR::oper_,
  //    &Val_IP_ADDR::oper_, &Val_IP_ADDR::oper_, &Val_IP_ADDR::oper_}
  // };
  
  ValuePtr operator<<(const ValuePtr& v1, const ValuePtr& v2) {
    return (*Oper::oper_table_[v1->typeCode()][v2->typeCode()])->_lshift(v1, v2);
  };
  ValuePtr operator>>(const ValuePtr& v1, const ValuePtr& v2) { 
    return (*Oper::oper_table_[v1->typeCode()][v2->typeCode()])->_rshift(v1, v2);
  };
  ValuePtr operator+(const ValuePtr& v1, const ValuePtr& v2) { 
    // Changed to have the first argument determine how the second
    // argument casts
    return (*Oper::oper_table_[v1->typeCode()][v2->typeCode()])->_plus(v1, v2);
  };
//   struct timespec operator+(const struct timespec& v1, const struct timespec& v2) { 
//     struct timespec ts = v1;
//     ts.tv_sec  += v2.tv_sec; 
//     ts.tv_nsec += v2.tv_nsec; 
//     return ts;
//   };
  ValuePtr operator-(const ValuePtr& v1, const ValuePtr& v2) { 
    // Changed to have the first argument determine how the second
    // argument casts
    return (*Oper::oper_table_[v1->typeCode()][v2->typeCode()])->_minus(v1, v2);
  };
//   struct timespec operator-(const struct timespec& v1, const struct timespec& v2) { 
//     struct timespec ts = {0};
//     if (v1 < v2) return ts;
//     ts = v1;
//     while (v1.tv_nsec < v2.tv_nsec) {
//       ts.tv_nsec += 1000000000; 
//       ts.tv_sec--;
//     }
//     ts.tv_sec  -= v2.tv_sec;
//     ts.tv_nsec -= v2.tv_nsec;
//     return ts;
//   };
  ValuePtr operator--(const ValuePtr& v1) { 
    return (*Oper::oper_table_[v1->typeCode()][v1->typeCode()])->_dec(v1);
  };
  ValuePtr operator++(const ValuePtr& v1) { 
    return (*Oper::oper_table_[v1->typeCode()][v1->typeCode()])->_inc(v1);
  };
  ValuePtr operator*(const ValuePtr& v1, const ValuePtr& v2) { 
    // Changed to have the first argument determine how the second
    // argument casts
    return (*Oper::oper_table_[v1->typeCode()][v2->typeCode()])->_times(v1, v2);
  };
  ValuePtr operator/(const ValuePtr& v1, const ValuePtr& v2) { 
    // Changed to have the first argument determine how the second
    // argument casts
    return (*Oper::oper_table_[v1->typeCode()][v2->typeCode()])->_divide(v1, v2);
  };
  ValuePtr operator%(const ValuePtr& v1, const ValuePtr& v2) { 
    return (*Oper::oper_table_[v1->typeCode()][v2->typeCode()])->_mod(v1, v2);
  };
  ValuePtr operator~ (const ValuePtr& v) {
    return (*Oper::oper_table_[v->typeCode()][v->typeCode()])->_bnot(v);
  };
  ValuePtr operator& (const ValuePtr& v1, const ValuePtr& v2) { 
    return (*Oper::oper_table_[v1->typeCode()][v2->typeCode()])->_band(v1, v2);
  };
  ValuePtr operator| (const ValuePtr& v1, const ValuePtr& v2) { 
    return (*Oper::oper_table_[v1->typeCode()][v2->typeCode()])->_bor(v1, v2);
  };
  ValuePtr operator^ (const ValuePtr& v1, const ValuePtr& v2) { 
    return (*Oper::oper_table_[v1->typeCode()][v2->typeCode()])->_bxor(v1, v2);
  };
  
  bool operator==(const ValuePtr& v1, const ValuePtr& v2) { 
    return (*Oper::oper_table_[v1->typeCode()][v2->typeCode()])->_eq(v1, v2);
  };
  bool operator!=(const ValuePtr& v1, const ValuePtr& v2) { 
    return (*Oper::oper_table_[v1->typeCode()][v2->typeCode()])->_neq(v1, v2);
  };
  bool operator< (const ValuePtr& v1, const ValuePtr& v2) { 
    return (*Oper::oper_table_[v1->typeCode()][v2->typeCode()])->_lt(v1, v2);
  };
  bool operator<=(const ValuePtr& v1, const ValuePtr& v2) { 
    return (*Oper::oper_table_[v1->typeCode()][v2->typeCode()])->_lte(v1, v2);
  };
  bool operator> (const ValuePtr& v1, const ValuePtr& v2) { 
    return (*Oper::oper_table_[v1->typeCode()][v2->typeCode()])->_gt(v1, v2);
  };
  bool operator>=(const ValuePtr& v1, const ValuePtr& v2) { 
    return (*Oper::oper_table_[v1->typeCode()][v2->typeCode()])->_gte(v1, v2);
  };

//   bool operator==(const struct timespec& v1, const struct timespec& v2) { 
//     return (v1.tv_sec == v2.tv_sec && v1.tv_nsec == v2.tv_nsec);
//   };
//   bool operator!=(const struct timespec& v1, const struct timespec& v2) { 
//     return (v1.tv_sec != v2.tv_sec || v1.tv_nsec != v2.tv_nsec);
//   };
//   bool operator< (const struct timespec& v1, const struct timespec& v2) { 
//     return ((v1.tv_sec < v2.tv_sec) || 
//             (v1.tv_sec == v2.tv_sec && v1.tv_nsec < v2.tv_nsec));
//   };
//   bool operator<=(const struct timespec& v1, const struct timespec& v2) { 
//     return v1 < v2 || v1 == v2;
//   };
//   bool operator> (const struct timespec& v1, const struct timespec& v2) { 
//     return ((v1.tv_sec > v2.tv_sec) || 
//             (v1.tv_sec == v2.tv_sec && v1.tv_nsec > v2.tv_nsec));
//   };
//   bool operator>=(const struct timespec& v1, const struct timespec& v2) { 
//     return v1 > v2 || v1 == v2;
//   };
  
  bool inOO(const ValuePtr& v1, const ValuePtr& v2, const ValuePtr& v3) {
    return (*Oper::oper_table_[v2->typeCode()][v3->typeCode()])->_inOO(v1, v2, v3);
  };
  bool inOC(const ValuePtr& v1, const ValuePtr& v2, const ValuePtr& v3) {
    return (*Oper::oper_table_[v2->typeCode()][v3->typeCode()])->_inOC(v1, v2, v3);
  };
  bool inCO(const ValuePtr& v1, const ValuePtr& v2, const ValuePtr& v3) {
    return (*Oper::oper_table_[v2->typeCode()][v3->typeCode()])->_inCO(v1, v2, v3);
  };
  bool inCC(const ValuePtr& v1, const ValuePtr& v2, const ValuePtr& v3) {
    return (*Oper::oper_table_[v2->typeCode()][v3->typeCode()])->_inCC(v1, v2, v3);
  };
}; // END NAMESPACE OPR

/*
 * oper.C
 */
