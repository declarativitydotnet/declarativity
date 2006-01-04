// -*- c-basic-offset: 2; related-file-name: "tupleSource.h" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include <tupleSource.h>

TupleSource::TupleSource(string name,
                         TuplePtr tuple)
  : FunctorSource(name, NULL), _tupleGenerator(new TupleGenerator(tuple))
{
  _generator = _tupleGenerator.get();
}
