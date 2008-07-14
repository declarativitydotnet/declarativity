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
#include "dupRemove.h"
#include "val_int64.h"
#include "val_double.h"
#include "val_str.h"
#include "val_tuple.h"
#include "netglobals.h"
#include <boost/bind.hpp>

DEFINE_ELEMENT_INITS(DupRemove, "DupRemove")

#define CONNECTION_TIMEOUT 240

DupRemove::DupRemove(TuplePtr args)
  : Element(Val_Str::cast((*args)[2]), 1, 1) { }

bool DupRemove::Connection::received(TuplePtr tp) {
  ValuePtr src = (*tp)[SRC]; 
  SeqNum seq  = Val_Int64::cast((*tp)[SEQ]);
  SeqNum cseq = Val_Int64::cast((*tp)[CUMSEQ]);

  if (_cum_seq < cseq) {
    if (_cum_seq && _receiveMap.size() > 0) {
      for (SeqNum s = _cum_seq; s <= cseq; s++) {
        _receiveMap.erase(s);
      }
    }
    _cum_seq = cseq;
  }

  if (seq < _cum_seq) {
    return true;
  }
  else if (_receiveMap.find(seq) != _receiveMap.end()) {
    return true;
  }
  _receiveMap.insert(std::make_pair(seq, true));
  return false;
}

long DupRemove::Connection::touch_duration() 
{
  boost::posix_time::time_duration duration = 
    boost::posix_time::microsec_clock::universal_time() - last_touched;
  return duration.seconds();
}

void DupRemove::Connection::touch() {
  last_touched = boost::posix_time::microsec_clock::universal_time();
}

int DupRemove::push(int port, TuplePtr tp, b_cbv cb)
{
  assert(port == 0);

  ValuePtr src = (*tp)[SRC];
  SeqNum  cseq = Val_Int64::cast((*tp)[CUMSEQ]);
  ConnectionPtr cp = lookup(src);

  if (cseq == 0 && cp) {
    if (cp->_tcb != 0) EventLoop::loop()->cancel_timer(cp->_tcb);
    unmap(src);
    cp.reset();
  }

  if (!cp) { 
    cp.reset(new Connection());
    map(src, cp);
  }
  cp->touch();

  if (cp->_tcb != 0) 
    EventLoop::loop()->cancel_timer(cp->_tcb);
  cp->_tcb = EventLoop::loop()->enqueue_timer(CONNECTION_TIMEOUT,
                     boost::bind(&DupRemove::unmap, this, src));
  if (!cp->received(tp))
    return output(0)->push(tp, cb);
  return 1;
}

REMOVABLE_INLINE void DupRemove::map(ValuePtr src, DupRemove::ConnectionPtr cp)
{
  unmap(src);
  cmap_.insert(std::make_pair(src, cp));
}

REMOVABLE_INLINE void DupRemove::unmap(ValuePtr src)
{
  DupRemove::ConnectionIndex::iterator iter = cmap_.find(src);
  if (iter != cmap_.end()) {
    cmap_.erase(iter);
  }
}

REMOVABLE_INLINE DupRemove::ConnectionPtr DupRemove::lookup(ValuePtr src)
{
  DupRemove::ConnectionIndex::iterator iter = cmap_.find(src);
  return iter == cmap_.end() ? DupRemove::ConnectionPtr() : iter->second;
}

