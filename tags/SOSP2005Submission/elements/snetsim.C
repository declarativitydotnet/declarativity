// -*- cr-basic-offset: 2; related-file-name: "tupleseq.h" -*-
/*
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include <async.h>
#include "snetsim.h"

SimpleNetSim::SimpleNetSim(str name, uint32_t min, uint32_t max, double p)
  : Element(name,1, 1), _out_cb(cbv_null), min_delay_(min), max_delay_(max), drop_prob_(p)
{
}

TuplePtr SimpleNetSim::pull(int port, cbv cb)
{
  if (!ready_q_.empty()) {
    TuplePtr p = ready_q_.at(0);
    ready_q_.pop_front();
    return p;
  }
  else {
    TuplePtr t = NULL; 
    do {
      t = input(0)->pull(cb); 	// Grab the next one and delay it
    } while (t != NULL && double(rand())/(RAND_MAX + 1.0) < drop_prob_);

    if (t) {
      uint32_t d = min_delay_ + uint32_t((max_delay_ - min_delay_)*rand()/(RAND_MAX+1.0));
      strbuf sb; 
      sb << "SimpleNetSim: Delay " << d << "(ms)"; 
      log(LoggerI::WARN, 0, sb);

      if (d < 1000)
        delaycb(0, d * 1000000, wrap(this, &SimpleNetSim::tuple_ready, t));
      else
        delaycb((d/1000), (d%1000) * 1000000, wrap(this, &SimpleNetSim::tuple_ready, t));
      _out_cb = cb;
    } 
    return NULL;
  }
}

void SimpleNetSim::tuple_ready(TupleRef t) 
{ 
  ready_q_.push_back(t); 

  if (_out_cb != cbv_null) {
    (*_out_cb)();
    _out_cb = cbv_null;
  }
}



