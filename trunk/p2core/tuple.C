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
#include "value.h"
#include "tuple.h"
#include <assert.h>
#include "val_uint32.h"
#include "val_str.h"
#include "val_int32.h"
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

  // Metadata to handle tuple tracing across wire
  // Add following bits to the fields
  // 1. ID of the tuple
  // 2. ID of local node
  // this is used during unmarshalling at destination
  // to set the tags of the newly created tuple
  int count = 0;
  if(_tags){
    count = _tags->size();
    (Val_Int32::mk(count))->xdr_marshal(x);
    (Val_Str::mk("sourceNode"))->xdr_marshal(x);
    tag("localNode")->xdr_marshal(x);
    (Val_Str::mk("ID"))->xdr_marshal(x);
    (Val_UInt32::mk(ID()))->xdr_marshal(x);
  }
  else {
    (Val_Int32::mk(count))->xdr_marshal(x);
  }

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

  // retrieve debugging metadata in the tags
  int numTags = Val_Int32::cast(Value::xdr_unmarshal(x));
  if(numTags > 0){
    // create a tag depending on the tags coming from wire
    ValuePtr sourceNodeTag = Value::xdr_unmarshal(x);
    ValuePtr sn = Value::xdr_unmarshal(x);
    ValuePtr idTag = Value::xdr_unmarshal(x);
    ValuePtr idt = Value::xdr_unmarshal(x);

    t->tag(sourceNodeTag->toString(), sn);
    t->tag(idTag->toString(), idt);

  }

  return t;
}


string
Tuple::toString() const
{ 
  ostringstream sb;
  
  sb << "<"; 
  for(uint32_t i = 0;
      i < fields.size();
      i++) {
    sb << fields[i]->toString();
    if (i != fields.size() - 1) {
      sb << ", ";
    }
  }
  sb << ">";
  return sb.str();
}


string
Tuple::toConfString() const
{ 
  ostringstream sb;
  
  sb << "<"; 
  for(uint32_t i = 0;
      i < fields.size();
      i++) {
    sb << fields[i]->toConfString();
    if (i != fields.size() - 1) {
      sb << ", ";
    }
  }
  sb << ">";
  return sb.str();
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


void
Tuple::tag(string key,
           ValuePtr value)
{
  // Tags could be added later after
  // constructing the tuple, especially
  // for debugging purposes -- Atul.

  //assert(!frozen);

  // Is the tag map created?
  if (_tags == 0) {
    // Create it
    _tags = new std::map< string, ValuePtr >();

    // We'd better still have memory for this
    assert(_tags != 0);
  }

  _tags->insert(std::make_pair(key, value));
}


ValuePtr
Tuple::tag(string key)
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


TuplePtr
Tuple::EMPTY = Tuple::mk();


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

  if (_tags) {
    delete _tags;
  }
}


uint32_t
Tuple::ID()
{
  return _ID;
}


Tuple::Tuple()
  : fields(),
    frozen(false),
    _tags(0),
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


void
Tuple::append(ValuePtr tf)
{
  assert(!frozen);
  fields.push_back(tf);
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
  return fields[i];
}


const ValuePtr
Tuple::operator[] (ptrdiff_t i) const
{
  return fields[i];
}



bool
Tuple::Comparator::operator()(const TuplePtr first,
                              const TuplePtr second) const
{
  return first->compareTo(second);
}


void
Tuple::set(uint32_t i, ValuePtr val)
{
  assert(!frozen);
  assert(size() > i);

  fields[i] = val;
}

