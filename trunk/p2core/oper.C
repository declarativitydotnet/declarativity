/*
 * Copyright (c) 2004 Intel Corporation. All rights reserved.
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
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

ValueRef operator<<(const ValueRef& v1, const ValueRef& v2) {
  return Oper::oper_table_[v1->typeCode()][v2->typeCode()]->_lshift(v1, v2);
};
ValueRef operator>>(const ValueRef& v1, const ValueRef& v2) { 
  return Oper::oper_table_[v1->typeCode()][v2->typeCode()]->_rshift(v1, v2);
};
ValueRef operator+(const ValueRef& v1, const ValueRef& v2) { 
  return Oper::oper_table_[v1->typeCode()][v2->typeCode()]->_plus(v1, v2);
};
ValueRef operator-(const ValueRef& v1, const ValueRef& v2) { 
  return Oper::oper_table_[v1->typeCode()][v2->typeCode()]->_minus(v1, v2);
};
ValueRef operator*(const ValueRef& v1, const ValueRef& v2) { 
  return Oper::oper_table_[v1->typeCode()][v2->typeCode()]->_times(v1, v2);
};
ValueRef operator/(const ValueRef& v1, const ValueRef& v2) { 
  return Oper::oper_table_[v1->typeCode()][v2->typeCode()]->_divide(v1, v2);
};
ValueRef operator%(const ValueRef& v1, const ValueRef& v2) { 
  return Oper::oper_table_[v1->typeCode()][v2->typeCode()]->_mod(v1, v2);
};
ValueRef operator~ (const ValueRef& v) {
  return Oper::oper_table_[v->typeCode()][v->typeCode()]->_bnot(v);
};
ValueRef operator& (const ValueRef& v1, const ValueRef& v2) { 
  return Oper::oper_table_[v1->typeCode()][v2->typeCode()]->_band(v1, v2);
};
ValueRef operator| (const ValueRef& v1, const ValueRef& v2) { 
  return Oper::oper_table_[v1->typeCode()][v2->typeCode()]->_bor(v1, v2);
};
ValueRef operator^ (const ValueRef& v1, const ValueRef& v2) { 
  return Oper::oper_table_[v1->typeCode()][v2->typeCode()]->_bxor(v1, v2);
};

bool operator==(const ValueRef& v1, const ValueRef& v2) { 
  return Oper::oper_table_[v1->typeCode()][v2->typeCode()]->_eq(v1, v2);
};
bool operator!=(const ValueRef& v1, const ValueRef& v2) { 
  return Oper::oper_table_[v1->typeCode()][v2->typeCode()]->_neq(v1, v2);
};
bool operator< (const ValueRef& v1, const ValueRef& v2) { 
  return Oper::oper_table_[v1->typeCode()][v2->typeCode()]->_lt(v1, v2);
};
bool operator<=(const ValueRef& v1, const ValueRef& v2) { 
  return Oper::oper_table_[v1->typeCode()][v2->typeCode()]->_lte(v1, v2);
};
bool operator> (const ValueRef& v1, const ValueRef& v2) { 
  return Oper::oper_table_[v1->typeCode()][v2->typeCode()]->_gt(v1, v2);
};
bool operator>=(const ValueRef& v1, const ValueRef& v2) { 
  return Oper::oper_table_[v1->typeCode()][v2->typeCode()]->_gte(v1, v2);
};

bool inOO(const ValueRef& v1, const ValueRef& v2, const ValueRef& v3) {
  return Oper::oper_table_[v2->typeCode()][v3->typeCode()]->_inOO(v1, v2, v3);
};
bool inOC(const ValueRef& v1, const ValueRef& v2, const ValueRef& v3) {
  return Oper::oper_table_[v2->typeCode()][v3->typeCode()]->_inOC(v1, v2, v3);
};
bool inCO(const ValueRef& v1, const ValueRef& v2, const ValueRef& v3) {
  return Oper::oper_table_[v2->typeCode()][v3->typeCode()]->_inCO(v1, v2, v3);
};
bool inCC(const ValueRef& v1, const ValueRef& v2, const ValueRef& v3) {
  return Oper::oper_table_[v2->typeCode()][v3->typeCode()]->_inCC(v1, v2, v3);
};

/*
 * NULLV,  STR,    INT32,  UINT32, INT64,
 * UINT64, DOUBLE, OPAQUE, TUPLE,  TIME,
 * ID
 */

