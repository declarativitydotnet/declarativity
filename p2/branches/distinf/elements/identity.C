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

#include "identity.h"

DEFINE_ELEMENT_INITS(Identity, "Identity");

Identity::Identity(string name)
  : Element(name, 1, 1)
{
}

/**
 * Generic constructor.
 * Arguments:
 * 2. Val_Str: Name.
 */
Identity::Identity(TuplePtr args)
  : Element((args->size() > 2 ? (*args)[2]->toString() : "identity"), 1, 1)
{
}

Identity::~Identity()
{
}

TuplePtr Identity::simple_action(TuplePtr p)
{
  return p;
}
