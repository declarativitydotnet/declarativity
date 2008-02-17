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
 * DESCRIPTION: P2's concrete type system.  The base type for values.
 *
 */

#include "loop.h"
#include "value.h"

#include "val_null.h"
#include "val_str.h"
#include "val_int64.h"
#include "val_double.h"
#include "val_opaque.h"
#include "val_tuple.h"
#include "val_time.h"
#include "val_list.h"
#include "val_set.h"
#include "val_id.h"
#include "val_vector.h"
#include "val_matrix.h"
#include "val_gaussian_factor.h"
#include "val_table_factor.h"

typedef ValuePtr (*_unmarshal_fn)( XDR *);

static _unmarshal_fn jump_tab[] = {
  Val_Null::xdr_unmarshal,
  Val_Str::xdr_unmarshal,
  Val_Int64::xdr_unmarshal,
  Val_Double::xdr_unmarshal,
  Val_Opaque::xdr_unmarshal,
  Val_Tuple::xdr_unmarshal,
  Val_Time::xdr_unmarshal,
  Val_ID::xdr_unmarshal,
  Val_Time_Duration::xdr_unmarshal,
  Val_Set::xdr_unmarshal,
  Val_List::xdr_unmarshal,
  Val_Vector::xdr_unmarshal,
  Val_Matrix::xdr_unmarshal,
  Val_Gaussian_Factor::xdr_unmarshal,
  Val_Table_Factor::xdr_unmarshal
};

#include <time.h>
Value::Value() 
{
  //  TELL_ERROR << "Create_value " << time(NULL) << "\n";
}

Value::~Value()
{
  // TELL_ERROR << "Destroy_value " << time(NULL) << "\n";
}

//
// Marshalling
//

void Value::xdr_marshal( XDR *x ) 
{
  TRACE_FUNCTION;
  uint32_t tc = typeCode();
  TRACE_WORDY << "TypeCode is " << tc << "\n";
  xdr_uint32_t(x, &tc);
  xdr_marshal_subtype(x);
}

//
// Unmarshalling
//
ValuePtr Value::xdr_unmarshal(XDR *x)
{
  TRACE_FUNCTION;
  uint32_t tc;
  xdr_uint32_t(x, &tc);
  TRACE_WORDY << "TypeCode is " << tc << "\n";
  if ((unsigned) tc >= (sizeof(jump_tab)/sizeof(_unmarshal_fn))) {
    TELL_WARN << "Unmarshalling: Bad typecode " << tc << "\n";
    return Val_Null::mk();
  } else {
    return jump_tab[tc](x);
  }
}

Value::TypeError::TypeError(TypeCode t1, const char* t1Name,
                            TypeCode t2, const char* t2Name)
  : realType(t1),
    realTypeName(t2Name),
    toType(t2),
    toTypeName(t2Name)
{
  std::ostringstream oss;
  oss << "Failed to cast type "
      << t1
      << ":"
      << t1Name 
      << " to type "
      << t2
      << ":"
      << t2Name;
  _message = oss.str().c_str();
}


const char*
Value::TypeError::what() const throw()
{
  return _message;
}

bool
Value::Comparator::operator()(const ValuePtr first,
                              const ValuePtr second) const
{
  return first->compareTo(second) < 0;
}


/*
 * Value.C
 */