const Oper* Oper::oper_table_[Value::TYPES][Value::TYPES] = {
/* NULLV */
{Val_Null::oper_,   Val_Null::oper_,   Val_Int32::oper_, Val_UInt32::oper_, Val_Int64::oper_, 
 Val_UInt64::oper_, Val_Double::oper_, Val_Null::oper_,  Val_Null::oper_,   Val_Time::oper_, 
 Val_Null::oper_},
/* STR */
{Val_Null::oper_, Val_Str::oper_, Val_Str::oper_,    Val_Str::oper_,   Val_Str::oper_, 
 Val_Str::oper_,  Val_Str::oper_, Val_Opaque::oper_, Val_Tuple::oper_, Val_Str::oper_, 
 Val_Str::oper_},
/* INT32 */
{Val_Null::oper_,   Val_Str::oper_,    Val_Int32::oper_,  Val_UInt32::oper_, Val_Int64::oper_, 
 Val_UInt64::oper_, Val_Double::oper_, Val_Opaque::oper_, Val_Tuple::oper_,  Val_Time::oper_, 
 Val_ID::oper_},
/* UINT32 */
{Val_Null::oper_,   Val_Str::oper_,    Val_UInt32::oper_, Val_UInt32::oper_, Val_Int64::oper_, 
 Val_UInt64::oper_, Val_Double::oper_, Val_Opaque::oper_, Val_Tuple::oper_,  Val_Time::oper_, 
 Val_ID::oper_},
/* INT64 */
{Val_Null::oper_,   Val_Str::oper_,    Val_Int64::oper_,  Val_Int64::oper_, Val_Int64::oper_, 
 Val_UInt64::oper_, Val_Double::oper_, Val_Opaque::oper_, Val_Tuple::oper_, Val_Time::oper_, 
 Val_ID::oper_},
/* UINT64 */
{Val_Null::oper_,   Val_Str::oper_,    Val_UInt64::oper_, Val_UInt64::oper_, Val_UInt64::oper_, 
 Val_UInt64::oper_, Val_Double::oper_, Val_Opaque::oper_, Val_Tuple::oper_,  Val_Time::oper_, 
 Val_ID::oper_},
/* DOUBLE */
{Val_Null::oper_,   Val_Double::oper_, Val_Double::oper_, Val_Double::oper_, Val_Double::oper_, 
 Val_Double::oper_, Val_Double::oper_, Val_Opaque::oper_, Val_Tuple::oper_,  Val_Time::oper_, 
 Val_ID::oper_},
/* OPAQUE */
{Val_Opaque::oper_, Val_Opaque::oper_, Val_Opaque::oper_, Val_Opaque::oper_, Val_Opaque::oper_, 
 Val_Opaque::oper_, Val_Opaque::oper_, Val_Opaque::oper_, Val_Opaque::oper_, Val_Opaque::oper_, 
 Val_Opaque::oper_},
/* TUPLE */
{Val_Tuple::oper_, Val_Tuple::oper_, Val_Tuple::oper_, Val_Tuple::oper_, Val_Tuple::oper_, 
 Val_Tuple::oper_, Val_Tuple::oper_, Val_Tuple::oper_, Val_Tuple::oper_, Val_Tuple::oper_, 
 Val_Tuple::oper_},
/* TIME */
{Val_Null::oper_, Val_Str::oper_,  Val_Time::oper_,   Val_Time::oper_,  Val_Time::oper_, 
 Val_Time::oper_, Val_Time::oper_, Val_Opaque::oper_, Val_Tuple::oper_, Val_Time::oper_, 
 Val_Str::oper_},
/* ID */
{Val_Null::oper_, Val_Str::oper_, Val_ID::oper_,     Val_ID::oper_,    Val_ID::oper_, 
 Val_ID::oper_,   Val_ID::oper_,  Val_Opaque::oper_, Val_Tuple::oper_, Val_Str::oper_, 
 Val_ID::oper_}
};

/*
 * oper.C
 */
