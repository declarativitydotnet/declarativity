/*
 * 
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 */

#include <iostream>
#include "rdelivery.h"
#include "p2Time.h"
#include "val_uint64.h"
#include "val_uint32.h"
#include "val_double.h"
#include "val_str.h"
#include "val_tuple.h"
#include "netglobals.h"


RDelivery::RDelivery(string n, unsigned m) 
  : Element(n, 2, 2),
    _out_cb(0),
    _in_on(true),
    _max_retry(m)
{
}

/**
 * New tuple to send
 */
TuplePtr RDelivery::pull(int port, b_cbv cb)
{
  assert (port == 0);
  double flip = rand() / (float)RAND_MAX;

  TuplePtr tp;
  if (_in_on && 
      (_retry_q.empty() ||
       (flip < 0.5 &&
        (_in_on = ((tp = input(0)->pull(boost::bind(&RDelivery::input_cb,this))) != NULL))))) {
    /* Store the tuple for retry, if failure */
    return tuple(tp);
  }
  else if (!_retry_q.empty()) {
    tp = _retry_q.front();
    _retry_q.pop_front();
    return tp;
  }

  _out_cb = cb;
  return TuplePtr();
}

/**
 * The push method handles input on 2 ports.
 * port 1: Acknowledgement or Failure Tuple
 */
int RDelivery::push(int port, TuplePtr tp, b_cbv cb)
{
  assert(port == 1);

  try {
    for (uint i = 0;
         i < tp->size();
         i++) {
      if ((*tp)[i]->typeCode() == Value::STR && 
          Val_Str::cast((*tp)[i]) == "ACK") {
        // Acknowledge tuple and update measurements.
        ack(tp);
      }
    }
  }
  catch (Value::TypeError e) { }
  return output(1)->push(tp, cb);
}

REMOVABLE_INLINE void RDelivery::handle_failure(ConnectionPtr cp) 
{
  cp->_tcb = NULL;

  for (std::deque<RDelivery::Connection::TuplePtr>::iterator iter = 
       cp->_outstanding.begin(); iter != cp->_outstanding.end(); ) {
    RDelivery::Connection::TuplePtr ctp = *iter;
    if (ctp->_retry_cnt < _max_retry) {
      ctp->_retry_cnt++;
      _retry_q.push_back(ctp->_tp);
      if (_out_cb) {
        _out_cb();
        _out_cb = 0;
      }
      iter++;
    }
    else {
      std::cerr << "RETRY TOO MANY: DEST = " << cp->_dest->toString()
                << " SEQ = " << ctp->_seq << std::endl;
      if (cp->_cum_seq < ctp->_seq)
        cp->_cum_seq = ctp->_seq;
      assert(iter == cp->_outstanding.begin());
      iter = cp->_outstanding.erase(iter);
    }
  }

  if (cp->_outstanding.size() > 0) {
    cp->_tcb = delayCB((cp->_rtt) / 1000.0,
                       boost::bind(&RDelivery::handle_failure, this, cp), this);
  }
}

RDelivery::ConnectionPtr RDelivery::lookup(ValuePtr dest)
{
  ValueConnectionMap::iterator iter = _index.find(dest);
  if (iter != _index.end()) {
    return iter->second;		// Found it.
  }
  return RDelivery::ConnectionPtr();	// Didn't find it. 
}

void RDelivery::map(ValuePtr dest, ConnectionPtr cp) 
{
  ValueConnectionMap::iterator iter = _index.find(dest);
  if (iter == _index.end()) {
    _index.insert(std::make_pair(dest,cp)); 
  }
}

void RDelivery::unmap(ValuePtr dest)
{
  ValueConnectionMap::iterator iter = _index.find(dest);
  if (iter != _index.end()) {
      _index.erase(iter);
  }
}

TuplePtr RDelivery::tuple(TuplePtr tp)
{
  ValuePtr dest = (*tp)[DEST];
  SeqNum   seq  = Val_UInt64::cast((*tp)[SEQ]);
  double   rtt  = Val_Double::cast((*tp)[RTT]);

  RDelivery::ConnectionPtr cp = lookup(dest);  
  if (!cp) {
    cp.reset(new Connection(dest, rtt));
    map(dest, cp);
  }

  TuplePtr rtp = Tuple::mk();
  for (unsigned i = 0; i < tp->size(); i++) {
    if (i == CUMSEQ) {
      rtp->append(Val_UInt64::mk(cp->_cum_seq));
    }
    else {
      rtp->append((*tp)[i]);
    }
  }
  rtp->freeze();

  RDelivery::Connection::TuplePtr ctp(new RDelivery::Connection::Tuple(seq, rtp));
  cp->_outstanding.push_back(ctp);
  if (cp->_tcb == NULL) {
    cp->_tcb = delayCB((2 * cp->_rtt) / 1000.0,
                       boost::bind(&RDelivery::handle_failure, this, cp), this);
  }
  return rtp;
}

void RDelivery::ack(TuplePtr tp)
{
  ValuePtr      dest = (*tp)[DEST + 2];
  ConnectionPtr cp   = lookup(dest);
  SeqNum        cseq = Val_UInt64::cast((*tp)[CUMSEQ + 2]); 

  if (cp && cp->_cum_seq < cseq) {
    if (cp->_tcb != NULL) {
      timeCBRemove(cp->_tcb);
      cp->_tcb = NULL;
    }
    cp->_cum_seq = cseq;
    while (cp->_outstanding.size() > 0 && 
           cp->_outstanding.front()->_seq <= cp->_cum_seq) {
      cp->_outstanding.pop_front();
    }
    if (cp->_outstanding.size() > 0) {
      cp->_tcb = delayCB((2 * cp->_rtt) / 1000.0,
                         boost::bind(&RDelivery::handle_failure, this, cp), this);
    }
  }
}

void RDelivery::input_cb()
{
  _in_on = true;
  if (_out_cb) {
    _out_cb();
    _out_cb = 0;
  }
}
