// -*- c-basic-offset: 2; related-file-name: "functorSource.h" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include <functorSource.h>

FunctorSource::FunctorSource(string name,
                             FunctorSource::Generator* generator)
  : Element(name, 0, 1),
    _generator(generator)
{
}
    
TuplePtr FunctorSource::pull(int port, b_cbv cb)
{
  // Always produce a result, never block
  TuplePtr generated = (*_generator)();
  return generated;
}
