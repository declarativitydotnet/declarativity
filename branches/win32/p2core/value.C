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

#include "value.h"
#include "loop.h"

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
#include "val_list.h"
#include "val_id.h"
#include "val_ip_addr.h"
#include "val_vector.h"
#include "val_matrix.h"


typedef ValuePtr (*_unmarshal_fn)( boost::archive::text_iarchive *);

static _unmarshal_fn jump_tab[] = {
  Val_Null::unmarshal,
  Val_Str::unmarshal,
  Val_Int32::unmarshal,
  Val_UInt32::unmarshal,
  Val_Int64::unmarshal,
  Val_UInt64::unmarshal,
  Val_Double::unmarshal,
  Val_Opaque::unmarshal,
  Val_Tuple::unmarshal,
  Val_Time::unmarshal,
  Val_ID::unmarshal,
  Val_IP_ADDR::unmarshal,
  Val_Time_Duration::unmarshal,
  Val_List::unmarshal,
  Val_Vector::unmarshal,
  Val_Matrix::unmarshal
};

//
// Marshalling
//

void Value::marshal( boost::archive::text_oarchive *x ) 
{
  TRACE_FUNCTION;
  uint32_t tc = typeCode();
  TRACE_WORDY << "TypeCode is " << tc << "\n";
  *x & tc;
  marshal_subtype(x);
}

//
// Unmarshalling
//
ValuePtr Value::unmarshal(boost::archive::text_iarchive *x)
{
  TRACE_FUNCTION;
  uint32_t tc;
  *x & tc;
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
