// -*- c-basic-offset: 2; related-file-name: "element.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Element which pulls tuples in order from an array of
 * tuple references to its single output.
 *
 */

#ifndef __MEMORY_PULL_H__
#define __MEMORY_PULL_H__

#include "element.h"

class MemoryPull : public Element { 
 public:
  
  /** Initialized with the tuple ref array and size.  Eventually, this
      will be given via a configure method or some such. */
  MemoryPull(TupleRef * tupleRefBuffer,
             int bufferSize);

  /** Remove the tuple ref array */
  ~MemoryPull();
  
  /** An error. To be replaced by more general exception machinery. */
  struct MemoryPullError {};
  
  const char *class_name() const		{ return "MemoryPull";}
  const char *processing() const		{ return PULL; }

  /** Overridden because we have no input ports */
  TuplePtr pull(int port, cbv cb);
  
 private:
  /** The tuple ref array from which I pull */
  TupleRef * const _tupleRefBuffer;

  /** The current index within the tuple ref buffer. */
  int _tupleIndex;

  /** The size of the tuple ref buffer */
  const int _bufferSize;
};

#endif /* __MEMORY_PULL_H_ */
