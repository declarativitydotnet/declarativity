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
#include "val_uint32.h"
#include "val_str.h"
#include "val_int32.h"
#include "val_null.h"
#include "reporting.h"

void
Tuple::marshal( boost::archive::text_oarchive *x ) 
{
  assert(frozen);

  // Tuple size overall
  uint32_t sz = fields.size();
  u_int32_t i = (u_int32_t)sz;
  *x & i;
  // Marshal the fields
  for(uint32_t i = 0;
      i < fields.size();
      i++) {
    fields[i]->marshal(x);
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
    (Val_Int32::mk(count))->marshal(x);
    (Val_Str::mk("sourceNode"))->marshal(x);
    tag("localNode")->marshal(x);
    (Val_Str::mk("ID"))->marshal(x);
    (Val_UInt32::mk(ID()))->marshal(x);
  }
  else {
    (Val_Int32::mk(count))->marshal(x);
  }

}


TuplePtr
Tuple::unmarshal(boost::archive::text_iarchive *x) 
{
  TuplePtr t = Tuple::mk();
  // Tuple size overall
  u_int32_t ui;
  *x & ui;
  // Marshal the fields
  uint32_t sz = ui;
  for(uint32_t i = 0;
      i < sz;
      i++) {
    t->append(Value::unmarshal(x));
  }

  // retrieve debugging metadata in the tags
  int numTags = Val_Int32::cast(Value::unmarshal(x));
  if(numTags > 0){
    // create a tag depending on the tags coming from wire
    ValuePtr sourceNodeTag = Value::unmarshal(x);
    ValuePtr sn = Value::unmarshal(x);
    ValuePtr idTag = Value::unmarshal(x);
    ValuePtr idt = Value::unmarshal(x);

    t->tag(sourceNodeTag->toString(), sn);
    t->tag(idTag->toString(), idt);

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

    sb << fields[0]->toConfString()
       << "(";

    for (uint32_t i = 1;
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


u_int
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
    p->append(Val_UInt32::mk(Plumber::catalog()->uniqueIdentifier()));
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

