// -*- cr-basic-offset: 2; related-file-name: "tupleseq.h" -*-
/*
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include <iostream>
#include <async.h>
#include "snetsim.h"

SimpleNetSim::SimpleNetSim(str name, uint32_t min, uint32_t max, double p)
  : Element(name,1, 1), _out_cb(cbv_null), min_delay_(min), max_delay_(max), 
    drop_prob_(p), pull_pending(true)
{
}

TuplePtr SimpleNetSim::pull(int port, cbv cb)
{
  grab();
  if (!ready_q_.empty()) {
    TuplePtr p = ready_q_.at(0);
    ready_q_.pop_front();
    return p;
  }
  _out_cb = cb;
  return NULL;
}

void SimpleNetSim::grab() {
  log(LoggerI::INFO, 0, "SimpleNetSim::grab"); 
  if (!pull_pending) {
     log(LoggerI::INFO, 0, "SimpleNetSim::grab !pull_pending FINISHED"); 
     return;
  }

  TuplePtr t = NULL; 
  do {
    t = input(0)->pull(wrap(this,&SimpleNetSim::element_cb)); 	// Grab the next one and delay it
  } while (t != NULL && (rand()/double(RAND_MAX)) < drop_prob_);

  if (t) {
    uint32_t d = min_delay_ + uint32_t((max_delay_ - min_delay_)*(rand()/double(RAND_MAX)));
    log(LoggerI::INFO, 0, strbuf() << "SimpleNetSim: Delaying for " << d << "(ms)"); 

    if (d < 1000)
      delaycb(0, d * 1000000, wrap(this, &SimpleNetSim::tuple_ready, t));
    else
      delaycb((d/1000), (d%1000) * 1000000, wrap(this, &SimpleNetSim::tuple_ready, t));
  } else {
    pull_pending = false; 
  }
  log(LoggerI::INFO, 0, strbuf() << "SimpleNetSim grab(): FINISHED"); 
}

void SimpleNetSim::element_cb() {
  log(LoggerI::INFO, 0, strbuf() << "SimpleNetSim element_cb(): Upstream callback called"); 

  if (!pull_pending) delaycb(0, wrap(this, &SimpleNetSim::grab));
  pull_pending = true;

  log(LoggerI::INFO, 0, strbuf() << "SimpleNetSim element_cb(): FINISHED"); 
}

void SimpleNetSim::tuple_ready(TupleRef t) 
{ 
  log(LoggerI::INFO, 0, strbuf() << "SimpleNetSim tuple_ready(): Down stream callback called"); 
  ready_q_.push_back(t); 

  if (_out_cb != cbv_null) {
    (*_out_cb)();
    _out_cb = cbv_null;
  }
  log(LoggerI::INFO, 0, strbuf() << "SimpleNetSim tuple_ready(): FINISHED"); 
}



