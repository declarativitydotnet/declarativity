// -*- c-basic-offset: 2; related-file-name: "element.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Element which pulls tuple refs in order from a memory
 * buffer. It is ready until it exhausts its buffer.
 * 
 */

#include "memoryPull.h"
#include "tuple.h"

MemoryPull::MemoryPull(TupleRef * tupleRefBuffer,
                       int bufferSize) :
  Element(0,1),
  _tupleRefBuffer(tupleRefBuffer),
  _tupleIndex(0),
  _bufferSize(bufferSize)
{
  // Sanity checks
  if (bufferSize < 0) {
    throw MemoryPullError();
  }
}

MemoryPull::~MemoryPull()
{
  free(_tupleRefBuffer);
}

/** Pull the next tuple ref from the buffer and advance the index. If at
    the end of the buffer, be unready. */
TuplePtr MemoryPull::pull(int port, cbv cb)
{
  if (_tupleIndex < _bufferSize) {
    TuplePtr p = _tupleRefBuffer[_tupleIndex++];
    return p;
  } else {
    return 0;
  }
}
