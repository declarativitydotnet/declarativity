// -*- c-basic-offset: 2; related-file-name: "element.C" -*-
/*
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#ifndef __Frag_H__
#define __Frag_H__

#include <deque>
#include "tuple.h"
#include "element.h"

#define SEQ_FIELD  0
#define PLD_FIELD  1
#define NUM_CHUNKS "chunks"
#define CHUNK_SIZE (block_size_ - sizeof(uint64_t))


class Frag : public Element { 
public:

  Frag(str name, uint32_t block_size);
  const char *class_name() const	{ return "Frag";};
  const char *processing() const	{ return PUSH_TO_PULL; };
  const char *flow_code()  const	{ return "-/-"; };

  int push(int port, TupleRef t, cbv cb);

  TuplePtr pull(int port, cbv cb);

 private:
  void fragment(TupleRef t);

  cbv _push_cb;
  cbv _pull_cb;

  const uint32_t block_size_;
  std::deque <TuplePtr> fragments_;
};

#endif /* __Frag_H_ */
