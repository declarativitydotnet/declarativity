/*
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#ifndef __RCCR_H__
#define __RCCR_H__

#include <deque>
#include <vector>
#include <async.h>
#include "tuple.h"
#include "element.h"
#include "inlines.h"

typedef uint64_t SeqNum;

class TupleInfo;
class LossRec;

class RateCCR : public Element {
public:
  RateCCR(str name);
  const char *class_name() const { return "RateCCR";};
  const char *processing() const { return "a/al"; };
  const char *flow_code()  const { return "-/--"; };

  TuplePtr simple_action(TupleRef p);

  TuplePtr pull(int port, b_cbv cb);

  int push(int port, TupleRef tp, b_cbv cb);	// Rate control input

private:
  class Connection;
  REMOVABLE_INLINE TuplePtr strip(TuplePtr p);

  b_cbv  _ack_cb;

  std::deque <TuplePtr>        ack_q_;	// Output ack queue
  std::map <str, Connection*>  cmap_;	// Interval weights
};
  
#endif /* __RCCR_H_ */
