// -*- c-basic-offset: 2; related-file-name: "tupleSource.h" -*-
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
 */

#include <staticTupleSource.h>
#include "val_str.h"
#include "val_tuple.h"

DEFINE_ELEMENT_INITS(StaticTupleSource, "StaticTupleSource")


StaticTupleSource::StaticTupleSource(string name,
                                     TuplePtr tuple)
  : TupleSource(name),
    _tuple(tuple)
{
}


/**
 * Generic constructor.
 * Arguments:
 * 2. Val_Str:    Element Name.
 * 3. Val_Tuple:  The source tuple.
 */
StaticTupleSource::StaticTupleSource(TuplePtr args)
  : TupleSource(Val_Str::cast((*args)[2])),
    _tuple(Val_Tuple::cast((*args)[3]))
{
}


TuplePtr
StaticTupleSource::generate()
{
  return _tuple;
}
