/*
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 */

#include "tupleCounter.h"
#include "val_str.h"
#include "reporting.h"
#include "systemTable.h"

DEFINE_ELEMENT_INITS(TupleCounter, "TupleCounter");

TupleCounter::TupleCounter(string name, string type)
  : Element(name, 1, 1),
    _type(type)
{
}

/**
 * Generic constructor.
 * Arguments:
 * 2. Val_Str: Type / Tuple Name.
 */
TupleCounter::TupleCounter(TuplePtr args)
  : Element((*args)[2]->toString(), 1, 1),
    _type((*args)[3]->toString()) 
{
}

TupleCounter::~TupleCounter()
{
}

TuplePtr TupleCounter::simple_action(TuplePtr p)
{
  if (_type == "" || (*p)[TNAME]->toString() == _type) {
    _counter++;
    TELL_OUTPUT << "TupleCounter[" << name();

    if (_type != "") TELL_OUTPUT << ", " << _type;

    TELL_OUTPUT << "]:  ["
                << _counter
                << "]" << std::endl;
  }
  return p;
}
