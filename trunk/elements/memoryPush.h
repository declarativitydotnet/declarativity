// -*- c-basic-offset: 2; related-file-name: "memoryPush.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Element which pushes tuples in order from an array of
 * tuple references to its single output.  Loosely simplified from
 * Click's infinite source.
 *
 */

#ifndef __MEMORY_PUSH_H__
#define __MEMORY_PUSH_H__

#include <element.h>

class MemoryPush : public Element { 
 public:
  
  /** Initialized with the tuple ref array and size.  Eventually, this
      will be given via a configure method or some such. */
  MemoryPush(ref< vec< TupleRef > > tupleRefBuffer,
             int bufferSize);

  /** Remove the tuple ref array */
  ~MemoryPush();
  
  /** An error. To be replaced by more general exception machinery. */
  struct MemoryPushError {};
  
  const char *class_name() const		{ return "MemoryPush"; }
  const char *flow_code() const			{ return "/x"; }
  const char *processing() const		{ return "/l"; }

  /** Overridden because we have no input ports, whereas the default
      processes inputs before returning them.  */
  int push(int port, TupleRef, cbv cb);
  
 private:
  /** The tuple ref array from which I pull */
  ref< vec< TupleRef > > const _tupleRefBuffer;

  /** The current index within the tuple ref buffer. */
  int _tupleIndex;

  /** The size of the tuple ref buffer */
  const int _bufferSize;
};

#endif /* __MEMORY_PULL_H_ */
