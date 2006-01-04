/*
 * @(#)$Id$
 *
 * Copyright (c) 2004 Intel Corporation. All rights reserved.
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Tuple fields and Tuple implementations
 *
 */

#include "tuple.h"
#include <assert.h>

//
// Serialize a Tuple into an XDR
//
void Tuple::xdr_marshal( XDR *x ) 
{
  assert(frozen);
  assert(sizeof(size_t) == sizeof(u_int32_t));
  // Tuple size overall
  size_t sz = fields.size();
  xdr_uint32_t(x, &sz );
  // Marshal the fields
  for(size_t i=0; i < fields.size(); i++) {
    fields[i]->xdr_marshal(x);
  };
}

//
// Deserialize a Tuple from an XDR
//
TuplePtr Tuple::xdr_unmarshal( XDR *x ) 
{
  TuplePtr t = Tuple::mk();
  assert(sizeof(size_t) == sizeof(u_int32_t));
  // Tuple size overall
  size_t sz;
  xdr_uint32_t(x, &sz );
  // Marshal the fields
  for(size_t i=0; i < sz; i++) {
    t->append(Value::xdr_unmarshal(x));
  }
  return t;
}

//
// Format as a string
//
string Tuple::toString() const
{ 
  ostringstream sb;
  
  sb << "<"; 
  for(size_t i=0; i < fields.size(); i++) {
    sb << fields[i]->toString();
    if (i != fields.size() - 1) {
      sb << ", ";
    }
  }
  sb << ">";
  return sb.str();
}

/** Compare this tuple to another.  If we have different numbers of
    fields, compare the field counts.  If we have the same numbers of
    fields, compare individual fields from first onwards. */
int Tuple::compareTo(TuplePtr other) const
{
  if (size() == other->size()) {
    for (size_t i = 0;
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


void Tuple::tag(string key,
                ValuePtr value)
{
  assert(!frozen);

  // Is the tag map created?
  if (_tags == 0) {
    // Create it
    _tags = new std::map< string, ValuePtr >();

    // We'd better still have memory for this
    assert(_tags != 0);
  }

  _tags->insert(std::make_pair(key, value));
}

ValuePtr Tuple::tag(string key)
{
  // Do we have a tag map?
  if (_tags == 0) {
    // Nope, just say no
    return ValuePtr();
  } else {
    // Find the pair for that map
    std::map< string, ValuePtr >::iterator result = _tags->find(key);

    // Did we get it?
    if (result == _tags->end()) {
      // Nope, no such tag
      return ValuePtr();
    } else {
      return result->second;
    }
  }
}

TuplePtr Tuple::EMPTY = Tuple::mk();

// Create an empty initializer object so that the EMPTY tuple is fully
// initialized.
Tuple::EmptyInitializer _theEmptyInitializer;

void Tuple::concat(TuplePtr tf)
{
  assert(!frozen);

  // Copy fields
  for (size_t i = 0;
       i < tf->size();
       i++) {
    append((*tf)[i]);
  }
};

Tuple::~Tuple()
{
  if (_tags) {
    delete _tags;
  }
}
