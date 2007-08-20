// -*- c-basic-offset: 2; related-file-name: "marshal.h" -*-
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

#include "marshal.h"

#include "val_opaque.h"
#include "fdbuf.h"
#include <sstream>
// #include "xdrbuf.h"
// the boost serialization implementer claims text is not much more expensive than portable binary
#include <boost/archive/text_oarchive.hpp>
#include "val_str.h"

DEFINE_ELEMENT_INITS(Marshal, "Marshal");

Marshal::Marshal(string name)
  : Element(name, 1, 1)
{
}

/**
 * Generic constructor.
 * Arguments:
 * 2. Val_Str:    Element Name.
 */
Marshal::Marshal(TuplePtr args)
  : Element(Val_Str::cast((*args)[2]), 1, 1)
{
}

Marshal::~Marshal()
{
}

TuplePtr Marshal::simple_action(TuplePtr p)
{
  // Taken straight from the tuples test.
  // FdbufPtr fb(new Fdbuf());
	std::stringstream outstr;
  boost::archive::text_oarchive xe(outstr);
  // xdrfdbuf_create(&xe, fb.get(), false, XDR_ENCODE);
  p->marshal(&xe);
  // xdr_destroy(&xe);


  // Now create a new tuple to host the opaque
  TuplePtr t = Tuple::mk();
  if (t == 0) {
    // Couldn't create one. Memory problems?
    ELEM_ERROR("Couldn't allocate new tuple");
    return TuplePtr();
  } else {
    // Stick the string into a tuple field and into the tuple
	  FdbufPtr fb(new Fdbuf());
	  fb->pushString(outstr.str());
    t->append(Val_Opaque::mk(fb));
    t->freeze();
    return t;
  }
}
