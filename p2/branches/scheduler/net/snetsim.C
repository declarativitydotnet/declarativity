// -*- cr-basic-offset: 2; related-file-name: "tupleseq.h" -*-
/*
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 */

#include "snetsim.h"

#include "eventLoop.h"
#include <boost/bind.hpp>

SimpleNetSim::SimpleNetSim(string name, uint32_t min, uint32_t max, double p)
  : Element(name,1, 1),
    _out_cb(0),
    min_delay_(min),
    max_delay_(max), 
    drop_prob_(p),
    pull_pending(true)
{
}

TuplePtr SimpleNetSim::pull(int port, b_cbv cb)
{
  grab();
  if (!ready_q_.empty()) {
    TuplePtr p = ready_q_.at(0);
    ready_q_.pop_front();
    return p;
  }
  _out_cb = cb;
  return TuplePtr();
}

void SimpleNetSim::grab() {
  ELEM_INFO("SimpleNetSim::grab"); 
  if (!pull_pending) {
    ELEM_INFO("SimpleNetSim::grab !pull_pending FINISHED"); 
    return;
  }

  TuplePtr t = TuplePtr(); 
  do {
    // Grab the next one and delay it
    t = input(0)->pull(boost::bind(&SimpleNetSim::element_cb, this)); 	
  } while (t != NULL && (rand()/double(RAND_MAX)) < drop_prob_);

  if (t) {
    uint32_t d = min_delay_ + uint32_t((max_delay_ - min_delay_)*(rand()/double(RAND_MAX)));

    EventLoop::loop()->enqueue_timer((0.0 + d) / 1000.0, boost::bind(&SimpleNetSim::tuple_ready, this, t));
  } else {
    pull_pending = false; 
  }
}

void SimpleNetSim::element_cb() {
  if (!pull_pending) {
    EventLoop::loop()->enqueue_dpc(boost::bind(&SimpleNetSim::grab, this));
  }
  pull_pending = true;
}

void SimpleNetSim::tuple_ready(TuplePtr t) 
{ 
  ready_q_.push_back(t); 

  if (_out_cb) {
    _out_cb();
    _out_cb = 0;
  }
}



