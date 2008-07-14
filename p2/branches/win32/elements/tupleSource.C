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

#ifdef WIN32
#include "p2_win32.h"
#endif // WIN32
#include <tupleSource.h>
#include "val_str.h"

TupleSource::TupleSource(string name)
  : Element(name, 0, 1)
{
}


/**
 * Generic constructor.
 * Arguments:
 * 2. Val_Str:    Element Name.
 */
TupleSource::TupleSource(TuplePtr args)
  : Element(Val_Str::cast((*args)[2]), 0, 1)
{
}


TuplePtr
TupleSource::pull(int port, b_cbv cb)
{
  // Always produce a result, never block
  TuplePtr generated = generate();
  return generated;
}
