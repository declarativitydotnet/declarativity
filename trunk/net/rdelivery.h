/*
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#ifndef __RDELIVERY_H__
#define __RDELIVERY_H__

#include <map>
#include <deque>
#include <async.h>
#include "tuple.h"
#include "element.h"
#include "inlines.h"


class RTuple;
typedef uint64_t SeqNum; 

class RDelivery : public Element {
public:
  RDelivery(str name, uint32_t max_retry=5, uint32_t seq_field = 0, uint32_t ack_seq_field=1);
  const char *class_name() const { return "RDelivery";};
  const char *processing() const { return "ah/ah"; };
  const char *flow_code() const	 { return "--/--"; };

  TuplePtr simple_action(TupleRef p); 
  int push(int port, TupleRef tp, cbv cb);	// Incoming ack or timeout

private:
  REMOVABLE_INLINE void handle_failure(SeqNum);		// Handles a failure to deliver a tuple
  void element_cb(); 					// Callback for retry push 

  bool      retry_on_;					// Indicates a retry can be made
  uint32_t  max_retry_;					// Max number of retries for a tuple
  uint32_t  seq_field_;
  uint32_t  ack_seq_field_;

  std::deque <RTuple*>  rtran_q_;			// Retransmit queue 

  typedef std::map<SeqNum, RTuple*> RTupleIndex;
  RTupleIndex tmap_;					// Map containing unacked in transit tuples
};
  
#endif /* __RDELIVERY_H_ */
