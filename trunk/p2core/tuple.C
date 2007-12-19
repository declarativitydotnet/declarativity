// -*- c-basic-offset: 2; related-file-name: "tuple.h" -*-
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
 * DESCRIPTION: Tuple fields and Tuple implementations
 *
 */
#include "plumber.h"
#include "value.h"
#include "tuple.h"
#include <assert.h>
#include "val_str.h"
#include "val_int64.h"
#include "val_null.h"
#include "reporting.h"

void
Tuple::xdr_marshal( XDR *x ) 
{
  assert(frozen);

  // Tuple size overall
  uint32_t sz = fields.size();
  u_int32_t i = (u_int32_t)sz;
  xdr_uint32_t(x, &i);
  // Marshal the fields
  for(uint32_t i = 0;
      i < fields.size();
      i++) {
    fields[i]->xdr_marshal(x);
  };
}


TuplePtr
Tuple::xdr_unmarshal(XDR* x) 
{
  TuplePtr t = Tuple::mk();
  // Tuple size overall
  u_int32_t ui;
  xdr_uint32_t(x, &ui);
  // Marshal the fields
  uint32_t sz = ui;
  for(uint32_t i = 0;
      i < sz;
      i++) {
    t->append(Value::xdr_unmarshal(x));
  }

  return t;
}


/** Generate a printout of the form firstField(otherField, otherField,
    ...). An empty tuple is EMPTY_TUPLE. */
string
Tuple::toString() const
{ 
  if (size() == 0) {
    return "EMPTY_TUPLE";
  } else {
    ostringstream sb;

    sb << fields[0]->toString()
       << "(";

    for (uint32_t i = 1;
         i < fields.size() - 1;
         i++) {
      sb << fields[i]->toString()
         << ", ";
    }
    
    if (fields.size() > 1) {
      sb << fields[fields.size() - 1]->toString();
    }
    
    sb << ")";
    return sb.str();
  }
}


string
Tuple::toConfString() const
{ 
  if (size() == 0) {
    return "EMPTY_TUPLE";
  } else {
    ostringstream sb;

    sb << "(";

    for (uint32_t i = 0;
         i < fields.size() - 1;
         i++) {
      sb << fields[i]->toConfString()
         << ", ";
    }
    
    if (fields.size() > 1) {
      sb << fields[fields.size() - 1]->toConfString();
    }
    
    sb << ")";
    return sb.str();
  }
}


int
Tuple::compareTo(TuplePtr other) const
{
  if (size() == other->size()) {
    for (uint32_t i = 0;
         i < size();
         i++) {
      ValuePtr mine = fields[i];
      ValuePtr its = (*other)[i];
      int result = mine->compareTo(its);
      if (result != 0) {
        // Found a field position for which we are different.  Return
        // the difference.
        return result;
      }
    }

    // All fields are equal.
    return 0;

  } else if (size() < other->size()) {
    return -1;
  } else {
    return 1;
  }
}


TuplePtr
Tuple::EMPTY()
{
  static TuplePtr __theEmptyTuple = Tuple::mk();
  return __theEmptyTuple;
}


uint
Tuple::_tupleIDCounter = 0;

// Create an empty initializer object so that the EMPTY tuple is fully
// initialized.
Tuple::EmptyInitializer
_theEmptyInitializer;


void
Tuple::concat(TuplePtr tf)
{
  assert(!frozen);

  // Copy fields
  for (uint32_t i = 0;
       i < tf->size();
       i++) {
    append((*tf)[i]);
  }
};


Tuple::~Tuple()
{
//   TELL_WORDY << "Destroying tuple "
//             << toString()
//             << " with ID "
//             << _ID
//             << "\n";
}


uint32_t
Tuple::ID()
{
  return _ID;
}


Tuple::Tuple()
  : fields(),
    frozen(false),
    _ID(_tupleIDCounter++)
{
//   TELL_WORDY << "Creating tuple " << toString()
//             << "with ID " << _ID
//             << "\n";
}


TuplePtr
Tuple::mk()
{
  TuplePtr p(new Tuple());
  return p;
};

TuplePtr
Tuple::mk(string name, bool id)
{
  TuplePtr p(new Tuple());
  p->append(Val_Str::mk(name));

  if (Plumber::catalog() && Plumber::catalog()->nodeid())
    p->append(Plumber::catalog()->nodeid());
  else
    p->append(Val_Null::mk());

  if (id)
    p->append(Val_Int64::mk(Plumber::catalog()->uniqueIdentifier()));
  return p;
};

TuplePtr
Tuple::clone(string name, bool newid) const
{
  TuplePtr tp;
  if (name != "") {
    tp = mk(name, newid);

    if (newid)
      for (uint i = 3; i < size(); tp->append(fields[i++]));
    else
      for (uint i = 2; i < size(); tp->append(fields[i++]));
  }
  else {
    tp = mk();
    for (uint i = 0; i < size(); tp->append(fields[i++]));
  }
  return tp;
}

void
Tuple::append(ValuePtr tf)
{
  assert(tf);
  assert(!frozen);
  fields.push_back(tf);
}

void
Tuple::prepend(ValuePtr tf)
{
  assert(tf);
  assert(!frozen);
  fields.insert(fields.begin(), tf);
}


void
Tuple::freeze()
{
  frozen = true;
//   TELL_WORDY << "Freezing tuple " << toString()
//             << "with ID " << _ID << "\n";
}


uint32_t
Tuple::size() const
{
  return fields.size();
}


ValuePtr
Tuple::operator[] (ptrdiff_t i)
{
  try {
    return fields.at(i);
  } catch (std::exception e) {
    TELL_ERROR << "Tuple::[] Attempted to fetch non existent field "
               << i
               << " from tuple "
               << toString()
               << ". Returning null\n";
    throw e;
    return Val_Null::mk();
  }
}


const ValuePtr
Tuple::operator[] (ptrdiff_t i) const
{
  try {
    return fields.at(i);
  } catch (std::exception e) {
    TELL_ERROR << "Tuple::[] Attempted to fetch non existent field "
               << i
               << " from tuple "
               << toString()
               << ". Returning null\n";
    throw e;
    return Val_Null::mk();
  }
}



bool
Tuple::Comparator::operator()(const TuplePtr first,
                              const TuplePtr second) const
{
  return first->compareTo(second) < 0;
}


void
Tuple::set(uint32_t i, ValuePtr val)
{
  assert(!frozen);
  assert(size() > i);

  fields[i] = val;
}

